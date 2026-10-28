#ifndef TENDRIL_HID_H
#define TENDRIL_HID_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <tendril/tendril.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HID 1.11 class requests and descriptors. */
#define TENDRIL_HID_REQ_GET_REPORT 0x01
#define TENDRIL_HID_REQ_GET_IDLE 0x02
#define TENDRIL_HID_REQ_GET_PROTOCOL 0x03
#define TENDRIL_HID_REQ_SET_REPORT 0x09
#define TENDRIL_HID_REQ_SET_IDLE 0x0a
#define TENDRIL_HID_REQ_SET_PROTOCOL 0x0b
#define TENDRIL_DT_HID 0x21
#define TENDRIL_DT_HID_REPORT 0x22

/* Report types */
#define TENDRIL_HID_INPUT 1
#define TENDRIL_HID_OUTPUT 2
#define TENDRIL_HID_FEATURE 3

/* Main item flags */
#define TENDRIL_HID_CONSTANT 0x001
#define TENDRIL_HID_VARIABLE 0x002
#define TENDRIL_HID_RELATIVE 0x004
#define TENDRIL_HID_WRAP 0x008
#define TENDRIL_HID_NONLINEAR 0x010
#define TENDRIL_HID_NO_PREFERRED 0x020
#define TENDRIL_HID_NULL_STATE 0x040
#define TENDRIL_HID_VOLATILE 0x080
#define TENDRIL_HID_BUFFERED_BYTES 0x100

/* Usages are the usage page in the high 16 bits and the usage ID in the low 16. */
#define TENDRIL_HID_USAGE(page, id) (((uint32_t)(page) << 16) | (uint32_t)(id))
#define TENDRIL_HID_PAGE_GENERIC_DESKTOP 0x01
#define TENDRIL_HID_PAGE_KEYBOARD 0x07
#define TENDRIL_HID_PAGE_LED 0x08
#define TENDRIL_HID_PAGE_BUTTON 0x09
#define TENDRIL_HID_PAGE_CONSUMER 0x0c
#define TENDRIL_HID_PAGE_DIGITIZER 0x0d

/* Keyboard LEDs, as in the boot keyboard output report. */
#define TENDRIL_HID_LED_NUM_LOCK 0x01
#define TENDRIL_HID_LED_CAPS_LOCK 0x02
#define TENDRIL_HID_LED_SCROLL_LOCK 0x04
#define TENDRIL_HID_LED_COMPOSE 0x08
#define TENDRIL_HID_LED_KANA 0x10

/* Modifiers, as in the boot keyboard input report. */
#define TENDRIL_HID_MOD_LCTRL 0x01
#define TENDRIL_HID_MOD_LSHIFT 0x02
#define TENDRIL_HID_MOD_LALT 0x04
#define TENDRIL_HID_MOD_LGUI 0x08
#define TENDRIL_HID_MOD_RCTRL 0x10
#define TENDRIL_HID_MOD_RSHIFT 0x20
#define TENDRIL_HID_MOD_RALT 0x40
#define TENDRIL_HID_MOD_RGUI 0x80

struct tendril_hid_field {
    uint8_t type;
    uint8_t report_id;
    uint16_t flags;
    /* Position in the report, after any report ID byte. */
    uint32_t bit_offset;
    uint16_t bit_size;
    uint16_t count;
    int32_t logical_min;
    int32_t logical_max;
    int32_t physical_min;
    int32_t physical_max;
    int32_t unit_exponent;
    uint32_t unit;
    /* Usage of the enclosing application collection. */
    uint32_t application;
    /*
     * A variable field's element i has usages[i] (the last usage repeats).
     * An array field's value v selects usage_min + v - logical_min, or
     * usages[v - logical_min] when usages were listed one by one.
     */
    uint32_t usage_min;
    uint32_t usage_max;
    uint32_t *usages;
    uint16_t num_usages;
};

struct tendril_hid_report_info {
    uint8_t type;
    uint8_t id;
    uint32_t bits;
};

struct tendril_hid_report_desc {
    struct tendril_hid_field *fields;
    unsigned num_fields;
    struct tendril_hid_report_info *reports;
    unsigned num_reports;
    /* Reports start with a report ID byte. */
    bool numbered;
};

enum tendril_status tendril_hid_parse(const uint8_t *desc, size_t size, struct tendril_hid_report_desc **out);
void tendril_hid_free(struct tendril_hid_report_desc *rd);
/* Size in bytes of a report, without its report ID byte. */
size_t tendril_hid_report_size(const struct tendril_hid_report_desc *rd, uint8_t type, uint8_t id);
uint32_t tendril_hid_extract(const uint8_t *data, size_t size, uint32_t bit_offset, unsigned bit_size);
void tendril_hid_insert(uint8_t *data, size_t size, uint32_t bit_offset, unsigned bit_size, uint32_t value);
/* Element `index` of a field, sign-extended when the logical range is signed. */
int32_t tendril_hid_field_get(const struct tendril_hid_field *f, const uint8_t *data, size_t size, unsigned index);
void tendril_hid_field_set(const struct tendril_hid_field *f, uint8_t *data, size_t size, unsigned index, int32_t value);
/* The usage of element `index` of a variable field. */
uint32_t tendril_hid_field_usage(const struct tendril_hid_field *f, unsigned index);
/* The usage an array field element value stands for, or 0. */
uint32_t tendril_hid_array_usage(const struct tendril_hid_field *f, int32_t value);
/* Finds the variable field element with `usage` in reports of `type`. */
const struct tendril_hid_field *tendril_hid_find(const struct tendril_hid_report_desc *rd, uint8_t type,
                                               uint32_t usage, unsigned *index);

/* Class driver */

struct tendril_hid;

/* What a HID interface looks like, from its application collections. */
#define TENDRIL_HID_KEYBOARD 0x01
#define TENDRIL_HID_MOUSE 0x02
#define TENDRIL_HID_ABSOLUTE 0x04
#define TENDRIL_HID_GAMEPAD 0x08
#define TENDRIL_HID_CONSUMER 0x10

struct tendril_hid_pointer_event {
    int32_t dx;
    int32_t dy;
    int32_t wheel;
    int32_t pan;
    /* Bit n is button n + 1. */
    uint32_t buttons;
    /* x and y are valid, in [x_min, x_max] and [y_min, y_max]. */
    bool absolute;
    int32_t x;
    int32_t y;
    int32_t x_min;
    int32_t x_max;
    int32_t y_min;
    int32_t y_max;
};

struct tendril_hid_callbacks {
    void (*attach)(struct tendril_hid *hid);
    /* The interface is gone; no callbacks follow. */
    void (*detach)(struct tendril_hid *hid);
    /* Every input report, without its report ID byte, which is passed as `id`. */
    void (*report)(struct tendril_hid *hid, uint8_t id, const uint8_t *data, size_t size);
    /* A key, as a keyboard usage page ID, went down or up. */
    void (*key)(struct tendril_hid *hid, uint16_t key, bool pressed, uint8_t modifiers);
    void (*pointer)(struct tendril_hid *hid, const struct tendril_hid_pointer_event *ev);
};

/* Use the boot protocol on keyboards and mice that support it. */
#define TENDRIL_HID_BOOT_PROTOCOL 0x1

/* Registers the HID class driver; call before the first controller is added. */
void tendril_hid_register(const struct tendril_hid_callbacks *callbacks, uint32_t flags);

struct tendril_interface *tendril_hid_interface(struct tendril_hid *hid);
uint32_t tendril_hid_kinds(struct tendril_hid *hid);
bool tendril_hid_boot_protocol(struct tendril_hid *hid);
const struct tendril_hid_report_desc *tendril_hid_descriptor(struct tendril_hid *hid);
void tendril_hid_set_user(struct tendril_hid *hid, void *user);
void *tendril_hid_user(struct tendril_hid *hid);

/* Sets the keyboard LEDs (TENDRIL_HID_LED_*), asynchronously. */
enum tendril_status tendril_hid_set_leds(struct tendril_hid *hid, uint8_t leds);

enum tendril_status tendril_hid_get_report(
    struct tendril_hid *hid, uint8_t type, uint8_t id, void *buf, size_t size,
    void (*done)(void *ctx, enum tendril_status status, size_t actual), void *ctx);
/* Output reports go to the interrupt OUT endpoint if there is one. */
enum tendril_status tendril_hid_set_report(
    struct tendril_hid *hid, uint8_t type, uint8_t id, const void *buf, size_t size,
    void (*done)(void *ctx, enum tendril_status status), void *ctx);

#ifdef __cplusplus
}
#endif

#endif
