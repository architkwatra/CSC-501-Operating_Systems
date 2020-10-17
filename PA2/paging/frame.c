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
	kprintf("Declring the page_frames array for all the frames\n");
	struct fr_map_t *ptr = (fr_map_t*)1024*4096 + 1;
	struct fr_map_t frm_tab[NFRAMES];
	kprintf("The first frame will be pointing at ptr\n");
	int i = 0;
	while (i < NFRAMES) {
		frm_tab[i] = ptr;
		++ptr;
	}

	//struct frm_map_t frm_tab = ptr;
	//frm_tab[NFRAMES];

	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
  kprintf("To be implemented!\n");
  return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{

  kprintf("To be implemented!\n");
  return OK;
}



