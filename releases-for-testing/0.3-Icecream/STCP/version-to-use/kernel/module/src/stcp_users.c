#include <linux/errno.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/seq_file.h>

#include "stcp_socket.h"
#include "stcp_users.h"

static LIST_HEAD(stcp_users);
static DEFINE_MUTEX(stcp_users_lock);
static struct proc_dir_entry *stcp_proc_dir;

static int stcp_users_show(struct seq_file *m, void *unused)
{
	struct stcp_sock *ssk;

	(void)unused;
	seq_puts(m, "pid\tsocket\n");

	mutex_lock(&stcp_users_lock);
	list_for_each_entry(ssk, &stcp_users, user_node)
		seq_printf(m, "%d\t%px\n", ssk->owner_tgid, &ssk->sk);
	mutex_unlock(&stcp_users_lock);

	return 0;
}

static int stcp_users_open(struct inode *inode, struct file *file)
{
	return single_open(file, stcp_users_show, NULL);
}

static const struct proc_ops stcp_users_proc_ops = {
	.proc_open = stcp_users_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

int stcp_users_init(void)
{
	stcp_proc_dir = proc_mkdir("stcp", NULL);
	if (!stcp_proc_dir)
		return -ENOMEM;

	if (!proc_create("users", 0444, stcp_proc_dir, &stcp_users_proc_ops)) {
		proc_remove(stcp_proc_dir);
		stcp_proc_dir = NULL;
		return -ENOMEM;
	}

	return 0;
}

void stcp_users_exit(void)
{
	proc_remove(stcp_proc_dir);
	stcp_proc_dir = NULL;
}

void stcp_user_register(struct stcp_sock *ssk)
{
	if (!ssk)
		return;

	INIT_LIST_HEAD(&ssk->user_node);
	ssk->owner_tgid = task_tgid_nr(current);
	ssk->user_registered = false;

	mutex_lock(&stcp_users_lock);
	if (!ssk->user_registered) {
		list_add_tail(&ssk->user_node, &stcp_users);
		ssk->user_registered = true;
	}
	mutex_unlock(&stcp_users_lock);
}

void stcp_user_unregister(struct stcp_sock *ssk)
{
	if (!ssk)
		return;

	mutex_lock(&stcp_users_lock);
	if (ssk->user_registered) {
		list_del_init(&ssk->user_node);
		ssk->user_registered = false;
	}
	mutex_unlock(&stcp_users_lock);
}
