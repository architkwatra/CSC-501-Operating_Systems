#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  /* release the backing store with ID bs_id */
	if (bsm_tab[bs_id].bs_isPrivate == 1) { 
			
		int ret = free_bsm((int)bs_id);
	
		if (ret == SYSERR) {
			return SYSERR;
		}
	} else {
		int i = 0;
		int count = 0;
		while (i < NPROC) {
			if (proctab[i].store == bs_id && getpid() != i && getpid() != 49) { 
				count++;
			}
			++i;
		}
		if (count == 0)
			free_bsm((int)bs_id);
	}
	
	proctab[getpid()].store = -1;
	return OK;

}

