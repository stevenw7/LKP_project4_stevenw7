#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/hashtable.h>
#include <linux/kprobes.h>
#include <linux/sched.h>
#include <linux/limits.h>
#include <asm/io.h>
#include "address_map_arm.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Steven Williams");
MODULE_DESCRIPTION("LKP Project 4");

void * LW_virtual;         // Lightweight bridge base address
volatile int *LEDR_ptr;    // virtual address for the LEDR port
volatile int *KEY_ptr;     // virtual address for the KEY port
volatile int *SW_ptr;     // virtual address for the SW port

static int hps2fpga_printk(void)
{
	printk(KERN_CONT "\nhps2fpga | module leaded\n");
	return 0;
}

static void run_tests(void)
{
	hps2fpga_printk();
}

static void cleanup(void)
{
	printk(KERN_INFO "hps2fpga | Cleaning up...\n");
}

static int hps2fpga_show(struct seq_file *m, void *v);
static int hps2fpga_open(struct inode *inode, struct  file *file);

static int hps2fpga_show(struct seq_file *m, void *v)
{
	//seq_printf(m, "Hello World");
	seq_printf(m, "Switch values: %d\n", *(SW_ptr)); 
	seq_printf(m, "\n");
	return 0;
}

static int hps2fpga_open(struct inode *inode, struct  file *file) {
  return single_open(file, hps2fpga_show, NULL);
}

static const struct file_operations hps2fpga_fops = {
  .owner = THIS_MODULE,
  .open = hps2fpga_open,
  .read = seq_read,
  .llseek = seq_lseek,
  .release = single_release,
};

static int __init hps2fpga_init(void)
{	
	int value = 0;
	// generate a virtual address for the FPGA lightweight bridge
	LW_virtual = ioremap_nocache (LW_BRIDGE_BASE, LW_BRIDGE_SPAN);

	LEDR_ptr = LW_virtual + LEDR_BASE;  // init virtual address for LEDR port
	*LEDR_ptr = 0x200;                  // turn on the leftmost light

	KEY_ptr = LW_virtual + KEY_BASE;    // init virtual address for KEY port
	// Clear the PIO edgecapture register (clear any pending interrupt)
	*(KEY_ptr + 3) = 0xF; 
	// Enable IRQ generation for the 4 buttons
	*(KEY_ptr + 2) = 0xF; 

	SW_ptr = LW_virtual + SW_BASE;    // init virtual address for KEY port

	// Register the interrupt handler.
	/*
	value = request_irq (KEYS_IRQ, 
			     (irq_handler_t) irq_handler, 
			     IRQF_SHARED, 
			     "pushbutton_irq_handler", 
			     (void *) (irq_handler)
	);
	*/

	printk(KERN_INFO "\nhps2fpga | inserted\n");
	proc_create("hps2fpga", 0, NULL, &hps2fpga_fops);
	return value;
}

static void __exit hps2fpga_exit(void)
{	
	remove_proc_entry("hps2fpga", NULL);
	cleanup();
	return;
}

module_init(hps2fpga_init);

module_exit(hps2fpga_exit);
