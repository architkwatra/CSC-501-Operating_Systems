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
	pd_t *pdePtr = (pd_t*) (proctab[currpid].pdbr + 4*vAddrStruct->pd_offset);
	int framePointer, store, pageth;
	fr_map_t *frmTabPtr;
	if (pdePtr->pd_pres == 0) {		
		int idx = get_frm(&framePointer);
		frmTabPtr = &frm_tab[idx];
		if (idx == SYSERR) {
            return SYSERR;
	    }
		frmTabPtr->fr_status = 1;		
		frmTabPtr->fr_pid = getpid();
		pdePtr->pd_write = 1;
		pdePtr->pd_pres = 1;		
		pdePtr->pd_base = (int)framePointer/NBPG;
		frmTabPtr->fr_type = FR_TBL;
	}
	int idx = pdePtr->pd_base - FRAME0;
	frmTabPtr->fr_refcnt++;
	idx = get_frm(&framePointer);
	if (idx == SYSERR) {
		return SYSERR;
	}

	frmTabPtr->fr_status = 1;
	frmTabPtr->fr_type = FR_PAGE;
	frmTabPtr->fr_pid = getpid();
	frmTabPtr->fr_vpno = faultingPage>>12;
	frmTabPtr->fr_dirty = 0;
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
		struct Aging *fPtr, frameToInsert;
		fPtr = &frameToInsert;
        fPtr->idx = idx;
		fPtr->age = 255;

		frameToInsert.next = agingPolicyPtr.next;
		agingPolicyPtr.next = &frameToInsert;
	}
	
	
	int temp = bsm_lookup(getpid(), faultingPage, &store, &pageth);
	if (temp < 0)
	if (temp == SYSERR) {
		return SYSERR;
	}
	read_bs( (char*)framePointer, store, pageth);
	
 	pt_t *ptePtr = (pt_t*) (pdePtr->pd_base*NBPG + 4*(vAddrStruct->pt_offset));
	 if (ptePtr) {
		 ptePtr->pt_pres = 1;
		ptePtr->pt_base = (int)framePointer/NBPG;
	 }
	write_cr3(proctab[currpid].pdbr);
	return OK;
}


