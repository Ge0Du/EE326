#ifndef CAMERA_H_
#define CAMERA_H_

#include <asf.h>
#include "tc.h"

#define BOARD_TWI               TWI0
#define CAM_IMAGE_BUFFER_SIZE   100000

typedef enum {
    VSYNC_IDLE = 0,
    VSYNC_FRAME_STARTED,
    VSYNC_FRAME_DONE
} vsync_state_t;
typedef enum {
	BUF_OWNER_NONE = 0,
	BUF_OWNER_CAMERA,
	BUF_OWNER_WIFI
} buf_owner_t;

extern volatile buf_owner_t g_buf_owner;
extern volatile vsync_state_t g_vsync_state;
extern volatile uint32_t      g_image_len;
extern uint8_t                g_image_buffer[CAM_IMAGE_BUFFER_SIZE];
extern volatile uint8_t g_network_ready;
extern volatile uint8_t g_clients_connected;
extern volatile uint32_t g_t_vsync_wait;
extern volatile uint32_t g_t_find_len;
extern volatile uint32_t g_bytes_captured;
extern volatile uint32_t g_t_capture_total;

void    init_vsync_interrupts(void);
void    configure_twi(void);
void    pio_capture_init(Pio *p_pio, uint32_t ul_id);
void    init_camera(void);
void    configure_camera(void);
uint8_t start_capture(void);
uint8_t find_image_len(uint32_t search_len);

#endif