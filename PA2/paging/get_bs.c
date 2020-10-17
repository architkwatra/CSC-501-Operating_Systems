#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

  /* requests a new mapping of npages with ID map_id */
	
	if (npages <= 0 or npages > 256) 
		return SYSERR;
	

	if (bsm_tab[bs_id].bs_status == 1) {
		
		if (bsm_tab[bs_id].bs_isPrivate == 1)
			return SYSERR;
		else {
			return bsm_tab[bs_id].bs_npages;
		}
	}
	
	return npages;
	
}

