#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/unistd.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");

char target_file[128] = { 0x0, } ;
uid_t target_uid = -1;

void ** sctable ;


asmlinkage int (*orig_sys_open)(const char __user * filename, int flags, umode_t mode) ; 

asmlinkage int openx_sys_open(const char __user * filename, int flags, umode_t mode){
        uid_t uid = current->cred->uid.val;
        if(uid == target_uid){
                if(!strcmp(filename, target_file)){
                        return -1;
                }
        }
        return orig_sys_open(filename, flags, mode) ;
}

static int openx_proc_open(struct inode *inode, struct file *file) {
        return 0 ;
}

static int openx_proc_release(struct inode *inode, struct file *file) {
        return 0 ;
}
static ssize_t openx_proc_read(struct file *file, char __user *ubuf, size_t size, loff_t *offset) 
{
        char buf[256] ;
        ssize_t toread ;

        sprintf(buf, "%d's opening %s is banned\n", target_uid, target_file) ;
        toread = strlen(buf) >= *offset + size ? size : strlen(buf) - *offset ;

        if (copy_to_user(ubuf, buf + *offset, toread))
                return -EFAULT ;
        *offset = *offset + toread ;

        return toread ;
}

static ssize_t openx_proc_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset) 
{
        char buf[128] ;

        if (*offset != 0 || size > 128)
                return -EFAULT ;

        if (copy_from_user(buf, ubuf, size))
                return -EFAULT ;
        sscanf(buf, "%d %s",&target_uid, target_file);
        printk("uid:%d,filename:%s \n", target_uid, target_file);

        *offset = strlen(buf) ;
        return *offset ;
}

static const struct file_operations openx_fops = {
        .owner =        THIS_MODULE,
        .open =         openx_proc_open,
        .read =         openx_proc_read,
        .write =        openx_proc_write,
        .llseek =       seq_lseek,
        .release =      openx_proc_release,
} ;

static int __init openx_init(void) {
        unsigned int level ; 
        pte_t * pte ;

        proc_create("openx", S_IRUGO | S_IWUGO, NULL, &openx_fops) ;

        sctable = (void *) kallsyms_lookup_name("sys_call_table") ;

        orig_sys_open = sctable[__NR_open] ;

        pte = lookup_address((unsigned long) sctable, &level) ;
        if (pte->pte &~ _PAGE_RW) 
                pte->pte |= _PAGE_RW ;

        sctable[__NR_open] = openx_sys_open ;

        return 0;
}

static void __exit openx_exit(void) {
        unsigned int level ;
        pte_t * pte ;
        remove_proc_entry("openx", NULL) ;

        sctable[__NR_open] = orig_sys_open ;
        pte = lookup_address((unsigned long) sctable, &level) ;
        pte->pte = pte->pte &~ _PAGE_RW ;
}

module_init(openx_init);
module_exit(openx_exit);