# dmcst328 API Reference

The node (`/dev/dmcst328<instance>`) follows the generic **input** interface of [dmdrvi](https://github.com/choco-technologies/dmdrvi)
(`DMDRVI_IOCTL_INPUT_*`): generic code needs only the path.

| | |
|---|---|
| `DMDRVI_IOCTL_INPUT_GET_INFO` | `dmdrvi_input_info_t`: "CST328", a touch screen, `width` x `height`, up to `max_contacts` contacts |
| `DMDRVI_IOCTL_INPUT_GET_STATE`, `read()` | `dmdrvi_input_state_t`: the contacts in screen coordinates; each has `event` DOWN, MOVE or UP (the last report of a lifted contact, once) |
| `DMDRVI_IOCTL_INPUT_WAIT_EVENT` | sleeps until the state differs from the one handed out last (`uint32_t*` timeout in ms, `-ETIMEDOUT`) |
| `dmcst328_ioctl_cmd_get_config` | `dmcst328_config_t`: the configuration, with the range of the chip as the driver uses it |

The chip is polled by `read()` / `GET_STATE` and, every `poll_ms`, by `WAIT_EVENT`; nothing runs in the background. A state that was
not handed out yet is kept as it is (no UP is lost between two calls).

```c
void *fp = Dmod_FileOpen("/dev/dmcst3280", "r");
uint32_t timeout = 1000;
if (Dmod_Ioctl(fp, DMDRVI_IOCTL_INPUT_WAIT_EVENT, &timeout) == 0) {
    dmdrvi_input_state_t state;
    Dmod_Ioctl(fp, DMDRVI_IOCTL_INPUT_GET_STATE, &state);
    for (int i = 0; i < state.contact_count; i++)
        Dmod_Printf("%u,%u\n", state.contacts[i].x, state.contacts[i].y);
}
```

## Functions

Pure functions, exported for the tests and for other code that reads the chip itself:

```c
bool    dmcst328_validate_config(const dmcst328_config_t *config);
uint8_t dmcst328_parse_report(const uint8_t *report, size_t size, dmcst328_raw_contact_t *contacts, uint8_t max_contacts);
void    dmcst328_map_point(const dmcst328_config_t *config, uint16_t raw_x, uint16_t raw_y, uint16_t *x, uint16_t *y);
```

## The chip

16-bit register addresses, high byte first (a write of the address, then a read with a repeated START). `0xD000`: 27 bytes, the report
- 5 bytes per contact (finger and status, x high, y high, x low / y low nibbles, pressure); the byte 5 is the number of contacts
(low nibble). The driver writes `0` to `0xD005` after every report. The registers `0xD101` (debug info mode: boot time, resolution)
and `0xD109` (normal mode) are used at the start.
