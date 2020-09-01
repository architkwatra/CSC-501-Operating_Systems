

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

static unsigned long *esp;
static unsigned long *ebp;

void printtos() {

        unsigned long *sp, *fp;

        asm("movl %esp,esp");
        asm("movl %ebp,ebp");
        
	sp = esp;
      	fp = ebp;                
       
	printf("\n\nBefore [0x%X]: 0x%X\n", fp, *fp);
	printf("After [0x%X]: 0x%X\n", sp, *sp);	
	
	++sp;
	int i = 0;	
	while (i<4) {
                if (sp != NULL) {
			kprintf("element[0x%X]: 0x%X\n", sp, *sp);
			++i;
			++sp;
		}
	}
}
