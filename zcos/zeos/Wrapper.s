# 0 "Wrapper.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "Wrapper.S"
# 1 "include/asm.h" 1
# 2 "Wrapper.S" 2
.globl write; .type write, @function; .align 0; write:

 pushl %ebp
 movl %esp,%ebp
 push %ebx

 # pasar parametros para el modo sistema
 movl 0x8(%ebp), %edx #tercer parametro
 movl 0xc(%ebp), %ecx #segundo parametro
 movl 0x10(%ebp), %ebx #primer parametro


 movl $4, %eax

 # salvar registros
 pushl %ecx
 pushl %edx

 pushl $end_syscall
 pushl %ebp
 movl %esp,%ebp
 sysenter

end_syscall:

 # Comprobamos si hay error en la ejecución de la syscall
 popl %ebp
 addl $4, %esp
 popl %edx
 popl %ecx
 cmpl $0, %eax
 jge no_error

 # Si hay error, preparamos el contexto para retornar correctamente el código.
 negl %eax # Negamos EAX para obtener el valor absoluto
 movl %eax, errno
 movl -1, %eax

no_error:
 pop %ebx
 movl %ebp,%esp
    pop %ebp
    ret



.globl gettime; .type gettime, @function; .align 0; gettime:
 pushl %ebp
 movl %esp,%ebp
 push %ebx

 # Now we need to put the identified of the system call in the EAX register
 movl $10, %eax


 # Save to user stack
 pushl %ecx
 pushl %edx

 # Fake dynamic link?
 push $gt_return
 push %ebp
 mov %esp,%ebp
 sysenter

gt_return:
 # Comprobamos si hay error en la ejecución de la syscall
 popl %ebp
 addl $4, %esp
 popl %edx
 popl %ecx
 cmpl $0, %eax
 jge gt_no_error

 # Si hay error, preparamos el contexto para retornar correctamente el código.
 negl %eax # Negamos EAX para obtener el valor absoluto
 movl %eax, errno
 movl -1, %eax

gt_no_error:
 pop %ebx
 movl %ebp,%esp
    pop %ebp
 ret
