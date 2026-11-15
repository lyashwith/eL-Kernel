
#include "terminal.h"

#include <stddef.h>

static struct limine_framebuffer *framebuffer = NULL;

static uint32_t fb_channel(uint8_t value,
                           uint8_t mask_size,
                           uint8_t mask_shift)
{
    uint64_t max = ((uint64_t)1 << mask_size) - 1;

    return (uint32_t)((value * max / 255) << mask_shift);
}

static uint32_t fb_pixel(uint8_t red, uint8_t green, uint8_t blue)
{
    return fb_channel(red, framebuffer->red_mask_size,
                      framebuffer->red_mask_shift)
         | fb_channel(green, framebuffer->green_mask_size,
                      framebuffer->green_mask_shift)
         | fb_channel(blue, framebuffer->blue_mask_size,
                      framebuffer->blue_mask_shift);
}

void terminal_init(struct limine_framebuffer *fb)
{
    framebuffer = fb;
}

void terminal_clear(uint8_t red, uint8_t green, uint8_t blue)
{
    if (framebuffer == NULL) {
        return;
    }

    volatile uint32_t *pixels = framebuffer->address;
    uint32_t colour = fb_pixel(red, green, blue);
    size_t pitch = framebuffer->pitch / 4;

    for (size_t y = 0; y < framebuffer->height; y++) {
        for (size_t x = 0; x < framebuffer->width; x++) {
            pixels[y * pitch + x] = colour;
        }
    }
}
