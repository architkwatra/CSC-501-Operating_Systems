/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
static unsigned long *eax;
SYSCALL init_frm()
{	
	
	kprintf("Declaring the page_frames array for all the frames\n");
	
	fr_map_t frm_tab[NFRAMES];
	//now I don't need this.
	fr_map_t *ptr = NFRAMES*NBPG;
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
			*avail = (int*) (FRAME0 + i)*NBPG;
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
		struct fifo *q = &fifohead;
		struct fifo *p = fifohead.next;
		struct fifo *minprev = q;	
		int min = 256;
		while (p != NULL) {

			p->age = p->age>>1;
			
			if (isAccSet(p->idx) != -1) {
				if (p->age + 128 > 255)
					p->age = 255;
				else
					p->age += 128;
			}
			if (p->age < min) {
				min = p->age;
				minprev = q;
			}
			q = p;
			p = p->next;
		}
		int idx = (minprev->next)->idx;
		minprev->next = (minprev->next)->next;
		free_frm(idx);
		markPTENonExistent(idx);
		*avail = (FRAME0 + idx)*NBPG;	
		
		return OK;
	}
	
	else if (currentPolicy == SC) {
		
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
				return OK;
			}
			
			scPointer = scPointer->next;
		}
	}
	
	return SYSERR;
}

int markPTENonExistent(int frameNumber) {

	int vpn = frm_tab[frameNumber].fr_vpno;
	unsigned long pdbr = proctab[frm_tab[frameNumber].fr_pid].pdbr;
	unsigned long ptNumber = vpn >> 10;
	unsigned long pageNumber = (vpn << 10) >> 10;
	pd_t *pdePtr = (pt_t*)(pdbr + sizeof(pt_t)*ptNumber);
	pt_t *ptePointer = (pt_t*) pdePtr->pd_base + sizeof(pt_t)*pageNumber;
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
	pd_t *pdePtr = (pd_t*) pdeAddress;
	
	unsigned int pt =  pdePtr->pd_base;
	pt_t *ptePointer = (pt_t*) pt + 4*pageNumber;
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
        pd_t *pdePtr = (pd_t*) pdeAddress;

        unsigned int pt =  pdePtr->pd_base;
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
                        kill(frm_tab[i].fr_pid);
                        return SYSERR;
                }   
                char *pointerToSrc = (FRAME0 + i)*NBPG;
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
	kprintf("PAGE REPLACED = %d", i);
	frm_tab[i].fr_status = 0;
    frm_tab[i].fr_vpno = -1;
	if (frm_tab[i].fr_dirty == 1)
		writeDirtyFrame(i);
	frm_tab[i].fr_pid = -1;
	return OK;
	
}















