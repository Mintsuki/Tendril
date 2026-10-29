# Tendril

A USB host stack in C99 for kernels, bootloaders, and embedded systems. It
needs no C library and asks the embedder for a small set of glue functions.

## Features

- Host controllers: UHCI, OHCI, EHCI (with companion controllers), and xHCI
  (with streams), all with isochronous transfers, firmware handoff, and PCI
  quirks.
- Hubs: USB 2.0 hubs, transaction translators included, and SuperSpeed hubs.
- Class drivers:
  - HID: keyboards, pointing devices, and raw reports (`tendril/hid.h`).
  - Mass storage: Bulk-Only, CBI, and UAS (`tendril/msc.h`).
  - Serial ports: CDC ACM, FTDI, PL2303, CH34x, and CP210x
    (`tendril/serial.h`).
  - Ethernet: CDC ECM, CDC NCM, and RNDIS (`tendril/net.h`).
- Power management: selective suspend and resume, remote wakeup, and system
  sleep.
- An asynchronous core driven by `tendril_hc_poll()`, with synchronous helpers
  for code that would rather wait.

## Building

Compile every `.c` file under `src/` with `include/` on the include path,
which code using Tendril needs too. The sources are C99 and build as C99
through C23 with GCC and Clang, freestanding. They use the compiler's
freestanding headers and `__builtin_memcpy` and friends, which may still call
`memcpy`, `memset`, `memmove`, and `memcmp`. With MSVC, which has no such
builtins and no packing attribute, the intrinsics of those names and
`#pragma pack` stand in for them. On 32-bit targets, link the compiler's
runtime library (libgcc or the compiler-rt builtins, for example) for 64-bit
division.

Defining `TENDRIL_NO_UHCI`, `TENDRIL_NO_OHCI`, `TENDRIL_NO_EHCI`, or
`TENDRIL_NO_XHCI` leaves the corresponding controller driver out.

## Using it

The embedder implements the glue functions of `tendril/glue.h`: memory, DMA
memory, controller register and PCI configuration access, a clock, a lock, and
logging. It then registers the class drivers it wants, adds each controller
with `tendril_hc_add()`, and calls `tendril_hc_poll()` whenever the controller
interrupts or the deadline the last call returned passes. Devices are
enumerated, and every callback runs, from within `tendril_hc_poll()`.

## Documentation

- [docs/API.md](docs/API.md): writing the glue, every function of the library,
  threading, and power management.
- [docs/SPECIFICATIONS.md](docs/SPECIFICATIONS.md): the specifications the code
  follows.
- [3RDPARTY.md](3RDPARTY.md): the code and data Tendril draws on from other
  projects, with their notices.

## License

BSD-2-Clause; see `LICENSE`. Every source file opens with that license. A file
with parts derived from other projects follows it with their notices, and
brackets those parts with `BEGIN` and `END` comments naming their source; see
`3RDPARTY.md`.
