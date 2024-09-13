%include "asmstd.inc"
section .text
; Accepts:
;   rdi = task
;   rsi = task
;   rdx = frame 
extern fork

; Accepts:
;   rdi = task
;   rsi = task
global fork_trampoline
fork_trampoline:
    mov rax, [rsp]
    lea rdx, [rsp+8]
    ; Push ss
    mov rcx, KERNEL_SS ; ss
    push rcx
    ; Push rsp
    push rdx
    ; Push rflags
    pushfq
    ; Push cs
    mov rcx, KERNEL_CS ; cs
    push rcx
    ; Push rip
    push rax
    ; Cleanup rax which will be "result" from fork.
    ; And I think its fairly reasonable to set it as 0
    ; given that if we ever got to iretq back to sys_fork() it means we probably succeeded in forking
    ; Maybe -YOU_ARE_CHILD and we have no conditional logic in sys_fork()?
    xor rax, rax
    irq_push_regs
    mov rdx, rsp
    call fork
    add rsp, 5*PTR_SIZE + IRQ_REGS_SIZE
    ret


