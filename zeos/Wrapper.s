# 0 "Wrapper.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "Wrapper.S"
# 1 "include/asm.h" 1
# 2 "Wrapper.S" 2
# 1 "include/segment.h" 1
# 3 "Wrapper.S" 2
.globl write; .type write, @function; .align 0; write:

 pushl %ebp
 movl %esp,%ebp


 pushl %ebx
 pushl %esi
 pushl %edi


 # salvar registros
 pushl %ecx
 pushl %edx

 # pasar parametros para el modo sistema
 movl 0x08(%ebp), %edx #tercer parametro
 movl 0x0c(%ebp), %ecx #segundo parametro
 movl 0x10(%ebp), %ebx #primer parametro

 #4 identificador de write
 movl $4, %eax



 pushl $end_syscall
 pushl %ebp
 movl %esp,%ebp
 sysenter

end_syscall:

 # ver error
 popl %ebp
 addl $4, %esp
 popl %edx
 popl %ecx
 cmpl $0, %eax
 jge no_error
 #retornar -1
 negl %eax # Negar EAX
 movl %eax, errno
 movl -1, %eax


no_error:
 popl %edi
 popl %esi
 popl %ebx
 movl %ebp,%esp
    popl %ebp
    ret



.globl gettime; .type gettime, @function; .align 0; gettime:
 pushl %ebp
 movl %esp,%ebp


 pushl %ebx
 pushl %esi
 pushl %edi

 pushl %ecx
 pushl %edx

 #identificador gettime
 movl $10, %eax


 pushl $gt_return
 pushl %ebp
 movl %esp,%ebp
 sysenter

gt_return:
 popl %ebp
 addl $4, %esp
 popl %edx
 popl %ecx

 popl %edi
 popl %esi
 popl %ebx
 cmpl $0, %eax
 jge gt_no_error

 negl %eax # Negamos EAX para obtener el valor absoluto
 movl %eax, errno
 movl -1, %eax

gt_no_error:
 movl %ebp,%esp
    popl %ebp
 ret

.globl getpid; .type getpid, @function; .align 0; getpid:
 push %ebp
 movl %esp,%ebp

 pushl %ebx
 pushl %esi
 pushl %edi


 # salvar registros
 pushl %ecx
 pushl %edx

 movl $20,%eax

 pushl $getpid_return
 pushl %ebp
 movl %esp,%ebp
 sysenter

getpid_return:
 popl %ebp
 addl $4, %esp
 popl %edx
 popl %ecx

 popl %edi
 popl %esi
 popl %ebx
 cmpl $0, %eax
 jge getpid_no_error

 negl %eax # Negamos EAX para obtener el valor absoluto
 movl %eax, errno
 movl -1, %eax

getpid_no_error:
 movl %ebp,%esp
    popl %ebp
 ret



.globl fork; .type fork, @function; .align 0; fork:
 push %ebp
 movl %esp,%ebp

 pushl %ebx
 pushl %esi
 pushl %edi


 # salvar registros
 pushl %ecx
 pushl %edx

 movl $2,%eax

 pushl $fork_return
 pushl %ebp
 movl %esp,%ebp
 sysenter

fork_return:
 popl %ebp
 addl $4, %esp
 popl %edx
 popl %ecx

 popl %edi
 popl %esi
 popl %ebx
 cmpl $0, %eax
 jge fork_no_error

 negl %eax # Negamos EAX para obtener el valor absoluto
 movl %eax, errno
 movl -1, %eax

fork_no_error:
 movl %ebp,%esp
    popl %ebp
 ret


.globl exit; .type exit, @function; .align 0; exit:
 push %ebp
 movl %esp,%ebp

 pushl %ebx
 pushl %esi
 pushl %edi

 # salvar registros
 pushl %ecx
 pushl %edx

 movl $1,%eax

 pushl $exit_return
 pushl %ebp
 movl %esp,%ebp
 sysenter

exit_return:
 popl %ebp
 addl $4, %esp
 popl %edx
 popl %ecx

 popl %edi
 popl %esi
 popl %ebx
 cmpl $0, %eax
 jge exit_no_error

 negl %eax # Negamos EAX para obtener el valor absoluto
 movl %eax, errno
 movl -1, %eax

exit_no_error:
 movl %ebp,%esp
    popl %ebp
 ret


.globl block; .type block, @function; .align 0; block:
 push %ebp
 movl %esp,%ebp

 pushl %ebx
 pushl %esi
 pushl %edi

 # salvar registros
 pushl %ecx
 pushl %edx

 movl $21,%eax

 pushl $block_return
 pushl %ebp
 movl %esp,%ebp
 sysenter

block_return:
 popl %ebp
 addl $4, %esp
 popl %edx
 popl %ecx

 popl %edi
 popl %esi
 popl %ebx
 cmpl $0, %eax
 jge block_no_error

 negl %eax # Negamos EAX para obtener el valor absoluto
 movl %eax, errno
 movl -1, %eax

block_no_error:
 movl %ebp,%esp
    popl %ebp
 ret

.globl unblock; .type unblock, @function; .align 0; unblock:
 push %ebp
 movl %esp,%ebp

 pushl %ebx
 pushl %esi
 pushl %edi

 # salvar registros
 pushl %ecx
 pushl %edx

 movl 0x08(%ebp), %edx

 movl $22,%eax

 pushl $unblock_return
 pushl %ebp
 movl %esp,%ebp
 sysenter

unblock_return:
 popl %ebp
 addl $4, %esp
 popl %edx
 popl %ecx

 popl %edi
 popl %esi
 popl %ebx
 cmpl $0, %eax
 jge unblock_no_error

 negl %eax # Negamos EAX para obtener el valor absoluto
 movl %eax, errno
 movl -1, %eax

unblock_no_error:
 movl %ebp,%esp
    popl %ebp
 ret
