# 0 "task_switch.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "task_switch.S"
# 1 "include/asm.h" 1
# 2 "task_switch.S" 2
# 1 "include/segment.h" 1
# 3 "task_switch.S" 2


.globl task_switch; .type task_switch, @function; .align 0; task_switch:

    pushl %ebp
    movl %esp, %ebp


    pushl %esi
    pushl %edi
    pushl %ebx


    pushl 8(%ebp)


    call inner_task_switch


    addl $4, %esp


    popl %ebx
    popl %edi
    popl %esi


    movl %ebp, %esp
    popl %ebp
    ret

.globl store_ebp_in_pcb; .type store_ebp_in_pcb, @function; .align 0; store_ebp_in_pcb:

 movl %ebp, %eax
 ret

.globl change_stack; .type change_stack, @function; .align 0; change_stack:

 pushl %ebp
 movl %esp, %ebp


 movl 8(%ebp), %esp


 popl %ebp
 ret
