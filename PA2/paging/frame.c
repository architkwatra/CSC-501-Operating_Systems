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
	fr_map_t frm_tab[NFRAMES];
	//now I don't need this.
	fr_map_t *ptr = NFRAMES*NBPG;
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
			*avail = (FRAME0 + i)*NBPG;
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

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
	frm_tab[i].fr_status = 0;
    frm_tab[i].fr_vpno = -1;
	if (frm_tab[i].fr_dirty == 1)
		writeDirtyFrame(i);
	frm_tab[i].fr_pid = -1;
	return OK;
	
}















