#define DMOD_ENABLE_REGISTRATION ON
#include "dmod_test.h"
#include "dmcst328.h"

static dmcst328_config_t screen(void)
{
    dmcst328_config_t c = { .width = 240, .height = 320, .raw_width = 240, .raw_height = 320, .address = 0x1A,
                            .max_contacts = 5, .poll_ms = 10, .release_ms = 50 };
    return c;
}

DMOD_TEST_STEP(validate_accepts_the_board)
{
    dmcst328_config_t c = screen();
    DMOD_TEST_EXPECT_TRUE(dmcst328_validate_config(&c));
    c.raw_width = 0;                        /* the range of the chip */
    c.raw_height = 0;
    DMOD_TEST_EXPECT_TRUE(dmcst328_validate_config(&c));
}

DMOD_TEST_STEP(validate_rejects_bad_configurations)
{
    dmcst328_config_t c = screen();
    DMOD_TEST_EXPECT_FALSE(dmcst328_validate_config(NULL));
    c.width = 0;
    DMOD_TEST_EXPECT_FALSE(dmcst328_validate_config(&c));
    c = screen();
    c.address = 0x80;
    DMOD_TEST_EXPECT_FALSE(dmcst328_validate_config(&c));
    c = screen();
    c.max_contacts = 6;
    DMOD_TEST_EXPECT_FALSE(dmcst328_validate_config(&c));
    c.max_contacts = 0;
    DMOD_TEST_EXPECT_FALSE(dmcst328_validate_config(&c));
    c = screen();
    c.raw_width = 5000;
    DMOD_TEST_EXPECT_FALSE(dmcst328_validate_config(&c));
}

DMOD_TEST_STEP(parse_report_without_contacts)
{
    uint8_t report[DMCST328_REPORT_SIZE] = { 0 };
    dmcst328_raw_contact_t c[5];
    DMOD_TEST_EXPECT_EQ(dmcst328_parse_report(report, sizeof(report), c, 5), 0);
    DMOD_TEST_EXPECT_EQ(dmcst328_parse_report(report, sizeof(report) - 1, c, 5), 0);   /* too short */
}

DMOD_TEST_STEP(parse_report_one_contact)
{
    /* finger 1, down (0x06); x = 0x0AB, y = 0x123: high bytes 0x0A / 0x12, the nibbles x low 0xB, y low 0x3 */
    uint8_t report[DMCST328_REPORT_SIZE] = { 0x16, 0x0A, 0x12, 0xB3, 0x40, 0x01 };
    dmcst328_raw_contact_t c[5];
    DMOD_TEST_EXPECT_EQ(dmcst328_parse_report(report, sizeof(report), c, 5), 1);
    DMOD_TEST_EXPECT_EQ(c[0].x, 0x0AB);
    DMOD_TEST_EXPECT_EQ(c[0].y, 0x123);
    DMOD_TEST_EXPECT_EQ(c[0].id, 1);
    DMOD_TEST_EXPECT_EQ(c[0].pressure, 0x40);
    DMOD_TEST_EXPECT_TRUE(c[0].down);
}

DMOD_TEST_STEP(parse_report_second_contact_skips_the_count)
{
    /* The second contact starts at the byte 7 - the bytes 5 (the count) and 6 are not its */
    uint8_t report[DMCST328_REPORT_SIZE] = { 0x06, 0x10, 0x20, 0x00, 0x11, 0x02, 0xEE, 0x26, 0x30, 0x40, 0x55, 0x22 };
    dmcst328_raw_contact_t c[5];
    DMOD_TEST_EXPECT_EQ(dmcst328_parse_report(report, sizeof(report), c, 5), 2);
    DMOD_TEST_EXPECT_EQ(c[0].x, 0x100);
    DMOD_TEST_EXPECT_EQ(c[0].y, 0x200);
    DMOD_TEST_EXPECT_EQ(c[1].id, 2);
    DMOD_TEST_EXPECT_EQ(c[1].x, 0x305);
    DMOD_TEST_EXPECT_EQ(c[1].y, 0x405);
    DMOD_TEST_EXPECT_EQ(c[1].pressure, 0x22);
}

DMOD_TEST_STEP(parse_report_limits_the_contacts)
{
    uint8_t report[DMCST328_REPORT_SIZE] = { 0x06, 0, 0, 0, 0, 0x05 };      /* five contacts */
    dmcst328_raw_contact_t c[5];
    DMOD_TEST_EXPECT_EQ(dmcst328_parse_report(report, sizeof(report), c, 2), 2);
    DMOD_TEST_EXPECT_EQ(dmcst328_parse_report(report, sizeof(report), c, 5), 5);
    report[5] = 0x06;                       /* the chip never reports more than five */
    DMOD_TEST_EXPECT_EQ(dmcst328_parse_report(report, sizeof(report), c, 5), 0);
}

DMOD_TEST_STEP(map_point_identity_and_corners)
{
    dmcst328_config_t c = screen();
    uint16_t x, y;
    dmcst328_map_point(&c, 0, 0, &x, &y);
    DMOD_TEST_EXPECT_EQ(x, 0);
    DMOD_TEST_EXPECT_EQ(y, 0);
    dmcst328_map_point(&c, 239, 319, &x, &y);
    DMOD_TEST_EXPECT_EQ(x, 239);
    DMOD_TEST_EXPECT_EQ(y, 319);
    dmcst328_map_point(&c, 4000, 4000, &x, &y);        /* outside the range: clipped to the screen */
    DMOD_TEST_EXPECT_EQ(x, 239);
    DMOD_TEST_EXPECT_EQ(y, 319);
}

DMOD_TEST_STEP(map_point_scales)
{
    dmcst328_config_t c = screen();
    c.raw_width = 4096;
    c.raw_height = 4096;
    uint16_t x, y;
    dmcst328_map_point(&c, 4095, 4095, &x, &y);
    DMOD_TEST_EXPECT_EQ(x, 239);
    DMOD_TEST_EXPECT_EQ(y, 319);
    dmcst328_map_point(&c, 2048, 2048, &x, &y);
    DMOD_TEST_EXPECT_TRUE(x >= 119 && x <= 121);
    DMOD_TEST_EXPECT_TRUE(y >= 159 && y <= 161);
}

DMOD_TEST_STEP(map_point_mirrors)
{
    dmcst328_config_t c = screen();
    uint16_t x, y;
    c.mirror_x = true;
    dmcst328_map_point(&c, 0, 0, &x, &y);
    DMOD_TEST_EXPECT_EQ(x, 239);
    DMOD_TEST_EXPECT_EQ(y, 0);
    c.mirror_x = false;
    c.mirror_y = true;
    dmcst328_map_point(&c, 0, 0, &x, &y);
    DMOD_TEST_EXPECT_EQ(x, 0);
    DMOD_TEST_EXPECT_EQ(y, 319);
}

DMOD_TEST_STEP(map_point_swaps_the_axes)
{
    /* the chip is 240 x 320, the screen as shown 320 x 240 (rotated): x = raw y, y = raw x */
    dmcst328_config_t c = screen();
    c.width = 320;
    c.height = 240;
    c.swap_xy = true;
    uint16_t x, y;
    dmcst328_map_point(&c, 10, 300, &x, &y);
    DMOD_TEST_EXPECT_EQ(x, 300);
    DMOD_TEST_EXPECT_EQ(y, 10);
    dmcst328_map_point(&c, 239, 319, &x, &y);
    DMOD_TEST_EXPECT_EQ(x, 319);
    DMOD_TEST_EXPECT_EQ(y, 239);
}
