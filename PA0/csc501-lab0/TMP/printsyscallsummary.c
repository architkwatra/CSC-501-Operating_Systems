
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include "lab0.h"

int shouldRecord = 0;
//struct sysCallSummary sysCalls[27]; 
void syscallsummary_start() {
	// To clear the struct array before any call
	int pid = 0;
	int i = 0;
	while(pid <= NPROC) {
                int check = 0;
                struct pentry *temp = &proctab[pid];
                i = 0;
                while(1) {
                        if (i>=27) break;
                        struct sysCallSummary *ptr =  &(temp->sysCalls)[i];
			ptr->pid = -1;
			ptr->frq = 0;
			ptr->totalTime = 0;
                 
                ++i;
        	}
        ++pid;
   }
	
	shouldRecord = 1;
}

void syscallsummary_stop() {
	shouldRecord = 0;	
}

void printsyscallsummary() {
	
	kprintf("\n");
	
	int i = 0;
	int pid = 0;

	while(pid <= NPROC) {
		
		int check = 0;
		struct pentry *temp = &proctab[pid];
		i = 0;
		while(1) {
			if (i>=27) break;
			struct sysCallSummary *ptr =  &(temp->sysCalls)[i];
			if (ptr->pName[0] != '\0' && pid == ptr->pid) {
				if (check == 0) {
					kprintf("Process [pid: %d]\n", pid);
					check = 1;
				}

				int avg = 0;
				if (ptr->frq != 0)
					avg = (ptr->totalTime)/(ptr->frq);
kprintf("	Syscall: %s, count: %d, average execution time: %d \n", ptr->pName, ptr->frq, avg);		
			}		
		++i;
		}	
	++pid;
	}
	
}




























