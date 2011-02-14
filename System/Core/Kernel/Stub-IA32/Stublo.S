#
# Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
# This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
#

	.arch	i486

	# Они превратятся в метки, но тут ничего не поделаешь
	KERNEL_CODE_SELECTOR = 8
	KERNEL_DATA_SELECTOR = 16

	.section	.init.stack
	.fill	3072, 1, 0

	.globl		__stackptr
__stackptr:

	.section	.init.text
	.globl		StubEntryLo
StubEntryLo:
	# Первым делом мы инициализируем стек
	lea	__stackptr, %esp

	cld	# gcc вероятно думает что так должно быть всегда!?!

	pushl	%eax
	pushl	%ebx
	call	StubEntry
	addl	$8, %esp
1:	jmp	1b

	.section	.init.text
	.globl		StubBootstrapCaller
StubBootstrapCaller:
	movl	4(%esp), %edi
	movl	8(%esp), %eax

	ltr	%ax

	movl	$0, %esp	# Переходим в стек задачи.
	jmp	*%edi

	.section	.text
	.globl		StubGetCurrentTaskSelector
StubGetCurrentTaskSelector:
	movl	$0, %eax
	str	%ax
	ret

	.section	.text
	.globl		StubSoD
StubSoD:
	pushl	%cs
	pushl	%ds
	pushl	%es
	pushl	%gs
	pushl	%fs
	pushl	%ss
	pushfl
	pushal
	call	StubSoD_Imm

1:	hlt
	jmp	1b


	.section	.text
	.globl		StubGetCPUFutures
StubGetCPUFutures:
	pushl	%ebx
	pushl	%ecx
	pushl	%edx

	pushfl
	movl	(%esp), %ecx
	xorl	$0x200000, (%esp)
	popfl
	pushfl
	popl	%eax
	xorl	%ecx, %eax
	jz	1f

	movl	$1, %eax
	cpuid
	movl	%edx, %eax

1:	popl	%edx
	popl	%ecx
	popl	%ebx
	ret

	.section	.init.text
	.globl		StubSetGDT
StubSetGDT:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%eax

	# 12(%ebp) - gdt_size
	# 8(%ebp) - gdt
	# 4(%ebp) - %eip
	# 0(%ebp) - %ebp

	movl	8(%ebp), %eax
	movl	%eax, dt_descriptor + 2
	movl	12(%ebp), %eax
	decl	%eax
	movw	%ax, dt_descriptor
	lgdtl	dt_descriptor

	movl	$KERNEL_DATA_SELECTOR, %eax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %ss
	movw	%ax, %fs
	movw	%ax, %gs

	ljmp	$KERNEL_CODE_SELECTOR, $1f

1:	popl	%eax
	popl	%ebp
	ret

	.section	.init.text
	.globl		StubSetIDT
StubSetIDT:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%eax

	# 12(%ebp) - idt_size
	# 8(%ebp) - idt
	# 4(%ebp) - %eip
	# 0(%ebp) - %ebp

	movl	8(%ebp), %eax
	movl	%eax, dt_descriptor + 2
	movl	12(%ebp), %eax
	decl	%eax
	movw	%ax, dt_descriptor
	lidtl	dt_descriptor

	popl	%eax
	popl	%ebp
	ret

	.section	.init.data
dt_descriptor:
	.word	0
	.long	0


	.section	.init.text
	.globl		StubInitCR3
StubInitCR3:
	movl	4(%esp), %eax
	movl	%eax, %cr3

	movl	%cr0, %eax
	btsl	$31, %eax
	movl	%eax, %cr0

	ret

	.section	.init.text
	.globl		StubGetCR3
StubGetCR3:
	movl	%cr3, %eax
	ret

	.section	.text
	.globl		StubIsPaged
StubIsPaged:
	movl	%cr0, %eax
	andl	$0x80000000, %eax
	ret

	.section	.text
	.globl		StubPageFlush
StubPageFlush:
	movl	%cr3, %eax
	movl	%eax, %cr3
	ret

	.section	.text
	.globl		StubTaskSwitch
StubTaskSwitch:
	#	4(%esp)	- task selector
	#	0(%esp) - offset - ignored

	ljmp	*(%esp)

	ret

	.section	.text
	.globl		StubIdle
StubIdle:
	sti
	hlt
	ret

# ------------------------------------------------------------------------------
# Блокировки

	.section	.text
	.globl		StubLock
StubLock:
	pushl	%eax
	pushl	%ebx

	movl	12(%esp), %ebx
1:	cmpl	$0, (%ebx)
	jnz	1b

	movl	$0x10cfed, %eax		# locked :)
	xchgl	(%ebx), %eax
	cmpl	$0, %eax
	jnz	1b

	popl	%ebx
	popl	%eax
	ret

	.section	.text
	.globl		StubUnlock
StubUnlock:
	pushl		%ebx
	movl		8(%esp), %ebx
	lock andl 	$0, (%ebx)	# с lock нельзя использовать mov :(
	popl		%ebx
	ret

	.section	.text
	.globl		CAS
CAS:
	pushl		%ebx
	pushl		%ecx
	movl		12(%esp), %ebx
	movl		16(%esp), %eax
	movl		20(%esp), %ecx
	cmpxchgl	%ecx, (%ebx)
	jz		1f
	xorl		%eax, %eax
	jmp		2f
1:	movl		$1, %eax
2:	popl		%ecx
	popl		%ebx
	ret

# ------------------------------------------------------------------------------
# Поддержка компилятора...

# деление
# unsigned long long __udivdi3 (unsigned long long a, unsigned long long b);
	.section	.text
	.globl		__udivdi3
__udivdi3:
	pushl	%ebx
	movl	8(%esp), %eax
	movl	12(%esp), %edx
	movl	16(%esp), %ebx
	cmpl	$0, 20(%esp)	# старшая часть делителя.
	jz	1f
	movl	$0, %ebx	# Инициируем ошибку деления.
1:	divl	%ebx
	movl	$0, %edx
	popl	%ebx
	ret

# Остаток от деления
# unsigned long long __umoddi3 (unsigned long long a, unsigned long long b);
	.section	.text
	.globl		__umoddi3
__umoddi3:
	pushl	%ebx
	movl	8(%esp), %eax
	movl	12(%esp), %edx
	movl	16(%esp), %ebx
	cmpl	$0, 20(%esp)	# старшая часть делителя.
	jz	1f
	movl	$0, %ebx	# Инициируем ошибку деления.
1:	divl	%ebx
	movl	%edx, %eax
	movl	$0, %edx
	popl	%ebx
	ret

# ------------------------------------------------------------------------------
# Исключения/Прерывания

.macro	gate_prolog
	pushfl
	pushal
	pushl	%ds
	pushl	%es

	movl	$KERNEL_DATA_SELECTOR, %ebx
	movw	%bx, %ds
	movw	%bx, %es
.endm

.macro	gate_epilog
	popl	%ds
	popl	%es
	popal
	popfl
.endm


	# Типичные исключения
.macro	exc_pattern	code, num, error=no
	.section	.text
	.globl		StubException\code
StubException\code:
.ifc	\error, yes
	xchgl	(%esp), %eax
	gate_prolog
	pushl	%eax
.else
	gate_prolog
	pushl	$0
.endif

	pushl	$\num

	call	StubUnhandledException
	add	$8, %esp

	gate_epilog

.ifc	\error, yes
	popl	%eax
.endif
	iret
.endm

	exc_pattern	DE, 0
	exc_pattern	DB, 1
	exc_pattern	NMI, 2
	exc_pattern	BP, 3
	exc_pattern	OF, 4
	exc_pattern	BR, 5
	exc_pattern	UD, 6
	exc_pattern	NM, 7
	exc_pattern	DF, 8, yes
	exc_pattern	CSO, 9
	exc_pattern	TS, 10, yes
	exc_pattern	NP, 11, yes
	exc_pattern	SS, 12, yes
	exc_pattern	GP, 13, yes
	# exc_pattern	PF, 14, yes	<-- Нетипичный, смотри ниже.
	# 15 Intel reserved
	exc_pattern	MF, 16
	exc_pattern	AC, 17, yes
	exc_pattern	MC, 18
	exc_pattern	XF, 19
	# 20-31 Intel Reserved

	# Нетипичные исключения
	.section	.text
	.globl		StubExceptionPF
StubExceptionPF:
	xchgl	(%esp), %eax
	gate_prolog
	# eax - код ошибки

	movl	%cr2, %ecx
	# ecx - линейный адрес

	pushl	%eax
	pushl	%ecx
	call	StubPageFault
	add	$8, %esp

	gate_epilog
	popl	%eax
	iret


	# Типичные прерывания
.macro	int_pattern	num, proc
	.section	.text
	.globl		StubInterrupt\num
StubInterrupt\num:
	gate_prolog

	pushl	$\num
	call	StubInterruptHandler
	add	$4, %esp

	gate_epilog
	iret
.endm

	int_pattern	0
	int_pattern	1
	int_pattern	2
	int_pattern	3
	int_pattern	4
	int_pattern	5
	int_pattern	6
	int_pattern	7
	int_pattern	8
	int_pattern	9
	int_pattern	10
	int_pattern	11
	int_pattern	12
	int_pattern	13
	int_pattern	14
	int_pattern	15


	# Системные вызовы
.macro	syscall_pattern	name
	.section	.text
	.globl		Kernel\name
Kernel\name:
	pushfl
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	pushl	%edi
	pushl	%ebp

	pushl	%ds
	pushl	%es

	pushl	%eax
	movl	$KERNEL_DATA_SELECTOR, %eax
	movw	%ax, %ds
	movw	%ax, %es
	popl	%eax

	pushl	%edx
	pushl	%ecx
	pushl	%ebx
	pushl	%eax
	call	Stub\name
	add	$16, %esp

	popl	%ds
	popl	%es

	popl	%ebp
	popl	%edi
	popl	%esi
	popl	%edx
	popl	%ecx
	popl	%ebx
	popfl

	iret
.endm


	syscall_pattern		Wait
	syscall_pattern		Find
	syscall_pattern		Create
	syscall_pattern		Call
	syscall_pattern		Attach
	syscall_pattern		Detach
	syscall_pattern		Modify
	syscall_pattern		Info
