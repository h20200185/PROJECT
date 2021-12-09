#include <linux/kernel.h>      
#include <linux/module.h>      
#include <linux/fs.h>
#include <asm/uaccess.h>       
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/gpio.h>

#define DEVICE_NAME "mcp9808"
#define MAJOR_NO 115

//Register definition
#define REG_TEMP 0x05

static dev_t dev_number;
struct class *cl;
static unsigned int irqNumber;
static struct i2c_client * my_client;
struct i2c_adapter * my_adap;
static dev_t first;
static struct cdev c_dev;
static u8 temp;


static u8 mcp_read(struct i2c_client *client, u8 reg)
{
	int ret;
	
	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0)
		dev_err(&client->dev, "Can not read register, returned %d\n", ret);

	return (u8)ret;
}

static int mcp_write(struct i2c_client *client, u8 reg, u8 data)
{
	int ret;

	ret = i2c_smbus_write_byte_data(client, reg, data);
	if (ret < 0)
		dev_err(&client->dev,"can not write register, returned %d\n", ret);

	return ret;
}

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off) 
{	
	u8 readvalue;

	printk(KERN_INFO "Driver read()\n");
	readvalue = mcp_read(my_client, REG_TEMP);
	printk(KERN_INFO "Temperature = %d",readvalue>>3);

	return 0;
}


static int my_open(struct inode *i, struct file *f)
{
	printk(KERN_INFO "Driver: open()\n");
	return 0;
}
static int my_close(struct inode *i, struct file *f)
{
	printk(KERN_INFO "Driver: close()\n");
	return 0;
}

static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
	printk(KERN_INFO "Driver: write()\n");
	return len;
}

static struct file_operations my_dev_ops = {.read = my_read,
						.owner = THIS_MODULE,
						.open = my_open,
						.release = my_close,
						.write = my_write};



static int __init mcp_init(void)
{
	
	u8 readvalue;
	printk(KERN_INFO "Welcome to MCP9808\n");
       
    if (alloc_chrdev_region(&first, 0, 1, DEVICE_NAME) < 0) {
    printk(KERN_DEBUG "Can't register device\n");
    return -1;
}

	if (IS_ERR(cl = class_create(THIS_MODULE, "chardrv"))){ unregister_chrdev_region(first, 1);
		}

	if (IS_ERR(device_create(cl, NULL, first, NULL, "MCP9808"))) {
		class_destroy(cl);
		unregister_chrdev_region(first,1);
	}
	
	cdev_init(&c_dev,&my_dev_ops);
	if (cdev_add(&c_dev, first,1) < 0)
	{
	device_destroy(cl, first);
	class_destroy(cl);
	unregister_chrdev_region(first,1);
	return -1;
	}

      
	my_adap = i2c_get_adapter(1); // 1 means i2c-1 bus
	if (!(my_client = i2c_new_dummy_device(my_adap, 0x18))){
		
		printk(KERN_INFO "Couldn't acquire i2c slave");
		unregister_chrdev(MAJOR_NO, DEVICE_NAME);
		device_destroy(cl, first);
		class_destroy(cl);
		return -1;
	}
	printk(KERN_INFO "acquire i2c slave");
		
	return 0;
}
 

static void __exit mcp_exit(void)
{
	printk(KERN_INFO "MCP: Removing module");
 	
	i2c_unregister_device(my_client);
    cdev_del(&c_dev);
 	device_destroy(cl, first);
	class_destroy(cl);
	unregister_chrdev_region(first, 1); 
}


module_init(mcp_init);
module_exit(mcp_exit);

MODULE_DESCRIPTION("driver");
MODULE_AUTHOR("GROUP PROJECT");
MODULE_LICENSE("GPL");
