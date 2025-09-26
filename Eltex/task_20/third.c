#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/rwlock.h>
#include <linux/uaccess.h>

static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);

static ssize_t procfile_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t procfile_write(struct file *, const char __user *, size_t, loff_t *);

#define DEVICE_NAME "third"
#define PROCFS_NAME "third"
#define BUF_LEN 127

static int major;
static char msg[BUF_LEN + 1] = "Hello World\n\0";
static rwlock_t lock;

static struct proc_dir_entry *proc_file;
static unsigned long msg_size = 12;

static struct file_operations secondchardev_fops = {
  .owner = THIS_MODULE,
  .read = device_read,
  .write = device_write,
};

static const struct proc_ops proc_third_fops = {
  .proc_read = procfile_read,
  .proc_write = procfile_write,
};

static int __init third_module_init(void) {
  pr_info("Hello world 3.\n");
  rwlock_init(&lock);
  major = register_chrdev(0, DEVICE_NAME, &secondchardev_fops);
  if (major < 0) {
    pr_alert("Registration failed!\n");
    return major;
  }
  pr_info("Device major: %d\n", major);

  proc_file = proc_create(PROCFS_NAME, 0666, NULL, &proc_third_fops);
  if (proc_file == NULL) {
    unregister_chrdev(major, DEVICE_NAME);
    return -ENOMEM;
  }
  pr_info("/proc/%s created\n", PROCFS_NAME);

  return 0;
}

static void __exit third_module_exit(void) {
  pr_info("Goodbye world 3.\n");

  unregister_chrdev(major, DEVICE_NAME);

  proc_remove(proc_file);
}

static ssize_t device_read(struct file *fd, char __user *buffer, size_t length, loff_t *offset) {
  return simple_read_from_buffer(buffer, length, offset, msg, BUF_LEN);
}

static ssize_t device_write(struct file *fd, const char __user *buffer, size_t length, loff_t *offset) {
  ssize_t bytes_written;
  if (length > BUF_LEN) return -EINVAL;

  write_lock(&lock);
  bytes_written = simple_write_to_buffer(msg, BUF_LEN, offset, buffer, length);
  write_unlock(&lock);

  pr_info("New msg: %s\n", msg);
  return bytes_written;
}

static ssize_t procfile_read(struct file *fp, char __user *buffer, size_t length, loff_t *offset) {
  return simple_read_from_buffer(buffer, length, offset, msg, msg_size);
}

static ssize_t procfile_write(struct file *fp, const char __user *buffer, size_t length, loff_t *offset) {
  ssize_t bytes_written;
  if (length > BUF_LEN) return -EINVAL;

  bytes_written = simple_write_to_buffer(msg, BUF_LEN, offset, buffer, length);
  msg_size = bytes_written;
  msg[bytes_written] = '\0';

  return bytes_written;
}

module_init(third_module_init);
module_exit(third_module_exit);

MODULE_LICENSE("GPL");