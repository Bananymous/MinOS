#include "./serial.h"
static FsOps serialOps = {0};
static InodeOps inodeOps = {0};

static intptr_t serial_open(struct Inode* this, VfsFile* file, fmode_t mode) {
    if(mode != MODE_WRITE) return -UNSUPPORTED;
    file->ops = &serialOps;
    return 0;
}
static intptr_t init_inode(struct Device* this, struct Inode* inode) {
    inode->ops = &inodeOps;
    return 0;
}
static intptr_t serial_dev_write(VfsFile* file, const void* buf, size_t size, off_t offset) {
    (void)file;
    (void)offset; 
    for(size_t i = 0; i < size; ++i) {
       serial_print_u8(((const uint8_t*)buf)[i]);
    }
    return size;
}
intptr_t serial_dev_init() {
    memset(&serialOps, 0, sizeof(serialOps));
    inodeOps.open = serial_open;
    serialOps.write = serial_dev_write;
    return 0;
}

Device serialDevice = {
    .ops=&inodeOps,
    .init_inode=init_inode,
    .private=NULL,
};
