/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_TRAP_H
#define JOS_KERN_TRAP_H
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/trap.h>
#include <inc/mmu.h>

/* The kernel's interrupt descriptor table */
extern struct Gatedesc idt[];
extern struct Pseudodesc idt_pd;

void trap_init(void);
void trap_init_percpu(void);
void print_regs(struct PushRegs *regs);
void print_trapframe(struct Trapframe *tf);
void page_fault_handler(struct Trapframe *);
void backtrace(struct Trapframe *);


void page_fault_handler(struct Trapframe *);



// IDT handle func, in kern/trapentry.s
extern void IRQ0(struct Trapframe *);
extern void IRQ1(struct Trapframe *);
extern void IRQ2(struct Trapframe *);
extern void IRQ3(struct Trapframe *);
extern void IRQ4(struct Trapframe *);
extern void IRQ5(struct Trapframe *);
extern void IRQ6(struct Trapframe *);
extern void IRQ7(struct Trapframe *);
extern void IRQ8(struct Trapframe *);
extern void IRQ9(struct Trapframe *);
extern void IRQ10(struct Trapframe *);
extern void IRQ11(struct Trapframe *);
extern void IRQ12(struct Trapframe *);
extern void IRQ13(struct Trapframe *);
extern void IRQ14(struct Trapframe *);
extern void IRQ15(struct Trapframe *);
extern void IRQ16(struct Trapframe *);
extern void IRQ17(struct Trapframe *);
extern void IRQ18(struct Trapframe *);
extern void IRQ19(struct Trapframe *);
extern void IRQ20(struct Trapframe *);
extern void IRQ21(struct Trapframe *);
extern void IRQ22(struct Trapframe *);
extern void IRQ23(struct Trapframe *);
extern void IRQ24(struct Trapframe *);
extern void IRQ25(struct Trapframe *);
extern void IRQ26(struct Trapframe *);
extern void IRQ27(struct Trapframe *);
extern void IRQ28(struct Trapframe *);
extern void IRQ29(struct Trapframe *);
extern void IRQ30(struct Trapframe *);
extern void IRQ31(struct Trapframe *);
extern void IRQ32(struct Trapframe *);


// system call
extern void IRQ48(struct Trapframe *);

// irq hardware request
extern void irq_0();
extern void irq_1();
extern void irq_2();
extern void irq_3();
extern void irq_4();
extern void irq_5();
extern void irq_6();
extern void irq_7();
extern void irq_8();
extern void irq_9();
extern void irq_10();
extern void irq_11();
extern void irq_12();
extern void irq_13();
extern void irq_14();
extern void irq_15();

extern void trap(struct Trapframe *);




#endif /* JOS_KERN_TRAP_H */
