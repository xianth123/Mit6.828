// Test Conversation between parent and child environment
// Contributed by Varun Agrawal at Stony Brook

#include <inc/lib.h>

 char *str1 = "hello child environment! how are you?";
 char *str2 = "hello parent environment! I'm good.";

#define TEMP_ADDR	((char*)0xa00000)
#define TEMP_ADDR_CHILD	((char*)0xb00000)

void
umain(int argc, char **argv)
{
	envid_t who;
	cprintf("\n\n\n");
	cprintf("-------------------------------\n");
	cprintf("str1 addr: %x \n", &str1);
	cprintf("str2 addr: %x \n", &str2);
	cprintf("str1 : %s \n", str1);
	cprintf("str2 : %s \n", str2);

	if ((who = fork()) == 0) {
		// Child
		ipc_recv(&who, TEMP_ADDR_CHILD, 0);
		cprintf("%x got message: %s\n", who, TEMP_ADDR_CHILD);
		if (strncmp(TEMP_ADDR_CHILD, str1, strlen(str1)) == 0)
			cprintf("child received correct message\n");

		memcpy(TEMP_ADDR_CHILD, str2, strlen(str2) + 1);
		ipc_send(who, 0, TEMP_ADDR_CHILD, PTE_P | PTE_W | PTE_U);
		return;
	}

	// Parent
	sys_page_alloc(thisenv->env_id, TEMP_ADDR, PTE_P | PTE_W | PTE_U);
	memcpy(TEMP_ADDR, str1, strlen(str1) + 1);
	ipc_send(who, 0, TEMP_ADDR, PTE_P | PTE_W | PTE_U);

	ipc_recv(&who, TEMP_ADDR, 0);
	cprintf("%x got message: %s\n", who, TEMP_ADDR);
	if (strncmp(TEMP_ADDR, str2, strlen(str2)) == 0)
		cprintf("parent received correct message\n");
	return;
}
