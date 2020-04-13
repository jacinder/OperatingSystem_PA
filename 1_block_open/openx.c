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

asmlinkage int openhook_sys_open(const char __user * filename, int flags, umode_t mode){
	uid_t uid = current->cred->uid.val;
	if(uid == target_uid){
		if(!strcmp(filename, target_file)){
			return -1;
		}
	}
	return orig_sys_open(filename, flags, mode) ;
}

static int strcmp(const char __user * filename, char * target_file){

}

static int openhook_proc_open(struct inode *inode, struct file *file) {
	return 0 ;
}

static int openhook_proc_release(struct inode *inode, struct file *file) {
	return 0 ;
}

static ssize_t openhook_proc_read(struct file *file, char __user *ubuf, size_t size, loff_t *offset) 
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

static ssize_t openhook_proc_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset) 
{
	char buf[128] ;
	char uid_buf[12];
	int i=0,len=0;

	if (*offset != 0 || size > 128)
		return -EFAULT ;

	if (copy_from_user(buf, ubuf, size))
		return -EFAULT ;
	//Form of targetid,filename
	for(len=0;buf[len]!='\0';len++);
	for(i=0;buf[i]!=',';i++);
	strncpy(uid_buf,buf,i);
	strncpy(target_file,buf+i+1,len-1);
	kstrtoint(uid_buf, 0, &target_uid);

	*offset = strlen(buf) ;
	return *offset ;
}

static const struct file_operations openhook_fops = {
	.owner = 	THIS_MODULE,
	.open = 	openhook_proc_open,
	.read = 	openhook_proc_read,
	.write = 	openhook_proc_write,
	.llseek = 	seq_lseek,
	.release = 	openhook_proc_release,
} ;

static int __init openhook_init(void) {
	unsigned int level ; 
	pte_t * pte ;

	proc_create("openhook", S_IRUGO | S_IWUGO, NULL, &openhook_fops) ;

	sctable = (void *) kallsyms_lookup_name("sys_call_table") ;

	orig_sys_open = sctable[__NR_open] ;

	pte = lookup_address((unsigned long) sctable, &level) ;
	if (pte->pte &~ _PAGE_RW) 
		pte->pte |= _PAGE_RW ;	

	sctable[__NR_open] = openhook_sys_open ;

	return 0;
}

static void __exit openhook_exit(void) {
	unsigned int level ;
	pte_t * pte ;
	remove_proc_entry("openhook", NULL) ;

	sctable[__NR_open] = orig_sys_open ;
	pte = lookup_address((unsigned long) sctable, &level) ;
	pte->pte = pte->pte &~ _PAGE_RW ;
}

module_init(openhook_init);
module_exit(openhook_exit);