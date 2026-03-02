#include "wifi.h"
#include "camera.h"
#include <string.h> 
#include <stdio.h>


g_wifi_buf_idx = 0;
g_wifi_command_complete = 0;
g_wifi_provision_flag = 0;
g_spi_transfer_idx = 0;
g_counts = 0;

void wifi_usart_handler(void) {
	uint32_t status = usart_get_status(BOARD_USART);
	if(status & US_CSR_RXRDY) {
		uint32_t received_byte;
		usart_read(BOARD_USART, &received_byte);
		process_incoming_byte_wifi((uint8_t)received_byte);
	}
}

void process_incoming_byte_wifi(uint8_t in_byte) {
	if(g_wifi_buf_idx < sizeof(g_wifi_buffer)) {
		g_wifi_buffer[g_wifi_buf_idx++] = in_byte;
	}
}

void wifi_command_resposne_handler(uint32_t ul_id, uint32_t ul_mask) {
	unused(ul_id);
	unused(ul_mask);
	g_wifi_command_complete = 1;
}

void process_data_wifi(void) {
	if(g_wifi_buf_idx < WIFI_BUFFER_SIZE) {
		g_wifi_buffer[g_wifi_buf_idx] = '\0';
	} else {
		g_wifi_buffer[WIFI_BUFFER_SIZE -1] = '\0'; 
	}
	if(strstr((char*) g_wifi_buffer, "SUCCESS")) {
		g_wifi_command_complete = 1;
	} else if (strstr((char*) g_wifi_buffer, "invalid command")) {
		printf("Error: ESP32 received an invalid command \n");
	}
	g_wifi_buf_idx = 0;
	memset((void*)g_wifi_buffer, 0, WIFI_BUFFER_SIZE);
}

void wifi_provision_handler(uint32_t ul_id, uint32_t ul_mask) {
	unused(ul_id);
	unused(ul_mask);
	g_wifi_provision_flag = 1;
}

void wifi_spi_handler(void) {
	uint32_t status = spi_read_status(WIFI_SPI);
	
	if (status & SPI_SR_RDRF) {
		if (g_spi_transfer_idx < g_image_len) {
			spi_write(WIFI_SPI, g_image_buffer[g_spi_transfer_idx++], 0, 0);
		} else {
			spi_write(WIFI_SPI, 0x00, 0, 0); 
		}
	}
}

void configure_usart_wifi(void) {
	const sam_usart_opt_t settings = {
		BOARD_USART_BAUDRATE,
		US_MR_CHRL_8_BIT,
		US_MR_PAR_NO,
		US_MR_NBSTOP_1_BIT,
		US_MR_CHMODE_NORMAL
	};
	sysclk_enable_peripheral_clock(BOARD_ID_USART);
	usart_init_rs232(BOARD_USART, &settings, sysclk_get_main_hz());
	usart_enable_tx(BOARD_USART);
	usart_enable_rx(BOARD_USART);
	usart_enable_interrupt(BOARD_USART, US_IER_RXRDY); 
	NVIC_EnableIRQ((IRQn_Type)BOARD_ID_USART);
}

void configure_spi(void) {
	pmc_enable_periph_clk(WIFI_SPI_ID); 
	spi_disable(WIFI_SPI);
	spi_reset(WIFI_SPI);
	spi_set_lastxfer_disable(WIFI_SPI);
	spi_set_peripheral_chip_select_value(WIFI_SPI, 0);
	spi_set_fixed_peripheral_select(WIFI_SPI);
	spi_set_slave_mode(WIFI_SPI);
	spi_set_bits_per_transfer(WIFI_SPI, 0, SPI_CSR_BITS_8_BIT);
	spi_enable_interrupt(WIFI_SPI, SPI_IER_RDRF);
	NVIC_EnableIRQ((IRQn_Type)WIFI_SPI_ID); 
	spi_enable(WIFI_SPI);
}

void write_wifi_command(char* comm, uint8_t cnt) {
	g_wifi_command_complete = 0;
	g_counts = 0;
	
	while(*comm) {
		while(!(BOARD_USART->US_CSR & US_CSR_TXRDY));
		usart_write(BOARD_USART, *comm++);
	}
	while(!g_wifi_command_complete && g_counts < cnt);
}

void write_image_to_web(void) {
	if (g_image_len == 0) return;
	g_spi_transfer_idx = 0;
	spi_write(WIFI_SPI, g_image_buffer[g_spi_transfer_idx++], 0, 0);
	
	char cmd[64];
	sprintf(cmd, "image_transfer %u\n", (unsigned int)g_image_len);
	write_wifi_command(cmd, 5);
	while(!g_wifi_command_complete);
}