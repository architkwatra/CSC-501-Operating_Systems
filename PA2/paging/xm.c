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
	// Ask about semaphore in this since getpid() might contxswch and return the wrong pid
	if (npages <= 0 || 
		npages > 255 || 
		source < 0 || 
		source > 7 || 
		bsm_tab[source].bs_pid != getpid() ||
		bsm_tab[source].bs_status == 1)
		return SYSERR;
		
	bsm_tab[source].bs_status = 1;
	bsm_tab[source].bs_vpno = virtpage;
	bsm_tab[source].bs_npages = npages;
	return(OK)
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
