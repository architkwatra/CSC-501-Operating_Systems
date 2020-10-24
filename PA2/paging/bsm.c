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
	struct bs_map_t bsm_tab[8];
	 int i = 0;
        while (i < 8) {
		//check if any other variables need to be set
		bsm_tab[i].bs_status = 0;
		//check if this needs to be 256
		bsm_tab[i].bs_npages = 256;
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
			return (i);	
		}
	}
	return -1;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
	kprintf("Inside free_bsm() called from release_bs()\n");	
	//check if bs_isPrivate is set to 1 in vcreate
	if (bsm_tab[i].bs_isPrivate == 1) {
		//check if the below statement is even required or not, or do it from where the function is being called;
		proctab[bsm_tab[i].bs_pid].store = -1;
		bsm_tab[i].bs_status = 0;
		bsm_tab[i].bs_pid = -1;
		bsm_tab[i].bs_vpno = 0;
		bsm_tab[i].bs_npages = 256;
		bsm_tab[i].bs_isPrivate = 0;
	} else {
		return SYSERR;
	}
	return (OK);
		
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
		
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
	proctab[currpid].store = source;
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	int store = proctab[pid].store;
	bsm_tab[store].bs_status = 0;
	bsm_tab[store].bs_isPrivate = 0;
	proctab[pid].store = -1;
}


