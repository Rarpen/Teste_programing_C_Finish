#include "uart_protocol.h"
#include <stdio.h>
#include <string.h>

static int g_received = 0;
static void on_message(const uart_message_t *m, void *ctx)
{
    (void)ctx;
    g_received++;
    printf("RX  cmd=0x%02X  src=%u  dst=%u  len=%u  payload=[",
           m->cmd, m->src, m->dst, m->len);
    for (uint8_t i = 0; i < m->len; ++i) {
        printf("%d%s", m->payload[i], (i + 1 < m->len) ? ", " : "");
    }
    printf("]\n");
}

static void feed_stream(uart_parser_t *p, const uint8_t *buf, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        uart_status_t st = uart_parser_feed(p, buf[i]);
        if (st == UART_ERR_CRC)    printf("   [parser] erro de CRC, frame descartado\n");
        if (st == UART_ERR_LENGTH) printf("   [parser] erro de comprimento, frame descartado\n");
    }
}

int main(void)
{
    const uint8_t MY_ADDR = 0x10;
    uart_parser_t parser;
    uart_parser_init(&parser, MY_ADDR, on_message, NULL);

    uint8_t frame[7 + UART_MAX_PAYLOAD];
    int8_t pl1[] = {10, -5, 127, -128, 0};
    size_t n1 = uart_build_frame(MY_ADDR, 0x01, 0x42, pl1,
                                 (uint8_t)sizeof(pl1), frame, sizeof(frame));
    printf("Enviando frame 1 (%zu bytes)\n", n1);
    feed_stream(&parser, frame, n1);

    int8_t pl2[] = {-1, -2, -3};
    size_t n2 = uart_build_frame(UART_BROADCAST_ADDR, 0x02, 0x07, pl2,
                                 (uint8_t)sizeof(pl2), frame, sizeof(frame));
    printf("Enviando frame 2 (broadcast)\n");
    feed_stream(&parser, frame, n2);


    size_t n3 = uart_build_frame(0x20, 0x03, 0x01, NULL, 0, frame, sizeof(frame));
    printf("Enviando frame 3 (endereçado a 0x20, não é para nós)\n");
    feed_stream(&parser, frame, n3);

    /* 4) Corrompe um byte para disparar a verificação de CRC. */
    size_t n4 = uart_build_frame(MY_ADDR, 0x04, 0x55, pl1,
                                 (uint8_t)sizeof(pl1), frame, sizeof(frame));
    frame[6] ^= 0xFF;   /* inverte um byte do payload */
    printf("Enviando frame 4 (corrompido)\n");
    feed_stream(&parser, frame, n4);

    uint8_t noise[] = {0x00, 0xAB, 0x7E /*SOF falso*/, 0x99};
    printf("Injetando ruído, intervalo ocioso, depois um frame válido\n");
    feed_stream(&parser, noise, sizeof(noise));
    uart_parser_timeout_reset(&parser);  
    size_t n5 = uart_build_frame(MY_ADDR, 0x05, 0x99, pl2,
                                 (uint8_t)sizeof(pl2), frame, sizeof(frame));
    feed_stream(&parser, frame, n5);
    
    printf("\nResumo: ok=%u crc_err=%u len_err=%u  (callbacks=%d)\n",
           parser.rx_ok, parser.rx_crc_err, parser.rx_len_err, g_received);
    return 0;
}
