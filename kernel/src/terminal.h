
#ifndef TERMINAL_H
#define TERMINAL_H

#include <limine.h>

void terminal_init(struct limine_framebuffer *fb);
void terminal_clear(void);
void terminal_putchar(char c);

#endif
