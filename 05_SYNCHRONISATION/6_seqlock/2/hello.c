#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/seqlock.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

seqlock_t *seqlock;

static int __init test_hello_init(void)
{
    seqlock = kmalloc(sizeof(seqlock), GFP_KERNEL);

    seqlock_init(seqlock);
    pr_info("Value of sequence after seqlock_init:%d\n", seqlock->seqcount.sequence);

    write_seqlock(seqlock);
    pr_info("Value of sequence after write_seqlock:%d\n", seqlock->seqcount.sequence);
    write_sequnlock(seqlock);
    pr_info("Value of sequence after write_sequnlock:%d\n", seqlock->seqcount.sequence);

    write_seqlock(seqlock);
    pr_info("Value of sequence after write_seqlock:%d\n", seqlock->seqcount.sequence);
    write_sequnlock(seqlock);
    pr_info("Value of sequence after write_sequnlock:%d\n", seqlock->seqcount.sequence);

    kfree(seqlock);
    return -1;
}

static void __exit test_hello_exit(void)
{
}

module_init(test_hello_init);
module_exit(test_hello_exit);
