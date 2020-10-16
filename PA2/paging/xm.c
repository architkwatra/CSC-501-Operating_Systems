/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, int source, int npages)
{	
	// Ask about semaphore in this since getpid() might contxswch and return the wrong pid
	if (npages <= 0 || 
		npages > 256 || 
		source < 0 || 
		source > 7)
		return SYSERR;


	
	//Remember to set isPrivate variable in vcreate
	if (proctab[currpid].isPrivate == 1) {
		if (bsm_tab[source].bs_status == 1) 
			return SYSERR;
		bsm_map(currpid, virtpage, source, npages);
		bsm_tab[source].bs_isPrivate = 1;
	} else {
		if (bsm_tab[source].bs_status == 1) 
			if (bsm_tab[source].bs_isPrivate == 1)
				return SYSERR;
			else {
				if (bsm_tab[source].bs_npages < npages)
					return SYSERR;
			}

		bsm_map(currpid, virtpage, source, npages);

	}
	
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{	
	int i = 0;
	while (i < 8) {
		if (bsm_tab[i].bs_vpno == virtpage) {
			bsm_tab[i].bs_status = 0;
			return (OK);
		}
			
	}
        
        return SYSERR;	

}
