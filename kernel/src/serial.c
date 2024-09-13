#include "serial.h"
#include "logger.h"
#define COM_PORT 0x3f8
#define COM_5 (COM_PORT+5)
#define COM_STATUS COM_5

#define COM_LINE_CONTROL_PORT (COM_PORT+3)
#define COM_FIFO_CONTROL_PORT (COM_PORT+2)
#define COM_INT_ENABLE_PORT   (COM_PORT+1)
void serial_init() {
    // Set baud rate (for example, 9600 baud)
    outb(COM_LINE_CONTROL_PORT, 0x80); // Enable DLAB (divisor latch access)
    outb(COM_PORT, 0x03); // Set divisor to 3 (lo byte) 38400 baud
    outb(COM_INT_ENABLE_PORT, 0x00); // Hi byte
    outb(COM_LINE_CONTROL_PORT, 0x03); // 8 bits, no parity, one stop bit
    outb(COM_FIFO_CONTROL_PORT, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
    outb(COM_INT_ENABLE_PORT, 0x00); // Enable received data available interrupt
}

void serial_print_u8(uint8_t c) {
    while ((inb(COM_STATUS) & 0x20) == 0) {}
    outb(COM_PORT, c);
}

static inline bool data_avail() {
    return (inb(COM_STATUS) & 0x01) != 0; 
}
void serial_printstr(const char* str) {
    while(*str) {
        char c = *str++;
        serial_print_u8(c);
    }
}


static intptr_t serial_log_write_str(Logger* this, const char* str, size_t len) {
    for(size_t i = 0; i < len; ++i) {
        serial_print_u8(str[i]);
    }
    return 0;
}
static intptr_t serial_log_draw_color(Logger* this, uint32_t color) {
    serial_printstr(get_ansi_color_from_log(color));
    return 0;
}
Logger serial_logger = {
    .log = NULL,
    .write_str = serial_log_write_str,
    .draw_color = serial_log_draw_color,
    .level = LOG_ALL,
    .private = NULL,
};
