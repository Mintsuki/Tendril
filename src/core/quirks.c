#include "../internal.h"

#define ANY 0xffff

struct hc_quirk {
    uint16_t vendor;
    uint16_t device;
    enum tendril_hc_type type;
    uint32_t quirks;
};

/* From FreeBSD's sys/dev/usb/controller/xhci_pci.c (BSD-2-Clause). */
static const struct hc_quirk hc_quirks[] = {
    { 0x1033, 0x0194, TENDRIL_HC_XHCI, TENDRIL_HC_QUIRK_NO_64BIT },
    { 0x1912, 0x0014, TENDRIL_HC_XHCI, TENDRIL_HC_QUIRK_NO_64BIT },
    { 0x1b73, 0x1000, TENDRIL_HC_XHCI, TENDRIL_HC_QUIRK_NO_MSI },
    { 0x8086, 0x0f35, TENDRIL_HC_XHCI, TENDRIL_HC_QUIRK_INTEL_PORT_ROUTING | TENDRIL_HC_QUIRK_XHCI_CTRL_STEP },
    { 0x8086, 0x9c31, TENDRIL_HC_XHCI, TENDRIL_HC_QUIRK_INTEL_PORT_ROUTING | TENDRIL_HC_QUIRK_XHCI_CTRL_STEP },
    { 0x8086, 0x1e31, TENDRIL_HC_XHCI, TENDRIL_HC_QUIRK_INTEL_PORT_ROUTING | TENDRIL_HC_QUIRK_XHCI_CTRL_STEP },
    { 0x8086, 0x8c31, TENDRIL_HC_XHCI, TENDRIL_HC_QUIRK_INTEL_PORT_ROUTING | TENDRIL_HC_QUIRK_XHCI_CTRL_STEP },
    { 0x8086, 0x8cb1, TENDRIL_HC_XHCI, TENDRIL_HC_QUIRK_INTEL_PORT_ROUTING | TENDRIL_HC_QUIRK_XHCI_CTRL_STEP },
    { 0x8086, 0x9cb1, TENDRIL_HC_XHCI, TENDRIL_HC_QUIRK_INTEL_PORT_ROUTING | TENDRIL_HC_QUIRK_XHCI_CTRL_STEP }
};

/*
 * From FreeBSD's sys/dev/usb/quirk/usb_quirk.c (BSD-2-Clause), naming the IDs as
 * its sys/dev/usb/usbdevs (BSD-2-Clause) does.
 */
static const struct tendril_id_quirk device_quirks[] = {
    { 0x1608, 0x0001, 0x0094, 0x0094, TENDRIL_DEV_QUIRK_SWAP_UNICODE }, /* INSIDEOUT EDGEPORT4 */
    { 0x046d, 0xc032, 0x0000, 0xffff, TENDRIL_DEV_QUIRK_NO_STRINGS }, /* LOGITECH UN53B */
    { 0x0bda, 0x8196, 0x0000, 0xffff, TENDRIL_DEV_QUIRK_CONFIG_INDEX_1 }, /* REALTEK RTL8196EU */
    { 0x05cc, 0x2265, 0x0000, 0xffff, TENDRIL_DEV_QUIRK_CONFIG_INDEX_1 }, /* ELSA MODEM1 */
    { 0x2019, 0xab2f, 0x0000, 0xffff, TENDRIL_DEV_QUIRK_CONFIG_INDEX_1 }, /* PLANEX2 MZKUE150N */
    { 0x13b1, 0x0041, 0x0000, 0xffff, TENDRIL_DEV_QUIRK_CONFIG_INDEX_1 }, /* CISCOLINKSYS USB3GIGV1 */
    { 0x19d2, 0x1420, 0x0000, 0xffff, TENDRIL_DEV_QUIRK_CONFIG_INDEX_0 }, /* QUALCOMMINC ZTE_MF730M */
    { 0x0a4a, 0xc150, 0x0000, 0xffff, TENDRIL_DEV_QUIRK_CONFIG_INDEX_1 }, /* PLOYTEC SPL_CRIMSON_1 */
    { 0x17ef, 0x3069, 0x0000, 0xffff, TENDRIL_DEV_QUIRK_CONFIG_INDEX_1 }, /* LENOVO TBT3LAN */
    { 0x0bda, 0x8153, 0x31fd, 0x31fd, TENDRIL_DEV_QUIRK_CONFIG_INDEX_1 }, /* REALTEK RTL8153 */
};

uint32_t tendril_pci_hc_quirks(enum tendril_hc_type type, uint16_t vendor, uint16_t device) {
    uint32_t quirks = 0;
    for (size_t i = 0; i < ARRAY_SIZE(hc_quirks); i++) {
        const struct hc_quirk *q = &hc_quirks[i];
        if (q->vendor == 0 || q->type != type || q->vendor != vendor) {
            continue;
        }
        if (q->device == ANY || q->device == device) {
            quirks |= q->quirks;
        }
    }
    return quirks;
}

uint32_t tendril_hc_quirk_lookup(struct tendril_hc *hc) {
    if (!hc->config.pci) {
        return 0;
    }
    return tendril_pci_hc_quirks(hc->config.type, hc->pci_vendor, hc->pci_device);
}

uint32_t tendril_id_quirks(const struct tendril_id_quirk *table, size_t count,
                          const struct tendril_device_descriptor *desc) {
    uint16_t vendor = tendril_le16(desc->vendor_id);
    uint16_t product = tendril_le16(desc->product_id);
    uint16_t bcd = tendril_le16(desc->bcd_device);
    uint32_t quirks = 0;
    for (size_t i = 0; i < count; i++) {
        const struct tendril_id_quirk *q = &table[i];
        if (q->vendor == vendor && (q->product == ANY || q->product == product) && bcd >= q->bcd_min &&
            bcd <= q->bcd_max) {
            quirks |= q->quirks;
        }
    }
    return quirks;
}

uint32_t tendril_device_quirk_lookup(const struct tendril_device_descriptor *desc) {
    return tendril_id_quirks(device_quirks, ARRAY_SIZE(device_quirks), desc);
}
