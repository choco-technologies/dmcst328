# dmcst328

DMOD driver for touch panels with a **CST328** controller (Hynitron) on I2C - e.g. the Waveshare ESP32-S3-Touch-LCD-2.8.

[![CI](https://github.com/choco-technologies/dmcst328/actions/workflows/ci.yml/badge.svg)](https://github.com/choco-technologies/dmcst328/actions/workflows/ci.yml)

The node follows the generic **input** interface of [dmdrvi](https://github.com/choco-technologies/dmdrvi) (`DMDRVI_IOCTL_INPUT_*`), so
generic code (a GUI library, a test tool) needs only the path: `/dev/dmcst3280`. It reports up to five contacts in screen coordinates
(the driver swaps, mirrors and scales the chip's axes as configured), `WAIT_EVENT` sleeps until they change.

It talks to the chip through the nodes of other drivers - [dmi2c](https://github.com/choco-technologies/dmi2c) for the bus and
[dmgpio](https://github.com/choco-technologies/dmgpio) for the reset line - so it has no hardware port and runs wherever those do.

## Use

One `friends_group` holds the touch panel and everything it is wired to; `friend_role` says who is who (`i2c`, `reset`).
[configs/board/esp32s3-lcd28/touch.ini](configs/board/esp32s3-lcd28/touch.ini) is the whole touch panel of the Waveshare
ESP32-S3-Touch-LCD-2.8 (its own I2C bus, the reset line). In the flash modules of the board:

```text
dmgpio
dmi2c
dmcst328 driver=board/esp32s3-lcd28/touch.ini
```

```c
void *fp = Dmod_FileOpen("/dev/dmcst3280", "r");
uint32_t timeout = 1000;
if (Dmod_Ioctl(fp, DMDRVI_IOCTL_INPUT_WAIT_EVENT, &timeout) == 0) {
    dmdrvi_input_state_t state;
    Dmod_Ioctl(fp, DMDRVI_IOCTL_INPUT_GET_STATE, &state);    /* contacts[i].x, .y, .event */
}
```

See [docs/api-reference.md](docs/api-reference.md) and [docs/configuration.md](docs/configuration.md). `tools/touchtest` prints the
contacts (and draws them on a display).

## Building

The code does not depend on the CPU, so there is no port: the module is built per architecture, as dmtty or dmspiflash are.

```bash
cmake -S . -B build && cmake --build build && ctest --test-dir build                       # the host: the tests
cmake -S . -B build-esp -DDMOD_TOOLS_NAME=arch/xtensa/esp32s3 && cmake --build build-esp   # an architecture
```

`DMOD_CPU_FAMILY` (stm32f7 by default) only picks the package of dmgpio and dmi2c whose headers are used (their packages are named
by family; the headers are the same for every architecture).
