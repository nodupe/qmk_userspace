// Copyright 2024 splitkb.com (support@splitkb.com)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "drashna_runtime.h"

#include "qp.h"
#include "qp_comms.h"
#include "qp_surface.h"
#include "qp_st77xx_opcodes.h"
#include "display/painter/painter.h"
#include "display/painter/st7789_135x240.h"

#ifndef ST7789_CS_PIN
#    define ST7789_CS_PIN DISPLAY_CS_PIN
#endif // ST7789_CS_PIN
#ifndef ST7789_DC_PIN
#    define ST7789_DC_PIN DISPLAY_DC_PIN
#endif // ST7789_DC_PIN
#ifndef ST7789_RST_PIN
#    define ST7789_RST_PIN DISPLAY_RST_PIN
#endif // ST7789_RST_PIN
#ifndef ST7789_SPI_DIVIDER
#    define ST7789_SPI_DIVIDER DISPLAY_SPI_DIVIDER
#endif // ST7789_SPI_DIVIDER

static painter_device_t st7789_135x240_display;
static painter_device_t st7789_135x240_surface_display;

static uint8_t display_buffer[SURFACE_REQUIRED_BUFFER_BYTE_SIZE(135, 240, 16)];
static bool    has_run = false, forced_reinit = false;

void init_display_st7789_135x240_inversion(void) {
    qp_comms_start(st7789_135x240_display);
    qp_comms_command(st7789_135x240_display,
                     userspace_config.display.inverted ? ST77XX_CMD_INVERT_OFF : ST77XX_CMD_INVERT_ON);
    qp_comms_stop(st7789_135x240_display);
    if (has_run) {
        forced_reinit = true;
    }
}

void init_display_st7789_135x240_rotation(void) {
    qp_init(st7789_135x240_display, userspace_config.display.rotation ? QP_ROTATION_0 : QP_ROTATION_180);
    qp_set_viewport_offsets(st7789_135x240_display, 52, 40);
    qp_clear(st7789_135x240_display);
    qp_rect(st7789_135x240_display, 0, 0, 170 - 1, 320 - 1, 0, 0, 0, true);

    // if needs inversion, run it only afetr the clear and rect functions or otherwise it won't work
    init_display_st7789_135x240_inversion();

    qp_power(st7789_135x240_display, true);
    qp_flush(st7789_135x240_display);
    if (has_run) {
        forced_reinit = true;
    }
    has_run = true;
}

void init_display_st7789_135x240(void) {
    st7789_135x240_display =
        qp_st7789_make_spi_device(170, 320, ST7789_CS_PIN, ST7789_DC_PIN, ST7789_RST_PIN, ST7789_SPI_DIVIDER, 3);
    st7789_135x240_surface_display = qp_make_rgb565_surface(135, 240, display_buffer);

    init_display_st7789_135x240_rotation();

    qp_init(st7789_135x240_surface_display, QP_ROTATION_0);

    qp_rect(st7789_135x240_surface_display, 0, 0, 135 - 1, 240 - 1, HSV_BLACK, true);
    qp_surface_draw(st7789_135x240_surface_display, st7789_135x240_display, 0, 0, 0);

    qp_flush(st7789_135x240_display);
}

void st7789_135x240_display_power(bool on) {
    qp_power(st7789_135x240_display, on);
}

__attribute__((weak)) void st7789_135x240_draw_user(void) {
    painter_render_menu(st7789_135x240_surface_display, font_oled, 0, 0, 135, 240, false);
    qp_surface_draw(st7789_135x240_surface_display, st7789_135x240_display, 0, 0, 0);
    qp_flush(st7789_135x240_display);
}

void st7789_135x240_display_shutdown(bool jump_to_bootloader) {
    st7789_135x240_display_power(true);
    painter_render_shutdown(st7789_135x240_display, jump_to_bootloader);
}