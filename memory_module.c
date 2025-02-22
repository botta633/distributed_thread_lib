#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/slab.h>

static void catpure memory_pages()
{
    struct task_struct *task;
    struct mm_struct *mm;
    struct vm_area_struct *vma;
    struct page *page;
    unsigned long pfn;
   

}

static int __init memory_module_init(void) {
    printk(KERN_INFO "Memory module loaded\n");
    return 0;
}


