        .text
        .align  2
        .global coroutines_goto_next_task
        .code   16
        .thumb_func
        .type   coroutines_goto_next_task, %function
// void coroutines_goto_next_task (jmp_buf this_task, jmp_buf next_task)
coroutines_goto_next_task:
        push    {lr}
        sub     sp, sp, #12
        str     r1, [sp, #4]
        bl      setjmp
        cmp     r0, #0
        beq     0f
        add     sp, sp, #12
        pop     {pc}
0:
        ldr     r0, [sp, #4]
        mov     r1, #1
        bl      longjmp
        .size   coroutines_goto_next_task, .-coroutines_goto_next_task


        .align  2
        .global coroutines_setup_task
        .code   16
        .thumb_func
        .type   coroutines_setup_task, %function
// void coroutines_setup_task
// (jmp_buf main_task, jmp_buf new_task, void * new_stack_ptr,
//  void (* fun) (jmp_buf new_task, void * arg), void * arg)
coroutines_setup_task:
        push    {lr}
        sub     sp, sp, #20
        str     r1, [sp, #4]        // r1 = new_task
        str     r2, [sp, #8]        // r2 = new_stack_ptr
        str     r3, [sp, #12]       // r3 = fun
        bl      setjmp
        cmp     r0, #0
        bne     0f
        ldr     r2, [sp, #8]        // r2 = new_stack_ptr
        ldr     r0, [sp, #4]        // r0 = new_task
        ldr     r1, [sp, #24]       // r1 = arg
        ldr     r3, [sp, #12]       // r3 = fun
        mov     sp, r2              // new_stack_ptr
        blx     r3
        bl      abort               // should be unreachable
0:
        add     sp, sp, #20
        pop     {pc}
        .size   coroutines_setup_task, .-coroutines_setup_task
