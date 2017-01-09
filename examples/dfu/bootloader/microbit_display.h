#include "stdint.h"
#include "nrf_drv_timer.h"

void mb_display_init();
void mb_display_set_image(uint32_t);
int mb_display_get_pixel(uint8_t col, uint8_t row);
void mb_display_set_pixel(uint8_t col, uint8_t row, uint8_t val);
static inline void mb_display_show_pixel(uint8_t col, uint8_t row);
static inline void mb_display_clr_pixel(uint8_t col, uint8_t row);

static void mb_display_tick_cb(nrf_timer_event_t event_type, void* p_context);
