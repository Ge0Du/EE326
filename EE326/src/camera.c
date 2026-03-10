#include <asf.h>
#include "camera.h"
#include "tc.h"
#include "ov2640.h"
#include "pio.h"
#include "pdc.h"
#include "conf_board.h"
#include "timer_interface.h"

uint8_t g_image_buffer[CAM_IMAGE_BUFFER_SIZE];
volatile uint32_t g_image_len   = 0;
volatile vsync_state_t g_vsync_state = VSYNC_IDLE;

extern volatile uint8_t g_wifi_command_complete;
extern volatile uint8_t g_wifi_provision_flag;
volatile uint8_t g_network_ready = 0;
volatile uint8_t g_clients_connected = 0;
volatile buf_owner_t g_buf_owner = BUF_OWNER_NONE;

void PIOA_Handler(void) {
	uint32_t isr = PIOA->PIO_ISR;

	if (isr & PIN_OV_VSYNC_MASK) {
		if (g_vsync_state == VSYNC_IDLE) {
			PIOA->PIO_PCMR |= PIO_PCMR_PCEN;
			g_vsync_state = VSYNC_FRAME_STARTED;
			} else if (g_vsync_state == VSYNC_FRAME_STARTED) {
			PIOA->PIO_PCMR  &= ~PIO_PCMR_PCEN;
			PDC_PIOA->PERIPH_PTCR = PERIPH_PTCR_RXTDIS;
			PIOA->PIO_IDR = PIN_OV_VSYNC_MASK;
			g_vsync_state = VSYNC_FRAME_DONE;
		}
	}

	if (isr & PIN_WIFI_COMM_MASK) {
		g_wifi_command_complete = 1;
	}
	if (isr & PIN_WIFI_PROVISION_MASK) {
		g_wifi_provision_flag   = 1;
	}
	if (isr & PIN_NETWORK_STATUS_MASK) {
		g_network_ready = pio_get(PIOA, PIO_TYPE_PIO_INPUT, PIN_NETWORK_STATUS_MASK);
	}
    if (isr & PIN_CLIENT_STATUS_MASK) {
		g_clients_connected = pio_get(PIOA, PIO_TYPE_PIO_INPUT, PIN_CLIENT_STATUS_MASK);
	}
}

void init_vsync_interrupts(void) {
	PIOA->PIO_ODR = PIN_OV_VSYNC_MASK;
	PIOA->PIO_AIMER = PIN_OV_VSYNC_MASK;
	PIOA->PIO_ESR = PIN_OV_VSYNC_MASK;
	PIOA->PIO_REHLSR = PIN_OV_VSYNC_MASK;
}

void configure_twi(void) {
	twi_options_t opt;
	pmc_enable_periph_clk(ID_TWI0);
	gpio_configure_pin(PIO_PA3_IDX, (PIO_PERIPH_A | PIO_DEFAULT));
	gpio_configure_pin(PIO_PA4_IDX, (PIO_PERIPH_A | PIO_DEFAULT));
	opt.master_clk = sysclk_get_cpu_hz();
	opt.speed = 100000;
	twi_master_init(BOARD_TWI, &opt);
}

void pio_capture_init(Pio *p_pio, uint32_t ul_id) {
	pmc_enable_periph_clk(ul_id);
	p_pio->PIO_PCMR &= ~PIO_PCMR_PCEN;
	p_pio->PIO_PCIDR = 0xFFFFFFFF;
}

void init_camera(void) {
	pmc_enable_periph_clk(ID_PIOA);
	gpio_configure_pin(PIN_OV_RST, PIN_OV_RST_FLAGS);

	PIOA->PIO_ABCDSR[0] |= (1u << 17);
	PIOA->PIO_ABCDSR[1] &= ~(1u << 17);
	PIOA->PIO_PDR = (1u << 17);
	PMC->PMC_PCK[1] = PMC_PCK_CSS_PLLA_CLK | PMC_PCK_PRES_CLK_4;
	PMC->PMC_SCER = PMC_SCER_PCK1;
	while (!(PMC->PMC_SR & PMC_SR_PCKRDY1));
	delay_ms(10);

	gpio_set_pin_low(PIN_OV_RST);
	delay_ms(10);
	gpio_set_pin_high(PIN_OV_RST);
	delay_ms(500);

	configure_twi();
	init_vsync_interrupts();
	pio_capture_init(PIOA, ID_PIOA);
}

void configure_camera(void) {
	ov_init(BOARD_TWI);
	ov_configure(BOARD_TWI, JPEG_INIT);
	ov_configure(BOARD_TWI, YUV422);
	ov_configure(BOARD_TWI, JPEG);
	ov_configure(BOARD_TWI, JPEG_320x240);
	
}

uint8_t start_capture(void) {
	while (g_buf_owner == BUF_OWNER_WIFI);
    g_buf_owner = BUF_OWNER_CAMERA;

    uint32_t cap_pins = PIN_OV_CAP_PINS;
    PIOA->PIO_PDR = cap_pins;
    PIOA->PIO_ABCDSR[0] |= cap_pins;
    PIOA->PIO_ABCDSR[1] |= cap_pins;
    PIOA->PIO_PUDR = cap_pins;
    PIOA->PIO_PCIDR = 0xFFFFFFFF;

    volatile uint32_t dummy = PIOA->PIO_PCRHR;
    (void)dummy;

    Pdc *pdc = PDC_PIOA;
    pdc->PERIPH_PTCR = PERIPH_PTCR_RXTDIS;
    pdc->PERIPH_RPR = (uint32_t)(g_image_buffer+1);
    pdc->PERIPH_RCR = CAM_IMAGE_BUFFER_SIZE-1;
    pdc->PERIPH_RNPR = 0;
    pdc->PERIPH_RNCR = 0;
    pdc->PERIPH_PTCR = PERIPH_PTCR_RXTEN;
    PIOA->PIO_PCMR = 0;

    g_vsync_state = VSYNC_IDLE;
    volatile uint32_t isr_flush = PIOA->PIO_ISR;
    (void)isr_flush;
    PIOA->PIO_IER = PIN_OV_VSYNC_MASK;

    while (g_vsync_state != VSYNC_FRAME_DONE);

    g_bytes_captured = (CAM_IMAGE_BUFFER_SIZE - 1) - PDC_PIOA->PERIPH_RCR;

    uint8_t result = find_image_len(g_bytes_captured);
    if (!result) g_buf_owner = BUF_OWNER_NONE;
    return result;
}


uint8_t find_image_len(uint32_t search_len) {
    uint32_t soi_idx = 0;
	uint32_t eoi_idx = 0;
    uint8_t found_soi = 0;
	uint8_t found_sos = 0;
	uint8_t found_eoi = 0;

    for (uint32_t i = 0; i < search_len - 1; i++) {
        if (!found_soi) {
            if (g_image_buffer[i] == 0xFF && g_image_buffer[i+1] == 0xD8) {
                soi_idx = i;
                found_soi = 1;
            }
        } else if (!found_sos) {
            if (g_image_buffer[i] == 0xFF && g_image_buffer[i+1] == 0xDA) {
                found_sos = 1;
            }
        } else {
            if (g_image_buffer[i] == 0xFF && g_image_buffer[i+1] == 0xD9) {
                eoi_idx = i;
                found_eoi = 1;
                break;
            }
        }

        if ((i & 0x1FFF) == 0) {
            WDT->WDT_CR = WDT_CR_KEY_PASSWD | WDT_CR_WDRSTT;
        }
    }

    if (found_soi && found_eoi && eoi_idx > soi_idx) {
        g_image_len = (eoi_idx - soi_idx) + 2;
        return 1;
    }
    g_image_len = 0;
    return 0;
}