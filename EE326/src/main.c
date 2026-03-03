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
#include <string.h>
#include <stdio.h>
#include "camera.h"
#include "wifi.h"
#include "timer_interface.h"
uint8_t status = 0;
volatile uint8_t g_wifi_buffer[];

void configure_status_pins(void);

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	status = 1;
	sysclk_init();
	board_init();
	status = 2;
	configure_status_pins();
	configure_tc();
	status = 3;
	configure_usart_wifi();
	configure_spi();
	status = 4;
	configure_wifi_comm_pin();
	configure_wifi_provision_pin();
	status = 5;
	
	init_camera();
	configure_camera();
	
	status = 6;
	
	//check comm to ESP
	bool wifi_connected = false;
	while(!wifi_connected) {
		gpio_set_pin_low(PIN_WIFI_RST);
		delay_ms(100);
		gpio_set_pin_high(PIN_WIFI_RST);
		
		write_wifi_command("test", 50);
		if(strstr((char*) g_wifi_buffer, "SUCCESS")) {
			wifi_connected = true;
		}else {
			delay_ms(10000);
		}
	}
	
	status = 8;
	
	// main loop post connection 
	while (1) {
		//WIFI Provision Button
		if (g_wifi_provision_flag) {
			write_wifi_command("provision", 5);
			g_wifi_provision_flag = 0;
			while (!strstr((char*)g_wifi_buffer, "SUCCESS")) {
				write_wifi_command("test", 5);
				delay_ms(5000);
			}
		}
		//check network ready and client connected status
		bool network_ready = ioport_get_pin_level(PIN_NETWORK_STATUS);
		bool clients_connected = ioport_get_pin_level(PIN_CLIENT_STATUS);
		if(network_ready && clients_connected) {
			if(start_capture()) {
				//find image
				find_image_len();
				//send image to esp
				write_image_to_web();
			}
		}
		delay_ms(500);
	}
}




void configure_status_pins(void) {
	pmc_enable_periph_clk(ID_PIOA);
	ioport_set_pin_dir(PIN_NETWORK_STATUS, IOPORT_DIR_INPUT);
	ioport_set_pin_dir(PIN_CLIENT_STATUS, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(PIN_NETWORK_STATUS, IOPORT_MODE_PULLDOWN);
	ioport_set_pin_mode(PIN_CLIENT_STATUS, IOPORT_MODE_PULLDOWN);
}

