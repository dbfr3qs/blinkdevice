#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
//#include <linux/sched.h>
//#include <linux/timer.h>
#include <linux/gpio.h>
#include <linux/string.h>

MODULE_LICENSE("Dual BSD/GPL");

#define LED1 4

// fake device 
struct fake_device {
	char data[100];
	struct semaphore sem;
} virtual_device;

struct cdev *mcdev;
int major_number;
int ret;

dev_t dev_num;

#define DEVICE_NAME "blinkdevice"

int device_open(struct inode *inode, struct file *filp) {
	if(down_interruptible(&virtual_device.sem) != 0) {
		printk(KERN_ALERT "chris' device: could not lock device during open\n");
		return -1;
	}

	printk(KERN_INFO "chris's device: opened device\n");
	return 0;
}


ssize_t device_read(struct file *filp, char* bufStoreData, size_t bufCount, loff_t* curOffset) {
	printk(KERN_INFO "chris's device: reading from device\n");
	ret = copy_to_user(bufStoreData, virtual_device.data, bufCount);
	return ret;
}

ssize_t device_write(struct file* filp, char*bufStoreData, size_t bufCount, loff_t* curOffset) {
	printk(KERN_INFO "chris's device: writing to device\n");
	ret = copy_from_user(virtual_device.data, bufStoreData, bufCount);
	//printk(KERN_INFO "bufStoreData: %s bufCount %d\n", bufStoreData, bufCount);
	if(strcmp(bufStoreData,"HIGH")==0) {
		gpio_set_value(LED1, 1L);
		printk(KERN_INFO "Go high\n");
	}
	if(strcmp(bufStoreData,"LOW")==0) {
		gpio_set_value(LED1, 0);
		printk(KERN_INFO "Go low\n");
	}
	//init_timer(&blink_timer);
	//blink_timer.function = blink_timer_func;
	//blink_timer.data = 1L;
	//blink_timer.expires = jiffies + (1*HZ);
	//add_timer(&blink_timer);

	return ret;
}

int device_close(struct inode *inode, struct file *filp) {
	up(&virtual_device.sem);
	printk(KERN_INFO "chris's device: device closed\n");
	return 0;
}
struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_close,
	.write = device_write,
	.read = device_read
};

static int driver_entry(void) {
	// put major number into dev_num
	ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if(ret<0) {
		printk(KERN_ALERT "chris's device: failed to allocate a major number\n");
		return ret;
	}
	major_number = MAJOR(dev_num); // extract major number from dev_num
	printk(KERN_INFO "chris's device: major number is %d\n", major_number);
	printk(KERN_INFO "\tuse \"mknod /dev/%s c %d 0\" for device file\n", DEVICE_NAME, major_number);

	mcdev = cdev_alloc();
	mcdev->ops = &fops;
	mcdev->owner = THIS_MODULE;

	ret = cdev_add(mcdev, dev_num, 1);
	if(ret < 0) {
		printk(KERN_ALERT "chris's device: unable to load cdev to kernel\n");
		return ret;
	}

	sema_init(&virtual_device.sem, 1);
	
	ret = gpio_request_one(LED1, GPIOF_OUT_INIT_LOW, "led1");
	if (ret) {
		printk(KERN_ALERT "Unable to request GPIOs: %d\n", ret);
		return ret;
	}
	return 0;
}

static void driver_exit(void) {
	cdev_del(mcdev);
	//del_timer_sync(&blink_timer);
	gpio_set_value(LED1, 0);
	gpio_free(LED1);
	unregister_chrdev_region(dev_num, 1);
	printk(KERN_ALERT "chris's device: unloaded module\n");
	
}

module_init(driver_entry);
module_exit(driver_exit);
