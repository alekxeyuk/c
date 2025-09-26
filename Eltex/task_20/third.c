#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/sysfs.h>
#include <linux/rwlock.h>
#include <linux/uaccess.h>
#include <linux/string.h>

static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);

static ssize_t procfile_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t procfile_write(struct file *, const char __user *, size_t, loff_t *);

static ssize_t sysfs_show(struct kobject *, struct kobj_attribute *, char *);
static ssize_t sysfs_store(struct kobject *, struct kobj_attribute *, const char *, size_t);

#define DEVICE_NAME "third"
#define PROCFS_NAME "third"
#define SYSFS_NAME  "third"
#define BUF_LEN 127

static int major;
static char msg[BUF_LEN + 1] = "Hello World\n\0";
static rwlock_t lock;

static struct proc_dir_entry *proc_file;
static struct kobject *mymodule;

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

static struct kobj_attribute myattribute = __ATTR(msg, 0660, sysfs_show, sysfs_store);

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
    pr_info("Failed to create proc file\n");
    unregister_chrdev(major, DEVICE_NAME);
    return -ENOMEM;
  }
  pr_info("/proc/%s created\n", PROCFS_NAME);

  mymodule = kobject_create_and_add(SYSFS_NAME, kernel_kobj);
  if (!mymodule) {
    pr_info("Failed to create kobject\n");
    unregister_chrdev(major, DEVICE_NAME);
    proc_remove(proc_file);
    return -ENOMEM;
  }
  pr_info("kobject /sys/kernel/%s created\n", SYSFS_NAME);

  int error = sysfs_create_file(mymodule, &myattribute.attr);
  if (error) {
    pr_info("Failed to create sysfs file");
    unregister_chrdev(major, DEVICE_NAME);
    proc_remove(proc_file);
    kobject_put(mymodule);
  }
  pr_info("sysfs file /sys/kernel/%s/msg created\n", SYSFS_NAME);

  return error;
}

static void __exit third_module_exit(void) {
  pr_info("Goodbye world 3.\n");

  unregister_chrdev(major, DEVICE_NAME);

  proc_remove(proc_file);

  kobject_put(mymodule);
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

static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
  memcpy(buf, msg, msg_size);
  return msg_size;
};

static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
  msg_size = min(count, BUF_LEN);
  memcpy(msg, buf, msg_size);
  return msg_size;
};

module_init(third_module_init);
module_exit(third_module_exit);

MODULE_LICENSE("GPL");