
#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include <limine.h>

void terminal_init(struct limine_framebuffer *fb);
void terminal_clear(uint8_t red, uint8_t green, uint8_t blue);

#endif
