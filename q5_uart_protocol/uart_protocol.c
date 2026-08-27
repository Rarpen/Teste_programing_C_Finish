#include "uart_protocol.h"

uint16_t uart_crc16(const uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFFu;
    for (size_t i = 0; i < len; ++i) {
        crc ^= (uint16_t)data[i] << 8;
        for (int b = 0; b < 8; ++b) {
            if (crc & 0x8000u)
                crc = (uint16_t)((crc << 1) ^ 0x1021u);
            else
                crc = (uint16_t)(crc << 1);
        }
    }
    return crc;
}

static void crc_step(uint16_t *crc, uint8_t byte)
{
    *crc ^= (uint16_t)byte << 8;
    for (int b = 0; b < 8; ++b) {
        if (*crc & 0x8000u)
            *crc = (uint16_t)((*crc << 1) ^ 0x1021u);
        else
            *crc = (uint16_t)(*crc << 1);
    }
}

void uart_parser_init(uart_parser_t *p, uint8_t my_addr,
                      uart_msg_handler_t on_message, void *ctx)
{
    if (p == NULL) return;
    p->phase      = RX_WAIT_SOF;
    p->idx        = 0;
    p->crc_calc   = 0xFFFFu;
    p->crc_rx     = 0;
    p->my_addr    = my_addr;
    p->on_message = on_message;
    p->ctx        = ctx;
    p->rx_ok      = 0;
    p->rx_crc_err = 0;
    p->rx_len_err = 0;
}

static void reset_to_sof(uart_parser_t *p)
{
    p->phase    = RX_WAIT_SOF;
    p->idx      = 0;
    p->crc_calc = 0xFFFFu;
}

void uart_parser_timeout_reset(uart_parser_t *p)
{
    if (p == NULL) return;
    if (p->phase != RX_WAIT_SOF) {
        reset_to_sof(p);
    }
}

uart_status_t uart_parser_feed(uart_parser_t *p, uint8_t byte)
{
    if (p == NULL) return UART_ERR_NULL;

    switch (p->phase) {
    case RX_WAIT_SOF:
        if (byte == UART_SOF) {
            p->crc_calc = 0xFFFFu;   
            p->idx      = 0;
            p->phase    = RX_DST;
        }
        break;

    case RX_DST:
        p->msg.dst = byte;
        crc_step(&p->crc_calc, byte);
        p->phase = RX_SRC;
        break;

    case RX_SRC:
        p->msg.src = byte;
        crc_step(&p->crc_calc, byte);
        p->phase = RX_CMD;
        break;

    case RX_CMD:
        p->msg.cmd = byte;
        crc_step(&p->crc_calc, byte);
        p->phase = RX_LEN;
        break;

    case RX_LEN:
        p->msg.len = byte;
        crc_step(&p->crc_calc, byte);
        p->idx   = 0;
        p->phase = (p->msg.len == 0) ? RX_CRC_LO : RX_PAYLOAD;
        break;

    case RX_PAYLOAD:
        p->msg.payload[p->idx++] = (int8_t)byte;  
        crc_step(&p->crc_calc, byte);
        if (p->idx >= p->msg.len) {
            p->phase = RX_CRC_LO;
        }
        break;

    case RX_CRC_LO:
        p->crc_rx = byte;                 
        p->phase  = RX_CRC_HI;
        break;

    case RX_CRC_HI:
        p->crc_rx |= (uint16_t)byte << 8;
        {
            uart_status_t result;
            if (p->crc_rx != p->crc_calc) {
                p->rx_crc_err++;
                result = UART_ERR_CRC;
            } else {
                p->rx_ok++;
                if ((p->msg.dst == p->my_addr ||
                     p->msg.dst == UART_BROADCAST_ADDR) &&
                    p->on_message != NULL) {
                    p->on_message(&p->msg, p->ctx);
                }
                result = UART_OK;
            }
            reset_to_sof(p);
            return result;
        }

    default:
        reset_to_sof(p);
        break;
    }

    return UART_OK; 
}

size_t uart_build_frame(uint8_t dst, uint8_t src, uint8_t cmd,
                        const int8_t *payload, uint8_t len,
                        uint8_t *out, size_t out_cap)
{
    if (out == NULL) return 0;
    if (len > 0 && payload == NULL) return 0;
    size_t needed = 7u + (size_t)len;  
    if (out_cap < needed) return 0;

    size_t i = 0;
    out[i++] = UART_SOF;
    out[i++] = dst;
    out[i++] = src;
    out[i++] = cmd;
    out[i++] = len;
    for (uint8_t k = 0; k < len; ++k) {
        out[i++] = (uint8_t)payload[k];
    }
    uint16_t crc = uart_crc16(&out[1], (size_t)(4u + len));
    out[i++] = (uint8_t)(crc & 0xFF);      
    out[i++] = (uint8_t)((crc >> 8) & 0xFF); 
    return i;
}
