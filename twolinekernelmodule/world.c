#include <linux/module.h>   // Needed for all kernel modules

// Declare the license of the module
// - "GPL" means GNU General Public License
// - This avoids "tainted kernel" warnings when loading the module
MODULE_LICENSE("GPL");
