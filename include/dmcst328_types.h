#ifndef DMCST328_TYPES_H
#define DMCST328_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include "dmdrvi_ioctl.h"

/** Most contacts the controller reports */
#define DMCST328_MAX_CONTACTS       5U

/**
 * @brief Configuration of a touch panel (the ini section of the driver)
 *
 * The coordinates the node reports are screen coordinates: x in 0..width-1, y in 0..height-1, after the raw ones (0..raw_width-1,
 * 0..raw_height-1, as the chip counts them) were scaled, swapped and mirrored.
 */
typedef struct
{
    uint16_t width;             /**< Screen width the contacts are reported in (as shown, after the rotation of the display) */
    uint16_t height;            /**< Screen height */
    uint16_t raw_width;         /**< Range of the chip's x, 0 = what the chip says (its resolution register) */
    uint16_t raw_height;        /**< Range of the chip's y, 0 = what the chip says */
    uint8_t  address;           /**< 7-bit I2C address of the chip (0x1A) */
    uint8_t  max_contacts;      /**< Contacts reported, 1..DMCST328_MAX_CONTACTS */
    bool     swap_xy;           /**< Exchange the axes (before the mirroring) */
    bool     mirror_x;          /**< x counted from the other side */
    bool     mirror_y;          /**< y counted from the other side */
    uint16_t poll_ms;           /**< Period of the polling of the chip while a client waits (WAIT_EVENT) */
    uint16_t release_ms;        /**< A contact is lifted when the chip has not reported it for this long */
} dmcst328_config_t;

/** A contact as the chip reports it, before the mapping */
typedef struct
{
    uint16_t x;                 /**< Raw column (0..4095) */
    uint16_t y;                 /**< Raw line (0..4095) */
    uint8_t  id;                /**< Number of the finger the chip tracks */
    uint8_t  pressure;
    bool     down;              /**< The chip says the finger touches (it may also report the last position of a lifted one) */
} dmcst328_raw_contact_t;

/** Commands of the node, besides the generic input ones (DMDRVI_IOCTL_INPUT_*) */
typedef enum
{
    dmcst328_ioctl_cmd_get_config = DMDRVI_IOCTL_CUSTOM_BASE,  /**< arg: dmcst328_config_t* (raw_width/raw_height as the driver uses them) */
    dmcst328_ioctl_cmd_max
} dmcst328_ioctl_cmd_t;

#endif /* DMCST328_TYPES_H */
