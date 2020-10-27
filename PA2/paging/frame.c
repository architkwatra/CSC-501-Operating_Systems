
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

	if (grpolicy() == AGING) {
		int minAge = 256;
		while (slow) {
			slow->age = (int) slow->age/2;
			if (isAccSet(slow->idx) != -1) {
				if (slow->age + 128 > 255)
					slow->age = 255;
				else
					slow->age += 128;
			}
			if (slow->age < minAge) {
				minAge = slow->age;
				prev = fast;
			}
			fast = slow;
			slow = slow->next;
		}
		int idx = (prev->next)->idx;
		prev->next = (prev->next)->next;
		free_frm(idx);
		markPTENonExistent(idx);
		*avail = (FRAME0 + idx)*NBPG;	
		
		return idx;
	}
	
	else {
		
		while (1) {
			if (scPointer->next == &scqhead) {
				scPointer = scqhead.next;
			}
			
			markIfDirty((scPointer->next)->idx);
			int idx = isAccSet((scPointer->next)->idx);
			if (idx == (scPointer->next)->idx) {
				//call free frame
				free_frm(idx);
				markPTENonExistent(idx);
				*avail = (FRAME0 + idx)*NBPG;
				//deleting node from scq
				scPointer->next = (scPointer->next)->next;
				restore(ps);
				return idx;
			}
			
			scPointer = scPointer->next;
		}
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
		writeBackBS(i);
	}
	return OK;
}

void getTranslatedAddress(virt_addr_t *a) {
		return a;
} 

static unsigned long *eax;

int markPTENonExistent(int frameNumber) {

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
	
	--frm_tab[frameNumber].fr_refcnt;
	if (frm_tab[frameNumber].fr_refcnt == 0)
		pdePtr->pd_pres = 0; 
	return OK;
}

int isAccSet(int idx) {
	virt_addr_t *vAddrStruct = (virt_addr_t*)&frm_tab[idx].fr_vpno;
        unsigned long pdbr = proctab[frm_tab[idx].fr_pid].pdbr;
        unsigned long ptNumber = vAddrStruct->pd_offset;
        unsigned long pageNumber = vAddrStruct->pt_offset;
        unsigned long pdeAddress = pdbr + 4*ptNumber;
	pd_t *pdePtr = (pd_t*) pdeAddress;
	unsigned int pt = pdePtr->pd_base*NBPG;
	pt_t *ptePointer = (pt_t*) pt + 4*pageNumber;
	if (ptePointer->pt_acc == 0) {
		return idx;
	}
	
	ptePointer->pt_acc = 0;
	return -1;
	
}


int markIfDirty(int idx) {

	virt_addr_t *vAddrStruct = (virt_addr_t*)& frm_tab[idx].fr_vpno;
	unsigned long pdbr = proctab[frm_tab[idx].fr_pid].pdbr;
	unsigned long ptNumber = vAddrStruct->pd_offset;
	unsigned long pageNumber = vAddrStruct->pt_offset;

	unsigned long pdeAddress = pdbr + 4*ptNumber;
	pd_t *pdePtr = (pd_t*) pdeAddress;

	unsigned int pt =  pdePtr->pd_base*NBPG;
	pt_t *ptePointer = (pt_t*) pt + 4*pageNumber;
	if (ptePointer->pt_dirty == 1) {
		frm_tab[idx].fr_dirty = 1;
	}
	else
		frm_tab[idx].fr_dirty = 0;
    return OK;

}


int writeBackDirtyFrames(int pid) {

	int i = 0;
	while (i < NFRAMES) {
		if (frm_tab[i].fr_pid == pid && frm_tab[i].fr_type == FR_PAGE) {
			markIfDirty(i);
		}
		if (frm_tab[i].fr_status == 1 && frm_tab[i].fr_pid == pid && frm_tab[i].fr_dirty == 1 && frm_tab[i].fr_type == FR_PAGE) {
			if (writeBackBS(i) == SYSERR) {
				return SYSERR;
			}
		}

		i++;
	}
	return OK;
}

int writeBackBS(int i) {
		int store, pageth, pid = frm_tab[i].fr_pid, vpno = NBPG*frm_tab[i].fr_vpno;
		int catch = bsm_lookup(pid, vpno, &store, &pageth); 
		if (catch == SYSERR) 
				return SYSERR;  		
		write_bs((char *) ((FRAME0 + i)*NBPG), store, pageth);
		frm_tab[i].fr_dirty = 0;
		return OK;
}









