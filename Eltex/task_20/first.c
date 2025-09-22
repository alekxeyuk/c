#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>

static int __init first_module_init(void) {
  pr_info("Hello world 1.\n");

  return 0;
}

static void __exit first_module_exit(void) {
  pr_info("Goodbye world 1.\n");
}

module_init(first_module_init);
module_exit(first_module_exit);

MODULE_LICENSE("GPL");
