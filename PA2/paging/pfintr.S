/* pfintr.S - pfintr */


    	   .text
pferrcode: .long 0
           .globl  pfintr,pferrcode 
pfintr:

	popl pferrcode
	;check if it is pushfd or pushfl?????????
	pushfl
	cli
	;check if esi, esp registers are also considered as general purpose 
	;and need to be saved
	;pushl %eax
	;pushl %edx
	;pushl %ecx
	;pushl %ebx
	
	pushal
	
	;calling pfintr
	call pfint
	
	;popl %ebx
	;popl %ecx
	;popl %edx
	;popl %eax
	
	popal
	popfl
	iret
