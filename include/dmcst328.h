#ifndef DMCST328_H
#define DMCST328_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "dmod_types.h"
#include "dmcst328_defs.h"
#include "dmcst328_types.h"

/**
 * @brief Check a configuration without touching hardware
 *
 * @return true when the screen size, the address and the number of contacts make sense.
 */
dmod_dmcst328_api(1.0, bool, _validate_config, ( const dmcst328_config_t *config ));

/** Size of the report of the chip (what is read from the register 0xD000) */
#define DMCST328_REPORT_SIZE        27U

/**
 * @brief Decode the report of the chip
 *
 * @param report        DMCST328_REPORT_SIZE bytes read from register 0xD000: the first contact, the number of contacts (the low
 *                      nibble of the byte 5), the other contacts
 * @param size          Bytes in report
 * @param contacts      Output
 * @param max_contacts  Size of contacts
 *
 * @return Contacts decoded; 0 when the chip reports none (or the report is not valid)
 */
dmod_dmcst328_api(1.0, uint8_t, _parse_report, ( const uint8_t *report, size_t size, dmcst328_raw_contact_t *contacts, uint8_t max_contacts ));

/**
 * @brief Map a raw point to screen coordinates (scaling, swap, mirroring), the result is clipped to the screen
 *
 * @param config  The configuration; raw_width and raw_height must be set (not 0)
 */
dmod_dmcst328_api(1.0, void, _map_point, ( const dmcst328_config_t *config, uint16_t raw_x, uint16_t raw_y, uint16_t *x, uint16_t *y ));

#endif // DMCST328_H
