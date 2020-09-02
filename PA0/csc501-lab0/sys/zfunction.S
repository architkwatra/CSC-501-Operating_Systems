














.section .text
.globl zfunction
zfunction:
	
	pushl %ebp
	movl %esp,%ebp
		
	movl $0xfffc01ff,%eax
	
	andl %eax,8(%ebp)
	shrl $8,8(%ebp)
	movl $0xff000000,%eax
	orl %eax,8(%ebp)
	movl 8(%ebp),%eax
	movl $0xfff00fff, 8(%ebp)
	popl %ebp
	ret

