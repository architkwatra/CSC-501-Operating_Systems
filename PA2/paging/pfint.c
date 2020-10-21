/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{

	kprintf("Inside the pfint.c file and function!\n");
	unsigned long vPageNumber = read_cr2();
		
	return OK;
}


