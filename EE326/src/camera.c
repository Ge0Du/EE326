#include "camera.h"

g_vsync_flag = 0;
g_image_buffer[CAM_IMAGE_BUFFER_SIZE]; 
g_image_len = 0;

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
	p_pio->PIO_PCMR = PIO_PCMR_PCEN | PIO_PCMR_DSIZE_BYTE;
}

void init_camera(void) {
	//Camera Pins Config
	gpio_configure_pin(PIN_OV_RST, PIN_OV_RST_FLAGS);
	gpio_configure_pin(PIN_OV_XCLK, PIN_OV_XCLK_FLAGS);
	
	//8 bit data bus and vsync Config
	gpio_configure_pin(PIN_OV_DATA_BUS, PIN_OV_DATA_BUS_FLAGS);
	
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
	
	//search for rising edge
	while (g_vsync_flag == 0);
	
	pio_capture_to_buffer(PIOC, g_image_buffer, CAM_IMAGE_BUFFER_SIZE);
	
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