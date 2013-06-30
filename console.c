/* console.c - Console IO interface for Mink.
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */

#include "hal.h"

static console_t cons[4];
static int next_con = 0;

bool console_register(console_t con) {
	if (next_con == 4) {
		return false;
	} else {
		cons[next_con++] = con;
		return true;
	}
}

void console_writestring(const char *str) {
	for (int i = 0; i < next_con; i++) {
		cons[i].writestring(str);
	}
}

void console_setcolor(uint8_t color) {
	for (int i = 0; i < next_con; i++) {
		cons[i].setcolor(color);
	}
}

uint8_t console_getcolor() {
	if (next_con > 0) {
		return cons[0].getcolor();
	} else {
		return 0;
	}
}
