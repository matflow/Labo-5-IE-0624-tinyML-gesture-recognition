/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "main_functions.h"
// This is the default main used on systems that have the standard C entry
// point. Other devices (for example FreeRTOS or ESP32) that have different
// requirements for entry code (like an app_main function) should specialize
// this main.cc file in a target-specific subfolder.
#include <errno.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/systick.h>
#include "printf.h"
#include <libopencm3/stm32/spi.h>
#include <gyro.h>
//#include <console.h>
//#include <clock.h>


extern "C" {
    void sys_tick_handler(void);
}

extern "C" void _putchar(char character)
{
  usart_send_blocking(USART1, character); /* USART1: Send byte. */
}

// setup de spi para comunicación con regs de gyro
void spi_setup(void)
{
    rcc_periph_clock_enable(RCC_SPI5);
	/* Enable the GPIO ports whose pins we are using */
    rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_GPIOF);

    /* Chip select line */
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);
    gpio_set(GPIOC, GPIO1);

	/*gpio_mode_setup(GPIOF, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN,
			GPIO7 | GPIO8 | GPIO9);*/
    gpio_mode_setup(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE,
		GPIO7 | GPIO8 | GPIO9);   
	gpio_set_af(GPIOF, GPIO_AF5, GPIO7 | GPIO8 | GPIO9);
	/*gpio_set_output_options(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ,
				GPIO7 | GPIO9);*/

    // spi initialization
    spi_set_master_mode(SPI5);
	spi_set_baudrate_prescaler(SPI5, SPI_CR1_BR_FPCLK_DIV_64);
	spi_set_clock_polarity_0(SPI5);
	spi_set_clock_phase_0(SPI5);
	spi_set_full_duplex_mode(SPI5);
	spi_set_unidirectional_mode(SPI5); /* bidirectional but in 3-wire */
    //spi_set_data_size(SPI5, SPI_CR2_DS_8BIT);  //
	spi_enable_software_slave_management(SPI5);
	spi_send_msb_first(SPI5);
	spi_set_nss_high(SPI5);
    //spi_fifo_reception_threshold_8bit(SPI5); //
    SPI_I2SCFGR(SPI5) &= ~SPI_I2SCFGR_I2SMOD;  //
	spi_enable(SPI5);
}

static void usart_setup(void)
{
	/* Setup USART1 parameters. */
	usart_set_baudrate(USART1, 115200);
	usart_set_databits(USART1, 8);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_mode(USART1, USART_MODE_TX);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

	/* Finally enable the USART. */
	usart_enable(USART1);
}

static void clock_setup() {
    // First, let's ensure that our clock is running off the high-speed internal
    // oscillator (HSI) at 48MHz.
    //rcc_clock_setup_in_hsi_out_48mhz();

    // Since our LED is on GPIO bank G, we need to enable
    // the peripheral clock to this GPIO bank in order to use it.
    //El GPIOA es por el led de USART
    rcc_periph_clock_enable(RCC_GPIOG);
    rcc_periph_clock_enable(RCC_GPIOA);

    // In order to use our UART, we must enable the clock to it as well.
    rcc_periph_clock_enable(RCC_USART1);
}
static void systick_setup() {
    // Set the systick clock source to our main clock
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
    // Clear the Current Value Register so that we start at 0
    STK_CVR = 0;
    // In order to trigger an interrupt every millisecond, we can set the reload
    // value to be the speed of the processor / 1000 -1
    systick_set_reload(rcc_ahb_frequency / 1000 - 1);
    // Enable interrupts from the system tick clock
    systick_interrupt_enable();
    // Enable the system tick counter
    systick_counter_enable();
}

// Storage for our monotonic system clock.
// Note that it needs to be volatile since we're modifying it from an interrupt.
static volatile uint64_t _millis = 0;

uint64_t millis() {
    return _millis;
}

// This is our interrupt handler for the systick reload interrupt.
// The full list of interrupt services routines that can be implemented is
// listed in libopencm3/include/libopencm3/stm32/f0/nvic.h
void sys_tick_handler(void) {
    // Increment our monotonic clock
    _millis++;
}
/**
 * Delay for a real number of milliseconds
 */

void delay(uint64_t duration) {
    const uint64_t until = millis() + duration;
    while (millis() < until);
}

static void gpio_setup() {
    // Our test LED is connected to Port G pin 13, so let's set it as output
    gpio_mode_setup(GPIOG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);

	/* Setup GPIO pins for USART1 transmit. */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);

	/* Setup USART2 TX pin as alternate function. */
	gpio_set_af(GPIOA, GPIO_AF7, GPIO9);
}

void* __dso_handle;


int main(int argc, char* argv[]) {

    clock_setup();
    systick_setup();
    gpio_setup();
    usart_setup();
    spi_setup();
    printf("Initial setup done!\n");

    // Config de CTRL_REG1
    gpio_clear(GPIOC, GPIO1);         // baja CS
	spi_send(SPI5, GYR_CTRL_REG1);   // indica cual reg configurar 
	spi_read(SPI5);                  // lee respuesta
	spi_send(SPI5, GYR_CTRL_REG1_PD | GYR_CTRL_REG1_XEN |   // escribe el valor del registro
			GYR_CTRL_REG1_YEN | GYR_CTRL_REG1_ZEN |
			(3 << GYR_CTRL_REG1_BW_SHIFT));
	spi_read(SPI5);                    // lee respuesta
	gpio_set(GPIOC, GPIO1);             // levanta CS


    // Config de CTRL_REG4
    gpio_clear(GPIOC, GPIO1);
	spi_send(SPI5, GYR_CTRL_REG4);
	spi_read(SPI5);
	spi_send(SPI5, (1 << GYR_CTRL_REG4_FS_SHIFT));  // valor para 500 dps
	spi_read(SPI5);
	gpio_set(GPIOC, GPIO1);


//  while(true){
    gpio_set(GPIOG, GPIO13);
    delay(500);
    gpio_clear(GPIOG, GPIO13);
    delay(500);
    printf("Blink!\n");
//  }

  setup();
  printf("Second setup done!\n");
  while (true){
    loop();
  }

}
