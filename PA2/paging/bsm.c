/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

#define MAX_BSTORE 8
/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
	 int i = 0;
    while (i < MAX_BSTORE) {
		bsm_tab[i].bs_status = 0;
		bsm_tab[i].bs_pid = -1;
		bsm_tab[i].bs_vpno = -1;
		bsm_tab[i].bs_npages = 256;
		bsm_tab[i].bs_isPrivate = 0;
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
	while (i < MAX_BSTORE) {
		bs_map_t *ptr = &bsm_tab[i];
		if (!ptr->bs_status) {
			*avail = i;
			return i;	
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
	if (ptr->store >= 0 && ptr->store < MAX_BSTORE) {
		
		*store = ptr->store;
		int a = (unsigned int) vaddr/NBPG;
		int b = ptr->vhpno;
		*pageth = (unsigned int) (a-b);
		return *pageth;
	}
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
	bs_map_t *ptr = &bsm_tab[source];
	ptr->bs_status = 1;
	ptr->bs_pid = pid;
	proctab[pid].store = source;
	ptr->bs_npages = npages;	
	ptr->bs_vpno = vpno;
	proctab[pid].vhpno = vpno;
	
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */

void freeBsm(int count, int pid) {
	int i = 1;
		while (i < NPROC-1) {
				if (proctab[i].store == proctab[pid].store && pid != i)
						count += 1;
				++i;			
		}
		if (count == 0)
				free_bsm((int)proctab[pid].store);
        }

SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	if (bsm_tab[proctab[pid].store].bs_isPrivate == 0) {
		freeBsm(0, pid);
		proctab[pid].store = -1;
	}
}


