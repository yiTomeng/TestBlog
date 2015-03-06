	.file	"abs.c"
	.section	.rodata
.LC0:
	.string	"a:%p\n"
.LC1:
	.string	"b:%p\n"
.LC2:
	.string	"%d\n"
	.text
.globl main
	.type	main, @function
main:
	leal	4(%esp), %ecx
	andl	$-16, %esp
	pushl	-4(%ecx)
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ecx
	subl	$36, %esp
	movl	$5, -8(%ebp)
	movl	$0, -12(%ebp)
	movl	-8(%ebp), %eax
	sarl	$3, %eax
	movl	%eax, -8(%ebp)
	leal	-8(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC0, (%esp)
	call	printf
	leal	-12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC1, (%esp)
	call	printf
	movl	-8(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC2, (%esp)
	call	printf
	movl	$0, %eax
	addl	$36, %esp
	popl	%ecx
	popl	%ebp
	leal	-4(%ecx), %esp
	ret
	.size	main, .-main
	.ident	"GCC: (GNU) 4.1.2 20070626 (Asianux 3.0 4.1.2-14)"
	.section	.note.GNU-stack,"",@progbits
