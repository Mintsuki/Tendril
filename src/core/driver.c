#include "../internal.h"

struct tendril_driver *tendril_drivers;

void tendril_driver_register(struct tendril_driver *driver) {
    struct tendril_driver **p = &tendril_drivers;
    while (*p != NULL) {
        if (*p == driver) {
            return;
        }
        p = &(*p)->next;
    }
    driver->next = NULL;
    *p = driver;
}

bool tendril_interface_claim(struct tendril_interface *intf, struct tendril_driver *driver) {
    struct tendril_device *dev = intf->dev;
    if (intf->driver != NULL) {
        return intf->driver == driver && intf->primary == (dev->probing != NULL ? dev->probing : intf);
    }
    intf->driver = driver;
    intf->primary = dev->probing != NULL ? dev->probing : intf;
    return true;
}

void tendril_bind_interfaces(struct tendril_device *dev) {
    for (unsigned i = 0; i < dev->num_interfaces && !dev->gone; i++) {
        struct tendril_interface *intf = &dev->interfaces[i];
        if (intf->driver != NULL || (dev->hub != NULL && i == 0)) {
            continue;
        }
        for (struct tendril_driver *drv = tendril_drivers; drv != NULL; drv = drv->next) {
            if (drv->probe == NULL) {
                continue;
            }
            intf->driver = drv;
            intf->primary = intf;
            dev->probing = intf;
            bool claimed = drv->probe(intf);
            dev->probing = NULL;
            if (claimed) {
                break;
            }
            /* A declined probe must not leave companion claims behind. */
            for (unsigned j = 0; j < dev->num_interfaces; j++) {
                if (dev->interfaces[j].primary == intf) {
                    dev->interfaces[j].driver = NULL;
                    dev->interfaces[j].primary = NULL;
                }
            }
        }
    }
}

void tendril_unbind_interfaces(struct tendril_device *dev) {
    for (unsigned i = 0; i < dev->num_interfaces; i++) {
        struct tendril_interface *intf = &dev->interfaces[i];
        struct tendril_driver *drv = intf->driver;
        if (drv == NULL || intf->primary != intf) {
            continue;
        }
        if (drv->disconnect != NULL) {
            drv->disconnect(intf);
        }
        for (unsigned j = 0; j < dev->num_interfaces; j++) {
            struct tendril_interface *other = &dev->interfaces[j];
            if (other->primary == intf) {
                other->driver = NULL;
                other->primary = NULL;
                other->driver_data = NULL;
            }
        }
    }
}

void tendril_notify_added(struct tendril_device *dev) {
    for (struct tendril_driver *drv = tendril_drivers; drv != NULL && !dev->gone; drv = drv->next) {
        if (drv->device_added != NULL) {
            drv->device_added(dev);
        }
    }
}

void tendril_notify_removed(struct tendril_device *dev) {
    for (struct tendril_driver *drv = tendril_drivers; drv != NULL; drv = drv->next) {
        if (drv->device_removed != NULL) {
            drv->device_removed(dev);
        }
    }
}

void tendril_notify_woke(struct tendril_device *dev) {
    for (struct tendril_driver *drv = tendril_drivers; drv != NULL && !dev->gone; drv = drv->next) {
        if (drv->device_woke != NULL) {
            drv->device_woke(dev);
        }
    }
}

static bool primary_bound(struct tendril_interface *intf) {
    return intf->driver != NULL && intf->primary == intf;
}

bool tendril_suspend_interfaces(struct tendril_device *dev, enum tendril_status *status) {
    for (unsigned i = 0; i < dev->num_interfaces; i++) {
        struct tendril_interface *intf = &dev->interfaces[i];
        if (primary_bound(intf) && intf->driver->suspend == NULL) {
            *status = TENDRIL_ERR_UNSUPPORTED;
            return false;
        }
    }
    for (unsigned i = 0; i < dev->num_interfaces; i++) {
        struct tendril_interface *intf = &dev->interfaces[i];
        if (!primary_bound(intf) || intf->driver->suspend(intf)) {
            continue;
        }
        for (unsigned j = 0; j < i; j++) {
            struct tendril_interface *done = &dev->interfaces[j];
            if (primary_bound(done) && done->driver->resume != NULL) {
                done->driver->resume(done);
            }
        }
        *status = TENDRIL_ERR_BUSY;
        return false;
    }
    *status = TENDRIL_OK;
    return true;
}

void tendril_resume_interfaces(struct tendril_device *dev) {
    for (unsigned i = 0; i < dev->num_interfaces && !dev->gone; i++) {
        struct tendril_interface *intf = &dev->interfaces[i];
        if (primary_bound(intf) && intf->driver->resume != NULL) {
            intf->driver->resume(intf);
        }
    }
}
