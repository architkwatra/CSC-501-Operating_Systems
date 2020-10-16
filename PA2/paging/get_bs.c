#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

  /* requests a new mapping of npages with ID map_id */
	if (npages == 0 or npages > 256) 
		return SYSERR;
	
	if (bsm_tab[bs_id].bs_status == 1) 
		return SYSERR;
	else {
		bsm_tab[bs_id].bs_pid = currpid;
		bsm_tab[bs_id].bs_npages = npages;
		bsm_tab[bs_id].bs_status = 1;
	}
	
    return npages;
	
	//  if npages is not valid:
	//  	return SYSERR:
	// 	if bsm_tab[bs_id] is mapped:
	// 		return SYSERR
	// 	else:
	// 		set bsm_tab[bs_id].pid = currpid;
	// 		
	// 		set bsm_tab[bs_id].npages = npages
}


