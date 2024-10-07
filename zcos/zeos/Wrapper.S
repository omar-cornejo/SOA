#include <asm.h>
#include <segment.h>
ENTRY(write)

	pushl %ebp
	movl %esp,%ebp
	pushl %ebx

	# pasar parametros para el modo sistema
	movl 0x08(%ebp), %edx #tercer parametro
	movl 0x0c(%ebp), %ecx #segundo parametro
	movl 0x10(%ebp), %ebx #primer parametro

	#4 identificador de write
	movl $4, %eax

	# salvar registros
	pushl %ecx
	pushl %edx
	
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
	popl %ebx
	movl %ebp,%esp
    popl %ebp
    ret 



ENTRY(gettime)
	pushl %ebp
	movl %esp,%ebp
	

	#identificador gettime
	movl $10, %eax


	pushl %ecx
	pushl %edx

	pushl $gt_return
	pushl %ebp
	movl %esp,%ebp
	sysenter

gt_return:
	popl %ebp
	addl $4, %esp
	popl %edx
	popl %ecx
	cmpl $0, %eax
	jge gt_no_error

	negl %eax # Negamos EAX para obtener el valor absoluto
	movl %eax, errno
	movl -1, %eax

gt_no_error:
	movl %ebp,%esp
    popl %ebp
	ret