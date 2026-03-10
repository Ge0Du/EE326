#include <asf.h>
#include <string.h>
#include <stdio.h>
#include "camera.h"
#include "wifi.h"
#include "timer_interface.h"
#include "ov2640.h"

uint8_t status = 0;
volatile uint8_t g_wifi_buffer[WIFI_BUFFER_SIZE];
void configure_status_pins(void);

int main(void) {
	sysclk_init();
	board_init();
	status = 2;
	configure_status_pins();
	configure_tc();
	NVIC_SetPriority(PIOA_IRQn, 0);
	NVIC_SetPriority(TC0_IRQn,  4);
	NVIC_EnableIRQ(PIOA_IRQn);
	status = 3;
	configure_usart_wifi();
	configure_spi();
	status = 4;
	configure_wifi_comm_pin();
	configure_wifi_provision_pin();
	status = 5;
	init_camera();
	status = 1;
	delay_ms(500);
	configure_camera();
	status = 6;

	// Reset WiFi and wait for it to boot
	gpio_set_pin_low(PIN_WIFI_RST);
	delay_ms(100);
	gpio_set_pin_high(PIN_WIFI_RST);
	delay_ms(2000);
	g_wifi_provision_flag = 0;

	//Provision Flag Check
	bool wifi_connected = false;
	while (!wifi_connected) {
		while (!g_network_ready) {
			if (g_wifi_provision_flag) {
				write_wifi_command("provision", 5);
				g_wifi_provision_flag = 0;
				g_network_ready = 0;
				while(!g_network_ready) {
					if (g_wifi_provision_flag) {
						write_wifi_command("provision", 5);
						g_wifi_provision_flag = 0;
					}
				}
			}
		}

		write_wifi_command("test", 60);
		if (strstr((char*)g_wifi_buffer, "SUCCESS")) {
			wifi_connected = true;
			} else {
			delay_ms(10000);
			gpio_set_pin_low(PIN_WIFI_RST);
			delay_ms(100);
			gpio_set_pin_high(PIN_WIFI_RST);
			delay_ms(2000);
			g_wifi_provision_flag = 0;
		}
	}

	write_wifi_command("set wlan_gpio 27", 0);
	write_wifi_command("set websocket_gpio 26", 0);
	write_wifi_command("set ap_gpio 25", 0);
	write_wifi_command("set comm_gpio 21", 0);
	write_wifi_command("set net_gpio 22", 0);
	write_wifi_command("set clients_gpio 32", 0);
	write_wifi_command("set spi_baud 4000000", 0);

	status = 8;

	while (1) {
		if (g_wifi_provision_flag) {
			write_wifi_command("provision", 5);
			g_wifi_provision_flag = 0;
			g_network_ready = 0;
			while (!g_network_ready) {
				if (g_wifi_provision_flag) {
					write_wifi_command("provision", 5);
					g_wifi_provision_flag = 0;
				}
			}
		}
		if (g_network_ready && g_clients_connected) {
			write_image_to_web();
			start_capture();
		}
	}
}

void configure_status_pins(void) {
	pmc_enable_periph_clk(ID_PIOA);
	gpio_configure_pin(PIN_WIFI_RST, PIO_TYPE_PIO_OUTPUT_1 | PIO_DEFAULT);
	pio_set_input(PIOA, PIN_WIFI_COMM_MASK, PIO_DEFAULT);
	ioport_set_pin_dir(PIN_NETWORK_STATUS, IOPORT_DIR_INPUT);
	ioport_set_pin_dir(PIN_CLIENT_STATUS,  IOPORT_DIR_INPUT);
	ioport_set_pin_mode(PIN_NETWORK_STATUS, IOPORT_MODE_PULLDOWN);
	ioport_set_pin_mode(PIN_CLIENT_STATUS,  IOPORT_MODE_PULLDOWN);
	PIOA->PIO_AIMER = PIN_NETWORK_STATUS_MASK | PIN_CLIENT_STATUS_MASK;
	PIOA->PIO_ESR = PIN_NETWORK_STATUS_MASK | PIN_CLIENT_STATUS_MASK;
	PIOA->PIO_REHLSR = PIN_NETWORK_STATUS_MASK | PIN_CLIENT_STATUS_MASK;
	PIOA->PIO_IER = PIN_NETWORK_STATUS_MASK | PIN_CLIENT_STATUS_MASK;
	PIOA->PIO_IDR = PIO_PA3 | PIO_PA4;
}