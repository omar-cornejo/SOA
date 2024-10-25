#include <asm.h>
#include <segment.h>


ENTRY(task_switch)
	pushl %ebp
	movl %esp, %ebp

	pushl %esi
	pushl %edi
	pushl %ebx

	pushl 8(%ebp)

	call inner_task_switch	

	popl %ebx
	popl %edi
	popl %esi

	movl %ebp,%esp
	popl %ebp

	ret

ENTRY(inner_task_switch)
	pushl %ebp
	movl %esp, %ebp

	movl  (%ebp),%esp