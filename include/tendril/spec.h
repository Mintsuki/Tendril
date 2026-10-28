#ifndef TENDRIL_SPEC_H
#define TENDRIL_SPEC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Definitions from the USB 2.0 and USB 3.2 (revision 1.1) specifications. Multi-byte fields
 * of the structures are little-endian, as on the wire; use tendril_le16() and
 * tendril_le32() to read them.
 */

#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define TENDRIL_BIG_ENDIAN 1
#else
#define TENDRIL_BIG_ENDIAN 0
#endif

static inline uint16_t tendril_le16(uint16_t v) {
#if TENDRIL_BIG_ENDIAN
    return __builtin_bswap16(v);
#else
    return v;
#endif
}

static inline uint32_t tendril_le32(uint32_t v) {
#if TENDRIL_BIG_ENDIAN
    return __builtin_bswap32(v);
#else
    return v;
#endif
}

static inline uint64_t tendril_le64(uint64_t v) {
#if TENDRIL_BIG_ENDIAN
    return __builtin_bswap64(v);
#else
    return v;
#endif
}

/* bmRequestType */
#define TENDRIL_DIR_OUT 0x00
#define TENDRIL_DIR_IN 0x80
#define TENDRIL_TYPE_STANDARD 0x00
#define TENDRIL_TYPE_CLASS 0x20
#define TENDRIL_TYPE_VENDOR 0x40
#define TENDRIL_TYPE_MASK 0x60
#define TENDRIL_RECIP_DEVICE 0x00
#define TENDRIL_RECIP_INTERFACE 0x01
#define TENDRIL_RECIP_ENDPOINT 0x02
#define TENDRIL_RECIP_OTHER 0x03
#define TENDRIL_RECIP_MASK 0x1f

/* Standard requests */
#define TENDRIL_REQ_GET_STATUS 0
#define TENDRIL_REQ_CLEAR_FEATURE 1
#define TENDRIL_REQ_SET_FEATURE 3
#define TENDRIL_REQ_SET_ADDRESS 5
#define TENDRIL_REQ_GET_DESCRIPTOR 6
#define TENDRIL_REQ_SET_DESCRIPTOR 7
#define TENDRIL_REQ_GET_CONFIGURATION 8
#define TENDRIL_REQ_SET_CONFIGURATION 9
#define TENDRIL_REQ_GET_INTERFACE 10
#define TENDRIL_REQ_SET_INTERFACE 11
#define TENDRIL_REQ_SYNCH_FRAME 12
#define TENDRIL_REQ_SET_SEL 48
#define TENDRIL_REQ_SET_ISOCH_DELAY 49

/* Descriptor types */
#define TENDRIL_DT_DEVICE 1
#define TENDRIL_DT_CONFIGURATION 2
#define TENDRIL_DT_STRING 3
#define TENDRIL_DT_INTERFACE 4
#define TENDRIL_DT_ENDPOINT 5
#define TENDRIL_DT_DEVICE_QUALIFIER 6
#define TENDRIL_DT_OTHER_SPEED_CONFIGURATION 7
#define TENDRIL_DT_INTERFACE_POWER 8
#define TENDRIL_DT_OTG 9
#define TENDRIL_DT_DEBUG 10
#define TENDRIL_DT_INTERFACE_ASSOCIATION 11
#define TENDRIL_DT_BOS 15
#define TENDRIL_DT_DEVICE_CAPABILITY 16
#define TENDRIL_DT_CS_INTERFACE 0x24
#define TENDRIL_DT_CS_ENDPOINT 0x25
#define TENDRIL_DT_HUB 0x29
#define TENDRIL_DT_SS_HUB 0x2a
#define TENDRIL_DT_SS_ENDPOINT_COMPANION 48
#define TENDRIL_DT_SSP_ISO_ENDPOINT_COMPANION 49

/* Standard feature selectors */
#define TENDRIL_FEATURE_ENDPOINT_HALT 0
#define TENDRIL_FEATURE_FUNCTION_SUSPEND 0
#define TENDRIL_FEATURE_DEVICE_REMOTE_WAKEUP 1
#define TENDRIL_FEATURE_TEST_MODE 2
#define TENDRIL_FEATURE_U1_ENABLE 48
#define TENDRIL_FEATURE_U2_ENABLE 49
#define TENDRIL_FEATURE_LTM_ENABLE 50

/* SET_FEATURE(FUNCTION_SUSPEND) options, wIndex bits 15:8 (USB 3.2 revision 1.1, Table 9-9) */
#define TENDRIL_FUNCTION_SUSPEND_LOW_POWER 0x01
#define TENDRIL_FUNCTION_SUSPEND_REMOTE_WAKE 0x02

/* Configuration descriptor bmAttributes */
#define TENDRIL_CONFIG_ATTR_SELF_POWERED 0x40
#define TENDRIL_CONFIG_ATTR_REMOTE_WAKEUP 0x20

/* Class codes */
#define TENDRIL_CLASS_PER_INTERFACE 0x00
#define TENDRIL_CLASS_AUDIO 0x01
#define TENDRIL_CLASS_CDC 0x02
#define TENDRIL_CLASS_HID 0x03
#define TENDRIL_CLASS_PHYSICAL 0x05
#define TENDRIL_CLASS_IMAGE 0x06
#define TENDRIL_CLASS_PRINTER 0x07
#define TENDRIL_CLASS_MASS_STORAGE 0x08
#define TENDRIL_CLASS_HUB 0x09
#define TENDRIL_CLASS_CDC_DATA 0x0a
#define TENDRIL_CLASS_SMART_CARD 0x0b
#define TENDRIL_CLASS_VIDEO 0x0e
#define TENDRIL_CLASS_MISC 0xef
#define TENDRIL_CLASS_APP_SPEC 0xfe
#define TENDRIL_CLASS_VENDOR 0xff

/* Endpoint descriptor fields */
#define TENDRIL_ENDPOINT_NUMBER_MASK 0x0f
#define TENDRIL_ENDPOINT_DIR_MASK 0x80
#define TENDRIL_ENDPOINT_XFER_MASK 0x03
#define TENDRIL_ENDPOINT_XFER_CONTROL 0
#define TENDRIL_ENDPOINT_XFER_ISOC 1
#define TENDRIL_ENDPOINT_XFER_BULK 2
#define TENDRIL_ENDPOINT_XFER_INT 3
#define TENDRIL_ENDPOINT_SYNC_MASK 0x0c
#define TENDRIL_ENDPOINT_USAGE_MASK 0x30
#define TENDRIL_ENDPOINT_USAGE_FEEDBACK 0x10
#define TENDRIL_ENDPOINT_MAXP_MASK 0x07ff
#define TENDRIL_ENDPOINT_MAXP_MULT_SHIFT 11
#define TENDRIL_ENDPOINT_MAXP_MULT_MASK 0x1800

/* Configuration descriptor attributes */
#define TENDRIL_CONFIG_ATT_ONE 0x80
#define TENDRIL_CONFIG_ATT_SELFPOWER 0x40
#define TENDRIL_CONFIG_ATT_WAKEUP 0x20

/* Device capability types */
#define TENDRIL_CAP_USB20_EXTENSION 0x02
#define TENDRIL_CAP_SUPERSPEED 0x03
#define TENDRIL_CAP_CONTAINER_ID 0x04
#define TENDRIL_CAP_PLATFORM 0x05
#define TENDRIL_CAP_SUPERSPEED_PLUS 0x0a

/* Hub class requests, USB 2.0 11.24.2 and USB 3.2 revision 1.1 10.16.2 */
#define TENDRIL_HUB_REQ_GET_STATUS 0
#define TENDRIL_HUB_REQ_CLEAR_FEATURE 1
#define TENDRIL_HUB_REQ_SET_FEATURE 3
#define TENDRIL_HUB_REQ_GET_DESCRIPTOR 6
#define TENDRIL_HUB_REQ_CLEAR_TT_BUFFER 8
#define TENDRIL_HUB_REQ_RESET_TT 9
#define TENDRIL_HUB_REQ_GET_TT_STATE 10
#define TENDRIL_HUB_REQ_STOP_TT 11
#define TENDRIL_HUB_REQ_SET_HUB_DEPTH 12
#define TENDRIL_HUB_REQ_GET_PORT_ERR_COUNT 13

/* Hub class feature selectors */
#define TENDRIL_HUB_C_LOCAL_POWER 0
#define TENDRIL_HUB_C_OVER_CURRENT 1
#define TENDRIL_PORT_FEAT_CONNECTION 0
#define TENDRIL_PORT_FEAT_ENABLE 1
#define TENDRIL_PORT_FEAT_SUSPEND 2
#define TENDRIL_PORT_FEAT_OVER_CURRENT 3
#define TENDRIL_PORT_FEAT_RESET 4
#define TENDRIL_PORT_FEAT_LINK_STATE 5
#define TENDRIL_PORT_FEAT_POWER 8
#define TENDRIL_PORT_FEAT_LOW_SPEED 9
#define TENDRIL_PORT_FEAT_C_CONNECTION 16
#define TENDRIL_PORT_FEAT_C_ENABLE 17
#define TENDRIL_PORT_FEAT_C_SUSPEND 18
#define TENDRIL_PORT_FEAT_C_OVER_CURRENT 19
#define TENDRIL_PORT_FEAT_C_RESET 20
#define TENDRIL_PORT_FEAT_TEST 21
#define TENDRIL_PORT_FEAT_INDICATOR 22
#define TENDRIL_PORT_FEAT_U1_TIMEOUT 23
#define TENDRIL_PORT_FEAT_U2_TIMEOUT 24
#define TENDRIL_PORT_FEAT_C_LINK_STATE 25
#define TENDRIL_PORT_FEAT_C_CONFIG_ERROR 26
#define TENDRIL_PORT_FEAT_REMOTE_WAKE_MASK 27
#define TENDRIL_PORT_FEAT_BH_RESET 28
#define TENDRIL_PORT_FEAT_C_BH_RESET 29
#define TENDRIL_PORT_FEAT_FORCE_LINKPM_ACCEPT 30

/* wPortStatus */
#define TENDRIL_PORT_STAT_CONNECTION 0x0001
#define TENDRIL_PORT_STAT_ENABLE 0x0002
#define TENDRIL_PORT_STAT_SUSPEND 0x0004
#define TENDRIL_PORT_STAT_OVER_CURRENT 0x0008
#define TENDRIL_PORT_STAT_RESET 0x0010
#define TENDRIL_PORT_STAT_POWER 0x0100
#define TENDRIL_PORT_STAT_LOW_SPEED 0x0200
#define TENDRIL_PORT_STAT_HIGH_SPEED 0x0400
#define TENDRIL_PORT_STAT_TEST 0x0800
#define TENDRIL_PORT_STAT_INDICATOR 0x1000
#define TENDRIL_SS_PORT_STAT_LINK_STATE_MASK 0x01e0
#define TENDRIL_SS_PORT_STAT_LINK_STATE_SHIFT 5
#define TENDRIL_SS_PORT_STAT_POWER 0x0200
#define TENDRIL_SS_PORT_STAT_SPEED_MASK 0x1c00

/* SuperSpeed link states */
#define TENDRIL_SS_LINK_U0 0x0
#define TENDRIL_SS_LINK_U1 0x1
#define TENDRIL_SS_LINK_U2 0x2
#define TENDRIL_SS_LINK_U3 0x3
#define TENDRIL_SS_LINK_DISABLED 0x4
#define TENDRIL_SS_LINK_RX_DETECT 0x5
#define TENDRIL_SS_LINK_INACTIVE 0x6
#define TENDRIL_SS_LINK_POLLING 0x7
#define TENDRIL_SS_LINK_RECOVERY 0x8
#define TENDRIL_SS_LINK_HOT_RESET 0x9
#define TENDRIL_SS_LINK_COMPLIANCE 0xa
#define TENDRIL_SS_LINK_LOOPBACK 0xb

/* wPortChange */
#define TENDRIL_PORT_STAT_C_CONNECTION 0x0001
#define TENDRIL_PORT_STAT_C_ENABLE 0x0002
#define TENDRIL_PORT_STAT_C_SUSPEND 0x0004
#define TENDRIL_PORT_STAT_C_OVER_CURRENT 0x0008
#define TENDRIL_PORT_STAT_C_RESET 0x0010
#define TENDRIL_PORT_STAT_C_BH_RESET 0x0020
#define TENDRIL_PORT_STAT_C_LINK_STATE 0x0040
#define TENDRIL_PORT_STAT_C_CONFIG_ERROR 0x0080

/* wHubCharacteristics */
#define TENDRIL_HUB_CHAR_LPSM_MASK 0x0003
#define TENDRIL_HUB_CHAR_COMPOUND 0x0004
#define TENDRIL_HUB_CHAR_OCPM_MASK 0x0018
#define TENDRIL_HUB_CHAR_TTTT_MASK 0x0060
#define TENDRIL_HUB_CHAR_TTTT_SHIFT 5
#define TENDRIL_HUB_CHAR_PORTIND 0x0080

struct tendril_setup {
    uint8_t request_type;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} __attribute__((packed));

struct tendril_descriptor_header {
    uint8_t length;
    uint8_t type;
} __attribute__((packed));

struct tendril_device_descriptor {
    uint8_t length;
    uint8_t type;
    uint16_t bcd_usb;
    uint8_t device_class;
    uint8_t device_subclass;
    uint8_t device_protocol;
    uint8_t max_packet_size0;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t bcd_device;
    uint8_t manufacturer_string;
    uint8_t product_string;
    uint8_t serial_string;
    uint8_t num_configurations;
} __attribute__((packed));

struct tendril_config_descriptor {
    uint8_t length;
    uint8_t type;
    uint16_t total_length;
    uint8_t num_interfaces;
    uint8_t configuration_value;
    uint8_t configuration_string;
    uint8_t attributes;
    uint8_t max_power;
} __attribute__((packed));

struct tendril_interface_assoc_descriptor {
    uint8_t length;
    uint8_t type;
    uint8_t first_interface;
    uint8_t interface_count;
    uint8_t function_class;
    uint8_t function_subclass;
    uint8_t function_protocol;
    uint8_t function_string;
} __attribute__((packed));

struct tendril_interface_descriptor {
    uint8_t length;
    uint8_t type;
    uint8_t interface_number;
    uint8_t alternate_setting;
    uint8_t num_endpoints;
    uint8_t interface_class;
    uint8_t interface_subclass;
    uint8_t interface_protocol;
    uint8_t interface_string;
} __attribute__((packed));

struct tendril_endpoint_descriptor {
    uint8_t length;
    uint8_t type;
    uint8_t endpoint_address;
    uint8_t attributes;
    uint16_t max_packet_size;
    uint8_t interval;
} __attribute__((packed));

struct tendril_ss_ep_companion_descriptor {
    uint8_t length;
    uint8_t type;
    uint8_t max_burst;
    uint8_t attributes;
    uint16_t bytes_per_interval;
} __attribute__((packed));

struct tendril_ssp_iso_ep_companion_descriptor {
    uint8_t length;
    uint8_t type;
    uint16_t reserved;
    uint32_t bytes_per_interval;
} __attribute__((packed));

struct tendril_string_descriptor {
    uint8_t length;
    uint8_t type;
    uint16_t data[];
} __attribute__((packed));

struct tendril_bos_descriptor {
    uint8_t length;
    uint8_t type;
    uint16_t total_length;
    uint8_t num_device_caps;
} __attribute__((packed));

struct tendril_device_capability_header {
    uint8_t length;
    uint8_t type;
    uint8_t capability_type;
} __attribute__((packed));

struct tendril_usb20_ext_capability {
    uint8_t length;
    uint8_t type;
    uint8_t capability_type;
    uint32_t attributes;
} __attribute__((packed));

struct tendril_ss_capability {
    uint8_t length;
    uint8_t type;
    uint8_t capability_type;
    uint8_t attributes;
    uint16_t speeds_supported;
    uint8_t functionality_support;
    uint8_t u1_dev_exit_lat;
    uint16_t u2_dev_exit_lat;
} __attribute__((packed));

struct tendril_hub_descriptor {
    uint8_t length;
    uint8_t type;
    uint8_t num_ports;
    uint16_t characteristics;
    uint8_t power_on_to_good;
    uint8_t controller_current;
    uint8_t variable[64];
} __attribute__((packed));

struct tendril_ss_hub_descriptor {
    uint8_t length;
    uint8_t type;
    uint8_t num_ports;
    uint16_t characteristics;
    uint8_t power_on_to_good;
    uint8_t controller_current;
    uint8_t header_decode_latency;
    uint16_t hub_delay;
    uint16_t device_removable;
} __attribute__((packed));

#ifdef __cplusplus
}
#endif

#endif
