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
		return SYSERR;
	}
	if (bsm_tab[source].bs_status == 0 || (bsm_tab[source].bs_npages >= npages)) {
		
		if (bsm_tab[source].bs_status == 0)  
			bsm_map(getpid(), virtpage, source, npages);
		else
			bsm_map(getpid(), virtpage, source, bsm_tab[source].bs_npages);

		return OK;
	}
	return SYSERR;

}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{	
	writeBackDF(getpid());
	bsm_unmap(getpid(), virtpage, 0);
	return OK;	

}
