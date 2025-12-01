#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/spinlock.h>
#include <asm/io.h>        /* inb() */
#include <linux/ktime.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Keyboard double-press detector using tasklet");
MODULE_VERSION("1.0");

#define KBD_DATA_REG        0x60
#define KBD_SCANCODE_MASK   0x7F
#define KBD_STATUS_MASK     0x80

static int irq = 1;           /* keyboard IRQ */
static int dev_id = 0xAA;     /* dev id for request_irq/free_irq */

/* scancode -> ascii map (partial, same as you used) */
static const unsigned char kbdus[128] = {
    0,  27, '1','2','3','4','5','6','7','8',
    '9','0','-','=','\b','\t','q','w','e','r',
    't','y','u','i','o','p','[',']','\n',0,
    'a','s','d','f','g','h','j','k','l',';',
    '\'','`',0,'\\','z','x','c','v','b','n',
    'm',',','.','/',0,'*',0,' ',0,0,0,0,0,0,0,0,0,0, /* fill rest with zeros */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

/* Small circular buffer to hold scancodes produced in ISR */
#define SC_BUF_SIZE 32
static unsigned char sc_buf[SC_BUF_SIZE];
static unsigned int sc_head = 0;
static unsigned int sc_tail = 0;
static DEFINE_SPINLOCK(sc_lock);

/* Double-press detection state (used by tasklet) */
static unsigned char prev_key = 0;
static unsigned long prev_time = 0; /* in jiffies */
static const unsigned int DOUBLE_MS = 150; /* threshold in ms */

/* forward declaration of tasklet function */
static void keyboard_tasklet_fn(unsigned long data);

/* Declare tasklet */
DECLARE_TASKLET(key_tasklet, keyboard_tasklet_fn, 0);

/* Helper: push scancode into circular buffer, returns 0 on success, -1 on full */
static int sc_buf_push(unsigned char sc)
{
    unsigned long flags;
    unsigned int next;

    spin_lock_irqsave(&sc_lock, flags);
    next = (sc_head + 1) % SC_BUF_SIZE;
    if (next == sc_tail) {
        /* buffer full - drop scancode */
        spin_unlock_irqrestore(&sc_lock, flags);
        return -1;
    }
    sc_buf[sc_head] = sc;
    sc_head = next;
    spin_unlock_irqrestore(&sc_lock, flags);
    return 0;
}

/* Helper: pop one scancode, returns 0 on success and sets *sc, -1 if empty */
static int sc_buf_pop(unsigned char *sc)
{
    unsigned long flags;

    spin_lock_irqsave(&sc_lock, flags);
    if (sc_head == sc_tail) {
        spin_unlock_irqrestore(&sc_lock, flags);
        return -1;
    }
    *sc = sc_buf[sc_tail];
    sc_tail = (sc_tail + 1) % SC_BUF_SIZE;
    spin_unlock_irqrestore(&sc_lock, flags);
    return 0;
}

/* Tasklet bottom-half: process all pending scancodes */
static void keyboard_tasklet_fn(unsigned long data)
{
    unsigned char sc;
    char ch;
    unsigned long now = jiffies;
    unsigned long delta_j;

    while (sc_buf_pop(&sc) == 0) {
        /* ignore key releases (high bit set) */
        if (sc & KBD_STATUS_MASK)
            continue;

        ch = kbdus[sc & KBD_SCANCODE_MASK];
        if (!ch)
            continue;

        /* compute time difference from previous key */
        delta_j = now - prev_time;

        /* convert delta_j (jiffies) to ms using jiffies_to_msecs() */
        if (prev_key == ch && time_before(prev_time + msecs_to_jiffies(DOUBLE_MS), now)) {
            /*
             * time_before(a, b) tests a < b. We want to check if prev_time + thresh > now
             * i.e., now - prev_time <= thresh. We handle the reverse logic below.
             */
            /* not a double press because too late; update state */
            prev_key = ch;
            prev_time = now;
            pr_info("TASKLET: key %c\n", ch);
        } else if (prev_key == ch && time_before(now, prev_time + msecs_to_jiffies(DOUBLE_MS))) {
            /* double press within threshold */
            pr_info("TASKLET: DOUBLE key press detected: %c\n", ch);
            /* reset prev state to avoid triple counting; or set to 0 to require fresh pair */
            prev_key = 0;
            prev_time = 0;
        } else {
            /* first press or different key; store as previous */
            prev_key = ch;
            prev_time = now;
            pr_info("TASKLET: key %c\n", ch);
        }

        /* update 'now' for next popped entry to be a recent timestamp */
        now = jiffies;
    }
}

/* Top-half: interrupt handler */
static irqreturn_t keyboard_handler(int irq_num, void *dev)
{
    unsigned char scancode;

    /* Read scancode from keyboard controller port */
    scancode = inb(KBD_DATA_REG);

    /* Push into buffer (drop if full) */
    sc_buf_push(scancode);

    /* Schedule the tasklet to process buffer */
    tasklet_schedule(&key_tasklet);

    /* We handled IRQ (keyboard IRQ is shared: returning HANDLED is correct when we serviced it) */
    return IRQ_HANDLED;
}

/* Module init */
static int __init dbl_tasklet_init(void)
{
    int ret;

    pr_info("kbd_double_tasklet: init\n");

    /* request IRQ1 (keyboard). Use IRQF_SHARED because many systems share IRQ1. */
    ret = request_irq(irq, keyboard_handler, IRQF_SHARED, "kbd_double_tasklet", &dev_id);
    if (ret) {
        pr_err("kbd_double_tasklet: request_irq failed: %d\n", ret);
        return ret;
    }

    /* ensure tasklet is initialized (DECLARE_TASKLET did it) */
    pr_info("kbd_double_tasklet: loaded (irq=%d)\n", irq);
    return 0;
}

/* Module exit */
static void __exit dbl_tasklet_exit(void)
{
    pr_info("kbd_double_tasklet: exit\n");

    /* Prevent tasklet from running further and wait if it is running */
    tasklet_kill(&key_tasklet);

    /* synchronize and free IRQ */
    synchronize_irq(irq);
    free_irq(irq, &dev_id);

    pr_info("kbd_double_tasklet: unloaded\n");
}

module_init(dbl_tasklet_init);
module_exit(dbl_tasklet_exit);

