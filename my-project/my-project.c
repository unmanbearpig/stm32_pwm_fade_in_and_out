#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <math.h>

// code mostly copied from http://www.kaltpost.de/?page_id=412

void pwm_init_timer(volatile uint32_t *reg,
                    uint32_t en,
                    uint32_t timer_peripheral,
                    uint32_t prescaler,
                    uint32_t period) {
  rcc_peripheral_enable_clock(reg, en);

  // Not a function on STM32F1, works without it
  // timer_reset(timer_peripheral);

  timer_set_mode(timer_peripheral,
                 TIM_CR1_CKD_CK_INT,
                 TIM_CR1_CMS_EDGE,
                 TIM_CR1_DIR_UP);

  timer_set_prescaler(timer_peripheral, prescaler);

  // Only valid for advanced timers (1 and 8)
  // I'm using basic timer (2)
  timer_set_repetition_counter(timer_peripheral, 0);

  timer_enable_preload(timer_peripheral);
  timer_continuous_mode(timer_peripheral);
  timer_set_period(timer_peripheral, period);
}

void pwm_init_output_channel(uint32_t timer_peripheral,
                             enum tim_oc_id oc_id,
                             volatile uint32_t *gpio_reg,
                             uint32_t gpio_en,
                             uint32_t gpio_port,
                             uint16_t gpio_pin) {
  rcc_peripheral_enable_clock(gpio_reg, gpio_en);

  gpio_set_mode(gpio_port, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                gpio_pin);

  timer_disable_oc_output(timer_peripheral, oc_id);
  timer_set_oc_mode(timer_peripheral, oc_id, TIM_OCM_PWM1);
  timer_set_oc_value(timer_peripheral, oc_id, 0);
  timer_enable_oc_output(timer_peripheral, oc_id);
}

void wait_nops(uint32_t nops) {
  while(nops != 0) {
    __asm__("nop");
    nops--;
  }
}

void fade_in_and_out(const uint32_t timer_peripheral,
                     const enum tim_oc_id oc_id,
                     const uint32_t max_value) {
  const uint32_t nops = 10;

  uint32_t i = 0;

  while(true) {
    // fade in
    for(; i < max_value; i++) {
      timer_set_oc_value(timer_peripheral, oc_id, i);

      // some whatever math to make it fade nicer
      // any number would do
      // TODO: can I use some timers and interrupts and counters
      //       to update the PWM duty cycle?
      wait_nops(nops + max_value / i * 10);
    }

    // fade out
    for(; i != 0; i--) {
      timer_set_oc_value(timer_peripheral, oc_id, i);
      wait_nops(nops + max_value / i * 10);
    }
  }
}

int main(void) {
  const uint32_t max_value = 0xFFFF;

  rcc_clock_setup_in_hse_8mhz_out_72mhz();

  // Using pin PA3 for PWM
  // PA3 is connected to channel 4 (TIM_OC4) (GPIO pin GPIO_TIM2_CH4)
  // of timer 2 (TIM2)

  pwm_init_timer(&RCC_APB1ENR, RCC_APB1ENR_TIM2EN, TIM2, 2, max_value);
  pwm_init_output_channel(TIM2,
                          TIM_OC4,
                          &RCC_APB2ENR,
                          RCC_APB2ENR_IOPAEN,
                          GPIOA,
                          GPIO_TIM2_CH4);
  timer_enable_counter(TIM2);

  fade_in_and_out(TIM2, TIM_OC4, max_value);
  return 0;
}
