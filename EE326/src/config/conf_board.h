/**
 * \file
 *
 * \brief Board configuration.
 *
 * Copyright (c) 2012-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef CONF_BOARD_H_INCLUDED
#define CONF_BOARD_H_INCLUDED

/** Enable Com Port. */
#define CONF_BOARD_UART_CONSOLE

//Clock Freq
#define BOARD_FREQ_SLCK_XTAL      (32768U)
#define BOARD_FREQ_SLCK_BYPASS    (32768U)
#define BOARD_FREQ_MAINCK_XTAL    (12000000U)
#define BOARD_FREQ_MAINCK_BYPASS  (12000000U)
#define BOARD_OSC_STARTUP_US      (15625U)

// WiFi USART Config
#define BOARD_ID_USART			ID_USART0
#define BOARD_USART				USART0
#define BOARD_USART_BAUDRATE	115200 

// WiFi SPI Config
#define WIFI_SPI				SPI
#define WIFI_SPI_ID				ID_SPI
#define WIFI_BUFFER_SIZE		1024


// WiFi-MCU Pins
#define PIN_WIFI_RST			PIO_PA19_IDX
#define PIN_WIFI_RST_FLAGS		(PIO_TYPE_PIO_OUTPUT_1 | PIO_DEFAULT)

// WiFi Handshake Pins
#define PIN_WIFI_COMM_IDX		PIO_PC16_IDX
#define PIN_WIFI_COMM_MASK		(1u << 16)
#define PIN_WIFI_COMM_FLAGS		(PIO_TYPE_PIO_INPUT | PIO_DEFAULT)

// Provision Button
#define PIN_WIFI_PROVISION_IDX	PIO_PC11_IDX
#define PIN_WIFI_PROVISION_MASK	(1u << 11)

//Webpage Status Pin
#define PIN_NETWORK_STATUS		PIO_PA24_IDX
#define PIN_CLIENT_STATUS		PIO_PA25_IDX

// Camera Pin
#define PIN_OV_RST				PIO_PA20_IDX
#define PIN_OV_RST_FLAGS		(PIO_TYPE_PIO_OUTPUT_1 | PIO_DEFAULT)

#define PIN_OV_XCLK				PIO_PA21_IDX
#define PIN_OV_XCLK_FLAGS		(PIO_PERIPH_B | PIO_DEFAULT)

#define PIN_OV_VSYNC			PIO_PA15_IDX
#define PIN_OV_VSYNC_FLAGS		(PIO_TYPE_PIO_INPUT | PIO_DEFAULT)

#define PIN_OV_DATA_BUS			(0xFF000000)
#define PIN_OV_DATA_BUS_FLAGS	(PIO_INPUT | PIO_DEFAULT)

//TWI
#define BOARD_TWI				TWI0
#define BOARD_ID_TWI			ID_TWI0

//! [tc_define_peripheral]
/* Use TC Peripheral 0. */
#define TC             TC0
#define TC_PERIPHERAL  0
//! [tc_define_peripheral]

//! [tc_define_ch1]
/* Configure TC0 channel 1 as waveform output. */
#define TC_CHANNEL_WAVEFORM 1
#define ID_TC_WAVEFORM      ID_TC1
#define PIN_TC_WAVEFORM     PIN_TC0_TIOA1
#define PIN_TC_WAVEFORM_MUX PIN_TC0_TIOA1_MUX
//! [tc_define_ch1]

//! [tc_define_ch2]
/* Configure TC0 channel 2 as capture input. */
#define TC_CHANNEL_CAPTURE 2
#define ID_TC_CAPTURE ID_TC2
#define PIN_TC_CAPTURE PIN_TC0_TIOA2
#define PIN_TC_CAPTURE_MUX PIN_TC0_TIOA2_MUX
//! [tc_define_ch2]

//! [tc_define_irq_handler]
/* Use TC2_Handler for TC capture interrupt. */
#define TC_Handler  TC2_Handler
#define TC_IRQn     TC2_IRQn
//! [tc_define_irq_handler]

#endif /* CONF_BOARD_H_INCLUDED */
