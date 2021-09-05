/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/syscall.h>
#include <kern/console.h>
#include <kern/sched.h>

// Print a string to the system console.
// The string is exactly 'len' characters long.
// Destroys the environment on memory errors.
static void
sys_cputs(const char *s, size_t len)
{
	// Check that the user has permission to read memory [s, s+len).
	// Destroy the environment if not.

	// LAB 3: Your code here.
	user_mem_assert(curenv, (void *)s, len, 0);

	// Print the string supplied by the user.
	cprintf("%.*s", len, s);
}

// Read a character from the system console without blocking.
// Returns the character, or 0 if there is no input waiting.
static int
sys_cgetc(void)
{
	return cons_getc();
}

// Returns the current environment's envid.
static envid_t
sys_getenvid(void)
{
	return curenv->env_id;
}

// Destroy a given environment (possibly the currently running environment).
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_destroy(envid_t envid)
{
	int r;
	struct Env *e;

	if ((r = envid2env(envid, &e, 1)) < 0)
		return r;
	env_destroy(e);
	return 0;
}

// Deschedule current environment and pick a different one to run.
static void
sys_yield(void)
{
	sched_yield();
}

// Allocate a new environment.
// Returns envid of new environment, or < 0 on error.  Errors are:
//	-E_NO_FREE_ENV if no free environment is available.
//	-E_NO_MEM on memory exhaustion.
static envid_t
sys_exofork(void)
{
	// Create the new environment with env_alloc(), from kern/env.c.
	// It should be left as env_alloc created it, except that
	// status is set to ENV_NOT_RUNNABLE, and the register set is copied
	// from the current environment -- but tweaked so sys_exofork
	// will appear to return 0.

	// LAB 4: Your code here.

	struct  Env *e;
	envid_t p_id = sys_getenvid();
	int r = env_alloc(&e, p_id);
	if(r < 0) return -E_NO_FREE_ENV;
	e->env_status = ENV_NOT_RUNNABLE;
	// copy parent trap frame;
	e->env_tf = curenv->env_tf;
	e->env_tf.tf_regs.reg_eax = 0;
	return e->env_id;
}

// Set envid's env_status to status, which must be ENV_RUNNABLE
// or ENV_NOT_RUNNABLE.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if status is not a valid status for an environment.
static int
sys_env_set_status(envid_t envid, int status)
{
	// Hint: Use the 'envid2env' function from kern/env.c to translate an
	// envid to a struct Env.
	// You should set envid2env's third argument to 1, which will
	// check whether the current environment has permission to set
	// envid's status.

	// LAB 4: Your code here.
	if(status != ENV_NOT_RUNNABLE && status != ENV_RUNNABLE) return -E_INVAL;
	struct Env *e;
	int r = envid2env(envid, &e, 1);
	if(r < 0) return r;
	e->env_status = status;
	return 0;
}

// Set envid's trap frame to 'tf'.
// tf is modified to make sure that user environments always run at code
// protection level 3 (CPL 3), interrupts enabled, and IOPL of 0.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_set_trapframe(envid_t envid, struct Trapframe *tf)
{
	// LAB 5: Your code here.
	// Remember to check whether the user has supplied us with a good
	// address!
	panic("sys_env_set_trapframe not implemented");
}

// Set the page fault upcall for 'envid' by modifying the corresponding struct
// Env's 'env_pgfault_upcall' field.  When 'envid' causes a page fault, the
// kernel will push a fault record onto the exception stack, then branch to
// 'func'.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_set_pgfault_upcall(envid_t envid, void *func)
{
	// LAB 4: Your code here.
	struct Env *e;
	int r = envid2env(envid, &e, 1);
	if(r<0) return -E_BAD_ENV;
	e->env_pgfault_upcall = func;
	return 0;
}

// Allocate a page of memory and map it at 'va' with permission
// 'perm' in the address space of 'envid'.
// The page's contents are set to 0.
// If a page is already mapped at 'va', that page is unmapped as a
// side effect.
//
// perm -- PTE_U | PTE_P must be set, PTE_AVAIL | PTE_W may or may not be set,
//         but no other bits may be set.  See PTE_SYSCALL in inc/mmu.h.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
//	-E_INVAL if perm is inappropriate (see above).
//	-E_NO_MEM if there's no memory to allocate the new page,
//		or to allocate any necessary page tables.
static int
sys_page_alloc(envid_t envid, void *va, int perm)
{
	// Hint: This function is a wrapper around page_alloc() and
	//   page_insert() from kern/pmap.c.
	//   Most of the new code you write should be to check the
	//   parameters for correctness.
	//   If page_insert() fails, remember to free the page you
	//   allocated!

	// LAB 4: Your code here.
	struct Env *e;
	struct PageInfo *page;

	// check va , va should below UTOP and page_aligned!
	if((uint32_t)va >= UTOP) return -E_INVAL;
	if(ROUNDDOWN((void *)va, PGSIZE) != va) return -E_INVAL;
	
	//check perm
	// perm shoud have PTE_U and PTE_P
	if((perm & PTE_U)==0 || (perm & PTE_P)==0) return -E_INVAL;
	if((perm | PTE_SYSCALL) != PTE_SYSCALL) return -E_INVAL;

	int r = envid2env(envid, &e, 1);
	if(r < 0) return -E_BAD_ENV;

	page = page_alloc(ALLOC_ZERO);
	if(page == NULL) return -E_NO_MEM;
	r = page_insert(e->env_pgdir, page, va, perm);
	if(r < 0){
		page_free(page);
		return -E_NO_MEM;
	}
	return 0;
}

// Map the page of memory at 'srcva' in srcenvid's address space
// at 'dstva' in dstenvid's address space with permission 'perm'.
// Perm has the same restrictions as in sys_page_alloc, except
// that it also must not grant write access to a read-only
// page.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if srcenvid and/or dstenvid doesn't currently exist,
//		or the caller doesn't have permission to change one of them.
//	-E_INVAL if srcva >= UTOP or srcva is not page-aligned,
//		or dstva >= UTOP or dstva is not page-aligned.
//	-E_INVAL is srcva is not mapped in srcenvid's address space.
//	-E_INVAL if perm is inappropriate (see sys_page_alloc).
//	-E_INVAL if (perm & PTE_W), but srcva is read-only in srcenvid's
//		address space.
//	-E_NO_MEM if there's no memory to allocate any necessary page tables.
static int
sys_page_map(envid_t srcenvid, void *srcva,
	     envid_t dstenvid, void *dstva, int perm)
{
	// Hint: This function is a wrapper around page_lookup() and
	//   page_insert() from kern/pmap.c.
	//   Again, most of the new code you write should be to check the
	//   parameters for correctness.
	//   Use the third argument to page_lookup() to
	//   check the current permissions on the page.

	// LAB 4: Your code here.
	struct Env *e_src;
	struct Env *e_dst;
	int r;
	r = envid2env(srcenvid, &e_src, 1);
	if(r < 0) return -E_BAD_ENV;
	r = envid2env(dstenvid, &e_dst, 1);
	if(r < 0) return -E_BAD_ENV;

	// check va
	if((uint32_t)srcva >= UTOP) return -E_INVAL;
	if(ROUNDDOWN((void *)srcva, PGSIZE) != srcva) return -E_INVAL;
	if((uint32_t)dstva >= UTOP) return -E_INVAL;
	if(ROUNDDOWN((void *)dstva, PGSIZE) != dstva) return -E_INVAL;

	//check perm
	// perm shoud have PTE_U and PTE_P
	if((perm & PTE_U)==0 || (perm & PTE_P)==0) return -E_INVAL;
	if((perm | PTE_SYSCALL) != PTE_SYSCALL) return -E_INVAL;

	struct PageInfo *page;
	pte_t *pte;
	page = page_lookup(e_src->env_pgdir, srcva, &pte);
	if((*pte & PTE_W)==0 && (perm &  PTE_W)!=0){
		return -E_INVAL;
	}
	if(page == NULL) return -E_INVAL;
	r = page_insert(e_dst->env_pgdir, page, dstva, perm);
	if(r < 0) return r;
	return 0;
}

// Unmap the page of memory at 'va' in the address space of 'envid'.
// If no page is mapped, the function silently succeeds.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
static int
sys_page_unmap(envid_t envid, void *va)
{
	// Hint: This function is a wrapper around page_remove().

	// LAB 4: Your code here.
	struct Env *e;
	int r = envid2env(envid, &e, 1);
	if(r < 0) return -E_BAD_ENV;
	if((uint32_t)va >= UTOP) return -E_INVAL;
	if(ROUNDDOWN(va, PGSIZE) != va) return -E_INVAL;
	page_remove(e->env_pgdir, va);
	return 0;
}

// Try to send 'value' to the target env 'envid'.
// If srcva < UTOP, then also send page currently mapped at 'srcva',
// so that receiver gets a duplicate mapping of the same page.
//
// The send fails with a return value of -E_IPC_NOT_RECV if the
// target is not blocked, waiting for an IPC.
//
// The send also can fail for the other reasons listed below.
//
// Otherwise, the send succeeds, and the target's ipc fields are
// updated as follows:
//    env_ipc_recving is set to 0 to block future sends;
//    env_ipc_from is set to the sending envid;
//    env_ipc_value is set to the 'value' parameter;
//    env_ipc_perm is set to 'perm' if a page was transferred, 0 otherwise.
// The target environment is marked runnable again, returning 0
// from the paused sys_ipc_recv system call.  (Hint: does the
// sys_ipc_recv function ever actually return?)
//
// If the sender wants to send a page but the receiver isn't asking for one,
// then no page mapping is transferred, but no error occurs.
// The ipc only happens when no errors occur.
//
// Returns 0 on success, < 0 on error.
// Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist.
//		(No need to check permissions.)
//	-E_IPC_NOT_RECV if envid is not currently blocked in sys_ipc_recv,
//		or another environment managed to send first.
//	-E_INVAL if srcva < UTOP but srcva is not page-aligned.
//	-E_INVAL if srcva < UTOP and perm is inappropriate
//		(see sys_page_alloc).
//	-E_INVAL if srcva < UTOP but srcva is not mapped in the caller's
//		address space.
//	-E_INVAL if (perm & PTE_W), but srcva is read-only in the
//		current environment's address space.
//	-E_NO_MEM if there's not enough memory to map srcva in envid's
//		address space.
static int
sys_ipc_try_send(envid_t envid, uint32_t value, void *srcva, unsigned perm)
{
	// LAB 4: Your code here.
	struct Env *recv_env ;
	int r;
	r = envid2env(envid, &recv_env, 0);
	if (r < 0) return -E_BAD_ENV;
	if (! recv_env->env_ipc_recving) return -E_IPC_NOT_RECV;
	if((uint32_t)srcva < UTOP && srcva != ROUNDDOWN(srcva, PGSIZE)) return -E_INVAL;

	if ((uint32_t)srcva < UTOP){
		// check perm
		if((perm & PTE_U)==0 || (perm & PTE_P)==0) return -E_INVAL;
		if((perm | PTE_SYSCALL) != PTE_SYSCALL) return -E_INVAL;

		// check send env address space
		pte_t *pte_entry; 
		struct PageInfo *pp;
		pp = page_lookup(curenv->env_pgdir, srcva, &pte_entry);
		if(pte_entry == NULL) return -E_INVAL;
		if(((*pte_entry) & PTE_P) == 0) return -E_INVAL;
		if(((*pte_entry) & PTE_W) == 0 && ((perm & PTE_W) == PTE_W)) return -E_INVAL;

		if((uint32_t)recv_env->env_ipc_dstva < UTOP){
			r = page_insert(recv_env->env_pgdir, pp, recv_env->env_ipc_dstva, perm);
			if (r < 0) return -E_NO_MEM;
			recv_env->env_ipc_perm = perm;
		}
	}

	recv_env->env_ipc_perm = 0;
	recv_env->env_ipc_from = curenv->env_id;
	recv_env->env_ipc_value = value;
	recv_env->env_ipc_recving = 0;
	recv_env->env_status = ENV_RUNNABLE;
	recv_env->env_tf.tf_regs.reg_eax = 0;
	return 0;
}

// Block until a value is ready.  Record that you want to receive
// using the env_ipc_recving and env_ipc_dstva fields of struct Env,
// mark yourself not runnable, and then give up the CPU.
//
// If 'dstva' is < UTOP, then you are willing to receive a page of data.
// 'dstva' is the virtual address at which the sent page should be mapped.
//
// This function only returns on error, but the system call will eventually
// return 0 on success.
// Return < 0 on error.  Errors are:
//	-E_INVAL if dstva < UTOP but dstva is not page-aligned.
static int
sys_ipc_recv(void *dstva)
{
	// LAB 4: Your code here.
	if ((uint32_t)dstva < UTOP && dstva != ROUNDDOWN(dstva, PGSIZE)) return -E_INVAL;
	
	if((uint32_t)dstva < UTOP) 
		{curenv->env_ipc_dstva = dstva;}
	else
		{curenv->env_ipc_dstva = (void *)~0;}
	
	curenv->env_status = ENV_NOT_RUNNABLE;
	curenv->env_ipc_recving = 1;
	sched_yield();
	return 0;
}

// print pgdir info, include pte, pde, perm
static int
sys_print_pgdir_va_info(pde_t *pgdir, void * va){

	if (pgdir == 0){
		pgdir = curenv->env_pgdir;
	}


	pde_t pde;
	pte_t *pte;
	int perm;
	struct PageInfo *pp;
	pp = page_lookup(pgdir, va, &pte);
	if (pp == NULL){
		cprintf("pgdir: %x has no va addr %x \n", pgdir, va);
		return 0;
	}
	pde_t pde_entry = pgdir[PDX(va)];
	perm = pde_entry & ~0xfffff000;
	cprintf("va addr is : %x\n", va);
	cprintf("page ph addr is %x, page ref is %d\n", page2pa(pp), pp->pp_ref);
	cprintf("pde_entry: %x, pte_entry perm is %x \n", pde_entry, perm);
	cprintf(" %s,", perm & 0x800? "COW page": "no cow page");
	cprintf(" %s,", perm & PTE_U? "user": "kernel");
	cprintf(" %s,", perm & PTE_W? "write": "read");
	cprintf(" %s\n", perm & PTE_P? "present": "no present");

	perm = *pte & ~0xfffff000;
	cprintf("pte_entry: %x, perm is %x \n", *pte, perm);
	cprintf(" %s,", perm & 0x800? "COW page": "no cow page");
	cprintf(" %s,", perm & PTE_U? "user": "kernel");
	cprintf(" %s,", perm & PTE_W? "write": "read");
	cprintf(" %s,\n", perm & PTE_P? "present": "no present");
	cprintf("---------------\n");
	return 0;
}


// Dispatches to the correct kernel function, passing the arguments.
int32_t
syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	// Call the function corresponding to the 'syscallno' parameter.
	// Return any appropriate return value.
	// LAB 3: Your code here.

	// panic("syscall not implemented");


	int32_t result;
	int print_flag = 0;
	if(print_flag){
		cprintf("syscall num %d\n", syscallno);
	}
	switch (syscallno) {
		case SYS_cputs:
			sys_cputs((char *)a1, (size_t) a2);
			result = 0;
			break;
		case SYS_cgetc:
			// cprintf("syscall: sys_cgetc\n");
			result = sys_cgetc();
			break;
		case SYS_getenvid:
			// cprintf("syscall: sys_getenvid\n");
			result = sys_getenvid();
			break;
		case SYS_env_destroy:
			// cprintf("syscall: sys_env_destroy\n");
			result = sys_env_destroy((envid_t)a1);
			break;
		case SYS_yield:
			// cprintf("syscall: sys_yield\n");
			sys_yield();
			result = 0;
			break;
		case SYS_exofork:
			// cprintf("syscall: sys_exofork\n");
			result = sys_exofork();
			break;
		case SYS_page_alloc:
			// cprintf("syscall: sys_page_alloc\n");
			result = sys_page_alloc((envid_t) a1, (void *) a2, (int)a3);
			break;
		case SYS_page_map:
			// cprintf("syscall: sys_page_map\n");
			result = sys_page_map((envid_t)a1, (void*)a2, (envid_t)a3, (void*)a4, (int)a5);
			break;
		case SYS_page_unmap:
			// cprintf("syscall: sys_page_unmap\n");
			result = sys_page_unmap((envid_t)a1, (void*)a2);
			break;
		case SYS_env_set_status:
			// cprintf("syscall: sys_env_set_status\n");
			return sys_env_set_status((envid_t)a1, (int)a2);
		case SYS_env_set_pgfault_upcall:
			// cprintf("syscall: sys_env_set_pgfault_upcall\n");
			return sys_env_set_pgfault_upcall((envid_t)a1, (void *)a2);
		case SYS_print_pgdir_va_info:
			return sys_print_pgdir_va_info((pde_t *)a1, (void *)a2);
		case SYS_ipc_try_send:
			return sys_ipc_try_send((envid_t)a1, (uint32_t)a2, (void *)a3, (unsigned int) a4);
		case SYS_ipc_recv:
			return sys_ipc_recv((void *)a1);
		default:
			return -E_INVAL;
	}
	return result;
}

