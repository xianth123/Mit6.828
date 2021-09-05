// test user-level fault handler -- alloc pages to fix faults
// doesn't work because we sys_cputs instead of cprintf (exercise: why?)

#include <inc/lib.h>

void
handler(struct UTrapframe *utf)
{
	int r;
	void *addr = (void*)utf->utf_fault_va;

	cprintf("fault %x\n", addr);
	if ((r = sys_page_alloc(0, ROUNDDOWN(addr, PGSIZE),
				PTE_P|PTE_U|PTE_W)) < 0)
		panic("allocating at %x in page fault handler: %e", addr, r);
	snprintf((char*) addr, 100, "this string was faulted in at %x", addr);
}

void
handler2(struct UTrapframe *utf)
{
	void *addr = (void*)utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	cprintf("i faulted at va %x, err %x\n", addr, err & 7);
	sys_env_destroy(sys_getenvid());
}

void
umain(int argc, char **argv)
{
	cprintf("umain---------------------------------------\n");
	set_pgfault_handler(handler2);
	cprintf("umain 2---------------------------------------\n");
	sys_cputs((char*)0xDEADBEEF, 4);
}
