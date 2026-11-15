
#include "terminal.h"

#include <stddef.h>
#include <stdint.h>

#define FONT_WIDTH  5
#define FONT_HEIGHT 7
#define CHAR_WIDTH  6
#define CHAR_HEIGHT 8

#define COLOR_BLACK 0x00000000

static struct limine_framebuffer *framebuffer = NULL;

static size_t cursor_x = 0;
static size_t cursor_y = 0;
static uint32_t color_white = 0x00FFFFFF;

/* 5x7 font: A-Z followed by 0-9. */
static const uint8_t font[36][7] = {
    {14,17,17,31,17,17,17}, /* A */
    {30,17,17,30,17,17,30}, /* B */
    {14,17,16,16,16,17,14}, /* C */
    {30,17,17,17,17,17,30}, /* D */
    {31,16,16,30,16,16,31}, /* E */
    {31,16,16,30,16,16,16}, /* F */
    {14,17,16,23,17,17,15}, /* G */
    {17,17,17,31,17,17,17}, /* H */
    {14,4,4,4,4,4,14},      /* I */
    {7,2,2,2,18,18,12},     /* J */
    {17,18,20,24,20,18,17}, /* K */
    {16,16,16,16,16,16,31}, /* L */
    {17,27,21,21,17,17,17}, /* M */
    {17,25,21,19,17,17,17}, /* N */
    {14,17,17,17,17,17,14}, /* O */
    {30,17,17,30,16,16,16}, /* P */
    {14,17,17,17,21,18,13}, /* Q */
    {30,17,17,30,20,18,17}, /* R */
    {15,16,16,14,1,1,30},   /* S */
    {31,4,4,4,4,4,4},       /* T */
    {17,17,17,17,17,17,14}, /* U */
    {17,17,17,17,17,10,4},  /* V */
    {17,17,17,21,21,21,10}, /* W */
    {17,17,10,4,10,17,17},  /* X */
    {17,17,10,4,4,4,4},     /* Y */
    {31,1,2,4,8,16,31},     /* Z */

    {14,17,19,21,25,17,14}, /* 0 */
    {4,12,4,4,4,4,14},      /* 1 */
    {14,17,1,2,4,8,31},     /* 2 */
    {30,1,1,14,1,1,30},     /* 3 */
    {2,6,10,18,31,2,2},     /* 4 */
    {31,16,16,30,1,1,30},   /* 5 */
    {14,16,16,30,17,17,14}, /* 6 */
    {31,1,2,4,8,8,8},       /* 7 */
    {14,17,17,14,17,17,14}, /* 8 */
    {14,17,17,15,1,1,14}    /* 9 */
};

static uint32_t channel_mask(uint8_t size, uint8_t shift)
{
    if (size == 0) {
        return 0;
    }

    uint64_t mask = ((uint64_t)1 << size) - 1;

    return (uint32_t)(mask << shift);
}

static void draw_pixel(size_t x, size_t y, uint32_t color)
{
    if (framebuffer == NULL) {
        return;
    }

    if (x >= framebuffer->width || y >= framebuffer->height) {
        return;
    }

    volatile uint32_t *pixels = framebuffer->address;
    size_t pitch = framebuffer->pitch / 4;

    pixels[y * pitch + x] = color;
}

static void scroll(void)
{
    if (framebuffer == NULL) {
        return;
    }

    size_t pitch = framebuffer->pitch / 4;
    size_t height = framebuffer->height;
    size_t width = framebuffer->width;

    if (height <= CHAR_HEIGHT) {
        terminal_clear();
        return;
    }

    volatile uint32_t *pixels = framebuffer->address;

    for (size_t y = 0; y < height - CHAR_HEIGHT; y++) {
        for (size_t x = 0; x < width; x++) {
            pixels[y * pitch + x] =
                pixels[(y + CHAR_HEIGHT) * pitch + x];
        }
    }

    for (size_t y = height - CHAR_HEIGHT; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            pixels[y * pitch + x] = COLOR_BLACK;
        }
    }

    cursor_y -= CHAR_HEIGHT;
}

static void newline(void)
{
    cursor_x = 0;
    cursor_y += CHAR_HEIGHT;

    if (cursor_y + CHAR_HEIGHT > framebuffer->height) {
        scroll();
    }
}

static const uint8_t *get_glyph(char c)
{
    static const uint8_t blank[7] = {0,0,0,0,0,0,0};
    static const uint8_t dash[7]  = {0,0,0,31,0,0,0};
    static const uint8_t dot[7]   = {0,0,0,0,0,12,12};
    static const uint8_t colon[7] = {0,12,12,0,12,12,0};
    static const uint8_t slash[7] = {1,2,2,4,8,8,16};
    static const uint8_t question[7] = {14,17,1,2,4,0,4};
    static const uint8_t exclamation[7] = {4,4,4,4,4,0,4};

    if (c >= 'a' && c <= 'z') {
        c -= ('a' - 'A');
    }

    if (c >= 'A' && c <= 'Z') {
        return font[c - 'A'];
    }

    if (c >= '0' && c <= '9') {
        return font[26 + c - '0'];
    }

    switch (c) {
        case ' ':
            return blank;
        case '-':
            return dash;
        case '.':
            return dot;
        case ':':
            return colon;
        case '/':
            return slash;
        case '?':
            return question;
        case '!':
            return exclamation;
        default:
            return question;
    }
}

static void draw_char(char c)
{
    const uint8_t *glyph = get_glyph(c);

    for (size_t row = 0; row < FONT_HEIGHT; row++) {
        for (size_t col = 0; col < FONT_WIDTH; col++) {
            if (glyph[row] & (1U << (FONT_WIDTH - 1 - col))) {
                draw_pixel(cursor_x + col,
                           cursor_y + row,
                           color_white);
            }
        }
    }

    cursor_x += CHAR_WIDTH;

    if (cursor_x + CHAR_WIDTH > framebuffer->width) {
        newline();
    }
}

void terminal_init(struct limine_framebuffer *fb)
{
    framebuffer = fb;

    cursor_x = 0;
    cursor_y = 0;

    color_white =
        channel_mask(fb->red_mask_size, fb->red_mask_shift) |
        channel_mask(fb->green_mask_size, fb->green_mask_shift) |
        channel_mask(fb->blue_mask_size, fb->blue_mask_shift);
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

    cursor_x = 0;
    cursor_y = 0;
}

void terminal_putchar(char c)
{
    if (framebuffer == NULL) {
        return;
    }

    if (c == '\n') {
        newline();
        return;
    }

    if (c == '\r') {
        cursor_x = 0;
        return;
    }

    if (c == '\t') {
        for (int i = 0; i < 4; i++) {
            draw_char(' ');
        }
        return;
    }

    draw_char(c);
}
