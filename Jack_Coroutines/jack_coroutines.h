#ifndef JACK_COROUTINES_H
#define JACK_COROUTINES_H

#include <setjmp.h>

#ifdef __cplusplus
extern "C" {
#endif

void coroutines_goto_next_task (jmp_buf this_task, jmp_buf next_task);
void coroutines_setup_task
       (jmp_buf main_task, jmp_buf new_task, void * new_stack_ptr,
        void (* task_fun) (jmp_buf new_task, void * arg), void * arg);
#ifdef __cplusplus
};
#endif

#endif

