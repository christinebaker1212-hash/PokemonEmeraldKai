/* The CRT turning on and off (include/platform/crt.h): the fight's effect, for any picture size and both formats */
#include <string.h>
#include "platform/crt.h"

static uint16_t Brighten(uint16_t c, int amount, int fmt)   /* amount 0..256 towards white */
{
    if (fmt == CRT_FMT_RGB565)
    {
        int r = c >> 11, g = (c >> 5) & 63, b = c & 31;
        r += (31 - r) * amount >> 8;
        g += (63 - g) * amount >> 8;
        b += (31 - b) * amount >> 8;
        return (uint16_t)(r << 11 | g << 5 | b);
    }
    else
    {
        int r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
        r += (31 - r) * amount >> 8;
        g += (31 - g) * amount >> 8;
        b += (31 - b) * amount >> 8;
        return (uint16_t)(r | g << 5 | b << 10 | 0x8000);
    }
}

void Crt_Apply(const uint16_t *src, uint16_t *dst, int w, int h, int phase, double p, int fmt)
{
    int cy = h / 2, cx = w / 2, x, y;
    uint16_t black = fmt == CRT_FMT_RGB565 ? 0 : 0x8000;
    double v, hw;

    if (p > 1.0)
        p = 1.0;
    if (p < 0.0)
        p = 0.0;
    if (phase == CRT_PHASE_ON)
    {
        /* on: the line widens (the first 30%), then the picture opens from it */
        hw = p < 0.3 ? p / 0.3 : 1.0;
        v = p < 0.3 ? 0.0 : (p - 0.3) / 0.7;
    }
    else
    {
        /* off: the picture collapses into the line (60%), the line shrinks to a point */
        v = p < 0.6 ? 1.0 - p / 0.6 : 0.0;
        hw = p < 0.6 ? 1.0 : 1.0 - (p - 0.6) / 0.4;
    }
    for (x = 0; x < w * h; x++)
        dst[x] = black;
    if (v * h < 2.0)
    {
        int half = (int)(hw * w / 2);
        for (y = cy - 1; y <= cy; y++)
            for (x = cx - half; x < cx + half; x++)
                dst[y * w + x] = 0xFFFF;
        return;
    }
    {
        int half = (int)(v * h / 2), boost = (int)((1.0 - v) * (1.0 - v) * 180);
        for (y = cy - half; y < cy + half; y++)
        {
            int sy = cy + (int)((y - cy) / v);
            const uint16_t *row;
            if (y < 0 || y >= h)
                continue;
            sy = sy < 0 ? 0 : sy >= h ? h - 1 : sy;
            row = src + sy * w;
            for (x = 0; x < w; x++)
                dst[y * w + x] = boost > 0 ? Brighten(row[x], boost, fmt) : row[x];
        }
    }
}
