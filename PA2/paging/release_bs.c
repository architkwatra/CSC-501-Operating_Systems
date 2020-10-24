#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  /* release the backing store with ID bs_id */
	int ret = free_bsm((int)bs_id);
	if (ret == SYSERR) {
		kprintf("free_bsm() returned -1 which means that the BS was shared\n");
	return SYSERR;
	}
	return OK;

}

