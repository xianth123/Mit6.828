/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

#define PUSH_STACK_VALUE(index)  \
	mov index(%esp), %eax;	\
	pushl %eax

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

 TRAPHANDLER_NOEC(IRQ0, 0);
 TRAPHANDLER_NOEC(IRQ1, 1);
 TRAPHANDLER_NOEC(IRQ2, 2);
 TRAPHANDLER_NOEC(IRQ3, 3);
 TRAPHANDLER_NOEC(IRQ4, 4);
 TRAPHANDLER_NOEC(IRQ5, 5);
 TRAPHANDLER_NOEC(IRQ6, 6);
 TRAPHANDLER_NOEC(IRQ7, 7);
 TRAPHANDLER(IRQ8, 8);
 TRAPHANDLER_NOEC(IRQ9, 9);
 TRAPHANDLER(IRQ10, 10);
 TRAPHANDLER(IRQ11, 11);
 TRAPHANDLER(IRQ12, 12);
 TRAPHANDLER(IRQ13, 13);
 TRAPHANDLER(IRQ14, 14);
 TRAPHANDLER_NOEC(IRQ15, 15);
 TRAPHANDLER_NOEC(IRQ16, 16);
 TRAPHANDLER(IRQ17, 17);
 TRAPHANDLER_NOEC(IRQ18, 18);
 TRAPHANDLER_NOEC(IRQ19, 19);
 TRAPHANDLER_NOEC(IRQ20, 20);
 TRAPHANDLER_NOEC(IRQ21, 21);
 TRAPHANDLER_NOEC(IRQ22, 22);
 TRAPHANDLER_NOEC(IRQ23, 23);
 TRAPHANDLER_NOEC(IRQ24, 24);

 TRAPHANDLER_NOEC(IRQ48, 48);


 TRAPHANDLER_NOEC(irq_0, IRQ_OFFSET + 0);
 TRAPHANDLER_NOEC(irq_1, IRQ_OFFSET + 1);
 TRAPHANDLER_NOEC(irq_2, IRQ_OFFSET + 2);
 TRAPHANDLER_NOEC(irq_3, IRQ_OFFSET + 3);
 TRAPHANDLER_NOEC(irq_4, IRQ_OFFSET + 4);
 TRAPHANDLER_NOEC(irq_5, IRQ_OFFSET + 5);
 TRAPHANDLER_NOEC(irq_6, IRQ_OFFSET + 6);
 TRAPHANDLER_NOEC(irq_7, IRQ_OFFSET + 7);
 TRAPHANDLER_NOEC(irq_8, IRQ_OFFSET + 8);
 TRAPHANDLER_NOEC(irq_9, IRQ_OFFSET + 9);
 TRAPHANDLER_NOEC(irq_10, IRQ_OFFSET + 10);
 TRAPHANDLER_NOEC(irq_11, IRQ_OFFSET + 11);
 TRAPHANDLER_NOEC(irq_12, IRQ_OFFSET + 12);
 TRAPHANDLER_NOEC(irq_13, IRQ_OFFSET + 13);
 TRAPHANDLER_NOEC(irq_14, IRQ_OFFSET + 14);
 TRAPHANDLER_NOEC(irq_15, IRQ_OFFSET + 15);


/*
 * Lab 3: Your code here for _alltraps
 */
 _alltraps:
 	/*
	* push data as TrapFrame, should test the esp and ss value.
*  	+--------------------+ KSTACKTOP             
*  	| 0x00000 | old SS   |     " - 4
*  	|      old ESP       |     " - 8
*  	|     old EFLAGS     |     " - 12
*  	| 0x00000 | old CS   |     " - 16
*  	|      old EIP       |     " - 20 <---- ES
*  	+--------------------+
*  	|      error code    |         24
*  	|      trap num      |         28
*  	+--------------------+      all trap push data
*  	|       ds           |         60
*  	|       es           |         64
*	|      PushRegs      |         96 
*  	+--------------------+      TrapFrame start
*  	| esp pointer of trap|         100    TrapFrame Pointer
*  	+--------------------+  
	*/

	pushl %ds;				// ds
	pushl %es;				// es
	pushal;

 	/*
	* change the ds and es register, because the segment has changed
	*/
	push $0x10;
	pop %ds;
	push $0x10;
	pop %es;

 	/*
	* pushl esp, as the TrapFrame pointer parameters
	*/
	pushl %esp;

 	/*
	* call trap func;
	*/
	call trap;