# Tendril

A freestanding USB host stack, which asks the embedder for a small set of glue
functions.

## Features

- Host controllers: UHCI, OHCI, EHCI (with companion controllers), and xHCI
  (with streams), all with isochronous transfers and firmware handoff, and the
  PCI quirks of EHCI and xHCI controllers.
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
which code using Tendril needs too. The environment provides `memcpy`,
`memset`, `memmove`, and `memcmp`.

Defining `TENDRIL_NO_UHCI`, `TENDRIL_NO_OHCI`, `TENDRIL_NO_EHCI`, or
`TENDRIL_NO_XHCI` leaves the corresponding controller driver out.

## Using it

The embedder implements the glue functions of `tendril/glue.h`: memory, DMA
memory and a DMA barrier, controller register and PCI configuration access, a
clock, a lock, and logging. It then registers the class drivers it wants, adds
each controller with `tendril_hc_add()`, and calls `tendril_hc_poll()` whenever
the controller interrupts, the deadline the last call returned passes, or it
has called into the library outside a callback. Devices are enumerated, and
their transfers complete, from within `tendril_hc_poll()`. Once
`tendril_hc_ready()` is true for every controller, the devices present at boot
are all up.

## Documentation

- [docs/API.md](docs/API.md): writing the glue, every function of the library,
  threading, and power management.
- [docs/SPECIFICATIONS.md](docs/SPECIFICATIONS.md): the specifications the code
  follows.
- [3RDPARTY.md](3RDPARTY.md): the code and data Tendril draws on from other
  projects, with their notices.

## License

BSD-2-Clause; see `LICENSE`. Every source file opens with that license. A file
with parts derived from other projects follows it with their notices, names
their licenses beside its own in its SPDX identifier, and brackets those parts
with `BEGIN` and `END` comments naming their source; see `3RDPARTY.md`. Those
parts are under BSD-2-Clause too, but for some in `src/class/net.c` and
`src/class/serial.c`, which are under ISC, so that those two files' SPDX
identifiers read `BSD-2-Clause AND ISC`.
