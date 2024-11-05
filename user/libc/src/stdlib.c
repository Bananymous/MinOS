#include <stdlib.h>
#include <string.h>
#include <minos/sysstd.h>
#include <strinternal.h>
#include <collections/list.h>
void exit(int64_t code) {
    _exit(code);
    for(;;);
}

#include <minos/heap.h>
typedef struct {
    size_t id;
    struct list alloc_list;
    MinOSHeap heap;
} _LibcInternalHeap;
#define _LIBC_INTERNAL_HEAPS_MAX 5
#define _LIBC_INTERNAL_INVALID_HEAPID ((size_t)-1)
_LibcInternalHeap _libc_internal_heaps[_LIBC_INTERNAL_HEAPS_MAX];
typedef struct _HeapNode _HeapNode;
// TODO: Migrate to a plist mechanism
struct _HeapNode {
    struct list list; 
    bool free;
    size_t size;
    char data[];
};

#include <assert.h>
#define MIN_HEAP_BLOCK_SIZE (sizeof(_HeapNode)+16)
#define alignup_to(n, size)   ((((n)+((size)-1))/(size))*(size))
#define aligndown_to(n, size) (((n)/(size))*(size))
void* libc_heap_allocate(_LibcInternalHeap* heap, size_t size) {
    if(heap->heap.size < sizeof(_HeapNode)) return NULL;
    for(_HeapNode* node=(_HeapNode*)heap->alloc_list.next; &node->list != &heap->alloc_list; node = (_HeapNode*)node->list.next) {
        if(node->size >= size && node->free) {
            size_t left = node->size-size;
            if(left > MIN_HEAP_BLOCK_SIZE) {
                // TODO: Align to 16
                _HeapNode* next = (_HeapNode*)(node->data+size);
                list_init(&next->list);
                next->free = true;
                next->size = left;
                list_append(&next->list, &node->list);
                node->size = size;
            }
            node->free = false;
            return node->data;
        }
    }
    return NULL;
}

// TODO: faster Deallocation infering address is a valid heap address
void libc_heap_deallocate(_LibcInternalHeap* heap, void* address) {
    if(heap->heap.size < sizeof(_HeapNode)) return;
    for(_HeapNode* node=(_HeapNode*)heap->alloc_list.next; &node->list != &heap->alloc_list; node = (_HeapNode*)node->list.next) {
        if(node->data == address) {
            assert((!node->free) && "Double free");
            node->free = true;
            _HeapNode* next = (_HeapNode*)node->list.next;
            if(&next->list != &heap->alloc_list && next->free) {
                list_remove(&next->list);
                node->size+=next->size+sizeof(_HeapNode);
            }
            return;
        }
    }
    assert(false && "Invalid address to free");
    // TODO: Error here. Invalid address
}

void init_libc_heap(_LibcInternalHeap* heap) {
    // Some sort of error
    if(heap->heap.size < sizeof(_HeapNode)) return;
    list_init(&heap->alloc_list);
    _HeapNode* node = (_HeapNode*)heap->heap.address;
    list_init(&node->list);
    node->free = true;
    node->size = aligndown_to(heap->heap.size-sizeof(_HeapNode), 16);
    list_append(&node->list, &heap->alloc_list);
}
void* malloc(size_t size) {
    size = alignup_to(size, 16);
    for(size_t i = 0; i < _LIBC_INTERNAL_HEAPS_MAX; ++i) {
        if(_libc_internal_heaps[i].id == _LIBC_INTERNAL_INVALID_HEAPID) {
            intptr_t e = heap_create(HEAP_RESIZABLE);
            if(e < 0) return NULL; // Failed to create heap. Most likely out of memory
            _libc_internal_heaps[i].id = e;
            e=heap_get(e, &_libc_internal_heaps[i].heap);
            if(e < 0) return NULL; // Really bad error. heap_create returned invalid id
            init_libc_heap(&_libc_internal_heaps[i]);
        }
        void* addr=libc_heap_allocate(&_libc_internal_heaps[i], size);
        if(addr) return addr;
    }
    // Reached limit
    return NULL;
}

void* calloc(size_t elm, size_t size) {
    void* buf = malloc(elm*size);
    if(buf) {
        memset(buf, 0, elm*size);
    }
    return buf;
}
#include <stdio.h>
void* realloc(void* ptr, size_t newsize) {
    fprintf(stderr, "ERROR: Unimplemented `realloc` (%p, %zu)", ptr, newsize);
    return NULL;
}
// TODO: Smarter free with heap_get() and checking heap ranges
void free(void* addr) {
    if(!addr) return;
    for(size_t i = 0; i < _LIBC_INTERNAL_HEAPS_MAX; ++i) {
        if(_libc_internal_heaps[i].id == _LIBC_INTERNAL_INVALID_HEAPID) continue;
        _LibcInternalHeap* heap = &_libc_internal_heaps[i];
        if(heap->heap.address <= addr && addr < heap->heap.address+heap->heap.size) {
            libc_heap_deallocate(heap, addr);
            return;
        }
    }
    // Reached limit
    assert(false && "Invalid address to free");
}
void _libc_internal_init_heap() {
    for(size_t i = 0; i < _LIBC_INTERNAL_HEAPS_MAX; ++i) {
        _libc_internal_heaps[i].id = _LIBC_INTERNAL_INVALID_HEAPID;
    }
}

int atoi(const char *str) {
    const char* end;
    return atoi_internal(str, &end);
}
float atof(const char *str) {
    const char* end;
    float result=0;
    int whole = atoi_internal(str, &end);
    if(end[0] == '.') {
        size_t fract = atosz_internal(end+1, &end);
        while(fract != 0) {
            result += fract%10;
            result /= 10.0;
            fract /= 10;
        }
    }
    result += (float)whole;
    return result;
}
int abs(int num) {
    return num < 0 ? -num : num;
}
