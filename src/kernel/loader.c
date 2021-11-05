#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include "loader.h"

extern Submodule submodule_tbl
	[]; // Pointers to the starting and ending positions of the table in .data.slime section
extern Submodule submodule_tbl_end[];

int register_modules(void)
{
	Submodule *submodule;
#ifdef SLIME_DEBUG
	printk("Slime: submodule_tbl: %p, submodule_tbl_end: %p.",
	       submodule_tbl, submodule_tbl_end);
#endif
	SLIME_FOREACH_SUBMODULE(submodule)
	{
#ifdef SLIME_DEBUG
		printk("Slime: Registering modules...");
#endif
		if ((*submodule->init)() != 0) {
#ifdef SLIME_DEBUG
			printk("Slime: Failed to initialize submodule %s.",
			       submodule->name);
#endif
			return -1;
		}
#ifdef SLIME_DEBUG
		printk("Slime: Submodule %s registered to the framework.",
		       submodule->name);
#endif
	}
	return 0;
}

int deregister_modules(void)
{
	Submodule *submodule;
	SLIME_FOREACH_SUBMODULE(submodule)
	{
		if (submodule->exit() != 0) {
#ifdef SLIME_DEBUG
			printk("Slime: Failed to de-initialize submodule %s.",
			       submodule->name);
#endif
			return -1;
		}
#ifdef SLIME_DEBUG
		printk("Slime: Submodule %s unregistered from the framework.",
		       submodule->name);
#endif
	}
	return 0;
}

static int __init slime_init(void)
{
#ifdef SLIME_DEBUG
	printk("Slime: Initialising...");
#endif
	if (register_modules() == 0 && khook_init() == 0) {
#ifdef SLIME_DEBUG
		printk("Slime: Initialising done...");
#endif
		return 0;
	} else
		return -1;
}

static void __exit slime_exit(void)
{
	khook_cleanup();
	deregister_modules();
}

MODULE_LICENSE("GPL");
module_init(slime_init)

	module_exit(slime_exit)
