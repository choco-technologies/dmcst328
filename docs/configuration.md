# dmcst328 configuration

The section of the touch panel (`driver_name=dmcst328`):

| Key | Default | |
|---|---|---|
| `instance` | 0 | the node is `/dev/dmcst328<instance>` |
| `width`, `height` | 240, 320 | the screen the contacts are reported in: the display as shown (after its rotation) |
| `raw_width`, `raw_height` | 0 | the range of the chip's x and y; 0 = what the chip says (its resolution registers) |
| `address` | 0x1A | the 7-bit I2C address of the chip |
| `max_contacts` | 5 | contacts reported, 1..5 |
| `swap_xy` | no | exchange the axes (before the mirroring) |
| `mirror_x`, `mirror_y` | no | count x / y from the other side |
| `poll_ms` | 10 | period of the polling of the chip while a client waits (`WAIT_EVENT`) |
| `release_ms` | 50 | a contact is lifted when the chip has not reported it for this long |
| `reset` | no | `yes`: the driver waits for a `reset` friend and resets the chip before it starts |

The axes are turned in this order: swap, mirror, then the scaling from the range of the chip to `width` x `height`. For a display shown
rotated by 90 or 270 degrees, `width` and `height` are those of the display as shown and `swap_xy` is usually on.

## Friends

`dmdevfs` tells every driver of a `friends_group` about the others (`friend_role=`), whatever order they are created in. The chip is
started once the required ones are there:

| `friend_role` | |
|---|---|
| `i2c` | the dmi2c node the chip is on (required) |
| `reset` | the reset line of the chip, a dmgpio output, low = reset (with `reset=yes`) |

The interrupt line of the chip is not used: the driver polls. Declare the touch panel last in the file: the order is the order of the
creation and, reversed, of the teardown.
