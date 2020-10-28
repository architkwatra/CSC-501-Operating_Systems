#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(int bs_id, unsigned int npages) {

  if (npages <= 0 || npages > 256) 
		return SYSERR;
	bs_map_t *bsPtr = &bsm_tab[bs_id];
	if (bsPtr->bs_status) {
		
		if (bsPtr->bs_isPrivate == 0)
			return bsPtr->bs_npages;

		else 
			return SYSERR;
	}
	
	return npages;
	
}


