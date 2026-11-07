#include "dmod.h"
#include "dmdrvi_ioctl.h"
#include <string.h>
#include <errno.h>

#define DOT_SIZE        16U
#define DOT_COLOR       0xFFFF8000U         /* orange; the contacts that are lifted are cleared to black */

static const char *event_name(uint8_t event)
{
    return event == DMDRVI_INPUT_CONTACT_DOWN ? "down" : event == DMDRVI_INPUT_CONTACT_UP ? "up" : "move";
}

static void dot(void *display, const dmdrvi_gfx_info_t *info, const dmdrvi_input_contact_t *c, uint32_t color)
{
    int x = (int)c->x - (int)DOT_SIZE / 2, y = (int)c->y - (int)DOT_SIZE / 2;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + (int)DOT_SIZE > (int)info->width) x = (int)info->width - (int)DOT_SIZE;
    if (y + (int)DOT_SIZE > (int)info->height) y = (int)info->height - (int)DOT_SIZE;
    dmdrvi_gfx_fill_rect_t rect = { (uint16_t)x, (uint16_t)y, DOT_SIZE, DOT_SIZE, color };
    dmdrvi_gfx_rect_t area = { (uint16_t)x, (uint16_t)y, DOT_SIZE, DOT_SIZE };
    (void)Dmod_Ioctl(display, DMDRVI_IOCTL_GFX_FILL_RECT, &rect);
    (void)Dmod_Ioctl(display, DMDRVI_IOCTL_GFX_PRESENT, &area);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        Dmod_Printf("Usage: touchtest <touch node> [<display node> [seconds]]   e.g. touchtest /dev/dmcst3280 /dev/dmst77890 30\n");
        return 1;
    }
    void *touch = Dmod_FileOpen(argv[1], "r");
    if (touch == NULL)
    {
        Dmod_Printf("touchtest: cannot open '%s'\n", argv[1]);
        return 1;
    }
    dmdrvi_input_info_t info;
    if (Dmod_Ioctl(touch, DMDRVI_IOCTL_INPUT_GET_INFO, &info) != 0)
    {
        Dmod_Printf("touchtest: not an input node\n");
        Dmod_FileClose(touch);
        return 1;
    }
    Dmod_Printf("%s: type %d, %ux%u, up to %u contacts, capabilities 0x%x\n", info.name, (int)info.type, info.width, info.height,
                info.max_contacts, (unsigned)info.capabilities);

    void *display = NULL;
    dmdrvi_gfx_info_t gfx = { 0 };
    if (argc > 2)
    {
        display = Dmod_FileOpen(argv[2], "r+");
        if (display == NULL || Dmod_Ioctl(display, DMDRVI_IOCTL_GFX_GET_INFO, &gfx) != 0)
        {
            Dmod_Printf("touchtest: '%s' is not a display\n", argv[2]);
            if (display != NULL)
                Dmod_FileClose(display);
            display = NULL;
        }
    }
    unsigned seconds = 30;
    if (argc > 3)
    {
        seconds = 0;
        for (const char *p = argv[3]; *p >= '0' && *p <= '9'; p++)
            seconds = seconds * 10 + (unsigned)(*p - '0');
    }
    Dmod_Printf("Touch the panel for %u s...\n", seconds);

    int failed = 0;
    unsigned events = 0;
    uint64_t start = Dmod_GetUptime();
    while (Dmod_GetUptime() - start < (uint64_t)seconds * 1000U)
    {
        uint32_t timeout = 200;
        int ret = Dmod_Ioctl(touch, DMDRVI_IOCTL_INPUT_WAIT_EVENT, &timeout);
        if (ret != 0)
        {
            if (ret != -ETIMEDOUT)                              /* nothing happened (the value of errno.h of the toolchain: 110 or 116) */
            {
                Dmod_Printf("touchtest: WAIT_EVENT failed (%d)\n", ret);
                failed++;
                break;
            }
            continue;
        }
        dmdrvi_input_state_t state;
        ret = Dmod_Ioctl(touch, DMDRVI_IOCTL_INPUT_GET_STATE, &state);
        if (ret != 0)
        {
            Dmod_Printf("touchtest: GET_STATE failed (%d)\n", ret);
            failed++;
            break;
        }
        events++;
        for (uint8_t i = 0; i < state.contact_count; i++)
        {
            const dmdrvi_input_contact_t *c = &state.contacts[i];
            Dmod_Printf("%u ms: contact %u %-4s x=%u y=%u pressure=%u\n", (unsigned)(Dmod_GetUptime() - start), c->id,
                        event_name(c->event), c->x, c->y, c->pressure);
            if (display != NULL)
                dot(display, &gfx, c, c->event == DMDRVI_INPUT_CONTACT_UP ? 0xFF000000U : DOT_COLOR);
        }
        if (state.contact_count == 0)
            Dmod_Printf("%u ms: no contacts\n", (unsigned)(Dmod_GetUptime() - start));
    }
    Dmod_Printf("touchtest: %u events, %s\n", events, failed == 0 ? "done" : "FAILED");
    if (display != NULL)
        Dmod_FileClose(display);
    Dmod_FileClose(touch);
    return failed != 0;
}
