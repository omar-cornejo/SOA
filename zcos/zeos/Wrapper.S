#include <asm.h>
#include <segment.h>
ENTRY(write)

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



ENTRY(gettime)
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

ENTRY(getpid)
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

ENTRY(fork)
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



ENTRY(exit)
	push %ebp
	mov %esp,%ebp

	# Save to user stack
	push %edx
	push %ecx


	# Now we need to put the identified of the system call in the EAX register
	movl $1, %eax

	# EDX and ECX will be modified by sysexit. We must save them so we can restore them.
	#push %ecx
	#push %edx

	# Fake dynamic link?
	push $exit_ret
	push %ebp
	mov %esp,%ebp

	# Entramos
	sysenter

exit_ret:
	# Comprobamos si hay error en la ejecución de la syscall
	pop %ebp
	add $4, %esp
	pop %edx
	pop %ecx
	cmp $0, %eax
	jge exit_no_error
	# Si hay error, preparamos el contexto para retornar correctamente el código.
	neg %eax # Negamos EAX para obtener el valor absoluto
	mov %eax, errno
	mov -1, %eax
exit_no_error:
	pop %ebp
	ret

