#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/acpi.h>

#define ALS_ALAE_PATH "\\_SB.ALAE"
#define ALS_ALSC_PATH "\\_SB.PCI0.LPCB.EC0.ALSC"

static acpi_handle als_acpi_handle_from_string(const char *str)
{
    acpi_handle handle;
    acpi_status status;

    status = acpi_get_handle(NULL, str, &handle);

    return ACPI_FAILURE(status) ? NULL : handle;
}

static u32 als_get_ali(struct acpi_device *device)
{
    u64 ali;
    acpi_status status;

    status = acpi_evaluate_integer(device->handle, "_ALI", NULL, &ali);

    if (ACPI_FAILURE(status)) {
        acpi_evaluation_failure_warn(device->handle, "_ALI", status);
        return -EIO;
    }

    return ali;
}

static u32 als_get_enable(void)
{
    u64 alae;
    acpi_status status;

    status = acpi_evaluate_integer(NULL, ALS_ALAE_PATH, NULL, &alae);

    if (ACPI_FAILURE(status)) {
        acpi_evaluation_failure_warn(NULL, ALS_ALAE_PATH, status);
        return -EIO;
    }

    return alae;
}

static ssize_t als_show_ali(struct device *dev,
                            struct device_attribute *attr, char *buf)
{
    struct acpi_device *device = to_acpi_device(dev);

    device = to_acpi_device(dev);

    return sprintf(buf, "%d\n", als_get_ali(device));
}

static ssize_t als_show_enable(struct device *dev,
                               struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", als_get_enable());
}

static ssize_t als_store_enable(struct device *dev, struct device_attribute *attr,
                                const char *buf, size_t count)
{
    if (count >= 1) {
        acpi_handle alsc_handle;
        acpi_status result;
        struct acpi_object_list arg;
        union acpi_object param;

        if (buf[0] == '0') {
            // Disable
            param.type = ACPI_TYPE_INTEGER;
            param.integer.value = 0;

            arg.count = 1;
            arg.pointer = &param;

            if ((alsc_handle = als_acpi_handle_from_string(ALS_ALSC_PATH)) == NULL) {
                pr_err("als: unable to get handle for " ALS_ALSC_PATH "\n");
                return count;
            }

            result = acpi_evaluate_object(alsc_handle, NULL, &arg, NULL);

            pr_info("als: disabled\n");
        } else if (buf[0] == '1') {
            // Enable
            param.type = ACPI_TYPE_INTEGER;
            param.integer.value = 1;

            arg.count = 1;
            arg.pointer = &param;

            if ((alsc_handle = als_acpi_handle_from_string(ALS_ALSC_PATH)) == NULL) {
                pr_err("als: unable to get handle for " ALS_ALSC_PATH "\n");
                return count;
            }

            result = acpi_evaluate_object(alsc_handle, NULL, &arg, NULL);

            pr_info("als: enabled\n");
        }
    }

    return count;
}

static DEVICE_ATTR(ali, S_IRUGO, als_show_ali, NULL);
static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR, als_show_enable, als_store_enable);

static struct attribute *als_attributes[] = {
    &dev_attr_ali.attr,
    &dev_attr_enable.attr,
    NULL
};

static const struct attribute_group als_attr_group = {
    .attrs = als_attributes
};

static int als_add(struct acpi_device *device)
{
    int result;
    result = sysfs_create_group(&device->dev.kobj, &als_attr_group);

    pr_info("als: added\n");

    return result;
}

static void als_remove(struct acpi_device *device)
{
    sysfs_remove_group(&device->dev.kobj, &als_attr_group);

    pr_info("als: removed\n");
}

static void als_notify(struct acpi_device *device, u32 event)
{
    pr_info("als: notify %x %x\n", event, als_get_ali(device));
    kobject_uevent(&device->dev.kobj, KOBJ_CHANGE);
}

static const struct acpi_device_id als_device_ids[] = {
    {"ACPI0008", 0},
    {},
};

MODULE_DEVICE_TABLE(acpi, als_device_ids);

static struct acpi_driver als_driver = {
    .name = "als",
    .class = "ALS",
    .ids = als_device_ids,
    .ops = {
        .add = als_add,
        .remove = als_remove,
        .notify = als_notify
    },
    .owner = THIS_MODULE,
};

module_acpi_driver(als_driver);

MODULE_AUTHOR("Viktar Vauchkevich <victorenator@gmail.com>");
MODULE_AUTHOR("Vlad Pirlog <vlad@pirlog.com>");
MODULE_VERSION("1.0.0");
MODULE_DESCRIPTION("Ambient Light Sensor Driver");
MODULE_LICENSE("GPL");
