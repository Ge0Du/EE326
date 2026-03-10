#ifndef CONF_BOARD_H_INCLUDED
#define CONF_BOARD_H_INCLUDED


#define CONF_BOARD_UART_CONSOLE

//Clock
#define BOARD_FREQ_SLCK_XTAL      (32768U)
#define BOARD_FREQ_SLCK_BYPASS    (32768U)
#define BOARD_FREQ_MAINCK_XTAL    (12000000U)
#define BOARD_FREQ_MAINCK_BYPASS  (12000000U)
#define BOARD_OSC_STARTUP_US      (15625U)

// USART Conf
#define BOARD_ID_USART          ID_USART0
#define BOARD_USART             USART0
#define BOARD_USART_BAUDRATE    115200

// WIFI SPI
#define WIFI_SPI                SPI
#define WIFI_SPI_ID             ID_SPI
#define WIFI_BUFFER_SIZE        1024

//Wifi Pin
#define PIN_WIFI_RST_IDX        PIO_PA7_IDX
#define PIN_WIFI_RST            PIO_PA7_IDX
#define PIN_WIFI_RST_FLAGS      (PIO_TYPE_PIO_OUTPUT_1 | PIO_DEFAULT)
#define PIN_WIFI_COMM_IDX       PIO_PA19_IDX
#define PIN_WIFI_COMM_MASK      (1u << 19)
#define PIN_WIFI_COMM_FLAGS     (PIO_TYPE_PIO_INPUT | PIO_DEFAULT)
#define PIN_WIFI_PROVISION_IDX  PIO_PA10_IDX
#define PIN_WIFI_PROVISION_MASK (1u << 10)
#define PIN_NETWORK_STATUS      PIO_PA1_IDX
#define PIN_CLIENT_STATUS       PIO_PA2_IDX
#define PIN_NETWORK_STATUS_MASK  (1u << 1)
#define PIN_CLIENT_STATUS_MASK   (1u << 2)

// Camera Pins
#define PIN_OV_RST              PIO_PA20_IDX
#define PIN_OV_RST_FLAGS        (PIO_TYPE_PIO_OUTPUT_1 | PIO_DEFAULT)
#define PIN_OV_XCLK             PIO_PA17_IDX
#define PIN_OV_XCLK_FLAGS       (PIO_PERIPH_B | PIO_DEFAULT)
#define PIN_OV_VSYNC_IDX        PIO_PA15_IDX
#define PIN_OV_VSYNC            PIO_PA15_IDX
#define PIN_OV_VSYNC_MASK       (1u << 15)
#define PIN_OV_DATA_BUS         (0xFF000000)
#define PIN_OV_DATA_BUS_FLAGS   (PIO_INPUT | PIO_DEFAULT)
#define PIN_OV_HREF_MASK        (1u << 16)
#define PIN_OV_PCLK_MASK        (1u << 23)
#define PIN_OV_CAP_PINS         (PIN_OV_DATA_BUS | PIN_OV_PCLK_MASK | PIN_OV_HREF_MASK)

// TWI
#define BOARD_TWI               TWI0
#define BOARD_ID_TWI            ID_TWI0

// TC
#define TC                      TC0
#define TC_PERIPHERAL           0
#define TC_CHANNEL_WAVEFORM     1
#define ID_TC_WAVEFORM          ID_TC1
#define PIN_TC_WAVEFORM         PIN_TC0_TIOA1
#define PIN_TC_WAVEFORM_MUX     PIN_TC0_TIOA1_MUX
#define TC_CHANNEL_CAPTURE      2
#define ID_TC_CAPTURE           ID_TC2
#define PIN_TC_CAPTURE          PIN_TC0_TIOA2
#define PIN_TC_CAPTURE_MUX      PIN_TC0_TIOA2_MUX
#define TC_Handler              TC2_Handler
#define TC_IRQn                 TC2_IRQn

#endif /* CONF_BOARD_H_INCLUDED */