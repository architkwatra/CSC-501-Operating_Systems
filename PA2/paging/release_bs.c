#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  /* release the backing store with ID bs_id */
	if (bsm_tab[bs_id].bs_isPrivate == 1) { 
			
		int ret = free_bsm((int)bs_id);
	
		if (ret == SYSERR) {
			kprintf("free_bsm() returned SYSERR which means that the BS was shared\n");
			return SYSERR;
		}
	} else {
		int i = 0;
		int count = 0;
		while (i < NPROC) {
			if (proctab[i].store == bs_id && getpid() != i) { 
				kprintf("release_bs, i = %d and bs_id = %d\n", i, bs_id);
				count++;
			}
			++i;
		}
		kprintf("Final count = %d\n", count);
		if (count == 0)
			free_bsm((int)bs_id);
	}
	
	proctab[getpid()].store = -1;
	return OK;

}

