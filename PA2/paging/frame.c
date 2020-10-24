/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */

SYSCALL init_frm()
{	
	
	kprintf("Declaring the page_frames array for all the frames\n");
	
	struct fr_map_t frm_tab[NFRAMES];
	
	struct fr_map_t *ptr = (fr_map_t*)NFRAMES*NBPG;
	
	
	kprintf("The first frame will be pointing at ptr. Also check the assigning of the fr_status\n");
	int i = 0;
	while (i < NFRAMES) {
		//frm_tab[i] = ptr;
		frm_tab[i].fr_status = 0;
		frm_tab[i].fr_pid = -1;
		frm_tab[i].fr_vpno = -1;
		frm_tab[i].fr_refcnt = 0;
		frm_tab[i].fr_type = FR_PAGE;
		frm_tab[i].fr_dirty = 0;
		++ptr;
		i++;
	}

	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
	int currentPolicy = grpolicy();
	//starting with 5 because the first 5 frames 
	//after kernel memory are assigned for global 
	//page tables and global page directories
	int i = 5;
	while (i < NFRAMES) {
		if (frm_tab[i].fr_status == 0) {
			//FRAME0 will give the frame till kernel memory 
			//and after that adding i will give the free frame
			*avail = (int*) (FRAM0 + i)*NBPG;
			//can I do avail = &frm_tab[i]???
			return i;
		}
		i++;
	}

	
	
	//extern struct scPolicyStruct scHead;
	//extern struct agingPolicyStruct agingHead;
	//extern scPolicyStruct *scHeadPointer;


	//This gives the frame which should be swapped out
	int frameNumber = -1;
	if (currentPolicy == AGING) {
		struct agingPolicyStruct *p = &fifoHead;
		struct agingPolicyStruct *q = &(fifoHead->next);
		struct agingPolicyStruct *minPrev = q;
		int min = 256;
		while (p != NULL) {
			p->age = p->age >> 1;
			if (isAccSet(p->frameNumber) != SYSERR) 
				p->age += 128;
			if (p->age < min) {
				min = p->age;
				minPrev = q;
			}
			q = p;
			p = p->next;
		}
		frameNumber = minPrev->next->frameNumber;
		minPrev->next = minPrev->next->next;
		free_frm(frameNumber);
		markPTENonExistent(frameNumber);
		avail = (int*)NBPG*(FRAME0 + frameNumber)
		return OK;
	}
	
	else if (currentPolicy == SC) {
		
		struct scPolicy *scPtr = scHeadPointer;

		while (scHeadPointer != NULL && scHeadPointer->next != scPtr) {
			
			markIfDirty();
			frameNumber = isAccSet(scHeadPointer->next->frameNumber);
			if ( frameNumber == scHeadPointer->next->frameNumber ) {
				//call free frame
				free_frm(frameNumber);
				markPTENoneExistent(frameNumber);
				avail = (int*)NBPG*(FRAME0 + frameNumber);
				scHeadPointer->next = scHeadPointer->next->next;
				return OK;
			}
			
			scHeadPointer = scHeadPointer->next;
		}
	}
	
	return SYSERR;
}

int markPTENonExistent(int frameNumber) {

	int vpn = frm_tab[frameNumber];
	unsigned long pdbr = proctab[frm_tab[frameNumber].fr_pid].pdbr;
	unsigned long ptNumber = vpn >> 10;
	unsigned long pageNumber = (von << 10) >> 10;
	struct pt_t *ptePointer = (pt_t*)(pdbr + sizeof(pt_t)*ptNumber);
	struct pt_t *ptePointer = (pt_t*) pdePtr->pd_base + sizeof(pt_t)*pageNumber;
	ptePointer->pt_pres = 0;
	
	if (getpid() == frm_tab[frameNumber].fr_pid) {
		static unsigned long *eax;
		eax = vpn*NBPG;
		asm("invlpg eax");	
	}
	
	--frm_tab[frameNumber].fr_refcnt;
	if (frm_tab[frameNumber].fr_refcnt == 0)
		pdePtr->pd_pres = 0; 
	return OK;
}

int isAccSet(int idx) {
	int vpn = frm_tab[idx].fr_vpno;
	
	unsigned long pdbr = proctab[frm_tab[idx].fr_pid].pdbr;
        unsigned long ptNumber = vpn>>10;
        unsigned long pageNumber = (vpn<<10)>>10;
	
        unsigned long pdeAddress = pdbr + 4*ptNumber;
	struct pd_t *pdePtr = (pd_t*) pdeAddress;
	
	unsigned int pt =  pdePtr->pd_base;
	struct pt_t *ptePointer = (pt_t*) pt + 4*pageNumber;
	if (ptePointer->pt_acc == 0) {
		return idx;
	}
	
	ptePointer->pt_acc = 0;
	return -1;
	
}


int markIfDirty(int idx) {
        int vpn = frm_tab[idx].fr_vpno;

        unsigned long pdbr = proctab[frm_tab[idx].fr_pid].pdbr;
        unsigned long ptNumber = vpn>>10;
        unsigned long pageNumber = (vpn<<10)>>10;

        unsigned long pdeAddress = pdbr + 4*ptNumber;
        struct pd_t *pdePtr = (pd_t*) pdeAddress;

        unsigned int pt =  pdePtr->pd_base;
        struct pt_t *ptePointer = (pt_t*) pt + 4*pageNumber;
        if (ptePointer->pt_dirty == 1) {
		frm_tab[i].fr_dirty = 1;
        }
	else
		frm_tab[i].fr_dirty = 0;

        return OK;

}


int writeBackDirtyFrames(int pid) {

	int i = 0;
	for (; i < NFRAMES; i++) {
		markIfDirty(i);
		if (frm_tab[i].fr_pid == pid && frm_tab[i].fr_dirty == 1) {
			if (writeDirtyFrame(i) == SYSERR) {
				return SYSERR;
			}
		}
	}
	return OK;
}


int writeDirtyFrame(int i) {

		int *store, *pageth; 
                if (bsm_lookup(frm_tab[i].fr_pid, frm_tab[i].fr_vpno*NBPG /*(vaddr)*/, store, pageth) == SYSERR) {
                        kill(getpid());
                        return SYSERR;
                }   
                char *pointerToSrc = (char*)(FRAME0 + i)*NBPG;
                write_bs(pointerToSrc, *store, *pageth);
                
                frm_tab[i].fr_dirty = 0;
		return OK;
}


int removeFramesOnKill(int pid) {

	struct scq *p = &scqhead;
	struct scq *q = p->next;
	
	while (1) {
		if (q == &scqhead) {
			return OK;
		}

		if (frm_tab[q->idx].fr_pid == pid) {
			q = q->next;
			p->next = q;
		}
		else {
			p = p->next;
			q = q->next;
		}

		
	}
	return SYSERR;

}


/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
	frm_tab[i].fr_status = 0;
	frm_tab[i].fr_pid = -1;
        frm_tab[i].fr_vpno = -1;
	
	if (frm_tab[i].fr_dirty == 1)
		writeDirtyFrame(i)
	return OK;
	
}















