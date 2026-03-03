#ifndef	 WIFI_H_
#define WIFI_H_

#include <asf.h>


extern volatile uint8_t g_wifi_buffer[WIFI_BUFFER_SIZE];
extern volatile uint32_t g_wifi_buf_idx;
extern volatile uint8_t g_wifi_command_complete;
extern volatile uint8_t g_wifi_provision_flag;
extern volatile uint32_t g_spi_transfer_idx; 
extern volatile uint32_t g_counts; 

void configure_usart_wifi(void);
void configure_wifi_comm_pin(void);
void configure_wifi_provision_pin(void);
void configure_spi(void);
void spi_peripheral_initialize(void);
void prepare_spi_transfer(void);
void wifi_usart_handler(void);
void process_incoming_byte_wifi(uint8_t in_byte);
void wifi_command_resposne_handler(uint32_t ul_id, uint32_t ul_mask);
void process_data_wifi(void);
void wifi_provision_handler(uint32_t ul_id, uint32_t ul_mask);
void wifi_spi_handler(void);
void write_wifi_command(char* comm, uint8_t cnt);
void write_image_to_web(void);

#endif