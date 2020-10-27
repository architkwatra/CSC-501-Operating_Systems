
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
	int i = 0;
	while (i < NFRAMES) {
		if (!frm_tab[i].fr_status) {
			*avail = (FRAME0 + i)*NBPG;
			restore(ps);
			return i;
		}
		i++;
	}

	struct fifo *fast = &fifohead;
	struct fifo *slow = fifohead.next;
	struct fifo *prev = fast;	
	int frameNumber = -1;



	if (grpolicy() == SC) {

		while (1) {
			if (scPointer->next == &scqhead) {
				scPointer = scqhead.next;
			}
			
			writeBackDF((scPointer->next)->idx);
			int frm_tab_index = getAccBit((scPointer->next)->idx);
			if (frm_tab_index == (scPointer->next)->idx) {
				
				policyCommonStuff(frm_tab_index);
				
				*avail = (FRAME0 + frm_tab_index)*NBPG;

				scPointer->next = (scPointer->next)->next;
				restore(ps);
				return frm_tab_index;
			}	
			scPointer = scPointer->next;
		}

	} else {
		int minAge = 10000;
		while (slow) {
			slow->age = (int) slow->age/2;

			if (getAccBit(slow->idx) != -1) slow->age += 128;

			if (slow->age < minAge) {
				minAge = slow->age;
				prev = fast;
			}
			
			fast = slow;
			slow = slow->next;
		}

		int freeFrameIndex = (prev->next)->idx;
		prev->next = (prev->next)->next;
		
		policyCommonStuff(freeFrameIndex);


		// free_frm(freeFrameIndex);
		// setPdPres(freeFrameIndex);


		*avail = (FRAME0 + freeFrameIndex)*NBPG;	
		return freeFrameIndex;
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

static unsigned long *eax;

int setPdPres(int frameNumber) {

	virt_addr_t *vAddrStruct = (virt_addr_t*)& frm_tab[frameNumber].fr_vpno;
	unsigned long pdbr = proctab[frm_tab[frameNumber].fr_pid].pdbr;
	unsigned long ptNumber = vAddrStruct->pd_offset;
	unsigned long pageNumber = vAddrStruct->pt_offset;

	pd_t *pdePtr = (pt_t*)(pdbr + sizeof(pt_t)*ptNumber);
	pt_t *ptePointer = pdePtr->pd_base*4096 + sizeof(pt_t)*pageNumber;
	ptePointer->pt_pres = 0;
	
	if (getpid() == frm_tab[frameNumber].fr_pid) {
		//static unsigned long *eax;
		eax = frm_tab[frameNumber].fr_vpno*NBPG;
		asm("invlpg eax");	
	}
	
	frm_tab[frameNumber].fr_refcnt = frm_tab[frameNumber].fr_refcnt - 1;
	if (frm_tab[frameNumber].fr_refcnt == 0)
		pdePtr->pd_pres = 0; 
	return OK;
}

int getAccBit(int frameIndex) {
	virt_addr_t *vAddrStruct = (virt_addr_t*)&frm_tab[frameIndex].fr_vpno;
	// unsigned long pdbr = proctab[frm_tab[idx].fr_pid].pdbr;
	// unsigned long ptNumber = vAddrStruct->pd_offset;
	// unsigned long pageNumber = vAddrStruct->pt_offset;

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


int setDirty(int frameIndex) {

	virt_addr_t *vAddrStruct = (virt_addr_t*)& frm_tab[frameIndex].fr_vpno;
	// unsigned long pdbr = proctab[frm_tab[idx].fr_pid].pdbr;
	// unsigned long ptNumber = vAddrStruct->pd_offset;
	// unsigned long pageNumber = vAddrStruct->pt_offset;

	pd_t *pdPtr = (pd_t*) ((virt_addr_t*)& frm_tab[frameIndex].fr_vpno + 4*vAddrStruct->pd_offset);
	// unsigned int pt =  pdPtr->pd_base*NBPG;
	pt_t *ptePtr = pdPtr->pd_base*NBPG + 4*vAddrStruct->pt_offset;

	frm_tab[frameIndex].fr_dirty = !ptePtr->pt_dirty ? 0 : 1;
	return OK;
	
	// if (!ptePtr->pt_dirty) 
	// 	frm_tab[frameIndex].fr_dirty = 0;
	// else
	// 	frm_tab[frameIndex].fr_dirty = 1;
    

}


int writeBackDF(int pid) {
	int i = 0;
	while (i < NFRAMES) {
		if (frm_tab[i].fr_pid == pid && frm_tab[i].fr_type == FR_PAGE) {
			setDirty(i);
		}
		if (frm_tab[i].fr_status == 1 && frm_tab[i].fr_pid == pid && frm_tab[i].fr_dirty == 1 && frm_tab[i].fr_type == FR_PAGE) {
			if (writeDF(i) == SYSERR) {
				return SYSERR;
			}
		}

		i++;
	}
	return OK;
}

int writeDF(int i) {
		int store, pageth, pid = frm_tab[i].fr_pid, vpno = NBPG*frm_tab[i].fr_vpno;
		int catch = bsm_lookup(pid, vpno, &store, &pageth); 
		if (catch != SYSERR) { 
			write_bs((char *) ((FRAME0 + i)*NBPG), store, pageth);
			frm_tab[i].fr_dirty = 0;
			return OK;
		}
		else {
			return SYSERR;
		}
		return SYSERR;
}









