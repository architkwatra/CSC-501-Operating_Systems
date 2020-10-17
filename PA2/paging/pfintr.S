/* pfintr.S - pfintr */


    	   .text
pferrcode: .long 0
           .globl  pfintr,pferrcode 
pfintr:

	popl pferrcode
	pushfd
	cli
	;check if esi, esp registers are also considered as general purpose 
	;and need to be saved
	pushl %eax
	pushl %edx
	pushl %ecx
	pushl %ebx
	;call pfintr
	popl %ebx
	popl %ecx
	popl %edx
	popl %eax
	popfd
	ret
