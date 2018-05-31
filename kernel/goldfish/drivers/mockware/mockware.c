/* 
 *  mockware.c -  create /proc/mockware for demo.
 */
 
#include <linux/kernel.h>    /* We're doing kernel work */
#include <linux/module.h>    /* Specifically, a module */
#include <linux/proc_fs.h>    /* Necessary because we use proc fs */
#include <asm/uaccess.h>    /* for copy_*_user */

#define PROC_ENTRY_FILENAME     "mockware"
#define PROC_ENTRY_MODE     0666   /* simple for demo */
#define PROCFS_MAX_SIZE     256

/**
 * The buffer for this module
 */
static char procfs_buffer[PROCFS_MAX_SIZE];

/**
 * The size of the data hold in the buffer
 */
static unsigned long procfs_buffer_size = 0;

/**
 * The structure keeping information about the /proc file
 */
static struct proc_dir_entry *mockware_file;

/**
 * This funtion is called when the /proc file is read
 */
static ssize_t procfs_read(struct file *filp,    /* see include/linux/fs.h   */
                 char *buffer,    /* buffer to fill with data */
                 size_t length,    /* length of the buffer     */
                 loff_t * offset)
{
    static int finished = 0;

    /* 
     * We return 0 to indicate end of file, that we have
     * no more information. Otherwise, processes will
     * continue to read from us in an endless loop, e.g.,
     * cat /proc/mockware
     */
    if ( finished ) {
        printk(KERN_INFO "procfs_read: END\n");
        finished = 0;
        return 0;
    }
    
    finished = 1;
        
    /* 
     * We use put_to_user to copy the string from the kernel's
     * memory segment to the memory segment of the process
     * that called us. get_from_user, BTW, is
     * used for the reverse. 
     */
    if ( copy_to_user(buffer, procfs_buffer, procfs_buffer_size) ) {
        return -EFAULT;
    }

    printk(KERN_INFO "procfs_read: read %lu bytes\n", procfs_buffer_size);

    return procfs_buffer_size;    /* Return the number of bytes "read" */
}

/*
 * This function is called when /proc is written
 */
static ssize_t
procfs_write(struct file *file, const char *buffer, size_t len, loff_t * off)
{
    if ( len > PROCFS_MAX_SIZE )    {
        procfs_buffer_size = PROCFS_MAX_SIZE;
    }
    else    {
        procfs_buffer_size = len;
    }
    
    if ( copy_from_user(procfs_buffer, buffer, procfs_buffer_size) ) {
        return -EFAULT;
    }

    printk(KERN_INFO "procfs_write: write %lu bytes\n", procfs_buffer_size);
    
    return procfs_buffer_size;
}

/* 
 * The file is opened - we don't really care about
 * that, but it does mean we need to increment the
 * module's reference count. 
 */
int procfs_open(struct inode *inode, struct file *file)
{
    try_module_get(THIS_MODULE);
    return 0;
}

/* 
 * The file is closed - again, interesting only because
 * of the reference count. 
 */
int procfs_close(struct inode *inode, struct file *file)
{
    module_put(THIS_MODULE);
    return 0;        /* success */
}

static struct file_operations file_ops_mockware_file = {
    .read      = procfs_read,
    .write      = procfs_write,
    .open      = procfs_open,
    .release = procfs_close,
};

/* 
 * Module initialization and cleanup 
 */
int init_module()
{
    /* create the /proc file */
    mockware_file = create_proc_entry(PROC_ENTRY_FILENAME, PROC_ENTRY_MODE, NULL);
    
    /* check if the /proc file was created successfuly */
    if (mockware_file == NULL){
        printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
               PROC_ENTRY_FILENAME);
        return -ENOMEM;
    }
    
    mockware_file->proc_fops = &file_ops_mockware_file;
    mockware_file->mode = PROC_ENTRY_MODE; 
    mockware_file->uid = 0;
    mockware_file->gid = 0;
    mockware_file->size = 80;

    printk(KERN_INFO "/proc/%s created\n", PROC_ENTRY_FILENAME);

    return 0;    /* success */
}

void cleanup_module()
{
    remove_proc_entry(PROC_ENTRY_FILENAME, NULL);
    printk(KERN_INFO "/proc/%s removed\n", PROC_ENTRY_FILENAME);
}

MODULE_LICENSE("GPL");   
module_init(init_module);
module_exit(cleanup_module);