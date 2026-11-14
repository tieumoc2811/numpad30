// Khung gia Pico SDK / TinyUSB: chi du de bien dich firmware/main.c tren may tinh va chay mo phong END-TO-END (xem ../fw_sim.c)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "config.h"
typedef unsigned int uint;
#define GPIO_OUT 1
#define GPIO_IN 0
#define GPIO_FUNC_I2C 3
typedef struct { int dummy; } i2c_inst_t; extern i2c_inst_t *i2c0;
typedef struct { int dummy; } PIO_t; typedef PIO_t *PIO; extern PIO pio0;
typedef struct { int dummy; } pio_sm_config; typedef struct { int dummy; } pio_program_t; extern pio_program_t ws2812_program;
#define PIO_FIFO_JOIN_TX 1
#define clk_sys 0
#define ws2812_T1 2
#define ws2812_T2 5
#define ws2812_T3 3
static inline pio_sm_config ws2812_program_get_default_config(uint o) { (void)o; pio_sm_config c = {0}; return c; }
static inline uint pio_add_program(PIO p, const pio_program_t *g) { (void)p; (void)g; return 0; }
static inline void sm_config_set_sideset_pins(pio_sm_config *c, uint p) { (void)c; (void)p; }
static inline void sm_config_set_out_shift(pio_sm_config *c, bool a, bool b, uint n) { (void)c; (void)a; (void)b; (void)n; }
static inline void sm_config_set_fifo_join(pio_sm_config *c, int j) { (void)c; (void)j; }
static inline void sm_config_set_clkdiv(pio_sm_config *c, float d) { (void)c; (void)d; }
static inline uint32_t clock_get_hz(int c) { (void)c; return 150000000; }
static inline void pio_gpio_init(PIO p, uint pin) { (void)p; (void)pin; }
static inline void pio_sm_set_consecutive_pindirs(PIO p, uint sm, uint pin, uint n, bool o) { (void)p; (void)sm; (void)pin; (void)n; (void)o; }
static inline void pio_sm_init(PIO p, uint sm, uint off, const pio_sm_config *c) { (void)p; (void)sm; (void)off; (void)c; }
static inline void pio_sm_set_enabled(PIO p, uint sm, bool e) { (void)p; (void)sm; (void)e; }
extern uint32_t sim_led; static inline void pio_sm_put_blocking(PIO p, uint sm, uint32_t v) { (void)p; (void)sm; sim_led = v; }
// GPIO mo phong ma tran: gpio_get(cot) = phim nhan tren hang dang keo cao (diode ROW2COL)
extern uint8_t sim_down[NROW][NCOL]; extern int sim_rowhi[32];
static inline void gpio_init(uint g) { (void)g; } static inline void gpio_set_dir(uint g, bool o) { (void)g; (void)o; }
static inline void gpio_put(uint g, bool v) { sim_rowhi[g] = v; }
static inline bool gpio_get(uint g) {
  static const int cp[NCOL] = COL_PINS, rp[NROW] = ROW_PINS;
  for (int c = 0; c < NCOL; c++) if ((int)g == cp[c]) for (int r = 0; r < NROW; r++) if (sim_rowhi[rp[r]] && sim_down[r][c]) return true;
  return false;
}
static inline void gpio_pull_up(uint g) { (void)g; } static inline void gpio_pull_down(uint g) { (void)g; } static inline void gpio_set_function(uint g, int f) { (void)g; (void)f; }
static inline void busy_wait_us_32(uint32_t u) { (void)u; }
extern uint32_t fake_ms; static inline uint32_t time_us_32(void) { return fake_ms * 1000u; }
extern int sim_i2c_writes; extern uint8_t sim_i2c_cmd[2];
static inline int i2c_write_timeout_us(i2c_inst_t *i, uint8_t a, const uint8_t *s, size_t n, bool nostop, uint t) { (void)i; (void)a; (void)nostop; (void)t; sim_i2c_writes++; if (n == 2 && s[0] == 0x00 && (s[1] == 0xAE || s[1] == 0xAF)) { sim_i2c_cmd[0] = s[0]; sim_i2c_cmd[1] = s[1]; } return (int)n; }
static inline void i2c_init(i2c_inst_t *i, uint b) { (void)i; (void)b; }
extern int sim_boot; static inline void reset_usb_boot(uint32_t a, uint32_t b) { (void)a; (void)b; sim_boot++; }
typedef int hid_report_type_t;
extern bool sim_suspended; extern int sim_wakeups;
bool tud_hid_n_ready(uint8_t i);
bool tud_hid_keyboard_report(uint8_t id, uint8_t mod, const uint8_t k[6]);
bool tud_hid_report(uint8_t id, const void *r, uint16_t n);
static inline bool tud_suspended(void) { return sim_suspended; }
static inline bool tud_remote_wakeup(void) { sim_wakeups++; return true; }
static inline void tusb_init(void) {}
bool tud_hid_n_report(uint8_t itf, uint8_t id, const void *r, uint16_t n);
// Flash gia: 1 sector 4 KB, XIP_BASE tro sao cho XIP_BASE + CFG_FLASH_OFFSET = sim_flash
#define FLASH_SECTOR_SIZE 4096
#define FLASH_PAGE_SIZE 256
extern uint8_t sim_flash[4096]; extern int sim_flash_writes;
#define XIP_BASE ((uintptr_t)sim_flash - CFG_FLASH_OFFSET)
static inline void flash_range_erase(uint32_t off, size_t n) { (void)off; memset(sim_flash, 0xFF, n); }
static inline void flash_range_program(uint32_t off, const uint8_t *d, size_t n) { (void)off; memcpy(sim_flash, d, n); sim_flash_writes++; }
static inline int flash_safe_execute(void (*fn)(void *), void *p, uint32_t t) { (void)t; fn(p); return 0; }
void tud_task(void);
void tud_hid_set_report_cb(uint8_t itf, uint8_t id, hid_report_type_t t, uint8_t const *b, uint16_t sz);
