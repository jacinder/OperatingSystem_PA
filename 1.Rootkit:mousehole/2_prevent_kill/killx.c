#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/unistd.h>
MODULE_LICENSE("GPL");

uid_t target_uid = -1;
void ** sctable ;

asmlinkage long (*orig_sys_kill)(pid_t pid, int sig);

asmlinkage long killx_sys_kill(pid_t pid, int sig) {
    uid_t uid = -1;
    uid = current->cred->uid.val;
    if (target_uid == uid)
        return -1;
    return orig_sys_kill(pid, sig);
}

static int killx_proc_open(struct inode *inode, struct file *file) {
    return 0 ;
}

static int killx_proc_release(struct inode *inode, struct file *file) {
    return 0 ;
}

static ssize_t killx_proc_read(struct file *file, char __user *ubuf, size_t size, loff_t *offset) 
{
    char buf[256] ;
    ssize_t toread ;
    sprintf("%d",target_uid) ;
    toread = strlen(buf) >= *offset + size ? size : strlen(buf) - *offset ;
    if (copy_to_user(ubuf, buf + *offset, toread))
        return -EFAULT ;
    *offset = *offset + toread ;
    return toread ;
}

static ssize_t killx_proc_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset) 
{
    char buf[128] ;
    if (*offset != 0 || size > 128)
        return -EFAULT ;
    if (copy_from_user(buf, ubuf, size))
        return -EFAULT ;
    sscanf(buf, "%d",&target_uid);
    *offset = strlen(buf) ;
    return *offset ;
}

static const struct file_operations killx_fops = {
    .owner =    THIS_MODULE,
    .open =     killx_proc_open,
    .read =     killx_proc_read,
    .write =    killx_proc_write,
    .llseek =   seq_lseek,
    .release =  killx_proc_release,
} ;

static int __init killx_init(void) {
    unsigned int level ; 
    pte_t * pte ;
    proc_create("killx", S_IRUGO | S_IWUGO, NULL, &killx_fops) ;
    sctable = (void *) kallsyms_lookup_name("sys_call_table") ;
    orig_sys_kill = sctable[__NR_kill] ;
    pte = lookup_address((unsigned long) sctable, &level) ;
    if (pte->pte &~ _PAGE_RW) 
        pte->pte |= _PAGE_RW ;
    sctable[__NR_kill] = killx_sys_kill;
    return 0;
}

static void __exit killx_exit(void) {
    unsigned int level ;
    pte_t * pte ;
    remove_proc_entry("killx", NULL) ;
    sctable[__NR_kill] = orig_sys_kill ;
    pte = lookup_address((unsigned long) sctable, &level) ;
    pte->pte = pte->pte &~ _PAGE_RW ;
}
module_init(killx_init);
module_exit(killx_exit);