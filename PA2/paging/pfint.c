/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{

	//might need to interrupt
	unsigned long faultingPage = read_cr2();
	int store, pageth;
	if (bsm_lookup(getpid(), faultingPage, &store, &pageth) == SYSERR) {
		// kill(getpid());
		return SYSERR;
	}	

	int vp = faultingPage>>12;
	unsigned long pdbrCurrentProcess = read_cr3();
	unsigned long ptNumber = faultingPage>>22;
	unsigned long pageNumber = (faultingPage & 0x3FF000)>>12;
	unsigned long offset = (faultingPage<<20)>>20;
	unsigned long pdeAddress = pdbrCurrentProcess + 4*ptNumber;
	pd_t *pdePtr = (pd_t*) pdeAddress;
	int framePointer;
	if (pdePtr->pd_pres == 0) {
		//create new page table for process
		//mark pd_pres = 1
		//add location of pt to pd_base
		
		if (get_frm(&framePointer) == SYSERR) {
 	             	// kill(getpid());
                 	return SYSERR;
	        }
		int idx = (framePointer)/NBPG - FRAME0;
		frm_tab[idx].fr_status = 1;
		frm_tab[idx].fr_type = FR_TBL;
		frm_tab[idx].fr_pid = getpid();
		pdePtr->pd_pres = 1;
		pdePtr->pd_base = (int)framePointer/NBPG;
	}

	int idx = pdePtr->pd_base - FRAME0;
	frm_tab[idx].fr_refcnt++;
	if (get_frm(&framePointer) == SYSERR) {
		// kill(getpid());
		return SYSERR;
	}
	idx = (int)(framePointer)/NBPG - FRAME0;
	frm_tab[idx].fr_status = 1;
	frm_tab[idx].fr_type = FR_PAGE;
	frm_tab[idx].fr_pid = getpid();
	frm_tab[idx].fr_vpno = vp;

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

	read_bs( (char*)framePointer, &store, &pageth);
	
	unsigned long pteAddress = pdePtr->pd_base*NBPG + 4*pageNumber;
 	pt_t *ptePtr = (pt_t*) pteAddress;
	ptePtr->pt_pres = 1;
	ptePtr->pt_base = (int)framePointer/NBPG;
	kprintf("\nRETURNING FROM PFINT.c\n");
	return OK;
}


