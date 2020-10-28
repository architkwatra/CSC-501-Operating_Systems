#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


void checkAndCallFreeBSM(bsd_t bs_id) {

	int i = 1;
	int count = 0;
	while (i < 49) {
		if (proctab[i].store == bs_id && getpid() != i) { 
			count++;
		}
		++i;
	}
	if (count == 0)
		free_bsm((int)bs_id);
}

SYSCALL release_bs(bsd_t bs_id) {
  /* release the backing store with ID bs_id */
	if (bsm_tab[bs_id].bs_isPrivate) { 					
		if (free_bsm((int)bs_id) == SYSERR) {
			return SYSERR;
		}
	}
	checkAndCallFreeBSM(bs_id);
	proctab[getpid()].store = -1;
	return OK;

}

