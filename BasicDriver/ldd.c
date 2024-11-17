#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

static struct proc_dir_entry* custom_proc;
MODULE_LICENSE("GPL"); //
MODULE_AUTHOR("Vimal"); //
MODULE_DESCRIPTION("Device driver"); //

static char *msg = "Hello\n";
module_param(msg, charp, S_IRUGO);
static ssize_t	my_proc_read(struct file * file_ptr, char *user_space_buffer , size_t len, loff_t *offset);
static ssize_t my_proc_write(struct file *file, const char *buffer, size_t count, loff_t  *data);

static struct proc_ops driver_ops = {

    .proc_read  = my_proc_read,
    .proc_write = my_proc_write
};

static int my_module_init(void){
    printk("Module entry \n");
    custom_proc = proc_create("test_proc", 0U, NULL, &driver_ops);
    return 0;
}

static ssize_t my_proc_read(struct file * file_ptr, char *user_space_buffer , size_t count, loff_t *offset)
{
    int result;
    printk("Proc read \n");
    size_t len = strlen(msg);
    printk("%ld", len);
    if(*offset > 0){
        len = 0;
    }
    else
    {
       result = copy_to_user(user_space_buffer, msg, len);
    }
    *offset += len;
    return len;
   
}

static ssize_t my_proc_write(struct file *file, const char *buffer, size_t count, loff_t  *data)
{
	ssize_t procfs_buffer_size = count;
    char procfs_buffer[256] = {0};
	if (procfs_buffer_size > 256 ) {
		procfs_buffer_size = 256;
	}
	
	if ( copy_from_user(procfs_buffer, buffer, procfs_buffer_size) ) {
		return -1;
	}
    printk("%s", procfs_buffer);
	return procfs_buffer_size;
}

static void my_module_exit(void){
    printk("Module exit : entry \n");
    proc_remove(custom_proc);
    printk("Module exit : exit \n");
}

module_init(my_module_init);
module_exit(my_module_exit);
