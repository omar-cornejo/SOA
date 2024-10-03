#include <asm.h>
ENTRY(write)

	pushl %ebp
	movl %esp,%ebp

	# salvar registros
	pushl %edx
	pushl %ecx

	# pasar parametros para el modo sistema
	movl 8(%ebp), %ebx #tercer parametro
	movl 12(%ebp), %ecx #segundo parametro
	movl 16(%ebp), %edx #primer parametro


	movl $4, %eax


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
	movl %ebp,%esp
    popl %ebp
    ret 

ENTRY(gettime)
	pushl %ebp
	movl %esp,%ebp

	# Save to user stack
	pushl %ecx
	pushl %edx


	# Now we need to put the identified of the system call in the EAX register
	movl $10, %eax


	# Fake dynamic link?
	push $gt_return
	push %ebp
	mov %esp,%ebp

	# Entramos
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
	popl %ebp
	ret