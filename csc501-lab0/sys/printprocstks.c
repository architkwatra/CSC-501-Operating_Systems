
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

static unsigned long *esp;

void printprocstks(int priority) {
	
	unsigned long *sp;
	asm("movl %esp, esp");
	sp = esp;
	//struct pentry *proc = &proctab[pid];
        //unsigned long   *sp, *fp;
        //kprintf("\n Current pid = %d\n",numproc);
	int i = 0;
	//int n = sizeof(&proctab)/sizeof(&proctab[0]);
	//kprintf("\nSIZE OF STRUCT = %d \n", n);
	//kprintf("\nHERE IS THE START OF PRINTPTICSTKS:\n\n");
	struct pentry *ptr;
	while (i <= NPROC) {
		ptr = &proctab[i];
		if (ptr->pprio > priority) {
			kprintf("Process [proc %s]\n",ptr->pname);	
			kprintf("	pid: %d\n", i);
			kprintf("	priority: %d\n", ptr->pprio);
			kprintf("	base: 0x%X\n", ptr->pbase);
			kprintf("	limit: 0x%X\n", ptr->plimit);
			kprintf("	len: %d\n", ptr->pstklen);
			if (currpid != i)
				kprintf("	pointer: 0x%X\n", ptr->pesp);
			else
				kprintf("	pointer: 0x%X", sp);
		}

		i++;
	}

}






