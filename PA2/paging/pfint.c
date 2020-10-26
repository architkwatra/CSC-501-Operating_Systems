/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
	//might need to interrupt
	unsigned long  faultingPage = read_cr2();
	int store, pageth;
	kprintf("\nIN PFINT FOR PID-%d and address is %lu\n", getpid(), faultingPage);
	if (bsm_lookup(getpid(), faultingPage, &store, &pageth) == SYSERR) {
		kprintf("BSM LOOKUP FAILING\n");
		return SYSERR;
	}
	

	int vp = faultingPage>>12;
	unsigned long pdbrCurrentProcess = proctab[currpid].pdbr;
	unsigned int ptNumber = faultingPage>>22;
	unsigned int pageNumber = (faultingPage & 0x3FF000)>>12;
	unsigned int offset = (faultingPage<<20)>>20;
	kprintf("\nVirtual Page Translation\n");
	unsigned long pdeAddress = pdbrCurrentProcess + 4*ptNumber;
	pd_t *pdePtr = (pd_t*) pdeAddress;
	
	if (pdePtr->pd_pres == 0) {
		//create new page table for process
		//mark pd_pres = 1
		//add location of pt to pd_base
		int framePointer = 0;
		if (get_frm(&framePointer) == SYSERR) {
 	             	//kill(getpid());
                 	return SYSERR;
	        }
		int idx = (int) (framePointer)/NBPG - FRAME0;
		frm_tab[idx].fr_status = 1;
		frm_tab[idx].fr_type = FR_TBL;
		frm_tab[idx].fr_pid = getpid();
		pdePtr->pd_pres = 1;
		pdePtr->pd_write = 1;
		pdePtr->pd_base = (int) framePointer/NBPG;
	}
	//kprintf("\nThrough Case A\n");
	int idx = (pdePtr->pd_base) - FRAME0;
	frm_tab[idx].fr_refcnt++;
	int framePointer = 0;
	if (get_frm(&framePointer) == SYSERR) {
		//kill(getpid());
		return SYSERR;
	}
	idx = (framePointer)/NBPG - FRAME0;
	frm_tab[idx].fr_status = 1;
	frm_tab[idx].fr_type = FR_PAGE;
	frm_tab[idx].fr_pid = getpid();
	frm_tab[idx].fr_vpno = vp;
	frm_tab[idx].fr_dirty = 0;
	// do we need to update ref_cnt here???!?
	if (grpolicy() != AGING) {
		struct scq frameToInsert;
		frameToInsert.idx = idx;
			
		struct scq *tmp = scPointer->next;
		scPointer->next = &frameToInsert;
		frameToInsert.next = tmp;
		scPointer = frameToInsert.next;
		//insert into scq
		//also delete in get_frm whenever reading from scq
		
		
	}
	else {
		struct fifo frameToInsert;
                frameToInsert.idx = idx;
		frameToInsert.age = 255;

		frameToInsert.next = fifohead.next;
		fifohead.next = &frameToInsert;
	}
	kprintf("\nFrame ID Being used for PF%d\n", (int) framePointer/NBPG);
	/*
        if (bsm_lookup(currpid, faultingPage, &store, &pageth) == SYSERR) {
                kprintf("BSM LOOKUP FAILING\n");
                return SYSERR;
        }*/

	read_bs( (char*)framePointer, store, pageth);
	
	unsigned long pteAddress = pdePtr->pd_base*NBPG + 4*pageNumber;
	pt_t *ptePtr = pteAddress;
	ptePtr->pt_pres = 1;
	ptePtr->pt_write = 1;
	ptePtr->pt_base = (int) framePointer/NBPG;
	kprintf("\nDone with PF for currpid-%d\n", getpid());

	write_cr3(pdbrCurrentProcess);
	return OK;
}


