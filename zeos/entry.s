# 0 "entry.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "entry.S"




# 1 "include/asm.h" 1
# 6 "entry.S" 2
# 1 "include/segment.h" 1
# 7 "entry.S" 2
# 1 "include/errno.h" 1
# 8 "entry.S" 2
# 73 "entry.S"
.globl keyboard_handler; .type keyboard_handler, @function; .align 0; keyboard_handler:
      pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %ebx; pushl %ecx; pushl %edx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es
      movb $0x20, %al; outb %al, $0x20;
      call keyboard_routine
      popl %edx; popl %ecx; popl %ebx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;

      iret

.globl pf_handler; .type pf_handler, @function; .align 0; pf_handler:
      call pf_routine
      movb $0x20, %al; outb %al, $0x20;

.globl clock_handler; .type clock_handler, @function; .align 0; clock_handler:
      pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %ebx; pushl %ecx; pushl %edx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es
      movb $0x20, %al; outb %al, $0x20;
      call clock_routine
      popl %edx; popl %ecx; popl %ebx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
      iret

.globl write_msr; .type write_msr, @function; .align 0; write_msr:

      #edx == 00
      #eax == el segundo parametro
      #ecx == el primer parametro

      push %ebp
      movl %esp, %ebp

      movl 0x08(%ebp), %ecx
      movl $0, %edx
      movl 0x0c(%ebp), %eax
      wrmsr

      movl %ebp,%esp
      popl %ebp
      ret





.globl syscall_handler_sysenter; .type syscall_handler_sysenter, @function; .align 0; syscall_handler_sysenter:
      push $0x2B
      push %EBP
      pushfl
      push $0x23
      push 4(%EBP)
      pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %ebx; pushl %ecx; pushl %edx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es
      cmpl $0, %EAX
      jl sysenter_err
      cmpl $MAX_SYSCALL, %EAX
      jg sysenter_err
      call *sys_call_table(, %EAX, 0x04)
      jmp sysenter_fin
sysenter_err:
      movl $-38, %EAX
sysenter_fin:
      movl %EAX, 0x18(%ESP)
      popl %edx; popl %ecx; popl %ebx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
      movl (%ESP), %EDX
      movl 12(%ESP), %ECX
      sti
      sysexit
