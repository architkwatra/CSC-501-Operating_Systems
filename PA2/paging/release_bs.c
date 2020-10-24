#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  /* release the backing store with ID bs_id */
	int ret = free_bsm((int)bs_id);
	if (ret == SYSERR) {
		kprintf("free_bsm() returned SYSERR which means that the BS was shared\n");
		return SYSERR;
	}
	proctab[getpid()].store = -1;
	return OK;

}

