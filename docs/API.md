# Tendril API

This document covers the glue functions an embedder writes and every
function the library offers, with their arguments, results, and the
conditions they are called under. The headers are `tendril/glue.h` (the
glue), `tendril/tendril.h` (controllers, devices, transfers, and drivers),
`tendril/spec.h` (USB constants and descriptor layouts), and one header per
class driver: `tendril/hid.h`, `tendril/msc.h`, `tendril/serial.h`, and
`tendril/net.h`.

## Contents

- [The glue](#the-glue)
  - [How it fits together](#how-it-fits-together)
  - [Glue functions](#glue-functions)
- [Conventions](#conventions)
- [Threading](#threading)
- [Status codes](#status-codes)
- [Controllers](#controllers)
- [Devices](#devices)
- [Interfaces](#interfaces)
- [Endpoints](#endpoints)
- [Transfers](#transfers)
- [Isochronous scheduling and frame numbers](#isochronous-scheduling-and-frame-numbers)
- [Drivers](#drivers)
- [Power management](#power-management)
- [Class drivers](#class-drivers)

## The glue

### How it fits together

**The `ctx` pointer.** A machine usually has several USB controllers, for
example an xHCI controller, an EHCI controller, and its UHCI companions. When
the library reads a register or allocates memory for a controller, the glue
must know which one, so every glue function that depends on the controller
takes a `ctx` argument: the pointer given as `ctx` in that controller's
`struct tendril_hc_config`. The library never looks inside it. It points at
whatever the embedder keeps per controller: how to reach the registers, the
PCI address for configuration space, the IOMMU or DMA domain of the device,
a lock, and so on.

Functions that do not depend on the controller (`alloc`, `free`,
`dma_barrier`, `time_ns`, and `log`) take no `ctx`.

**Register windows: port I/O or MMIO.** No glue call says which kind of
access it wants; the controller type decides it once, when the embedder sets
the controller up:

| Controller | Registers | PCI BAR |
| ---------- | --------- | ------- |
| UHCI | I/O space | BAR 4 (config offset 0x20) |
| OHCI, EHCI, xHCI | Memory space | BAR 0 (config offset 0x10) |

The library calls the register functions with a byte offset into that
window, and the embedder turns the offset into an access. That is a glue
function because the access differs by platform: the same UHCI register read
is `inw(port + offset)` on x86, a load from the host bridge's PCI I/O window
on arm64, riscv64, or loongarch64 (which have no port I/O instructions), and
a load from a device tree address for a controller that is not on PCI. The
BAR's own bit 0 says whether it is in I/O space, so the setup code can record
the kind of access in the `ctx`:

```c
struct my_hc {
    bool pio;
    uint16_t port;
    volatile uint8_t *mmio;
    struct pci_address pci;
};

uint32_t tendril_glue_read32(void *ctx, uint32_t offset) {
    struct my_hc *c = ctx;
    if (c->pio) {
        return inl((uint16_t)(c->port + offset));
    }
    return *(volatile uint32_t *)(c->mmio + offset);
}
```

Register values are numbers in CPU byte order. Controller registers are
little-endian, so on a big-endian CPU the register functions swap bytes (as
they would for a controller wired with big-endian registers on a
little-endian CPU). The library converts the byte order of its in-memory
structures itself.

**Normal memory and DMA memory.** `tendril_glue_alloc()` provides memory only
the CPU touches: the library's bookkeeping, copies of descriptors, strings.
Any heap will do.

`tendril_glue_dma_alloc()` provides memory the controller reads and writes on
its own, by bus mastering: transfer rings, queue heads, frame lists, device
contexts, and bounce buffers. That memory must meet requirements a heap does
not:

1. **A bus address.** `*dma` receives the address the controller uses. That
   is not the CPU's virtual address, and not always the physical address
   either: behind an IOMMU, or on systems where devices see memory at an
   offset, it differs.
2. **Contiguity.** The controller walks the block by address, so the whole
   block must be contiguous in bus address space.
3. **Alignment.** The hardware requires it, for example 4 KiB for UHCI and
   EHCI frame lists, 256 bytes for the OHCI HCCA, and the ring's own size for
   xHCI rings.
4. **An address limit.** UHCI and OHCI, and EHCI and xHCI controllers without
   64-bit addressing, only reach the first 4 GiB; the library then sets
   `dma32`.
5. **Coherence.** What the CPU writes must reach the controller without cache
   maintenance, and the other way around. Ordinary memory is coherent on x86
   and on most server-class systems; elsewhere this can mean an uncached
   mapping.

The library carves its small descriptors (32 to 160 bytes) out of 4 KiB
blocks itself, so `dma_alloc` can be a page allocator.

**Mapping buffers.** Data buffers belong to the caller, such as the buffer a
disk read goes to. The library does not copy them into DMA memory; it asks
`tendril_glue_dma_map()` for the bus address of each contiguous run of the
buffer, which is also where cache maintenance goes on systems that need it.
When a buffer cannot be reached (it lies above 4 GiB for a 32-bit
controller, or the embedder cannot map it), `dma_map` returns 0 and the
library bounces the transfer through `dma_alloc` memory instead.

A sketch of the memory glue for a kernel with a direct map of physical
memory and no IOMMU, on a cache-coherent system:

```c
void *tendril_glue_dma_alloc(void *ctx, size_t size, size_t align, bool dma32, uint64_t *dma) {
    (void)ctx;
    uint64_t phys = alloc_pages(size, align, dma32 ? (uint64_t)0x100000000 : UINT64_MAX);
    if (phys == 0) {
        return NULL;
    }
    /* Without an IOMMU, the controller sees physical addresses. */
    *dma = phys;
    return phys_to_virt(phys);
}

size_t tendril_glue_dma_map(void *ctx, void *ptr, size_t size, int dir, uint64_t *dma) {
    (void)ctx;
    (void)dir;
    *dma = virt_to_phys(ptr);
    /* Contiguous to the end of the page at least; the library asks again for the rest. */
    size_t room = 4096 - ((uintptr_t)ptr & 4095);
    return size < room ? size : room;
}
```

**Threads and interrupts.** Calls for a controller are serialised (see
[Threading](#threading)), except `tendril_hc_irq()`, which may run in an
interrupt handler at any time and only uses the register functions. With
calls serialised, `tendril_glue_lock()` and `tendril_glue_unlock()` may be
empty.

### Glue functions

Any glue function except `lock` and `unlock` may be called with the
controller's lock held, so none of them may take that lock.

#### `void *tendril_glue_alloc(size_t size)`

General purpose memory, aligned for any object.

- **Returns** the memory, or NULL, in which case the operation that needed it
  fails with `TENDRIL_ERR_NO_MEMORY`. It need not be zeroed.
- Sizes range from a few bytes to 64 KiB (a configuration descriptor can be
  that long).

#### `void tendril_glue_free(void *ptr, size_t size)`

Frees memory from `tendril_glue_alloc()`. `size` is the size it was allocated
with; `ptr` is never NULL.

#### `void *tendril_glue_dma_alloc(void *ctx, size_t size, size_t align, bool dma32, uint64_t *dma)`

Memory the controller accesses by itself (see
[Normal memory and DMA memory](#how-it-fits-together)).

- `size`: bytes.
- `align`: a power of two. Most requests ask for 64 bytes to 4 KiB; xHCI
  rings (4 or 16 KiB) and xHCI stream context arrays ask for their own size.
- `dma32`: the whole block must lie below 4 GiB in bus address space. The
  library checks the result and fails the allocation if it does not.
- `dma`: receives the block's bus address.
- **Returns** the CPU address of the block, contiguous in bus address space
  and coherent with the controller, or NULL. It need not be zeroed.
- Bounce buffers come from here too, sized as the whole transfer and aligned
  to 64 bytes: up to 1 MiB for SuperSpeed mass storage, so the allocator must
  be able to provide contiguous blocks that large if `dma_map` can fail.

#### `void tendril_glue_dma_free(void *ctx, void *ptr, size_t size)`

Frees memory from `tendril_glue_dma_alloc()`, with the size it was allocated
with.

#### `size_t tendril_glue_dma_map(void *ctx, void *ptr, size_t size, int dir, uint64_t *dma)`

Makes a caller's buffer reachable by the controller, one contiguous run at a
time.

- Called when a transfer with a data stage is submitted, unless it has
  `TENDRIL_XFER_DMA`: first with the whole buffer, then with what remains
  after each run.
- `dir`: `TENDRIL_DMA_TO_DEVICE` for OUT data, `TENDRIL_DMA_FROM_DEVICE` for IN
  data. `TENDRIL_DMA_BIDIRECTIONAL` is not used by the library.
- `dma`: receives the bus address of the run.
- **Returns** how many bytes from `ptr` are contiguous at `*dma`, at most
  `size`. A run shorter than `size` must end on a 4 KiB boundary in bus
  address space; otherwise, or when 0 is returned, or when a run lies above
  4 GiB for a 32-bit controller, the library unmaps what it had mapped and
  bounces the whole transfer.
- On systems without coherent DMA, this cleans (`TO_DEVICE`) or invalidates
  (`FROM_DEVICE`) the cache over the run, and should return 0 for a
  `FROM_DEVICE` buffer that does not start and end on cache line boundaries,
  as invalidating those lines would destroy neighbouring data.

#### `void tendril_glue_dma_unmap(void *ctx, void *ptr, size_t size, int dir, uint64_t dma)`

Called once for each run `dma_map` returned, with that run's pointer,
length, direction, and bus address: when the transfer completes (before its
callback), or straight away when the library falls back to a bounce buffer or
the submission fails. On systems without coherent DMA, this invalidates the
cache over `FROM_DEVICE` runs again, against lines fetched speculatively
during the transfer.

#### `void tendril_glue_dma_barrier(void)`

Orders all earlier memory accesses before all later ones, as the controller
sees them. The library calls it between filling in a descriptor and handing
it to the controller. Examples: `mfence` on x86, `dsb sy` on arm64,
`fence iorw, iorw` on riscv64, `dbar 0` on loongarch64.

#### `uint16_t tendril_glue_read16(void *ctx, uint32_t offset)`, `uint32_t tendril_glue_read32(void *ctx, uint32_t offset)`

#### `void tendril_glue_write16(void *ctx, uint32_t offset, uint16_t value)`, `void tendril_glue_write32(void *ctx, uint32_t offset, uint32_t value)`

Controller register access (see
[Register windows](#how-it-fits-together)).

- `offset`: a byte offset into the register window, naturally aligned.
- Values are in CPU byte order.
- Only UHCI uses the 16-bit functions. 8-bit registers are read with
  `read32`, and xHCI's 64-bit registers are written as two 32-bit writes, low
  half first.
- A write must take place only after earlier memory writes are visible to the
  controller (a doorbell after the ring entries it announces), and a read
  must complete before later memory reads.
- They are also called from `tendril_hc_irq()`, so they must work in interrupt
  context.

#### `uint8_t tendril_glue_pci_read8(void *ctx, uint16_t offset)`, and `read16`, `read32`, `write8`, `write16`, `write32` alike

PCI configuration space of the controller, at a byte offset.

- Only called for controllers added with `pci` set, and only from
  `tendril_hc_add()`, `tendril_hc_resume()`, and restarts after a power loss.
- Used for: the IDs (offsets 0x00, 0x02, and 0x08), which select quirks; the
  command register (0x04), to enable decoding and bus mastering; handing the
  controller over from firmware: UHCI LEGSUP (0xc0), EHCI USBLEGSUP and
  USBLEGCTLSTS (at the extended capability offset), and Intel xHCI port
  routing (0xd0 to 0xdc); and the chipset fixes of some EHCI controllers (AMD
  SB600 and SB700 at 0x53, VIA at 0x4b).

#### `bool tendril_glue_pci_find_read8(void *ctx, uint16_t vendor, uint16_t device, uint16_t offset, uint8_t *value)`

Reads the byte at `offset` of the configuration space of another PCI
function: the first one with these vendor and device IDs.

- Only called while adding or restarting a PCI controller (`ctx` is that
  controller's), for chipset fixes that depend on another function: the AMD
  SB700's EHCI fix checks the revision of the SMBus controller, 1002h:4385h.
- **Returns** true, with `*value` set, or false if there is no such function
  or the embedder cannot look, in which case the fix is not applied.

#### `uint64_t tendril_glue_time_ns(void)`

Monotonic time in nanoseconds.

- It drives timers and transfer timeouts, the deadline `tendril_hc_poll()`
  returns, and busy-waits: 100 microseconds to tens of milliseconds for port
  and controller resets, up to a second while waiting on slow firmware or
  hardware.
- It must keep advancing when interrupts are off, if polling can happen with
  them off, and resolve well under 100 microseconds, or short waits end
  early. A free-running counter (TSC, CNTVCT, `rdtime`) is ideal.

#### `void tendril_glue_lock(void *ctx)`, `void tendril_glue_unlock(void *ctx)`

Mutual exclusion around the library's state for one controller. Never taken
recursively, never held while a callback runs, and never taken by
`tendril_hc_irq()`. With every call for a controller serialised, as required
above, they may do nothing.

#### `void tendril_glue_log(int level, const char *message)`

One line of log output: NUL-terminated, without a trailing newline, at most
255 characters (longer messages are cut short). Messages do not name the
library; the embedder adds a prefix if it wants one. Only called for messages
at or below the level set with `tendril_set_log_level()`, `TENDRIL_LOG_INFO`
by default, and never from `tendril_hc_irq()`. Levels: `TENDRIL_LOG_ERROR`,
`TENDRIL_LOG_WARN`, `TENDRIL_LOG_INFO`, `TENDRIL_LOG_DEBUG`, and
`TENDRIL_LOG_TRACE`.

## Conventions

**Asynchronous operations** take a completion function and its context,
`void (*done)(void *ctx, int status)` and `void *ctx`.

- A return of `TENDRIL_OK` means the operation started; `done(ctx, status)`
  is then called exactly once, later, from within `tendril_hc_poll()`, with
  the final status.
- Any other return means nothing started, and `done` is never called.
- `done` may be NULL. Buffers passed in must stay valid until `done`.

**Callbacks** (transfer callbacks, `done` functions, driver hooks, and class
driver callbacks) only run from within `tendril_hc_poll()`, never from
`tendril_hc_irq()`. They may call any function of the library except
`tendril_hc_remove()`, `tendril_hc_suspend()`, and `tendril_hc_resume()`.

**Synchronous helpers** (`tendril_hc_wait()`, `tendril_transfer_run()`,
`tendril_control()`, the `_sync` functions, and `tendril_msc_read()`,
`tendril_msc_write()`, and `tendril_msc_flush()`) call `tendril_hc_poll()` until
their operation completes. They are meant for code that would rather wait,
such as bootloaders.

## Threading

- `tendril_hc_irq()` may be called from an interrupt handler, in any context.
  It only acknowledges and masks the controller's interrupt; call
  `tendril_hc_poll()` afterwards.
- Every other call concerning a controller, or a device or driver object
  behind it, must be serialised with that controller's `tendril_hc_poll()`.
  Calls made from within callbacks are fine.
- Controllers are independent of each other: each can be driven from its own
  thread, provided the glue functions can run on several threads at once.

## Status codes

Functions return, and completion functions receive, an `int` status:
`TENDRIL_OK` (0) or one of the `TENDRIL_ERR_*` codes, all macros in
`tendril/tendril.h`. `tendril_status_str()` gives each one's name. The other
sets of constants (speeds, controller types, log levels, DMA directions) are
macros used with `int` too.

| Code | Meaning |
| ---- | ------- |
| `TENDRIL_OK` | Success, or, for asynchronous calls, started |
| `TENDRIL_ERR_NO_MEMORY` | An allocation failed (normal, DMA, or bounce buffer) |
| `TENDRIL_ERR_INVALID` | A bad argument, or the wrong state (such as resuming a device that is not suspended) |
| `TENDRIL_ERR_UNSUPPORTED` | Compiled out, or not possible with this controller, device, or driver |
| `TENDRIL_ERR_BUSY` | Something conflicting is under way: the transfer is in flight, a configuration change or power operation is running, or a driver refused |
| `TENDRIL_ERR_TIMEOUT` | A timeout ran out: a transfer's `timeout_ms`, or a controller reset |
| `TENDRIL_ERR_CANCELLED` | Cancelled, by the caller or by the library (for a configuration change or a suspend) |
| `TENDRIL_ERR_NO_DEVICE` | The device was unplugged, or the controller died or is being removed |
| `TENDRIL_ERR_STALL` | The device stalled the endpoint |
| `TENDRIL_ERR_IO` | A bus error (CRC, no response, bit stuffing) that outlasted the controller's retries |
| `TENDRIL_ERR_BABBLE` | The device sent more than the transfer allowed for |
| `TENDRIL_ERR_OVERRUN` | The controller's data buffer overran or underran |
| `TENDRIL_ERR_SHORT` | A short packet on a transfer with `TENDRIL_XFER_SHORT_NOT_OK` |
| `TENDRIL_ERR_BANDWIDTH` | Not enough periodic bandwidth for the endpoints (xHCI) |
| `TENDRIL_ERR_MISSED` | An isochronous frame that cannot be, or was not, served |
| `TENDRIL_ERR_CONTROLLER` | The controller reported an internal error or stopped answering commands (xHCI) |
| `TENDRIL_ERR_PROTOCOL` | The device answered with something malformed |
| `TENDRIL_ERR_NO_MEDIUM` | Mass storage: no medium |
| `TENDRIL_ERR_MEDIUM_CHANGED` | Mass storage: the medium changed under the request |
| `TENDRIL_ERR_WRITE_PROTECTED` | Mass storage: the medium is write-protected |
| `TENDRIL_ERR_SUSPENDED` | The device is suspended |
| `TENDRIL_ERR_NO_POWER` | The port cannot supply the current the configuration draws |

## Controllers

#### `bool tendril_pci_hc_type(uint32_t class_code, int *type)`

Maps a PCI class code, `class << 16 | subclass << 8 | prog-if`, to a
controller type.

- **Returns** true and sets `*type` for 0x0c0300 (`TENDRIL_HC_UHCI`), 0x0c0310
  (`TENDRIL_HC_OHCI`), 0x0c0320 (`TENDRIL_HC_EHCI`), and 0x0c0330
  (`TENDRIL_HC_XHCI`); false for anything else, such as USB device
  controllers.

#### `uint32_t tendril_pci_hc_quirks(int type, uint16_t vendor, uint16_t device)`

**Returns** the built-in quirks of a PCI controller, for example to check
`TENDRIL_HC_QUIRK_NO_MSI` before setting up its interrupt. The quirks:

| Quirk | Effect |
| ----- | ------ |
| `TENDRIL_HC_QUIRK_NO_64BIT` | Use 32-bit addresses only |
| `TENDRIL_HC_QUIRK_NO_HANDOFF` | Skip the handover from firmware |
| `TENDRIL_HC_QUIRK_NO_MSI` | Advice for the embedder: MSI is broken |
| `TENDRIL_HC_QUIRK_INTEL_PORT_ROUTING` | Route the ports Intel chipsets share between EHCI and xHCI to xHCI |
| `TENDRIL_HC_QUIRK_NO_PORT_DISABLE` | Never disable a root port |
| `TENDRIL_HC_QUIRK_NO_STREAMS` | No bulk streams |
| `TENDRIL_HC_QUIRK_XHCI_CTRL_STEP` | Queue a control transfer's status stage only once its data stage completed |
| `TENDRIL_HC_QUIRK_XHCI_IMOD_LP` | Moderate xHCI interrupts to about 4000 a second instead of 8000 |
| `TENDRIL_HC_QUIRK_EHCI_LOST_INTR` | The EHCI controller can lose interrupts: poll again 1/128 s after each one |

#### `int tendril_hc_add(const struct tendril_hc_config *config, struct tendril_hc **hc)`

Takes a controller over from firmware, resets it, and starts it.

- `config->type`: the controller type.
- `config->ctx`: handed to the glue functions for this controller.
- `config->pci`: the PCI configuration space functions work for this
  controller. The library then enables decoding and bus mastering, reads the
  IDs to look up quirks, and hands the controller over from firmware.
- `config->irq`: the embedder routes the controller's interrupt to
  `tendril_hc_irq()`, so the controller's interrupts are enabled; otherwise
  they stay off and the embedder only polls.
- `config->quirks`: `TENDRIL_HC_QUIRK_*` to apply on top of the built-in
  tables.
- `hc`: receives the controller, only on success.
- **Returns** `TENDRIL_OK`; `TENDRIL_ERR_UNSUPPORTED` for a type compiled out
  with `TENDRIL_NO_UHCI`, `TENDRIL_NO_OHCI`, `TENDRIL_NO_EHCI`, or
  `TENDRIL_NO_XHCI`, or an xHCI controller the library cannot drive;
  `TENDRIL_ERR_NO_MEMORY`; `TENDRIL_ERR_TIMEOUT` if the controller does not halt
  or reset; or `TENDRIL_ERR_NO_DEVICE` if it does not respond.
- It runs synchronously and busy-waits: normally milliseconds, up to about a
  second per step when firmware or the controller is slow. Devices are
  enumerated afterwards, by `tendril_hc_poll()`.

#### `void tendril_hc_remove(struct tendril_hc *hc)`

Halts and resets the controller, disconnects every device, and frees
everything. Drivers' `disconnect` and class drivers' `detach` callbacks run
during the call, once their transfers have failed with
`TENDRIL_ERR_NO_DEVICE`. `hc` is invalid afterwards. Not to be called from one
of the controller's own callbacks.

#### `bool tendril_hc_irq(struct tendril_hc *hc)`

Interrupt handler entry point, safe in any context.

- **Returns** whether this controller was interrupting, for shared interrupt
  lines. If so, it acknowledged and masked the interrupt, and
  `tendril_hc_poll()` must be called, which unmasks it.
- It takes no lock and only uses the register functions. It returns false
  for a controller that died.

#### `uint64_t tendril_hc_poll(struct tendril_hc *hc)`

Processes controller events, port changes, expired timers, and completions,
and runs callbacks, one at a time.

- **Returns** the time, as `tendril_glue_time_ns()` counts, by which it must be
  called again, or `UINT64_MAX` if nothing is timed.
- Calls for one controller must not overlap, except when nested from within
  its callbacks.
- It can busy-wait briefly, for port resets.

#### `void tendril_hc_wait(struct tendril_hc *hc, volatile bool *flag)`

Calls `tendril_hc_poll()` until `*flag` is true. It has no timeout.

#### `int tendril_hc_suspend(struct tendril_hc *hc, bool wake)`

Prepares the controller for system sleep: waits up to 3 seconds for
enumeration and port activity to settle, suspends every device (deepest
first), arming remote wakeup on the devices that support it if `wake`, then
stops the controller. Putting it into a PCI low-power state and waking the
system through PME are left to the embedder.

- **Returns** `TENDRIL_OK`; `TENDRIL_ERR_NO_DEVICE` if the controller died;
  `TENDRIL_ERR_INVALID` if it is suspended already; `TENDRIL_ERR_BUSY` if it
  did not settle, a device kept signalling wakeup, or a driver refused;
  `TENDRIL_ERR_UNSUPPORTED` if a device's driver cannot suspend; or the error a
  device's suspend failed with.
- On failure, the devices suspended so far are resumed.
- It calls `tendril_hc_poll()` until done, so it must not be called from a
  callback.

#### `int tendril_hc_resume(struct tendril_hc *hc)`

Restarts the controller after system sleep, enabling PCI decoding and bus
mastering again, and resumes the devices `tendril_hc_suspend()` suspended. If
the controller lost its state, it is reset instead and every device is
enumerated again: drivers see `disconnect`, then new `probe` calls.

- **Returns** `TENDRIL_OK` (devices that fail to resume are logged);
  `TENDRIL_ERR_INVALID` if the controller was not suspended;
  `TENDRIL_ERR_NO_DEVICE` if it died; or the error a restart failed with.

#### Controller accessors

| Function | Returns |
| -------- | ------- |
| `int tendril_hc_type(hc)` | The type, a `TENDRIL_HC_*` value |
| `uint32_t tendril_hc_quirks(hc)` | The quirks in effect, built-in and given |
| `size_t tendril_hc_num_ports(hc)` | The number of root ports |
| `struct tendril_device *tendril_hc_port_device(hc, size_t port)` | The device on root port `port` (from 1), or NULL |
| `uint32_t tendril_hc_frame_number(hc)` | The current frame (see [frame numbers](#isochronous-scheduling-and-frame-numbers)), or 0 if the controller died |

Also: `tendril_status_str()`, `tendril_speed_str()`, and
`tendril_hc_type_str()` return constant names, and
`void tendril_set_log_level(int level)` sets which log
messages reach `tendril_glue_log()`.

## Devices

Device pointers come from `tendril_hc_port_device()`,
`tendril_device_child()`, `tendril_device_parent()`, the
`tendril_interface_device()` and `tendril_endpoint_device()` accessors, and
driver callbacks. A pointer stays valid until the device's `device_removed`
call, or for as long as a reference is held.

| Function | Returns |
| -------- | ------- |
| `struct tendril_hc *tendril_device_hc(dev)` | Its controller |
| `struct tendril_device *tendril_device_parent(dev)` | The hub it is on, or NULL on a root port |
| `size_t tendril_device_port(dev)` | Its port on that hub, or on the root hub, from 1 |
| `unsigned int tendril_device_depth(dev)` | The number of hubs between it and the root hub |
| `int tendril_device_speed(dev)` | Its speed, a `TENDRIL_SPEED_*` value |
| `uint8_t tendril_device_address(dev)` | Its USB address |
| `uint32_t tendril_device_quirks(dev)` | The `TENDRIL_DEV_QUIRK_*` applied to it |
| `bool tendril_device_gone(dev)` | Whether it was unplugged (useful while holding a reference) |
| `const struct tendril_device_descriptor *tendril_device_descriptor(dev)` | Its device descriptor |
| `const struct tendril_config_descriptor *tendril_device_config_descriptor(dev, size_t index)` | Configuration `index` (from 0) with all its subordinate descriptors (`total_length` bytes), or NULL |
| `const struct tendril_config_descriptor *tendril_device_active_config(dev)` | The active configuration, or NULL if unconfigured |
| `const struct tendril_bos_descriptor *tendril_device_bos(dev)` | Its BOS descriptor and capabilities, or NULL |
| `const char *tendril_device_manufacturer(dev)`, `tendril_device_product(dev)`, `tendril_device_serial(dev)` | Its strings as UTF-8, read at enumeration, or NULL |
| `struct tendril_device *tendril_device_child(hub, size_t port)` | The device on a hub's port (from 1), or NULL |
| `size_t tendril_device_num_ports(hub)` | A hub's port count, 0 for other devices |
| `struct tendril_endpoint *tendril_device_ep0(dev)` | Its default control endpoint |
| `size_t tendril_device_num_interfaces(dev)`, `struct tendril_interface *tendril_device_interface(dev, size_t index)` | The interfaces of the active configuration; NULL past the end |
| `struct tendril_interface *tendril_device_find_interface(dev, uint8_t number)` | The interface with `bInterfaceNumber` `number`, or NULL |
| `bool tendril_device_suspended(dev)` | Whether it is suspended |
| `bool tendril_device_can_wake(dev)` | Whether its active configuration supports remote wakeup |

The device quirks, from FreeBSD's `sys/dev/usb/quirk/usb_quirk.h`
(BSD-2-Clause) and meaning the same as there: `TENDRIL_DEV_QUIRK_NO_STRINGS`
(do not read strings), `TENDRIL_DEV_QUIRK_SWAP_UNICODE` (strings come
byte-swapped), `TENDRIL_DEV_QUIRK_CONFIG_INDEX_0` to `_4` (the configuration
to select), `TENDRIL_DEV_QUIRK_BUS_POWERED` (the device draws its power from
the bus, whatever its descriptors say), and `TENDRIL_DEV_QUIRK_DYMO_EJECT`
(the label printer is switched to its other mode and left unconfigured).

Enumeration follows FreeBSD's `sys/dev/usb/usb_device.c` and `usb_hub.c`
(BSD-2-Clause) in what it leaves alone:

- Without a configuration index quirk, the first configuration is used,
  unless it has fewer than two interfaces and no endpoints while others
  follow. The mass storage driver also moves a device on from a first
  configuration that only holds an auto-install CD-ROM, as FreeBSD's
  `usb_msctest.c` does.
- A configuration that draws more current than the port supplies
  (`bMaxPower` against 500 mA, 900 mA at SuperSpeed, or one unit load behind a
  bus-powered hub) is not selected: the device stays unconfigured. So does a
  device with no usable configuration, and one whose SET_CONFIGURATION fails
  again after the device was enumerated anew.
- A hub chained behind five others, and a bus-powered hub on a bus-powered
  hub, are left unused.
- After a transfer to a full- or low-speed device behind a high-speed hub ends
  with a bus error, the library clears the hub's transaction translator
  buffer for that endpoint, or resets the translator (USB 2.0 11.24.2.3 and
  11.24.2.9).

#### `void tendril_device_ref(struct tendril_device *dev)`, `void tendril_device_unref(struct tendril_device *dev)`

Keep the device structure valid past its removal; the last `unref` frees it.

#### `int tendril_device_set_configuration(struct tendril_device *dev, uint8_t value, done, ctx)`

Selects the configuration with `bConfigurationValue` `value`, or unconfigures
the device for 0. Everything outstanding on the device is cancelled with
`TENDRIL_ERR_CANCELLED`, ep0 included, and new transfers from drivers fail with
`TENDRIL_ERR_BUSY` until the change is done; then drivers are unbound, the
configuration changes, and the new interfaces are offered to drivers.

- **Returns** `TENDRIL_OK`; `TENDRIL_ERR_NO_DEVICE`; `TENDRIL_ERR_SUSPENDED`;
  `TENDRIL_ERR_BUSY` if the device is still being enumerated, is a hub, or a
  configuration change or power operation is under way;
  `TENDRIL_ERR_INVALID` if no configuration has that value, or
  `TENDRIL_ERR_NO_POWER` if the port cannot supply the current it draws, both
  leaving the device as it was; or `TENDRIL_ERR_NO_MEMORY`.
- `done` receives the outcome of the change.

#### `int tendril_device_read_string(struct tendril_device *dev, uint8_t index, char *buf, size_t size, done, ctx)`

Reads string descriptor `index` in the device's first language (US English
if it lists none) into `buf` as UTF-8, NUL-terminated and cut to `size - 1`
bytes.

- **Returns** `TENDRIL_OK`; `TENDRIL_ERR_INVALID` for index 0;
  `TENDRIL_ERR_NO_DEVICE`; `TENDRIL_ERR_NO_MEMORY`; or an error from submitting
  the request.
- `done` receives `TENDRIL_OK`, `TENDRIL_ERR_PROTOCOL` for a malformed
  descriptor, or the request's error.

#### `int tendril_device_suspend(struct tendril_device *dev, bool remote_wake, done, ctx)`

Selectively suspends a device. Its drivers' `suspend` hooks run during the
call, and whatever they left outstanding is cancelled; once its transfers are
done (within 5 seconds), remote wakeup is armed if `remote_wake` and the
device supports it, and its port is suspended. Transfers to a suspended
device fail with `TENDRIL_ERR_SUSPENDED`.

- **Returns** `TENDRIL_OK`; `TENDRIL_ERR_NO_DEVICE`; `TENDRIL_ERR_INVALID` if it
  is suspended already; `TENDRIL_ERR_BUSY` if it is still being enumerated, a
  configuration change or power operation is under way, it is a hub with
  children awake, or a driver refused; `TENDRIL_ERR_UNSUPPORTED` if one of its
  drivers has no `suspend` hook; or `TENDRIL_ERR_NO_MEMORY`.

#### `int tendril_device_resume(struct tendril_device *dev, done, ctx)`

Resumes a suspended device, resuming the suspended hubs above it first. Its
drivers' `resume` hooks run before `done`.

- **Returns** `TENDRIL_OK`; `TENDRIL_ERR_NO_DEVICE`; `TENDRIL_ERR_BUSY` if a
  power operation is under way; `TENDRIL_ERR_INVALID` if it is not suspended;
  or `TENDRIL_ERR_NO_MEMORY`.

## Interfaces

| Function | Returns |
| -------- | ------- |
| `struct tendril_device *tendril_interface_device(intf)` | Its device |
| `const struct tendril_interface_descriptor *tendril_interface_descriptor(intf)` | The descriptor of its current alternate setting |
| `const uint8_t *tendril_interface_descriptors(intf, size_t *size)` | The current setting's descriptors: its interface descriptor and everything after it up to the next interface descriptor (class-specific and endpoint descriptors); `*size` receives their length |
| `const struct tendril_interface_assoc_descriptor *tendril_interface_association(intf)` | The interface association covering it, or NULL |
| `size_t tendril_interface_num_alts(intf)`, `const struct tendril_interface_descriptor *tendril_interface_alt_descriptor(intf, size_t index)` | Its alternate settings; NULL past the end |
| `size_t tendril_interface_num_endpoints(intf)`, `struct tendril_endpoint *tendril_interface_endpoint(intf, size_t index)` | The endpoints of the current setting; NULL past the end |
| `struct tendril_endpoint *tendril_interface_find_endpoint(intf, uint8_t type, uint8_t dir)` | The first endpoint of the current setting with `type` (`TENDRIL_ENDPOINT_XFER_*`) and `dir` (`TENDRIL_DIR_IN` or `TENDRIL_DIR_OUT`), or NULL |
| `void tendril_interface_set_data(intf, void *data)`, `void *tendril_interface_data(intf)` | A pointer for the driver |

#### `int tendril_interface_set_alt(struct tendril_interface *intf, uint8_t alt, done, ctx)`

Selects the alternate setting with `bAlternateSetting` `alt`: reconfigures the
controller's endpoints, then sends SET_INTERFACE. A stall from an interface
with a single setting counts as success (USB 2.0, section 9.4.10).

- **Returns** `TENDRIL_OK`; `TENDRIL_ERR_INVALID` if there is no such setting;
  `TENDRIL_ERR_NO_DEVICE`; `TENDRIL_ERR_NO_MEMORY`; `TENDRIL_ERR_BUSY` if
  transfers are pending on the interface; or an error from the controller.
- `done` receives the outcome, including errors from the controller such as
  `TENDRIL_ERR_BANDWIDTH`.
- The interface's endpoint structures are replaced: look endpoints up again
  after `done`.

## Endpoints

| Function | Returns |
| -------- | ------- |
| `struct tendril_device *tendril_endpoint_device(ep)` | Its device |
| `const struct tendril_endpoint_descriptor *tendril_endpoint_descriptor(ep)` | Its descriptor (made up for ep0) |
| `const struct tendril_ss_ep_companion_descriptor *tendril_endpoint_companion(ep)` | Its SuperSpeed companion descriptor, or NULL |
| `uint8_t tendril_endpoint_address(ep)` | `bEndpointAddress`; bit 7 set for IN |
| `uint8_t tendril_endpoint_type(ep)` | `TENDRIL_ENDPOINT_XFER_CONTROL`, `_ISOC`, `_BULK`, or `_INT` |
| `uint16_t tendril_endpoint_max_packet(ep)` | The maximum packet size, without the high-bandwidth multiplier bits |
| `unsigned int tendril_endpoint_max_streams(ep)` | The stream count from the companion descriptor (2 to the power MaxStreams), or 0; stream IDs run from 1 to one less |

#### `int tendril_endpoint_clear_halt(struct tendril_endpoint *ep, done, ctx)`

Sends CLEAR_FEATURE(ENDPOINT_HALT), then resets the controller's data toggle
or sequence number for the endpoint.

- **Returns** `TENDRIL_OK`, `TENDRIL_ERR_NO_DEVICE`, `TENDRIL_ERR_NO_MEMORY`, or
  an error from submitting the request. `done` receives the outcome.

#### `int tendril_endpoint_alloc_streams(struct tendril_endpoint **eps, size_t count, unsigned int *streams, done, ctx)`

Gives `count` bulk endpoints of one device `*streams` streams each, with IDs 1
to `*streams`. Only xHCI does streams.

- **Returns** `TENDRIL_OK`, having lowered `*streams` to what the endpoints and
  controller allow; `TENDRIL_ERR_INVALID` if `count` or `*streams` is 0;
  `TENDRIL_ERR_UNSUPPORTED` if the controller has no streams, or the endpoints
  are not all bulk endpoints of one device with streams;
  `TENDRIL_ERR_NO_DEVICE`; or `TENDRIL_ERR_NO_MEMORY`.

## Transfers

`struct tendril_transfer` is allocated by the caller and must stay in place
until its callback. `tendril_transfer_init(xfer, ep)` zeroes it and sets
`endpoint`; then the caller fills in:

| Field | Meaning |
| ----- | ------- |
| `setup` | Control transfers only: the SETUP packet. `setup.length` is the data stage's length, and `length` must be at least that |
| `buffer`, `length` | The data. Isochronous packets lie back to back in it |
| `dma` | With `TENDRIL_XFER_DMA`, the bus address of `buffer`, which the caller guarantees to be contiguous and coherent; the library does not map it. It must lie below 4 GiB on 32-bit controllers |
| `flags` | `TENDRIL_XFER_SHORT_NOT_OK`: a short packet ends the transfer with `TENDRIL_ERR_SHORT`. `TENDRIL_XFER_ZERO_PACKET`: an OUT transfer that is a multiple of the packet size ends with a zero-length packet. `TENDRIL_XFER_DMA`: see `dma`. `TENDRIL_XFER_ISO_ASAP`: see [isochronous scheduling](#isochronous-scheduling-and-frame-numbers) |
| `timeout_ms` | 0 for none; otherwise the transfer is cancelled after this long and completes with `TENDRIL_ERR_TIMEOUT` |
| `stream` | A bulk stream ID from `tendril_endpoint_alloc_streams()`, or 0 |
| `num_iso_packets`, `iso_packets`, `start_frame` | Isochronous transfers: each packet's `length` goes in; its `actual` and `status` come out |
| `callback`, `context` | Called once the transfer completes, from within `tendril_hc_poll()`; it may resubmit or free the transfer |

The results are `status` and `actual`, the number of bytes transferred.

#### `int tendril_transfer_submit(struct tendril_transfer *xfer)`

- **Returns** `TENDRIL_OK`, after which the callback is called exactly once;
  `TENDRIL_ERR_INVALID` for no endpoint, `length` short of `setup.length`,
  missing isochronous packets or more of them than `length` holds, an
  isochronous packet longer than the endpoint's payload per service interval,
  a stream on
  a non-bulk endpoint or one not allocated, a NULL buffer with data, or an
  endpoint not open (not in the current alternate setting);
  `TENDRIL_ERR_BUSY` if this transfer is in flight already, or the device's
  configuration is changing; `TENDRIL_ERR_NO_DEVICE`; `TENDRIL_ERR_SUSPENDED`;
  `TENDRIL_ERR_NO_MEMORY` if a bounce buffer could not be had; or an error from
  the controller driver, such as `TENDRIL_ERR_MISSED` for an isochronous
  start frame it cannot schedule, or `TENDRIL_ERR_BUSY` for a full xHCI ring.
- The status the callback sees is one of `TENDRIL_OK`, `TENDRIL_ERR_SHORT`,
  `TENDRIL_ERR_STALL`, `TENDRIL_ERR_IO`, `TENDRIL_ERR_BABBLE`,
  `TENDRIL_ERR_OVERRUN`, `TENDRIL_ERR_CANCELLED`, `TENDRIL_ERR_TIMEOUT`,
  `TENDRIL_ERR_NO_DEVICE`, or `TENDRIL_ERR_CONTROLLER`; isochronous packets have
  their own statuses.

#### `void tendril_transfer_cancel(struct tendril_transfer *xfer)`

Cancels a transfer in flight, asynchronously: the callback still comes, with
`TENDRIL_ERR_CANCELLED` unless the transfer finished first. Does nothing for a
transfer not in flight.

#### `int tendril_transfer_run(struct tendril_transfer *xfer)`

Submits the transfer and calls `tendril_hc_poll()` until it completes.
**Returns** the submission's error, or the transfer's final status.

#### `bool tendril_transfer_done(struct tendril_transfer *xfer)`

**Returns** true if the transfer was never submitted or has completed (true
already when its callback runs), false while it is in flight.

#### `int tendril_control(struct tendril_device *dev, uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, void *data, uint16_t length, size_t *actual, uint32_t timeout_ms)`

A synchronous control request on ep0, with `value`, `index`, and `length` in
CPU byte order. `actual`, if not NULL, receives the bytes transferred.
**Returns** as `tendril_transfer_run()` does.

## Isochronous scheduling and frame numbers

`tendril_hc_frame_number()` counts 1 ms frames in 32 bits, the same way on
every controller, and `start_frame` counts the same way. The count follows the
controller's own frame counter (bits 13:3 of EHCI's FRINDEX, EHCI 1.0
section 2.3.4; bits 13:3 of xHCI's MFINDEX, xHCI 1.2 section 5.5.1; UHCI's
FRNUM; OHCI's HcFmNumber), which wraps every 2048 frames (65536 on OHCI). The
library reads the counter on every `tendril_hc_poll()`, so if polling stops
for longer than a wrap, the count falls behind by whole wraps; it never goes
backwards, and it carries on across a controller restart.

- With `TENDRIL_XFER_ISO_ASAP`, a transfer starts right after everything
  queued on the endpoint (after whichever queued transfer ends last), or a
  few frames from now if nothing is queued. Transfers submitted back to back
  this way follow each other without a gap.
- Without it, the transfer starts at `start_frame`, on every controller and
  whatever is queued. A start frame already begun fails with
  `TENDRIL_ERR_MISSED` on UHCI and OHCI, as does one less than 2 frames ahead
  on EHCI; on xHCI, the controller reports missed frames in the packets'
  statuses. Transfers placed this way should not overlap others on the same
  endpoint: what happens then depends on the controller.
- A transfer must fit in the controller's schedule, or submission fails with
  `TENDRIL_ERR_MISSED`: on UHCI and EHCI it must end within 1024 frames from
  now, and on OHCI start within 32768.
- On submission, `start_frame` is set to the frame the transfer starts in.
  xHCI picks that frame itself for `TENDRIL_XFER_ISO_ASAP`, so there it is the
  frame the transfer was queued in.

## Drivers

`struct tendril_driver` is filled in by the driver and registered with
`void tendril_driver_register(struct tendril_driver *driver)` before the first
controller is added. Registering the same structure again does nothing.

| Member | Meaning |
| ------ | ------- |
| `name` | For logs |
| `bool (*probe)(struct tendril_interface *intf)` | Offered each unclaimed interface of a newly configured device, drivers in the order they were registered; true claims it. A driver without `probe` only watches devices come and go. The first interface of a hub belongs to the hub driver |
| `void (*disconnect)(struct tendril_interface *intf)` | The interface it probed is going away (unplugged, configuration change, controller removal); its transfers have all completed. Interfaces it claimed along with it are released with it |
| `void (*device_added)(struct tendril_device *dev)`, `void (*device_removed)(struct tendril_device *dev)` | Called on every registered driver: after a device's interfaces were offered, and after they were disconnected |
| `bool (*suspend)(struct tendril_interface *intf)` | The device is being suspended: stop submitting; the core cancels what is still outstanding. False refuses, failing the suspend with `TENDRIL_ERR_BUSY`. A device bound to a driver without this hook cannot be suspended |
| `void (*resume)(struct tendril_interface *intf)` | The device resumed, or a suspend failed after `suspend` was called |
| `void (*device_woke)(struct tendril_device *dev)` | The device resumed because it signalled remote wakeup, after the `resume` hooks ran |
| `next` | The library's |

#### `bool tendril_interface_claim(struct tendril_interface *intf, struct tendril_driver *driver)`

Claims another interface of the device for the driver being probed, such as a
CDC data interface. Called from `probe`. **Returns** false if another driver
has it.

## Power management

- `tendril_device_suspend()` and `tendril_device_resume()` suspend and resume
  one device, arming its remote wakeup on request. A hub can be suspended once
  all its children are, and resuming a device resumes its hubs first.
  Transfers to a suspended device fail with `TENDRIL_ERR_SUSPENDED`.
- Drivers take part through the `suspend` and `resume` hooks of
  `struct tendril_driver`: a device whose driver lacks them is not suspended.
  The built-in class drivers all have them; they refuse while work is in
  progress (an unfinished disk request, a serial write, a frame being sent).
- A device that signals remote wakeup is resumed by the library, after which
  every driver's `device_woke` hook is called.
- `tendril_hc_suspend()` suspends every device of a controller, then stops it,
  for system sleep; `tendril_hc_resume()` brings everything back. A controller
  that lost its state meanwhile is reset, and its devices enumerated again.
  Putting the controller into a PCI low-power state, and waking the system
  through PME, is the embedder's part.
- Link power management (USB 2.0 LPM, and U1 and U2) is not implemented: links
  stay in their full-power states while devices are awake.

## Class drivers

Each class driver is registered before the first controller is added, with a
structure of callbacks; its `attach` callback hands over an object for each
function found (a keyboard, a disk, a serial port, a network adapter) and
`detach` takes it back once everything it had in flight has completed. Each
object can carry a pointer for the embedder, through its `set_user` and
`user` functions, and their asynchronous calls follow the
[conventions](#conventions).

### HID (`tendril/hid.h`)

- `void tendril_hid_register(const struct tendril_hid_callbacks *cb, uint32_t flags)`:
  `TENDRIL_HID_BOOT_PROTOCOL` in `flags` uses the boot protocol on keyboards
  and mice that support it. Callbacks: `attach`, `detach`,
  `report(hid, id, data, size)` for every input report (without its report ID
  byte, which is `id`), `key(hid, key, pressed, modifiers)` with keyboard usage
  IDs, and `pointer(hid, event)` with relative motion, absolute position,
  wheel, pan, and buttons.
- As FreeBSD's `sys/dev/hid/hkbd.c` and `hms.c` (BSD-2-Clause) do, a boot
  keyboard or mouse whose report descriptor cannot be read or parsed, and a
  boot keyboard whose report descriptor has no keys, use the boot protocol;
  so do the keyboards FreeBSD's quirk tables list, and the devices they list
  as not HID are left alone. The Kensington SlimBlade's two vendor-defined
  buttons are reported as buttons 2 and 3, as FreeBSD's `hms.c` maps them,
  and on the Apple ISO keyboards FreeBSD's `sys/dev/usb/input/ukbd.c` lists,
  keyboard usages 35h and 64h are swapped back.
- `int tendril_hid_set_leds(hid, uint8_t leds)` sets keyboard
  LEDs (`TENDRIL_HID_LED_*`) without a completion function. **Returns**
  `TENDRIL_OK` (sent, or kept for when the previous update is done or the
  device resumes); `TENDRIL_ERR_NO_DEVICE`; or `TENDRIL_ERR_UNSUPPORTED` if the
  device has no LED output report the driver can build.
- `tendril_hid_get_report(hid, type, id, buf, size, done, ctx)` and
  `tendril_hid_set_report(hid, type, id, buf, size, done, ctx)`: GET_REPORT,
  and SET_REPORT, or the interrupt OUT endpoint for output reports if the
  interface has one (HID 1.11, 4.4). `buf` holds the report without its
  report ID byte: for an `id` other than 0, the driver adds that byte to what
  it sends and removes it from what it reads (HID 1.11, 5.6). `size` is at
  most 65534. **Return** `TENDRIL_OK`, `TENDRIL_ERR_NO_DEVICE`,
  `TENDRIL_ERR_INVALID`, `TENDRIL_ERR_NO_MEMORY`, or an error from submitting
  the request; `get_report`'s `done` also receives the length read.
- `tendril_hid_interface()`, `tendril_hid_kinds()` (`TENDRIL_HID_KEYBOARD`,
  `_MOUSE`, `_ABSOLUTE`, `_GAMEPAD`, `_CONSUMER`),
  `tendril_hid_boot_protocol()`, and `tendril_hid_descriptor()` describe the
  device. The report descriptor parser (`tendril_hid_parse()`,
  `tendril_hid_free()`, and the field functions) also works on its own.

### Mass storage (`tendril/msc.h`)

Bulk-Only, CBI, and UAS transports, carrying the SCSI, UFI, ATAPI, and RBC
command sets.

- `void tendril_msc_register(const struct tendril_msc_callbacks *cb)`:
  callbacks `attach` and `detach` for each logical unit, and
  `medium_changed`.
- The devices FreeBSD's quirk tables list get the same treatment as in its
  `sys/dev/usb/storage/umass.c` (BSD-2-Clause): forced transports and command
  sets, commands left out or replaced, and residues and signatures tolerated.
  As its `usb_msctest.c` does, a first configuration whose first interface
  is a Bulk-Only CD-ROM, with more configurations to choose from, is taken for
  an auto-install disk and the next configuration used instead, and the
  devices FreeBSD lists for it are ejected with START STOP UNIT and left
  unconfigured, to come back in their other mode.
- `int tendril_msc_submit(lun, struct tendril_msc_request *req)`
  queues a request: `op` (`TENDRIL_MSC_READ`, `TENDRIL_MSC_WRITE`, or
  `TENDRIL_MSC_FLUSH`), `lba`, `count` in blocks, `buffer`, `done`, and
  `context`. Requests complete in order, through `done`. **Returns**
  `TENDRIL_OK`; `TENDRIL_ERR_NO_DEVICE`; `TENDRIL_ERR_SUSPENDED`;
  `TENDRIL_ERR_INVALID` for a bad `op`, no `done`, a zero `count`, a NULL
  buffer, or blocks past the end; `TENDRIL_ERR_NO_MEDIUM`; or
  `TENDRIL_ERR_WRITE_PROTECTED`.
- `tendril_msc_command(lun, cdb, cdb_len, data, len, in, sense, done, ctx)`
  runs one SCSI command; `done` receives the status, the SCSI status byte,
  and the length transferred, and `sense` up to 32 bytes of sense data. A
  CHECK CONDITION whose sense key is NO SENSE comes back as GOOD, as FreeBSD's
  `umass.c` takes it.
- `tendril_msc_check_medium(lun, done, ctx)` probes a removable medium again,
  reporting changes through `medium_changed`; **returns**
  `TENDRIL_ERR_BUSY` while a probe is under way.
- `tendril_msc_read()`, `tendril_msc_write()`, and `tendril_msc_flush()` are the
  synchronous forms. `tendril_msc_blocks()`, `tendril_msc_block_size()`,
  `tendril_msc_present()`, `tendril_msc_write_protected()`, and the other
  accessors describe the unit.

### Serial ports (`tendril/serial.h`)

CDC ACM, and adapters with FTDI, Prolific PL2303, WCH CH34x, and Silicon Labs
CP210x chips. CDC ACM interfaces are taken as FreeBSD's
`sys/dev/usb/serial/umodem.c` (BSD-2-Clause) takes them: those with no
protocol or AT commands, and the modems its table lists; their data
interface, and call management over it, are found the way it finds them.

- `void tendril_serial_register(const struct tendril_serial_callbacks *cb)`:
  callbacks `attach` (ports start at 115200 baud, 8 data bits, no parity, 1
  stop bit, with DTR and RTS on), `detach`, `receive(port, data, len)` (the
  data is valid during the call only), and `status(port, bits)` with the
  `TENDRIL_SERIAL_*` line state.
- `tendril_serial_configure(port, config, done, ctx)`,
  `tendril_serial_set_lines(port, dtr, rts, done, ctx)`,
  `tendril_serial_set_break(port, on, done, ctx)`, and
  `tendril_serial_write(port, data, len, done, ctx)`. **Return** `TENDRIL_OK`,
  `TENDRIL_ERR_NO_DEVICE`, `TENDRIL_ERR_INVALID` (a configuration the driver
  rejects, or an empty write), `TENDRIL_ERR_UNSUPPORTED` (a setting the chip
  cannot make, or a break on a CDC ACM device that does not support
  SendBreak), or `TENDRIL_ERR_NO_MEMORY`. Writes go out in order, and their
  data must stay valid until `done`.
- `tendril_serial_configure_sync()` and `tendril_serial_write_sync()` are the
  synchronous forms.

### Ethernet (`tendril/net.h`)

- `void tendril_net_register(const struct tendril_net_callbacks *cb)`:
  callbacks `attach`, `detach`, `link(net, up, rx_bps, tx_bps)`, and
  `receive(net, frame, len)` (valid during the call only). Frames run from the
  destination address to the end of the payload, without the frame check
  sequence.
- `int tendril_net_send(net, frame, len, done, ctx)` queues a
  frame. **Returns** `TENDRIL_OK`, `TENDRIL_ERR_NO_DEVICE`,
  `TENDRIL_ERR_INVALID` (shorter than 14 bytes or longer than
  `tendril_net_max_frame()`), or `TENDRIL_ERR_NO_MEMORY`. Frames go out in order;
  while the device is suspended, they wait for it to resume. RNDIS adapters
  take and deliver frames of at most 2046 bytes, the limit of FreeBSD's
  `sys/dev/usb/net/if_urndis.c` (ISC license).
- `tendril_net_set_filter()` and `tendril_net_set_multicast()` **return**
  `TENDRIL_ERR_UNSUPPORTED` if the adapter cannot filter.
  `tendril_net_send_sync()` is the synchronous form of `tendril_net_send()`.
