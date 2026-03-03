#include <sam4s8b.h>
#include <asf.h>
#include "pio.h"
#include "pmc.h"
#include "wifi.h"
#include "camera.h"
#include "conf_board.h"
#include <string.h>
#ifndef PIOC
#define PIOC       ((Pio    *)0x400E1200U)
#define ID_PIOC    (13U)
#endif
#ifndef PIOA
#define PIOA       ((Pio    *)0x400E0E00U)
#define ID_PIOA    (11U)
#endif
#ifndef PIO_TYPE_PIO_INPUT
#define PIO_TYPE_PIO_INPUT 0
#endif



volatile uint32_t g_wifi_buf_idx = 0;
volatile uint8_t g_wifi_command_complete = 0;
volatile uint8_t g_wifi_provision_flag = 0;
volatile uint32_t g_spi_transfer_idx = 0;
volatile uint32_t g_counts = 0;

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
		uint16_t dummy;
		spi_read(WIFI_SPI, &dummy, 0);
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

void configure_wifi_comm_pin(void) {
	pmc_enable_periph_clk(ID_PIOC);
	pio_set_input(PIOC, PIN_WIFI_COMM_MASK, PIO_DEFAULT);
	pio_handler_set(PIOC, ID_PIOC, PIN_WIFI_COMM_MASK, PIO_IT_RISE_EDGE, wifi_command_resposne_handler);
	pio_enable_interrupt(PIOC, PIN_WIFI_COMM_MASK);
	NVIC_EnableIRQ((IRQn_Type)ID_PIOC);
}

void configure_wifi_provision_pin(void) {
	pmc_enable_periph_clk(ID_PIOC);
	pio_set_input(PIOC, PIN_WIFI_PROVISION_MASK, PIO_PULLUP);
	pio_handler_set(PIOC, ID_PIOC, PIN_WIFI_PROVISION_MASK, PIO_IT_FALL_EDGE, wifi_provision_handler);
	pio_enable_interrupt(PIOC, PIN_WIFI_PROVISION_MASK);
	NVIC_EnableIRQ((IRQn_Type)ID_PIOC);
}

void configure_spi(void) {
	pmc_enable_periph_clk(WIFI_SPI_ID); 
	pio_configure(PIOA, PIO_PERIPH_A, PIO_PA12A_MISO | PIO_PA13A_MOSI | PIO_PA14A_SPCK | PIO_PA11A_NPCS0, PIO_DEFAULT);
	spi_disable(WIFI_SPI);
	spi_reset(WIFI_SPI);
	spi_set_slave_mode(WIFI_SPI);
	spi_set_bits_per_transfer(WIFI_SPI, 0, SPI_CSR_BITS_8_BIT);
	spi_disable_interrupt(WIFI_SPI, SPI_IER_RDRF);
	spi_enable(WIFI_SPI);
}

void write_wifi_command(char* comm, uint8_t cnt) {
	g_wifi_command_complete = 0;
	g_counts = 0;
	
	while(*comm) {
		while(!(BOARD_USART->US_CSR & US_CSR_TXRDY));
		usart_write(BOARD_USART, *comm++);
	}
	while(!(BOARD_USART->US_CSR & US_CSR_TXRDY));
	usart_write(BOARD_USART, '\r');
	while(!(BOARD_USART->US_CSR & US_CSR_TXRDY));
	usart_write(BOARD_USART, '\n');
	
	while(!g_wifi_command_complete && g_counts < cnt);
}

void write_image_to_web(void) {
	if (g_image_len == 0) return;
	
	char cmd[64];
	sprintf(cmd, "image_transfer %u", (unsigned int)g_image_len);
	write_wifi_command(cmd, 10);
	
	while(pio_get(PIOC, PIO_TYPE_PIO_INPUT, PIN_WIFI_COMM_MASK) == 0);
	g_wifi_command_complete = 0; 

	Pdc *p_spi_pdc = spi_get_pdc_base(WIFI_SPI);
	pdc_packet_t pdc_spi_packet; 
	pdc_spi_packet.ul_addr = (uint32_t)g_image_buffer;
	pdc_spi_packet.ul_size = g_image_len;
	
	pdc_tx_init(p_spi_pdc, &pdc_spi_packet, NULL);
	pdc_enable_transfer(p_spi_pdc, PERIPH_PTCR_TXTEN); 
	while(!g_wifi_command_complete);
	pdc_disable_transfer(p_spi_pdc, PERIPH_PTCR_TXTEN);
}