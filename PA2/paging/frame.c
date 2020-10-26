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
	/* 
	*/
	STATWORD ps;
	disable(ps);
	int i = 0;
	for (; i < 1024; i++) {

		frm_tab[i].fr_status = 0;
		frm_tab[i].fr_pid = -1;
		frm_tab[i].fr_vpno = -1;
		frm_tab[i].fr_refcnt = 0;
		frm_tab[i].fr_type = FR_PAGE;
		frm_tab[i].fr_dirty = 0;

		/*framePointer->fr_status = 0;
		framePointer->fr_pid = -1;
		framePointer->fr_vpno = -1;
		framePointer->fr_refcnt = 0;
		framePointer->fr_type = FR_PAGE;
		framePointer->fr_dirty = 0;
		frm_tab[i] = framePointer;
		framePointer++;*/
	}
	disable(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
	STATWORD ps;
        disable(ps);
	int i = 0;
	for (; i < NFRAMES; i++) { /*
		if (frm_tab[i].fr_type == FR_PAGE)
			markIfDirty(i);*/
		if (frm_tab[i].fr_status == 0) {
			*avail = (i + FRAME0)*NBPG;
			kprintf("\nInside the get_frm() and *avail = %d and i = %d\n", *avail, i);
			restore(ps);
			return OK;
		}
	}
/*
 38 struct scq scqhead;
 39 scqhead.next = NULL;
 40 struct scq *scPointer = &scqhead;*/
	
	if (grpolicy() != AGING) {

		while (1) {
			kprintf("\nHere2\n");
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
				return OK;
			}
			
			scPointer = scPointer->next;
		}		

	}

	else {

		struct fifo *q = &fifohead;
		struct fifo *p = fifohead.next;
		struct fifo *minprev = q;	
		int min = 256;
		while (p != NULL) {
			kprintf("\nHere3\n");
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
		restore(ps);
		return OK;
	}
	restore(ps);
	return SYSERR;
	
	//linear search for free frame
	//	if found: return
	//	else:
	//		call policy function which returns a frame to free
	//		free_frm()
	//		return frame id/address
}



// for the process whose frame is getting evicted, this method writes pt_pres = 0 
// for the PTE of that process which points to that frame.
int markPTENonExistent(int idx) {
        int vpn = frm_tab[idx].fr_vpno;

        unsigned long pdbr = proctab[frm_tab[idx].fr_pid].pdbr;
        unsigned long ptNumber = vpn>>10;
        unsigned long pageNumber = (vpn<<10)>>10;

        unsigned long pdeAddress = pdbr + 4*ptNumber;
        pd_t *pdePtr = (pd_t*) pdeAddress;

        unsigned int pt =  pdePtr->pd_base*NBPG;
        pt_t *ptePointer = pt + 4*pageNumber;
	
	ptePointer->pt_pres = 0;
       

	if (getpid() == frm_tab[idx].fr_pid) {
		eax = vpn*NBPG;
		asm("invlpg eax");
	}

	frm_tab[idx].fr_refcnt--;
	if (frm_tab[idx].fr_refcnt == 0) {
		pdePtr->pd_pres = 0;
	}

	return OK;

}


int isAccSet(int idx) {
	int vpn = frm_tab[idx].fr_vpno;
	
	unsigned long pdbr = proctab[frm_tab[idx].fr_pid].pdbr;
        unsigned long ptNumber = vpn>>10;
        unsigned long pageNumber = (vpn<<10)>>10;
	
        unsigned long pdeAddress = pdbr + 4*ptNumber;
	pd_t *pdePtr = (pd_t*) pdeAddress;
	
	unsigned int pt =  pdePtr->pd_base*NBPG;
	pt_t *ptePointer = (pt_t*) pt + 4*pageNumber;
	if (ptePointer->pt_acc == 0) {
		return idx;
	}
	
	ptePointer->pt_acc = 0;
	return -1;
	
}

/*
int markIfDirty(int idx) {
        int vpn = frm_tab[idx].fr_vpno;

        unsigned long pdbr = proctab[frm_tab[idx].fr_pid].pdbr;
        unsigned long ptNumber = vpn>>10;
        unsigned long pageNumber = (vpn<<10)>>10;

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

}*/




int writeBackDirtyFrames(int pid) {

	int i = 0;
	for (; i < 1024; i++) {
		//kprintf("Going to MID for pid-%d\n", pid);
		if (frm_tab[i].fr_pid == pid && frm_tab[i].fr_type == FR_PAGE) {
//			kprintf("TYPE=%d ID=%d", frm_tab[i].fr_type, i);
			markIfDirty(i);
		}
		//kprintf("1\n");
		if (frm_tab[i].fr_status == 1 && frm_tab[i].fr_pid == pid && frm_tab[i].fr_dirty == 1 && frm_tab[i].fr_type == FR_PAGE) {
			kprintf("2\n");
			if (writeDirtyFrame(i) == SYSERR) {
				kprintf("3\n");
				return SYSERR;
			}
		}
	}
//	kprintf("Returniing from wBDF\n");
	return OK;


}


int writeDirtyFrame(int i) {

		int store, pageth; 
                if (bsm_lookup(frm_tab[i].fr_pid, frm_tab[i].fr_vpno*NBPG /*(vaddr)*/, &store, &pageth) == SYSERR) {
                        return SYSERR;
                }   
                char *pointerToSrc = (FRAME0 + i)*NBPG;
                write_bs(pointerToSrc, store, pageth);
                //dirty no more
                frm_tab[i].fr_dirty = 0;
		return OK;
}



int removeFramesOnKill(int pid) {

	struct scq *p = &scqhead;
	struct scq *q = p->next;
	
	while (1) {
		kprintf("\nHere1\n");
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
	kprintf("\nFrame ID being replaced: %d", i);
	frm_tab[i].fr_status = 0;
        frm_tab[i].fr_vpno = -1;
	
	if (frm_tab[i].fr_dirty == 1) {
		writeDirtyFrame(i);
/*
		//copy back to bs
		int *store, *pageth; 
			return SYSERR;
		}


		char *pointerToSrc = (char*)(FRAME0 + i)*NBPG;
		write_bs(pointerToSrc, *store, *pageth);
		//dirty no more
		frm_tab[i].fr_dirty = 0;
*/
	}
	frm_tab[i].fr_pid = -1;
	// unmap
	// if dirty write back to bs

	return OK;
}


int markIfDirty(int idx) {
        int vpn = frm_tab[idx].fr_vpno;
//	kprintf("pid-%d frameId-%d\n", frm_tab[idx].fr_pid, idx);
        unsigned long pdbr = proctab[frm_tab[idx].fr_pid].pdbr;
        unsigned long ptNumber = vpn>>10;
        unsigned long pageNumber = (vpn<<10)>>10;

        unsigned long pdeAddress = pdbr + 4*ptNumber;
        pd_t *pdePtr = pdeAddress;

        unsigned int pt =  pdePtr->pd_base*NBPG;
        pt_t *ptePointer = pt + 4*pageNumber;
        if (ptePointer->pt_dirty == 1) {
  //              kprintf("FRAMEID-%d is DIRTY\n", idx);
		frm_tab[idx].fr_dirty = 1;
        }
        else
                frm_tab[idx].fr_dirty = 0;

        return OK;

}

