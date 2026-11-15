
#include "terminal.h"

#include <stddef.h>
#include <stdint.h>

#define COLOR_BLACK 0x00000000

static struct limine_framebuffer *framebuffer = NULL;

void terminal_init(struct limine_framebuffer *fb)
{
    framebuffer = fb;
}

void terminal_clear(void)
{
    if (framebuffer == NULL) {
        return;
    }

    volatile uint32_t *pixels = framebuffer->address;

    size_t pitch = framebuffer->pitch / 4;

    for (size_t y = 0; y < framebuffer->height; y++) {
        for (size_t x = 0; x < framebuffer->width; x++) {
            pixels[y * pitch + x] = COLOR_BLACK;
        }
    }
}
