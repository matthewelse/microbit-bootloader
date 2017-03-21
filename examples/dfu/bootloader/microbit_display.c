/* Copyright (c) 2017 Matthew Else.
 * 
 * C driver for the micro:bit 5x5 display. The aim of this implementation is to
 * be reasonably compact, since the flash space on the NRF51 is limited. For the
 * time being, it'll just be black and white.
 */

#include "nrf_gpio.h"
#include "nrf_drv_timer.h"
#include "stdint.h"
#include "app_error.h"
#include "nrf_drv_config.h"
#include "microbit_display.h"

#define ROW(n)                          (13 + n) // rows 0..2
#define COL(n)                          (4 + n)  // cols 0..8

#define NUM_ROWS 3
#define NUM_COLS 9

#define INTERVAL_US 20

static uint32_t bitmap;
static const nrf_drv_timer_t timer_display = NRF_DRV_TIMER_INSTANCE(1);

static uint8_t col = 0;
static uint8_t row = 0;

static const uint8_t matrix_map[27] = {
    /* 1 */ 0, 2, 4, 19, 18, 17, 16, 15, 11,
    /* 2 */ 14, 10, 12, 1, 3, 23, 21, 0, 0,
    /* 3 */ 22, 24, 20, 5, 6, 7, 8, 9, 13
};

void mb_display_init() {
    // Here, we'll initialize the timer that allows us to multiplex the
    // display, as well as setting up any necessary GPIO.

    // for the time-being, make everything an input

    for (int i = 0; i < NUM_ROWS; i++) {
        nrf_gpio_cfg_input(ROW(i), NRF_GPIO_PIN_NOPULL);
    }

    for (int i = 0; i < NUM_COLS; i++) {
        nrf_gpio_cfg_input(COL(i), NRF_GPIO_PIN_NOPULL);
    }

    // setup the timer
    uint32_t error_code = nrf_drv_timer_init(&timer_display, NULL, mb_display_tick_cb);
    APP_ERROR_CHECK(error_code);

    uint32_t ticks = nrf_drv_timer_us_to_ticks(&timer_display, INTERVAL_US);

    nrf_drv_timer_extended_compare(
         &timer_display, NRF_TIMER_CC_CHANNEL0, ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
}

void mb_display_deinit() {
    // this needs to be called before we branch to the application, since
    // the application might want to use the timer too :p

    nrf_drv_timer_uninit(&timer_display);
}

int mb_display_get_pixel(uint8_t col, uint8_t row) {
    return (bitmap >> ((5 * row) + col)) & 1;
}

void mb_display_set_pixel(uint8_t col, uint8_t row, uint8_t val) {
    if (val) {
        mb_display_show_pixel(col, row);
    } else {
        mb_display_clr_pixel(col, row);
    }
}

void mb_display_set_nth_pixel(uint8_t n) {
    if (bitmap == 0) {
        nrf_drv_timer_enable(&timer_display);
    }

    bitmap |= 1 << n;
}

void mb_display_clr_nth_pixel(uint8_t n) {
    bitmap &= ~(1 << n);

    if (bitmap == 0) {
        nrf_drv_timer_disable(&timer_display);
    }
}

void mb_display_set_image(uint32_t bmp) {
    if (bitmap == 0) {
        nrf_drv_timer_enable(&timer_display);
    }

    bitmap = bmp;

    if (bitmap == 0) {
        nrf_drv_timer_disable(&timer_display);
    }
}

static inline void mb_display_show_pixel(uint8_t col, uint8_t row) {
    if (bitmap == 0) {
        nrf_drv_timer_enable(&timer_display);
    }

    bitmap |= 1 << ((5 * row) + col);
}

static inline void mb_display_clr_pixel(uint8_t col, uint8_t row) {
    bitmap &= ~(1 << ((5 * row) + col));

    if (bitmap == 0) {
        nrf_drv_timer_disable(&timer_display);
    }
}

static void mb_display_tick_cb(nrf_timer_event_t event_type, void* p_context) {
    if (event_type == NRF_TIMER_EVENT_COMPARE0) {
        if ((row == 1 && col == 6) || col == 9) {
            // next row
            nrf_gpio_cfg_input(ROW(row), NRF_GPIO_PIN_NOPULL);
            nrf_gpio_cfg_input(COL(col), NRF_GPIO_PIN_NOPULL);
            row = row == 2 ? 0 : row + 1;
            col = 0;
        } else {
            // next col
            nrf_gpio_cfg_input(COL(col), NRF_GPIO_PIN_NOPULL);
            col += 1;
        }

        // calculate the next pixel to show
        uint8_t index = (row * 9) + col;
        uint32_t pixel_mask = 1 << matrix_map[index];

        if (bitmap & pixel_mask) {
            // show a pixel
            nrf_gpio_cfg_output(ROW(row));
            nrf_gpio_cfg_output(COL(col));
            nrf_gpio_pin_clear(COL(col));
            nrf_gpio_pin_set(ROW(row));
        }
    }
}
