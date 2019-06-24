#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#define LED_PORT GPIOA
#define LED_CLOCK RCC_GPIOA
#define LED_GPIO GPIO11

int main(void) {
  rcc_clock_setup_in_hsi_out_48mhz();
  rcc_periph_clock_enable(LED_CLOCK);
  gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_10_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LED_GPIO);

  while(true) {
    gpio_set(LED_PORT, LED_GPIO);
    gpio_clear(LED_PORT, LED_GPIO);
    for(uint32_t i = 0; i < 100; i++) {
      __asm__("nop");
    }
  }

  return 0;
}
