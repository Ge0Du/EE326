#include <sam4s8b.h>
#include <asf.h>
#include "pio.h"
#include "pdc.h"
#include "conf_board.h"
#include "camera.h"
#include "ov2640.h"

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

volatile uint32_t g_vsync_flag = 0;
uint8_t g_image_buffer[CAM_IMAGE_BUFFER_SIZE]; 
volatile uint32_t g_image_len = 0;

//VSYNC Interrupt Handler
void vsync_handler(uint32_t ul_id, uint32_t ul_mask) {
	unused(ul_id);
	unused(ul_mask);
	g_vsync_flag = 1;
}

void init_vsync_interrupts(void) {
	pio_handler_set(PIOC, ID_PIOC, PIN_OV_VSYNC, PIN_OV_VSYNC_FLAGS, vsync_handler);
	pio_enable_interrupt(PIOC, PIN_OV_VSYNC);
	NVIC_EnableIRQ((IRQn_Type) ID_PIOC);
}

void configure_twi(void) {
	twi_options_t opt;
	pmc_enable_periph_clk(ID_TWI0);
	opt.master_clk = sysclk_get_cpu_hz();
	opt.speed = 100000;
	twi_master_init(BOARD_TWI, &opt);
}

void pio_capture_init(Pio *p_pio, uint32_t ul_id) {
	pmc_enable_periph_clk(ul_id);
	p_pio->PIO_PCMR = PIO_PCMR_PCEN | PIO_PCMR_DSIZE_BYTE | PIO_PCMR_ALWYS;
	
	p_pio->PIO_PCIDR = 0xFFFFFFFF; 
}

void init_camera(void) {
	//Camera Pins Config
	gpio_configure_pin(PIN_OV_RST, PIN_OV_RST_FLAGS);
	gpio_configure_pin(PIN_OV_XCLK, PIN_OV_XCLK_FLAGS);
	
	//8 bit data bus and vsync Config
	gpio_configure_pin(PIN_OV_DATA_BUS, PIN_OV_DATA_BUS_FLAGS);
	
	init_vsync_interrupts();
	
	pio_capture_init(PIOC, ID_PIOC);
	
	//init xclk 
	pmc_switch_pck_to_mainck(PMC_PCK_0, PMC_PCK_PRES_CLK_2);
	pmc_enable_pck(PMC_PCK_0);
	
	//TWI configure
	configure_twi();
	
	//reset camera hardware
	gpio_set_pin_low(PIN_OV_RST);
	delay_ms(100);
	gpio_set_pin_high(PIN_OV_RST);
}

void configure_camera(void){
	ov_configure(BOARD_TWI, JPEG_INIT);
	ov_configure(BOARD_TWI, YUV422);
	ov_configure(BOARD_TWI, JPEG);
	ov_configure(BOARD_TWI, JPEG_320x240);
}

uint8_t start_capture(void) {
	g_vsync_flag = 0;
	
	Pdc *p_pio_pdc = (Pdc *)((uint32_t)PIOC + 0x100);
	pdc_packet_t pdc_pio_packet;
	
	pdc_pio_packet.ul_addr = (uint32_t)g_image_buffer;
	pdc_pio_packet.ul_size = CAM_IMAGE_BUFFER_SIZE; 
	pdc_rx_init(p_pio_pdc, &pdc_pio_packet, NULL);
	pdc_enable_transfer(p_pio_pdc, PERIPH_PTCR_RXTEN);
		
	uint32_t timeout = 0;
	
	//search for rising edge
	while (g_vsync_flag == 0) {
		if(timeout ++ > 1000000) {
			pdc_disable_transfer(p_pio_pdc, PERIPH_PTCR_RXTEN);
			return 0;// timeout
		}
	}
	
	pdc_disable_transfer(p_pio_pdc, PERIPH_PTCR_RXTEN);
	
	if(find_image_len()) {
		return 1; //correct transfer
	}
	return 0; //incorrect transfer.
}

uint8_t find_image_len(void) {
	uint32_t soi_idx = 0;
	uint32_t eoi_idx = 0;
	uint8_t found_soi = 0;
	uint8_t found_eoi = 0;
	for (uint32_t i = 0; i < CAM_IMAGE_BUFFER_SIZE-1; i++){
		if(!found_soi && g_image_buffer[i] == 0xFF && g_image_buffer[i+1] == 0xD8) {
			soi_idx = i;
			found_soi = 1;
		}
		if(found_soi && g_image_buffer[i] == 0xFF && g_image_buffer[i+1] == 0xD9) {
			eoi_idx = i;
			found_eoi = 1;
			break;
		}
	}
	if(found_eoi && found_soi && (eoi_idx > soi_idx)) {
		g_image_len = (eoi_idx - soi_idx) + 2;
		return 1;
	}
	g_image_len = 0;
	return 0;
}