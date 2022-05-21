// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

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
	// pgfault()如果不是因为写入一个PTE_COW的page而引发的错误的话，就panic所以前面 !,取反
	if (!((err & FEC_WR) && (uvpd[PDX(addr)] & PTE_P) && (uvpt[PGNUM(addr)] & PTE_COW) && (uvpt[PGNUM(addr)] & PTE_P)))
		panic("page cow check failed");

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	addr = ROUNDDOWN(addr, PGSIZE);
	uint32_t cur_proc = sys_getenvid();
	// panic("pgfault not implemented");
	// 课程网页中的描述，为一个暂时的虚拟地址(PFTEMP)分配一个page
	if ((r = sys_page_alloc(cur_proc, PFTEMP, PTE_P | PTE_U | PTE_W)))
		panic("sys_page_alloc:%e", r);
	// 为暂时的地址分配页后，将造成page fault的地址对应的内容复制到临时地址去
	memmove(PFTEMP, addr, PGSIZE);

	//这里要做的就是将虚拟地址以及引起错误的地址映射到相同的地方去，sys_page_map()函数完成的就是做这件事
    //另外，我们要让page是read/write的
	if ((r = sys_page_map(cur_proc, PFTEMP, cur_proc, addr, PTE_P | PTE_U | PTE_W)))
		panic("sys_page_map:%e", r);
	//然后，这个虚拟地址我们未来还是要用的，所以很自然地，我们需要取消mapping
	if ((r = sys_page_unmap(cur_proc, PFTEMP)))
		panic("sys_page_umap:%e", r);
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
	// panic("duppage not implemented");
	void *addr = (void*)(pn * PGSIZE);
	envid_t cur_proc = sys_getenvid();
	if (uvpt[pn] & (PTE_W | PTE_COW)) {
		//将父进程中addr对应的映射关系复制到envid(子进程)去,并且标记为copy-on-write
		if ((r = sys_page_map(cur_proc, addr, envid, addr, PTE_COW | PTE_U | PTE_P)) < 0) 
			panic("sys_page_map COW:%e", r);
		//父进程中可写入的页不再是它独有了,所以也要在父进程中标记为copy-on-wirte
		if ((r = sys_page_map(cur_proc, addr, cur_proc, addr, PTE_COW | PTE_U | PTE_P)) < 0)
			panic("sys_page_map COW:%e", r);
	} else {
		//如果不是writeable的,那么简单了,直接复制就行
		if ((r = sys_page_map(cur_proc, addr, envid, addr, PTE_U | PTE_P)) < 0) 
			panic("sys_page_map UP:%e", r);	
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
	// panic("fork not implemented");
	set_pgfault_handler(pgfault);
	envid_t envid = sys_exofork();

	uint32_t addr;
	if (envid < 0) 
		panic("sys_exofork:%e", envid);
	if (envid == 0) {
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}
    for(addr = 0; addr < USTACKTOP; addr += PGSIZE) {
        //并不是0-USTACKTOP所有的地址内容都要被复制到子子进程当中去，我们只复制PTE_P且PTE_U的
        if((uvpd[PDX(addr)] & PTE_P) && (uvpt[PGNUM(addr)] & PTE_P) && (uvpt[PGNUM(addr)] & PTE_U))
            duppage(envid, PGNUM(addr));
    }

	//根据页面描述，exception stack不能复制，要重新申请一个page
	int r;
	if ((r = sys_page_alloc(envid, (void *)(UXSTACKTOP - PGSIZE), PTE_P|PTE_U|PTE_W)))
		panic("sys_page_alloc:%e", r);
	//设置子进程的page fault handler
	extern void _pgfault_upcall();
	sys_env_set_pgfault_upcall(envid, _pgfault_upcall);
	//标记子进程为runnable
	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)))
		panic("sys_env_set_status:%e", r);
	return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
