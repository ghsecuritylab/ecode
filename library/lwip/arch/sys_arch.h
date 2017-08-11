#ifndef	__SYS_ARCH_H__
#define __SYS_ARCH_H__

#include "rtos.h"




u32_t sys_now(void);

typedef os_sem_t sys_sem_t;
typedef os_sem_t sys_mutex_t;
typedef os_mbox_t sys_mbox_t;
typedef os_thread_t sys_thread_t;

#define SYS_MBOX_NULL   (os_queue_t)0
#define SYS_SEM_NULL    (os_sem_t)0
#define SYS_DEFAULT_THREAD_STACK_DEPTH  configMINIMAL_STACK_SIZE


/*

void sys_init(void);
err_t sys_sem_new(sys_sem_t * sem, u8_t count);
void sys_sem_free(sys_sem_t * sem);
u32_t sys_arch_sem_wait(sys_sem_t * sem, u32_t timeout);
err_t sys_mbox_new(sys_mbox_t * mbox, int size);
void sys_mbox_free(sys_mbox_t * mbox);
void sys_mbox_post(sys_mbox_t * mbox, void *msg);
err_t sys_mbox_trypost(sys_mbox_t * mbox, void * msg);
u32_t sys_arch_mbox_fetch(sys_mbox_t * mbox, void ** msg, u32_t timeout);
u32_t sys_arch_mbox_tryfetch(sys_mbox_t * mbox, void ** msg);
int sys_mbox_valid(sys_mbox_t * mbox);
void sys_mbox_set_invalid(sys_mbox_t * mbox);
sys_thread_t sys_thread_new(char * name, void (* thread)(void * arg), void * arg, int stacksize, int prio);
sys_prot_t sys_arch_protect(void);
void sys_arch_unprotect(sys_prot_t pval);

*/


#endif