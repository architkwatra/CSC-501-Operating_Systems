/* gettime.c - gettime */

#include <conf.h>
#include <kernel.h>
#include <date.h>
#include <proc.h>
#include <stdio.h>
#include "lab0.h"
#include <i386.h>
#include <q.h>

extern int getutim(unsigned long *);

extern int ctr1000;
extern int shouldRecord;


/*------------------------------------------------------------------------
 *  gettime  -  get local time in seconds past Jan 1, 1970
 *------------------------------------------------------------------------
 */
SYSCALL	gettime(long *timvar)
{
    /* long	now; */

	/* FIXME -- no getutim */
	
	int start = ctr1000;
	if (shouldRecord) {
		int pid = getpid();
		struct pentry *temp = &proctab[pid];
                struct sysCallSummary *ptr = &(temp->sysCalls)[4];
                strcpy(ptr->pName, "gettime");
                ptr->pid = pid;
                ptr->frq++;
                ptr->totalTime = ptr->totalTime + ctr1000-start;
        }
	return OK;
}
