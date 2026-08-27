#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H
#include <stdint.h>
#include <stddef.h>

#define UART_SOF            0x7Eu
#define UART_MAX_PAYLOAD    255u
#define UART_BROADCAST_ADDR 0xFFu

typedef enum {
    UART_OK = 0,
    UART_ERR_CRC,
    UART_ERR_LENGTH,
    UART_ERR_NULL
} uart_status_t;
typedef struct {
    uint8_t cmd;
    uint8_t src;
    uint8_t dst;
    uint8_t len;
    int8_t  payload[UART_MAX_PAYLOAD];   /
} uart_message_t;

typedef void (*uart_msg_handler_t)(const uart_message_t *msg, void *ctx);
typedef enum {
    RX_WAIT_SOF = 0,
    RX_DST,
    RX_SRC,
    RX_CMD,
    RX_LEN,
    RX_PAYLOAD,
    RX_CRC_LO,
    RX_CRC_HI
} uart_rx_phase_t;

typedef struct {
    uart_rx_phase_t     phase;
    uart_message_t      msg;
    uint16_t            idx;       
    uint16_t            crc_calc;   
    uint16_t            crc_rx;     
    uint8_t             my_addr;    

    uart_msg_handler_t  on_message; 
    void               *ctx;        
    uint32_t rx_ok;
    uint32_t rx_crc_err;
    uint32_t rx_len_err;
} uart_parser_t;

uint16_t uart_crc16(const uint8_t *data, size_t len);

void uart_parser_init(uart_parser_t *p, uint8_t my_addr,
                      uart_msg_handler_t on_message, void *ctx);

uart_status_t uart_parser_feed(uart_parser_t *p, uint8_t byte);

void uart_parser_timeout_reset(uart_parser_t *p);

size_t uart_build_frame(uint8_t dst, uint8_t src, uint8_t cmd,
                        const int8_t *payload, uint8_t len,
                        uint8_t *out, size_t out_cap);

#endif 
