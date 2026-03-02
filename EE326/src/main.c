/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>
#include "camera.h"
#include "wifi.h"

void configure_timer(void);

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	sysclk_init();
	board_init();
	
	configure_timer();
	configure_usart_wifi();
	configure_spi();
	configure_wifi_comm_pin();
	configure_wifi_provision_pin();
	
	init_camera();
	configure_camera();
	
	gpio_set_pin_low(PIN_WIFI_RST);
	delay_ms(100);
	gpio_set_pin_high(PIN_WIFI_RST);
	
	while(1) {
		write_wifi_command("test", 10);
		if (strstr((char*)g_wifi_buffer, "SUCCESS")) {
			break;
		}
		delay_ms(10000);
	}
	// main loop post connection
	while (1) {
		if (g_wifi_provision_flag) {
			write_wifi_command("provision", 5);
			g_wifi_provision_flag = 0;
			while (!strstr((char*)g_wifi_buffer, "SUCCESS")) {
				write_wifi_command("test", 5);
				delay_ms(5000);
			}
		}
		//Setup netwrok and client logic NEED TO WORK AND FIX THIS
		bool network_ready = true;
		bool clients_connected = true;
		if(network_ready && clients_connected) {
			if(start_capture()) {
				write_image_to_web();
			}
		}
	}
}

void TC0_Handler(void) {
	tc_get_staus(TC0, 0);
	g_counts++;
}

void configure_timer(void) {
	pmc_enable_periph_clk(ID_TC0); 
	tc_init(TC0, 0, TC_CMR_CPCTRG | TC_CMR_TCCLKS_TIMER_CLOCK4); 
	tc_write_rc(TC0, 0, (sysclk_get_cpu_hz()/128));
	tc_enable_interrupt(TC0, 0, TC_IER_CPCS);
	NVIC_EnableIRQ(TC0_IRQn);
	tc_start(TC0, 0);
}