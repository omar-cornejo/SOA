ENTRY(write)

	push %ebp
	push %esp,%ebp

	movl 8(%ebp), %edx
	movl 12(%ebp), %ecx
	movl 16(%ebp), %ebx

	pushl %ecx
	pushl %edx

	movl $4, %eax


	pushl end_syscall
	pushl %ebp
	movl %esp,%ebp
	

	call sysenter

end_syscall:


	movl %ebp,%esp
    popl %ebp
    ret 

