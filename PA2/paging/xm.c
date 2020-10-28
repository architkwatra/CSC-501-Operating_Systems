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

	bs_map_t *ptr = &bsm_tab[source];
	if (ptr->bs_isPrivate == 1) {
		return SYSERR;
	}
	
	int bsNpages = ptr->bs_npages;
	if (!ptr->bs_status || (bsNpages >= npages)) {
		
		if (ptr->bs_status == 0)  
			bsm_map(getpid(), virtpage, source, npages);
		else
			bsm_map(getpid(), virtpage, source, bsNpages);

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
