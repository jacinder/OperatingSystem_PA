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

char target_file[128] = { 0x0, } ;
uid_t target_uid = -1;
void ** sctable ;
int option = -1;

asmlinkage long (*orig_sys_kill)(pid_t pid, int sig);
asmlinkage int (*orig_sys_open)(const char __user * filename, int flags, umode_t mode) ; 

asmlinkage long mousehole_sys_kill(pid_t pid, int sig) {
    uid_t uid = -1;
    uid = current->cred->uid.val;
    if (target_uid == uid)
        return -1;
    return orig_sys_kill(uid, sig);
}

asmlinkage int openx_sys_open(const char __user * filename, int flags, umode_t mode){
        uid_t uid = current->cred->uid.val;
        if(uid == target_uid){
			if(!strstr(filename, target_file)){
				return -1;
			}
        }
        return orig_sys_open(filename, flags, mode) ;
}

static int mousehole_proc_open(struct inode *inode, struct file *file) {
    return 0 ;
}

static int mousehole_proc_release(struct inode *inode, struct file *file) {
    return 0 ;
}

//This function is called then the /proc file is read
static ssize_t mousehole_proc_read(struct file *file, char __user *ubuf, size_t size, loff_t *offset) 
{
    char buf[256] ;
    ssize_t toread ;
    if(option==1)
    {
        sprintf("uid:%d,filename:%s \n", target_uid, target_file) ;
    }
    else
    {
        sprintf("uid:%d",target_uid);
    }
    toread = strlen(buf) >= *offset + size ? size : strlen(buf) - *offset ;
    if (copy_to_user(ubuf, buf + *offset, toread))
        return -EFAULT ;
    *offset = *offset + toread ;
    return toread ;
}

//This function is called with the /proc file is written
static ssize_t mousehole_proc_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset) 
{
    char buf[128] ;
    if (*offset != 0 || size > 128)
        return -EFAULT ;
    if (copy_from_user(buf, ubuf, size))
        return -EFAULT ;
    option=buf[0];
    if(option==1)
    {
        sscanf(buf,"%d %d%s",&option,&target_uid,target_file);
    }
    else
    {
        sscanf(buf, "%d %d",&option,&target_uid);
    }
    
    *offset = strlen(buf) ;
    return *offset ;
}

static const struct file_operations mousehole_fops = {
    .owner =    THIS_MODULE,
    .open =     mousehole_proc_open,
    .read =     mousehole_proc_read,
    .write =    mousehole_proc_write,
    .llseek =   seq_lseek,
    .release =  mousehole_proc_release,
} ;

static int __init mousehole_init(void) {
    unsigned int level ; 
    pte_t * pte ;
    proc_create("mousehole", S_IRUGO | S_IWUGO, NULL, &mousehole_fops) ;
    sctable = (void *) kallsyms_lookup_name("sys_call_table") ;

    orig_sys_open = sctable[__NR_open] ;
    orig_sys_kill = sctable[__NR_kill] ;

    pte = lookup_address((unsigned long) sctable, &level) ;
    if (pte->pte &~ _PAGE_RW) 
        pte->pte |= _PAGE_RW ;

    sctable[__NR_kill] = mousehole_sys_kill;
    sctable[__NR_open] = mousehole_sys_open ;
    return 0;
}

static void __exit mousehole_exit(void) {
    unsigned int level ;
    pte_t * pte ;
    remove_proc_entry("mousehole", NULL) ;

    sctable[__NR_kill] = orig_sys_kill ;
    sctable[__NR_open] = orig_sys_open ;

    pte = lookup_address((unsigned long) sctable, &level) ;
    pte->pte = pte->pte &~ _PAGE_RW ;
}

module_init(mousehole_init);
module_exit(mousehole_exit);