	.file	1 "vm.c"
	.section .mdebug.abi32
	.previous
	.nan	legacy
	.module	fp=32
	.module	nooddspreg
	.abicalls
	.text
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	INIT_LIST_HEAD
	.type	INIT_LIST_HEAD, @function
INIT_LIST_HEAD:
	.frame	$fp,8,$31		# vars= 0, regs= 1/0, args= 0, gp= 0
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-8
	sw	$fp,4($sp)
	move	$fp,$sp
	sw	$4,8($fp)
	lw	$2,8($fp)
	lw	$3,8($fp)
	nop
	sw	$3,0($2)
	lw	$2,8($fp)
	lw	$3,8($fp)
	nop
	sw	$3,4($2)
	nop
	move	$sp,$fp
	lw	$fp,4($sp)
	addiu	$sp,$sp,8
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	INIT_LIST_HEAD
	.size	INIT_LIST_HEAD, .-INIT_LIST_HEAD
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	__list_add
	.type	__list_add, @function
__list_add:
	.frame	$fp,8,$31		# vars= 0, regs= 1/0, args= 0, gp= 0
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-8
	sw	$fp,4($sp)
	move	$fp,$sp
	sw	$4,8($fp)
	sw	$5,12($fp)
	sw	$6,16($fp)
	lw	$2,16($fp)
	lw	$3,8($fp)
	nop
	sw	$3,0($2)
	lw	$2,8($fp)
	lw	$3,16($fp)
	nop
	sw	$3,4($2)
	lw	$2,8($fp)
	lw	$3,12($fp)
	nop
	sw	$3,0($2)
	lw	$2,12($fp)
	lw	$3,8($fp)
	nop
	sw	$3,4($2)
	nop
	move	$sp,$fp
	lw	$fp,4($sp)
	addiu	$sp,$sp,8
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	__list_add
	.size	__list_add, .-__list_add
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	list_add_tail
	.type	list_add_tail, @function
list_add_tail:
	.frame	$fp,32,$31		# vars= 0, regs= 2/0, args= 16, gp= 8
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$fp,24($sp)
	move	$fp,$sp
	sw	$4,32($fp)
	sw	$5,36($fp)
	lw	$2,36($fp)
	nop
	lw	$2,0($2)
	lw	$6,36($fp)
	move	$5,$2
	lw	$4,32($fp)
	.option	pic0
	jal	__list_add
	nop

	.option	pic2
	nop
	move	$sp,$fp
	lw	$31,28($sp)
	lw	$fp,24($sp)
	addiu	$sp,$sp,32
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	list_add_tail
	.size	list_add_tail, .-list_add_tail
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	__list_del
	.type	__list_del, @function
__list_del:
	.frame	$fp,8,$31		# vars= 0, regs= 1/0, args= 0, gp= 0
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-8
	sw	$fp,4($sp)
	move	$fp,$sp
	sw	$4,8($fp)
	sw	$5,12($fp)
	lw	$2,12($fp)
	lw	$3,8($fp)
	nop
	sw	$3,0($2)
	lw	$2,8($fp)
	lw	$3,12($fp)
	nop
	sw	$3,4($2)
	nop
	move	$sp,$fp
	lw	$fp,4($sp)
	addiu	$sp,$sp,8
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	__list_del
	.size	__list_del, .-__list_del
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	list_del
	.type	list_del, @function
list_del:
	.frame	$fp,32,$31		# vars= 0, regs= 2/0, args= 16, gp= 8
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$fp,24($sp)
	move	$fp,$sp
	sw	$4,32($fp)
	lw	$2,32($fp)
	nop
	lw	$3,0($2)
	lw	$2,32($fp)
	nop
	lw	$2,4($2)
	nop
	move	$5,$2
	move	$4,$3
	.option	pic0
	jal	__list_del
	nop

	.option	pic2
	nop
	move	$sp,$fp
	lw	$31,28($sp)
	lw	$fp,24($sp)
	addiu	$sp,$sp,32
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	list_del
	.size	list_del, .-list_del

	.comm	mm_current,4,4
	.rdata
	.align	2
$LC0:
	.ascii	"start test page fault\012\000"
	.align	2
$LC1:
	.ascii	"mm_current created.\012\000"
	.text
	.align	2
	.globl	test_tlb_refill
	.set	nomips16
	.set	nomicromips
	.ent	test_tlb_refill
	.type	test_tlb_refill, @function
test_tlb_refill:
	.frame	$fp,48,$31		# vars= 16, regs= 2/0, args= 16, gp= 8
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$fp,40($sp)
	move	$fp,$sp
	lui	$28,%hi(__gnu_local_gp)
	addiu	$28,$28,%lo(__gnu_local_gp)
	.cprestore	16
	lw	$2,%call16(init_page_pool)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,init_page_pool
1:	jalr	$25
	nop

	lw	$28,16($fp)
	lui	$2,%hi($LC0)
	addiu	$4,$2,%lo($LC0)
	lw	$2,%call16(kernel_printf)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,kernel_printf
1:	jalr	$25
	nop

	lw	$28,16($fp)
	.option	pic0
	jal	create_mm_struct
	nop

	.option	pic2
	lw	$28,16($fp)
	move	$3,$2
	lw	$2,%got(mm_current)($28)
	nop
	sw	$3,0($2)
	lui	$2,%hi($LC1)
	addiu	$4,$2,%lo($LC1)
	lw	$2,%call16(kernel_printf)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,kernel_printf
1:	jalr	$25
	nop

	lw	$28,16($fp)
$L7:
	.option	pic0
	b	$L7
	nop

	.option	pic2
	.set	macro
	.set	reorder
	.end	test_tlb_refill
	.size	test_tlb_refill, .-test_tlb_refill
	.rdata
	.align	2
$LC2:
	.ascii	"mm: %x\012\000"
	.text
	.align	2
	.globl	create_mm_struct
	.set	nomips16
	.set	nomicromips
	.ent	create_mm_struct
	.type	create_mm_struct, @function
create_mm_struct:
	.frame	$fp,40,$31		# vars= 8, regs= 2/0, args= 16, gp= 8
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$fp,32($sp)
	move	$fp,$sp
	lui	$28,%hi(__gnu_local_gp)
	addiu	$28,$28,%lo(__gnu_local_gp)
	.cprestore	16
	li	$4,24			# 0x18
	lw	$2,%call16(kmalloc)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,kmalloc
1:	jalr	$25
	nop

	lw	$28,16($fp)
	sw	$2,24($fp)
	lw	$5,24($fp)
	lui	$2,%hi($LC2)
	addiu	$4,$2,%lo($LC2)
	lw	$2,%call16(kernel_printf)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,kernel_printf
1:	jalr	$25
	nop

	lw	$28,16($fp)
	.option	pic0
	jal	create_pagetable
	nop

	.option	pic2
	lw	$28,16($fp)
	move	$3,$2
	lw	$2,24($fp)
	nop
	sw	$3,4($2)
	lw	$2,24($fp)
	nop
	sw	$0,12($2)
	lw	$2,24($fp)
	nop
	addiu	$2,$2,16
	move	$4,$2
	.option	pic0
	jal	INIT_LIST_HEAD
	nop

	.option	pic2
	lw	$28,16($fp)
	lw	$2,24($fp)
	move	$sp,$fp
	lw	$31,36($sp)
	lw	$fp,32($sp)
	addiu	$sp,$sp,40
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	create_mm_struct
	.size	create_mm_struct, .-create_mm_struct
	.rdata
	.align	2
$LC3:
	.ascii	"pgd: %x\012\000"
	.align	2
$LC4:
	.ascii	"%d, %x\012\000"
	.align	2
$LC5:
	.ascii	"error\012\000"
	.align	2
$LC6:
	.ascii	"%x\012\000"
	.align	2
$LC7:
	.ascii	"end\012\000"
	.text
	.align	2
	.globl	create_pagetable
	.set	nomips16
	.set	nomicromips
	.ent	create_pagetable
	.type	create_pagetable, @function
create_pagetable:
	.frame	$fp,48,$31		# vars= 16, regs= 2/0, args= 16, gp= 8
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$fp,40($sp)
	move	$fp,$sp
	lui	$28,%hi(__gnu_local_gp)
	addiu	$28,$28,%lo(__gnu_local_gp)
	.cprestore	16
	li	$4,4096			# 0x1000
	lw	$2,%call16(kmalloc)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,kmalloc
1:	jalr	$25
	nop

	lw	$28,16($fp)
	sw	$2,32($fp)
	lw	$5,32($fp)
	lui	$2,%hi($LC3)
	addiu	$4,$2,%lo($LC3)
	lw	$2,%call16(kernel_printf)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,kernel_printf
1:	jalr	$25
	nop

	lw	$28,16($fp)
	sw	$0,24($fp)
	.option	pic0
	b	$L11
	nop

	.option	pic2
$L19:
	li	$4,8192			# 0x2000
	lw	$2,%call16(kmalloc)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,kmalloc
1:	jalr	$25
	nop

	lw	$28,16($fp)
	sw	$2,36($fp)
	lw	$2,24($fp)
	nop
	sll	$2,$2,2
	lw	$3,32($fp)
	nop
	addu	$2,$3,$2
	lw	$3,36($fp)
	nop
	sw	$3,0($2)
	lw	$2,24($fp)
	nop
	sll	$2,$2,2
	lw	$3,32($fp)
	nop
	addu	$2,$3,$2
	lw	$2,0($2)
	nop
	move	$6,$2
	lw	$5,24($fp)
	lui	$2,%hi($LC4)
	addiu	$4,$2,%lo($LC4)
	lw	$2,%call16(kernel_printf)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,kernel_printf
1:	jalr	$25
	nop

	lw	$28,16($fp)
	lw	$2,24($fp)
	nop
	sll	$2,$2,2
	lw	$3,32($fp)
	nop
	addu	$2,$3,$2
	lw	$2,0($2)
	nop
	move	$3,$2
	li	$2,134217728			# 0x8000000
	sltu	$2,$3,$2
	beq	$2,$0,$L12
	nop

	lui	$2,%hi($LC5)
	addiu	$4,$2,%lo($LC5)
	lw	$2,%call16(kernel_printf)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,kernel_printf
1:	jalr	$25
	nop

	lw	$28,16($fp)
$L13:
	.option	pic0
	b	$L13
	nop

	.option	pic2
$L12:
	sw	$0,28($fp)
	.option	pic0
	b	$L14
	nop

	.option	pic2
$L18:
	lw	$2,28($fp)
	nop
	sll	$2,$2,3
	lw	$3,36($fp)
	nop
	addu	$2,$3,$2
	beq	$2,$0,$L15
	nop

	lw	$2,28($fp)
	nop
	sll	$2,$2,3
	lw	$3,36($fp)
	nop
	addu	$2,$3,$2
	sw	$0,0($2)
	lw	$2,28($fp)
	nop
	sll	$2,$2,3
	lw	$3,36($fp)
	nop
	addu	$2,$3,$2
	sw	$0,4($2)
	.option	pic0
	b	$L21
	nop

	.option	pic2
$L15:
	lw	$2,28($fp)
	nop
	sll	$2,$2,3
	lw	$3,36($fp)
	nop
	addu	$2,$3,$2
	move	$5,$2
	lui	$2,%hi($LC6)
	addiu	$4,$2,%lo($LC6)
	lw	$2,%call16(kernel_printf)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,kernel_printf
1:	jalr	$25
	nop

	lw	$28,16($fp)
$L17:
	.option	pic0
	b	$L17
	nop

	.option	pic2
$L21:
	lw	$2,28($fp)
	nop
	addiu	$2,$2,1
	sw	$2,28($fp)
$L14:
	lw	$2,28($fp)
	nop
	slt	$2,$2,1024
	bne	$2,$0,$L18
	nop

	lui	$2,%hi($LC7)
	addiu	$4,$2,%lo($LC7)
	lw	$2,%call16(kernel_printf)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,kernel_printf
1:	jalr	$25
	nop

	lw	$28,16($fp)
	lw	$2,24($fp)
	nop
	addiu	$2,$2,1
	sw	$2,24($fp)
$L11:
	lw	$2,24($fp)
	nop
	slt	$2,$2,1024
	bne	$2,$0,$L19
	nop

	lw	$5,32($fp)
	lui	$2,%hi($LC3)
	addiu	$4,$2,%lo($LC3)
	lw	$2,%call16(kernel_printf)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,kernel_printf
1:	jalr	$25
	nop

	lw	$28,16($fp)
$L20:
	.option	pic0
	b	$L20
	nop

	.option	pic2
	.set	macro
	.set	reorder
	.end	create_pagetable
	.size	create_pagetable, .-create_pagetable
	.align	2
	.globl	get_pte
	.set	nomips16
	.set	nomicromips
	.ent	get_pte
	.type	get_pte, @function
get_pte:
	.frame	$fp,32,$31		# vars= 16, regs= 1/0, args= 0, gp= 8
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-32
	sw	$fp,28($sp)
	move	$fp,$sp
	sw	$4,32($fp)
	sw	$5,36($fp)
	lw	$2,36($fp)
	nop
	sw	$2,8($fp)
	lw	$2,8($fp)
	nop
	srl	$2,$2,22
	sw	$2,12($fp)
	lw	$2,8($fp)
	nop
	sll	$2,$2,10
	srl	$2,$2,22
	sw	$2,16($fp)
	lw	$2,12($fp)
	nop
	sll	$2,$2,2
	lw	$3,32($fp)
	nop
	addu	$2,$3,$2
	lw	$3,0($2)
	lw	$2,16($fp)
	nop
	sll	$2,$2,3
	addu	$2,$3,$2
	move	$sp,$fp
	lw	$fp,28($sp)
	addiu	$sp,$sp,32
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	get_pte
	.size	get_pte, .-get_pte
	.align	2
	.globl	pte_is_valid
	.set	nomips16
	.set	nomicromips
	.ent	pte_is_valid
	.type	pte_is_valid, @function
pte_is_valid:
	.frame	$fp,8,$31		# vars= 0, regs= 1/0, args= 0, gp= 0
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-8
	sw	$fp,4($sp)
	move	$fp,$sp
	sw	$4,8($fp)
	lw	$2,8($fp)
	nop
	lw	$2,0($2)
	nop
	srl	$2,$2,1
	andi	$2,$2,0x1
	andi	$2,$2,0x00ff
	move	$sp,$fp
	lw	$fp,4($sp)
	addiu	$sp,$sp,8
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	pte_is_valid
	.size	pte_is_valid, .-pte_is_valid
	.align	2
	.globl	get_pte_tlb_entry
	.set	nomips16
	.set	nomicromips
	.ent	get_pte_tlb_entry
	.type	get_pte_tlb_entry, @function
get_pte_tlb_entry:
	.frame	$fp,8,$31		# vars= 0, regs= 1/0, args= 0, gp= 0
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-8
	sw	$fp,4($sp)
	move	$fp,$sp
	sw	$4,8($fp)
	lw	$2,8($fp)
	nop
	lw	$2,0($2)
	move	$sp,$fp
	lw	$fp,4($sp)
	addiu	$sp,$sp,8
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	get_pte_tlb_entry
	.size	get_pte_tlb_entry, .-get_pte_tlb_entry
	.align	2
	.globl	set_pte_tlb_entry
	.set	nomips16
	.set	nomicromips
	.ent	set_pte_tlb_entry
	.type	set_pte_tlb_entry, @function
set_pte_tlb_entry:
	.frame	$fp,8,$31		# vars= 0, regs= 1/0, args= 0, gp= 0
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-8
	sw	$fp,4($sp)
	move	$fp,$sp
	sw	$4,8($fp)
	sw	$5,12($fp)
	lw	$2,8($fp)
	lw	$3,12($fp)
	nop
	sw	$3,0($2)
	nop
	move	$sp,$fp
	lw	$fp,4($sp)
	addiu	$sp,$sp,8
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	set_pte_tlb_entry
	.size	set_pte_tlb_entry, .-set_pte_tlb_entry
	.align	2
	.globl	find_vma
	.set	nomips16
	.set	nomicromips
	.ent	find_vma
	.type	find_vma, @function
find_vma:
	.frame	$fp,8,$31		# vars= 0, regs= 1/0, args= 0, gp= 0
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-8
	sw	$fp,4($sp)
	move	$fp,$sp
	sw	$4,8($fp)
	sw	$5,12($fp)
	li	$2,1			# 0x1
	move	$sp,$fp
	lw	$fp,4($sp)
	addiu	$sp,$sp,8
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	find_vma
	.size	find_vma, .-find_vma
	.rdata
	.align	2
$LC8:
	.ascii	"do pagefault\012\000"
	.align	2
$LC9:
	.ascii	"alloc phy addr: %x\012\000"
	.text
	.align	2
	.globl	do_pagefault
	.set	nomips16
	.set	nomicromips
	.ent	do_pagefault
	.type	do_pagefault, @function
do_pagefault:
	.frame	$fp,56,$31		# vars= 24, regs= 2/0, args= 16, gp= 8
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-56
	sw	$31,52($sp)
	sw	$fp,48($sp)
	move	$fp,$sp
	lui	$28,%hi(__gnu_local_gp)
	addiu	$28,$28,%lo(__gnu_local_gp)
	.cprestore	16
	sw	$4,56($fp)
	lui	$2,%hi($LC8)
	addiu	$4,$2,%lo($LC8)
	lw	$2,%call16(kernel_printf)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,kernel_printf
1:	jalr	$25
	nop

	lw	$28,16($fp)
	nop
	lw	$2,%got(mm_current)($28)
	nop
	lw	$2,0($2)
	lw	$5,56($fp)
	move	$4,$2
	.option	pic0
	jal	find_vma
	nop

	.option	pic2
	lw	$28,16($fp)
	sw	$2,28($fp)
	lw	$2,28($fp)
	nop
	beq	$2,$0,$L37
	nop

	lw	$2,%got(mm_current)($28)
	nop
	lw	$2,0($2)
	nop
	lw	$2,4($2)
	lw	$5,56($fp)
	move	$4,$2
	.option	pic0
	jal	get_pte
	nop

	.option	pic2
	lw	$28,16($fp)
	sw	$2,32($fp)
	sw	$0,24($fp)
	lw	$2,%got(mm_current)($28)
	nop
	lw	$2,0($2)
	nop
	lw	$2,12($2)
	nop
	slt	$2,$2,3
	beq	$2,$0,$L33
	nop

	lw	$2,%call16(alloc_one_page)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,alloc_one_page
1:	jalr	$25
	nop

	lw	$28,16($fp)
	sw	$2,24($fp)
	lw	$2,24($fp)
	nop
	beq	$2,$0,$L34
	nop

	lw	$2,%got(all_pages)($28)
	nop
	lw	$4,0($2)
	lw	$2,24($fp)
	nop
	srl	$3,$2,12
	move	$2,$3
	sll	$2,$2,1
	addu	$2,$2,$3
	sll	$2,$2,3
	addu	$2,$4,$2
	sw	$2,36($fp)
	lw	$5,24($fp)
	lui	$2,%hi($LC9)
	addiu	$4,$2,%lo($LC9)
	lw	$2,%call16(kernel_printf)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,kernel_printf
1:	jalr	$25
	nop

	lw	$28,16($fp)
	lw	$2,36($fp)
	lw	$3,56($fp)
	nop
	sw	$3,12($2)
	lw	$3,36($fp)
	lw	$2,%got(mm_current)($28)
	nop
	lw	$2,0($2)
	nop
	addiu	$2,$2,16
	move	$5,$2
	move	$4,$3
	.option	pic0
	jal	list_add_tail
	nop

	.option	pic2
	lw	$28,16($fp)
	nop
	lw	$2,%got(mm_current)($28)
	nop
	lw	$2,0($2)
	nop
	lw	$3,12($2)
	nop
	addiu	$3,$3,1
	sw	$3,12($2)
	.option	pic0
	b	$L36
	nop

	.option	pic2
$L34:
	lw	$2,%got(mm_current)($28)
	nop
	lw	$2,0($2)
	lw	$5,56($fp)
	move	$4,$2
	.option	pic0
	jal	swap_one_page
	nop

	.option	pic2
	lw	$28,16($fp)
	sw	$2,24($fp)
	.option	pic0
	b	$L36
	nop

	.option	pic2
$L33:
	lw	$2,%got(mm_current)($28)
	nop
	lw	$2,0($2)
	lw	$5,56($fp)
	move	$4,$2
	.option	pic0
	jal	swap_one_page
	nop

	.option	pic2
	lw	$28,16($fp)
	sw	$2,24($fp)
$L36:
	lw	$2,24($fp)
	nop
	srl	$2,$2,12
	sll	$2,$2,6
	ori	$2,$2,0x1f
	sw	$2,40($fp)
	lw	$5,40($fp)
	lw	$4,32($fp)
	.option	pic0
	jal	set_pte_tlb_entry
	nop

	.option	pic2
	lw	$28,16($fp)
$L37:
	nop
	move	$sp,$fp
	lw	$31,52($sp)
	lw	$fp,48($sp)
	addiu	$sp,$sp,56
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	do_pagefault
	.size	do_pagefault, .-do_pagefault
	.rdata
	.align	2
$LC10:
	.ascii	"swap out vaddr_out %x\012\000"
	.align	2
$LC11:
	.ascii	"swap out paddr %x\012\000"
	.align	2
$LC12:
	.ascii	"new block num: %d\012\000"
	.text
	.align	2
	.globl	swap_one_page
	.set	nomips16
	.set	nomicromips
	.ent	swap_one_page
	.type	swap_one_page, @function
swap_one_page:
	.frame	$fp,48,$31		# vars= 16, regs= 2/0, args= 16, gp= 8
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$fp,40($sp)
	move	$fp,$sp
	lui	$28,%hi(__gnu_local_gp)
	addiu	$28,$28,%lo(__gnu_local_gp)
	.cprestore	16
	sw	$4,48($fp)
	sw	$5,52($fp)
	lw	$2,48($fp)
	nop
	lw	$2,20($2)
	nop
	sw	$2,24($fp)
	lw	$2,24($fp)
	nop
	lw	$2,12($2)
	nop
	sw	$2,28($fp)
	lw	$5,28($fp)
	lui	$2,%hi($LC10)
	addiu	$4,$2,%lo($LC10)
	lw	$2,%call16(kernel_printf)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,kernel_printf
1:	jalr	$25
	nop

	lw	$28,16($fp)
	lw	$2,48($fp)
	nop
	lw	$2,4($2)
	lw	$5,28($fp)
	move	$4,$2
	.option	pic0
	jal	get_pte
	nop

	.option	pic2
	lw	$28,16($fp)
	sw	$2,32($fp)
	lw	$2,32($fp)
	nop
	lw	$2,0($2)
	nop
	srl	$2,$2,6
	li	$3,16711680			# 0xff0000
	ori	$3,$3,0xffff
	and	$2,$2,$3
	sll	$2,$2,12
	sw	$2,36($fp)
	lw	$5,36($fp)
	lui	$2,%hi($LC11)
	addiu	$4,$2,%lo($LC11)
	lw	$2,%call16(kernel_printf)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,kernel_printf
1:	jalr	$25
	nop

	lw	$28,16($fp)
	lw	$2,32($fp)
	nop
	lw	$2,4($2)
	nop
	move	$5,$2
	lw	$4,28($fp)
	lw	$2,%call16(write_page_to_disk)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,write_page_to_disk
1:	jalr	$25
	nop

	lw	$28,16($fp)
	move	$5,$2
	lui	$2,%hi($LC12)
	addiu	$4,$2,%lo($LC12)
	lw	$2,%call16(kernel_printf)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,kernel_printf
1:	jalr	$25
	nop

	lw	$28,16($fp)
	lw	$2,48($fp)
	nop
	lw	$2,4($2)
	lw	$5,52($fp)
	move	$4,$2
	.option	pic0
	jal	get_pte
	nop

	.option	pic2
	lw	$28,16($fp)
	lw	$2,4($2)
	nop
	move	$5,$2
	lw	$4,28($fp)
	lw	$2,%call16(load_page_from_disk)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,load_page_from_disk
1:	jalr	$25
	nop

	lw	$28,16($fp)
	lw	$2,32($fp)
	nop
	lw	$4,0($2)
	li	$3,-3			# 0xfffffffffffffffd
	and	$3,$4,$3
	sw	$3,0($2)
	lw	$4,28($fp)
	lw	$2,%call16(tlb_reset)($28)
	nop
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,tlb_reset
1:	jalr	$25
	nop

	lw	$28,16($fp)
	lw	$2,24($fp)
	lw	$3,52($fp)
	nop
	sw	$3,12($2)
	lw	$2,24($fp)
	nop
	move	$4,$2
	.option	pic0
	jal	list_del
	nop

	.option	pic2
	lw	$28,16($fp)
	lw	$2,36($fp)
	move	$sp,$fp
	lw	$31,44($sp)
	lw	$fp,40($sp)
	addiu	$sp,$sp,48
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	swap_one_page
	.size	swap_one_page, .-swap_one_page
	.align	2
	.globl	get_entry_hi
	.set	nomips16
	.set	nomicromips
	.ent	get_entry_hi
	.type	get_entry_hi, @function
get_entry_hi:
	.frame	$fp,24,$31		# vars= 8, regs= 1/0, args= 0, gp= 8
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$fp,20($sp)
	move	$fp,$sp
	sw	$4,24($fp)
	sw	$0,8($fp)
	lw	$2,24($fp)
	nop
	srl	$3,$2,13
	li	$2,458752			# 0x70000
	ori	$2,$2,0xffff
	and	$2,$3,$2
	sll	$2,$2,13
	lw	$3,8($fp)
	nop
	andi	$3,$3,0x1fff
	or	$2,$3,$2
	sw	$2,8($fp)
	lw	$2,8($fp)
	move	$sp,$fp
	lw	$fp,20($sp)
	addiu	$sp,$sp,24
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	get_entry_hi
	.size	get_entry_hi, .-get_entry_hi
	.ident	"GCC: (GNU) 6.3.0"
