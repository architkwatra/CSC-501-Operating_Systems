/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
	bs_map_t bsm_tab[8];
	 int i = 0;
        while (i < 8) {
		//check if any other variables need to be set
		bsm_tab[i].bs_status = 0;
		//check if this needs to be 256
		bsm_tab[i].bs_npages = 256;
		++i;
	}
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
	int i = 0;
	while (i < 8) {
		//add pointer	
		if (bsm_tab[i].bs_status == 0) {
			*avail = i;
			return OK;	
		}
		++i;
	}
	return -1;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
	proctab[bsm_tab[i].bs_pid].store = -1;
	bsm_tab[i].bs_status = 0;
	bsm_tab[i].bs_pid = -1;
	bsm_tab[i].bs_vpno = -1;
	bsm_tab[i].bs_npages = 256;
	bsm_tab[i].bs_isPrivate = 0;
	return (OK);
		
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
	struct pentry *ptr = &proctab[pid];
	
	if (ptr->store >= 0 && vaddr <= NBPG*bsm_tab[ptr->store].bs_npages) {
		*store = ptr->store;
		*pageth = (int) vaddr >> 12;
		return OK;
	}
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
	bsm_tab[source].bs_status = 1;
	bsm_tab[source].bs_pid = pid;
	bsm_tab[source].bs_vpno = vpno;
	bsm_tab[source].bs_npages = npages;
	proctab[pid].store = source;
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	proctab[pid].store = -1;
}


