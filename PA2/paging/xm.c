/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{	

	if (bsm_tab[source].bs_isPrivate == 1) {
		kprintf("Failed in 1\n");
		return SYSERR;
	}
	//kprintf("bs_npages = %d, bs_status = %d and pid = %d\n", bsm_tab[source].bs_npages, bsm_tab[source].bs_status, getpid());
	if (bsm_tab[source].bs_status == 0 || (bsm_tab[source].bs_npages >= npages)) {
		
		if (bsm_tab[source].bs_status == 0)  
			bsm_map(getpid(), virtpage, source, npages);
		else
			bsm_map(getpid(), virtpage, source, bsm_tab[source].bs_npages);

		//kprintf("BS-%d booked for pid-%d\n", source, bsm_tab[source].bs_pid);
		return OK;
	}
	//kprintf("Failed in 2\n");
	return SYSERR;


	// Ask about semaphore in this since getpid() might contxswch and return the wrong pid
	// if (npages <= 0 || 
	// 	npages > 256 || 
	// 	source < 0 || 
	// 	source > 7)
	// 	return SYSERR;


	
	// //Remember to set isPrivate variable in vcreate
	// if (proctab[currpid].isPrivate == 1) {
	// 	if (bsm_tab[source].bs_status == 1) 
	// 		return SYSERR;
	// 	bsm_map(getpid(), virtpage, source, npages);
	// 	bsm_tab[source].bs_isPrivate = 1;
	// } else {
	// 	if (bsm_tab[source].bs_status == 1) 
	// 		if (bsm_tab[source].bs_isPrivate == 1)
	// 			return SYSERR;
	// 		else {
	// 			if (bsm_tab[source].bs_npages < npages)
	// 				return SYSERR;
	// 		}

	// 	bsm_map(currpid, virtpage, source, npages);
	// 	bsm_tab[source].bs_isPrivate = 0;

	// }
	// return OK;	
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{	
	writeBackDirtyFrames(getpid());
	bsm_unmap(getpid(), virtpage, 0);
	return OK;	

}
