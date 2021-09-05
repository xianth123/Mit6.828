// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

// define in lib/entry.S
extern volatile pte_t uvpt[];     // VA of "virtual page table"
extern volatile pde_t uvpd[];     // VA of current page directory

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	if(!(err & FEC_PR) || !(uvpt[PGNUM(addr)] & PTE_COW)){
		panic("[pgfault] fault on no COW page\n");
	}

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	r = sys_page_alloc(0, (void*)PFTEMP, PTE_P | PTE_U | PTE_W);
	if(r < 0) panic("[pgfault] failed to alloc page %e \n", r);

	uintptr_t fault_pg = ROUNDDOWN((uintptr_t)addr, PGSIZE);

	memcpy((void*)PFTEMP, (void*) fault_pg, PGSIZE);

	r = sys_page_map(0, PFTEMP, 0, (void*)fault_pg, PTE_P | PTE_U | PTE_W);
	if(r < 0) panic("[pgfault] failed to map new alloced page %e \n", r);
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	int perm = PTE_U | PTE_P;

	if(uvpt[pn] & (PTE_W | PTE_COW)){
		int new_perm = perm | PTE_COW;
		// remap chile

		r = sys_page_map(0, (void *)(pn * PGSIZE), envid, (void *)(pn * PGSIZE), new_perm);
		if (r < 0) panic("[duppage] failed to map parent(w|COW) -> child: %e\n", r);
		// remap parent
		r = sys_page_map(0, (void *)(pn * PGSIZE), 0, (void *)(pn * PGSIZE), new_perm);
		if (r < 0) panic("[duppage] failed to map parent(w|COW) -> parent: %e\n", r);
	}
	else{
		r = sys_page_map(0, (void *)(pn * PGSIZE), envid, (void*)(pn * PGSIZE), perm);
		if (r < 0) panic("[duppage] failed to map parent(P|U) -> child: %e\n", r);
	}
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	int err;
	set_pgfault_handler(pgfault);


	envid_t cld_envid = sys_exofork();

	// err
	if (cld_envid < 0){
		return cld_envid;
	}

	// cld env
	if (cld_envid == 0){
		// set_pgfault_handler(pgfault);
		thisenv = envs + ENVX(sys_getenvid());
		return 0;
	}

	uintptr_t vaddr;

	for(vaddr = 0; vaddr < USTACKTOP; vaddr += PGSIZE){
		if (vaddr == (UXSTACKTOP - PGSIZE)){ 
			continue;}
		if((uvpd[PDX(vaddr)] & PTE_P) &&
				(uvpt[PGNUM(vaddr)] & PTE_P)  &&
				(uvpt[PGNUM(vaddr)] & PTE_U)){
					duppage(cld_envid, vaddr / PGSIZE);
				}
	}

	err = sys_page_alloc(cld_envid, (void*)(UXSTACKTOP - PGSIZE), PTE_P | PTE_W | PTE_U);

	if (err < 0) return err;

	extern void _pgfault_upcall();
	err = sys_env_set_pgfault_upcall(cld_envid, _pgfault_upcall);
	if (err < 0) return err;

	err = sys_env_set_status(cld_envid, ENV_RUNNABLE);
	if(err < 0) return err;
	return cld_envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
