/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include<proc.h>

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
	unsigned long faultingPage = read_cr2();
	int vp = faultingPage>>12;
	virt_addr_t *vAddrStruct = (virt_addr_t*)&faultingPage;
	// unsigned long pdbrCurrentProcess = proctab[currpid].pdbr;
	// unsigned long ptNumber = vAddrStruct->pd_offset;
	// unsigned long pageNumber = vAddrStruct->pt_offset;
	// unsigned long pdeAddress = pdbrCurrentProcess + 4*ptNumber;
	pd_t *pdePtr = (pd_t*) (proctab[currpid].pdbr + 4*vAddrStruct->pd_offset);
	int framePointer;
	if (pdePtr->pd_pres == 0) {

		int idx = get_frm(&framePointer);
		if (idx == SYSERR) {
                 	return SYSERR;
	        }
		
		frm_tab[idx].fr_status = 1;
		frm_tab[idx].fr_type = FR_TBL;
		frm_tab[idx].fr_pid = getpid();
		pdePtr->pd_pres = 1;
		pdePtr->pd_write = 1;
		pdePtr->pd_base = (int)framePointer/NBPG;
	}
	int idx = pdePtr->pd_base - FRAME0;
	frm_tab[idx].fr_refcnt++;
	idx = get_frm(&framePointer);
	if (idx == SYSERR) {
		return SYSERR;
	}

	frm_tab[idx].fr_status = 1;
	frm_tab[idx].fr_type = FR_PAGE;
	frm_tab[idx].fr_pid = getpid();
	frm_tab[idx].fr_vpno = vp;
	frm_tab[idx].fr_dirty = 0;
	if (grpolicy() != AGING) {
		
		struct scPolicy *node;
		struct scPolicy *temp = scPtr->next;

		node->frame = idx;
		
		scPtr->next = node;
		node->prev = scPtr;
		node->next = temp;
		temp->prev = node;
		scPtr = node->next;
	}
	else {
		struct fifo frameToInsert;
        frameToInsert.idx = idx;
		frameToInsert.age = 255;

		frameToInsert.next = fifohead.next;
		fifohead.next = &frameToInsert;
	}
	
	int store, pageth;
	int temp = bsm_lookup(getpid(), faultingPage, &store, &pageth);
	if (temp < 0)
	if (temp == SYSERR) {
		return SYSERR;
	}
	read_bs( (char*)framePointer, store, pageth);
	
	// unsigned long pteAddress = pdePtr->pd_base*NBPG + 4*pageNumber;
 	pt_t *ptePtr = (pt_t*) (pdePtr->pd_base*NBPG + 4*(vAddrStruct->pt_offset));
	ptePtr->pt_pres = 1;
	ptePtr->pt_base = (int)framePointer/NBPG;
	write_cr3(proctab[currpid].pdbr);
	return OK;
}


