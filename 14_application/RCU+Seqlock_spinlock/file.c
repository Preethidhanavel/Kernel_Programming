#include <linux/module.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/rculist.h>     /* <-- required for list_for_each_entry_rcu */
#include <linux/rcupdate.h>
#include <linux/spinlock.h>
#include <linux/seqlock.h>
#include <linux/types.h>
#include <linux/printk.h>


MODULE_LICENSE("GPL");

struct client {
    int id;
    struct list_head list;

    seqlock_t seq;
    u64 packets;
    u64 bytes;
};

/* Global list + lock */
static LIST_HEAD(client_list);
static spinlock_t client_lock;

/* ---------- Helpers ---------- */

static void client_init(struct client *c, int id)
{
    c->id = id;
    INIT_LIST_HEAD(&c->list);
    seqlock_init(&c->seq);
    c->packets = 0;
    c->bytes = 0;
}

static bool find_client_and_read_stats(int id, u64 *packets, u64 *bytes)
{
    struct client *c;
    unsigned long seq;

    rcu_read_lock();

    list_for_each_entry_rcu(c, &client_list, list) {
        if (c->id == id) {
            do {
                seq = read_seqbegin(&c->seq);
                *packets = c->packets;
                *bytes   = c->bytes;
            } while (read_seqretry(&c->seq, seq));

            rcu_read_unlock();
            return true;
        }
    }

    rcu_read_unlock();
    return false;
}

static void update_client_stats(struct client *c, u64 add_packets, u64 add_bytes)
{
    write_seqlock(&c->seq);
    c->packets += add_packets;
    c->bytes   += add_bytes;
    write_sequnlock(&c->seq);
}

/* ---------- Add / Remove Clients ---------- */

static void add_client(int id)
{
    struct client *c;

    c = kmalloc(sizeof(*c), GFP_KERNEL);
    if (!c)
        return;

    client_init(c, id);

    spin_lock(&client_lock);
    list_add_rcu(&c->list, &client_list);
    spin_unlock(&client_lock);

    pr_info("Added client %d\n", id);
}

static void remove_client(int id)
{
    struct client *c, *tmp;

    spin_lock(&client_lock);

    list_for_each_entry_safe(c, tmp, &client_list, list) {
        if (c->id == id) {
            list_del_rcu(&c->list);
            spin_unlock(&client_lock);

            synchronize_rcu();
            kfree(c);

            pr_info("Removed client %d\n", id);
            return;
        }
    }

    spin_unlock(&client_lock);
}

/* ---------- Cleanup ---------- */

static void cleanup_all_clients(void)
{
    struct client *c, *tmp;
    LIST_HEAD(tmp_list);

    spin_lock(&client_lock);
    list_for_each_entry_safe(c, tmp, &client_list, list) {
        list_move_tail(&c->list, &tmp_list);
    }
    spin_unlock(&client_lock);

    synchronize_rcu();

    list_for_each_entry_safe(c, tmp, &tmp_list, list) {
        list_del(&c->list);
        kfree(c);
    }
}

/* ---------- Module Init ---------- */

static int __init client_module_init(void)
{
    struct client *c;
    u64 p, b;

    pr_info("client_mod: init\n");

    spin_lock_init(&client_lock);

    /* Add clients */
    add_client(1);
    add_client(2);

    /* Update stats */
    rcu_read_lock();
    list_for_each_entry_rcu(c, &client_list, list) {

        if (c->id == 1)
            update_client_stats(c, 10, 100);

        else if (c->id == 2)
            update_client_stats(c, 5, 50);
    }
    rcu_read_unlock();

    /* Print stats */
    if (find_client_and_read_stats(1, &p, &b))
        pr_info("Client 1 stats: packets=%llu bytes=%llu\n", p, b);

    if (find_client_and_read_stats(2, &p, &b))
        pr_info("Client 2 stats: packets=%llu bytes=%llu\n", p, b);

    return 0;
}

/* ---------- Module Exit ---------- */

static void __exit client_module_exit(void)
{
    pr_info("client_mod: exit\n");
    cleanup_all_clients();
    pr_info("client_mod: cleanup complete\n");
}

module_init(client_module_init);
module_exit(client_module_exit);

