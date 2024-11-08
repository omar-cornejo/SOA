#include <asm.h>
#include <segment.h>


ENTRY(task_switch)
    // Guardar el valor de %ebp y establecer el nuevo marco de pila
    pushl %ebp
    movl %esp, %ebp

    // Guardar los registros que podrían ser modificados
    pushl %esi
    pushl %edi
    pushl %ebx

    // Pasar el puntero de la nueva tarea a la pila
    pushl 8(%ebp)

    // Llamar a la función que realiza el cambio de contexto interno
    call inner_task_switch 

    // Ajustar el puntero de pila después de la llamada
    addl $4, %esp

    // Restaurar los registros guardados
    popl %ebx
    popl %edi
    popl %esi

    // Restaurar el marco de pila anterior
    movl %ebp, %esp
    popl %ebp
    ret

ENTRY(store_ebp_in_pcb)
	// Mover el valor en 8(%ebp) a EAX para retornar
	movl %ebp, %eax
	ret

ENTRY(change_stack)
	// Guardar el contexto actual
	pushl %ebp
	movl %esp, %ebp

	// Mover el valor en 8(%ebp) a esp
	movl 8(%ebp), %esp

	// Restaurar el contexto y retornar
	popl %ebp
	ret