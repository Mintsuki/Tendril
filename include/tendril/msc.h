#ifndef TENDRIL_MSC_H
#define TENDRIL_MSC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <tendril/tendril.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Mass storage: Bulk-Only 1.0, CBI 1.1, and UAS (UASP 1.0) transports, carrying SCSI-family commands. */

struct tendril_msc_lun;

struct tendril_msc_callbacks {
    /* A logical unit was probed; check tendril_msc_present() for a medium. */
    void (*attach)(struct tendril_msc_lun *lun);
    /* The logical unit is gone; its requests have all completed. */
    void (*detach)(struct tendril_msc_lun *lun);
    /* The medium was inserted, removed or swapped. */
    void (*medium_changed)(struct tendril_msc_lun *lun);
};

/* Registers the mass storage class driver; call before the first controller is added. */
void tendril_msc_register(const struct tendril_msc_callbacks *callbacks);

struct tendril_device *tendril_msc_device(struct tendril_msc_lun *lun);
unsigned tendril_msc_lun_number(struct tendril_msc_lun *lun);
/* SCSI peripheral device type: 0x00 direct access, 0x05 CD/DVD, and so on. */
uint8_t tendril_msc_type(struct tendril_msc_lun *lun);
bool tendril_msc_removable(struct tendril_msc_lun *lun);
bool tendril_msc_present(struct tendril_msc_lun *lun);
bool tendril_msc_write_protected(struct tendril_msc_lun *lun);
uint64_t tendril_msc_blocks(struct tendril_msc_lun *lun);
uint32_t tendril_msc_block_size(struct tendril_msc_lun *lun);
const char *tendril_msc_vendor(struct tendril_msc_lun *lun);
const char *tendril_msc_product(struct tendril_msc_lun *lun);
const char *tendril_msc_revision(struct tendril_msc_lun *lun);
void tendril_msc_set_user(struct tendril_msc_lun *lun, void *user);
void *tendril_msc_user(struct tendril_msc_lun *lun);

#define TENDRIL_MSC_READ 0
#define TENDRIL_MSC_WRITE 1
#define TENDRIL_MSC_FLUSH 2

struct tendril_msc_request_internal {
    struct tendril_list node;
    struct tendril_msc_lun *lun;
    uint32_t done_blocks;
    uint32_t chunk;
    uint8_t retries;
};

struct tendril_msc_request {
    uint8_t op;
    uint64_t lba;
    uint32_t count;
    void *buffer;
    void (*done)(struct tendril_msc_request *req);
    void *context;
    enum tendril_status status;
    struct tendril_msc_request_internal internal;
};

/* Queues a request; requests complete in order, through `done`. */
enum tendril_status tendril_msc_submit(struct tendril_msc_lun *lun, struct tendril_msc_request *req);

/* Synchronous helpers, running tendril_hc_poll() until done. */
enum tendril_status tendril_msc_read(struct tendril_msc_lun *lun, uint64_t lba, uint32_t count, void *buf);
enum tendril_status tendril_msc_write(struct tendril_msc_lun *lun, uint64_t lba, uint32_t count, const void *buf);
enum tendril_status tendril_msc_flush(struct tendril_msc_lun *lun);

/*
 * Runs one command, padded as the device's command set wants. `sense`, if
 * not NULL, receives up to 32 bytes of sense data after a CHECK CONDITION.
 */
enum tendril_status tendril_msc_command(
    struct tendril_msc_lun *lun, const uint8_t *cdb, unsigned cdb_len, void *data, size_t len, bool in,
    uint8_t *sense, void (*done)(void *ctx, enum tendril_status status, uint8_t scsi_status, size_t actual),
    void *ctx);

/* Probes a removable medium again, reporting changes through medium_changed. */
enum tendril_status tendril_msc_check_medium(struct tendril_msc_lun *lun,
                                           void (*done)(void *ctx, enum tendril_status status), void *ctx);

#ifdef __cplusplus
}
#endif

#endif
