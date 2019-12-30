/* serialterm.c - Serial debugging console for Mink.
 *
 * This is not a general-purpose serial driver - it simply
 * allows debugging information to be dumped to the serial
 * port.
 *
 * Probably also worth noting that it potentially blocks,
 * so isn't fit for use going forward...
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */

#ifdef SERIAL_DEBUG
#include "hal.h"
#include "sys.h"
#include "utils.h"

#define PORT 0x3f8   /* COM1 */

bool is_transmit_empty() {
  return inportb(PORT + 5) & 0x20;
}

void write_serial(char a) {
	while (!is_transmit_empty());

  outportb(PORT,a);
}

void serial_writestring(const char* data) {
	size_t datalen = strlen(data);
	for (size_t i = 0; i < datalen; i++) {
		write_serial(data[i]);
	}
}

void serial_setcolor(uint8_t __attribute__((unused)) color) {
	// noop
}

uint8_t serial_getcolor() {
	return 0;
}

static int init_serial() {
  outportb(PORT + 1, 0x00);    // Disable all interrupts
  outportb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
  outportb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
  outportb(PORT + 1, 0x00);    //                  (hi byte)
  outportb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
  outportb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
  outportb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set

	static console_t con = {
			.writestring = serial_writestring,
			.setcolor = serial_setcolor,
			.getcolor = serial_getcolor
	};

	console_register(con);

	serial_writestring("Serial debugging console is up ");

  return 1;
}

static feature_t x MINK_FEATURE = {
  .name = "x86/sdc",
  .required = NULL, 
  .load_after = NULL,
  .init = &init_serial
};
#else
// stop pedantic warning about empty translation unit
static char __ignored = 0;
char __serialterm_ignored() {
  return __ignored;
}
#endif
