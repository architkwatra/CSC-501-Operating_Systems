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
	 int i = 0;
    while (i < 8) {
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
	while (i < 8) {
		//add pointer	
		if (bsm_tab[i].bs_status == 0) {
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
	
	if (ptr->store >= 0 && ptr->store < 8) {
		*store = ptr->store;
		*pageth = (int) ((vaddr/NBPG) - ptr->vhpno);
		if (pageth < 0) {
			kprintf("1111111111111 ------ vaddr = %lu amnd vhpno = %d and *pageth = %d\n", vaddr, ptr->vhpno, *pageth);
		}
		//kprintf("vaddr = %lu amnd vhpno = %d and *pageth = %d\n", vaddr, ptr->vhpno, *pageth);
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
	proctab[pid].vhpno = vpno;
	bsm_tab[source].bs_npages = npages;
	proctab[pid].store = source;
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	if (bsm_tab[proctab[pid].store].bs_isPrivate == 0) {
		int i = 0;
                int count = 0;
                while (i < NPROC) {
                        if (proctab[i].store == proctab[pid].store && pid != i)
                                count++;
                        ++i;
                }
                if (count == 0)
                        free_bsm((int)proctab[pid].store);
        }
	
		proctab[pid].store = -1;
	
}


