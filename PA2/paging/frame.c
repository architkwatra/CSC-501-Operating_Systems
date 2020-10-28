
/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
STATWORD ps;
SYSCALL init_frm()
{	
	fr_map_t *ptr = NFRAMES*NBPG;
	disable(ps);
	int i = 0;
	while (i < NFRAMES) {
		frm_tab[i].fr_status = 0;
		frm_tab[i].fr_pid = -1;
		frm_tab[i].fr_vpno = -1;
		frm_tab[i].fr_type = FR_PAGE;
		frm_tab[i].fr_dirty = 0;
		frm_tab[i].fr_refcnt = 0;
		ptr++;
		i++;
	}
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */

void policyCommonStuff(int frm_tab_index) {

	free_frm(frm_tab_index);
	setPdPres(frm_tab_index);
}

SYSCALL get_frm(int* avail)
{
	disable(ps);
	int i;
	i = 0;
	int minAge = 10000;
	while (i < NFRAMES) {
		int frameStatus = frm_tab[i].fr_status;
		if (!frameStatus) {
			*avail = (FRAME0 + i)*NBPG;
			restore(ps);
			return i;
		}
		i++;
	}
	struct Aging *slow = &agingPolicyPtr;
	struct Aging *fast = agingPolicyPtr.next;
	struct Aging *prev = fast;	

	int idx = -1;
	int currPolicy = grpolicy();
	int check = 1;
	struct scPolicy *head = &scPolicyHead;
	if (currPolicy == SC) {
		while (check) {
			if (scPtr->next == head) {
				scPtr = scPolicyHead.next;
			}
			writeBackDF((scPtr->next)->frame);
			int frm_tab_index = getAccBit((scPtr->next)->frame);
			int policyFrame = (scPtr->next)->frame;
			if (frm_tab_index == policyFrame) {
				policyCommonStuff(frm_tab_index);
				*avail = (FRAME0 + frm_tab_index)*NBPG;
				scPtr->next = (scPtr->next)->next;
				restore(ps);
				return frm_tab_index;
			}	
			scPtr = scPtr->next;
		}

	} else {
		while (fast) {
			fast->age = (int) fast->age/2;
			if (getAccBit(fast->idx) != -1) fast->age += 128;
			if (fast->age < minAge) {
				minAge = fast->age;
				prev = fast;
			}			
			slow = fast;
			fast = fast->next;
		}
		int fFrame = (prev->next)->idx;
		prev->next = (prev->next)->next;
		prev->prev = slow;
		policyCommonStuff(fFrame);
		*avail = (FRAME0 + fFrame)*NBPG;	
		return fFrame;
	}
	restore(ps);
	return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
	fr_map_t *ptr = &frm_tab[i];
	ptr->fr_vpno = -1;
	ptr->fr_status = 0;
	ptr->fr_pid = -1;
	if (ptr->fr_dirty) {
		writeDF(i);
	}
	return OK;
}

void getTranslatedAddress(virt_addr_t *a) {
		return a;
} 
static unsigned long *tlb;
int setPdPres(int frameNumber) {
	virt_addr_t *vAddrStruct = (virt_addr_t*)& frm_tab[frameNumber].fr_vpno;
	pd_t *pdePtr = (pt_t*)(proctab[frm_tab[frameNumber].fr_pid].pdbr + sizeof(pt_t)*vAddrStruct->pd_offset);
	pt_t *ptePtr = pdePtr->pd_base*4096 + sizeof(pt_t)*vAddrStruct->pt_offset;

	ptePtr->pt_pres = 0;
	frm_tab[frameNumber].fr_refcnt = frm_tab[frameNumber].fr_refcnt - 1;
	if (frm_tab[frameNumber].fr_pid == getpid()) {
		tlb = frm_tab[frameNumber].fr_vpno*NBPG;
		asm("invlpg tlb");	
	}
	if (!frm_tab[frameNumber].fr_refcnt) pdePtr->pd_pres = 0; 
	return OK;
}

int getAccBit(int frameIndex) {
	virt_addr_t *vAddrStruct = (virt_addr_t*)&frm_tab[frameIndex].fr_vpno;
	unsigned long pdeAddress = proctab[frm_tab[frameIndex].fr_pid].pdbr + 4*vAddrStruct->pd_offset;
	pd_t *pdePtr = (pd_t*) pdeAddress;
	unsigned int pt = pdePtr->pd_base*NBPG;

	pt_t *ptePointer = (pt_t*) pt + 4*vAddrStruct->pt_offset;
	if (ptePointer->pt_acc == 0) {
		return frameIndex;
	} else {
		ptePointer->pt_acc = 0;
	}
	return SYSERR;
}











