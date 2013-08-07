/* kmain.c - The Mink Kernel entry point.
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */

#ifndef __cplusplus
#include <stdbool.h>
#endif
#include "mink.h"
#include "elf.h"
 
typedef struct {
  char *a;
  int b;
} test_struct;

// State of a feature (uninitialised, resolved, initialised).
enum feature_state {
  FEATURE_UNINIT,
  FEATURE_RESOLVED,
  FEATURE_INIT,
};

bool KDEBUG = false;

/* Defined by the linker... */
extern feature_t __start_features, __stop_features;

static void init_features();
static void greybox(char *msg, uint8_t color, bool newline);
static void process_args(int argc, const char **argv);
static inline void minklogo();

void kmain(int argc, const char** argv) {
  minklogo();
  
  process_args(argc, argv);

  printk("Starting platform features... \n");
  init_features();
  printk("Platform is up.\n");	  
  printk("Reticulating splines... ");
  greybox("OK", make_color(COLOR_LIGHT_GREEN, COLOR_BLACK), true);
  
  printk("Enabling interrupts... ");
  enable_interrupts();
  greybox("OK", make_color(COLOR_LIGHT_GREEN, COLOR_BLACK), true);
  
  printk("Kernel is up; Going idle.\n");
  
  // go into idle
  panic("Fucks sake...");
  idle();  
}

static inline void minklogo() {
  console_setcolor(make_color(COLOR_WHITE, COLOR_BLACK));
  printk("M");
  console_setcolor(make_color(COLOR_LIGHT_GREY, COLOR_BLACK));
  printk("i");
  console_setcolor(make_color(COLOR_WHITE, COLOR_BLACK));
  printk("NK ");
  console_setcolor(make_color(COLOR_LIGHT_GREY, COLOR_BLACK));
  printk("v" MINK_VERSION "\n\n");
}

static void process_args(int argc, const char **argv) {
	for (int i = 0; i < argc; i++) {
		if (strcmp("debug", argv[i])) {
			KDEBUG = true;
		}
	}
}

/* Just used to write a msg (usually OK or whatever) in a grey box, a'la Linux */
static void greybox(char *msg, uint8_t color, bool newline) {
  uint8_t oldcol = console_getcolor();
  console_setcolor(make_color(COLOR_DARK_GREY, COLOR_BLACK));
  console_writestring("[");
  console_setcolor(color);
  console_writestring(msg);
  console_setcolor(make_color(COLOR_DARK_GREY, COLOR_BLACK));
  console_writestring(newline ? "]\n" : "]");
  console_setcolor(oldcol);
}

static feature_t *find_feature(const char *name) {
  for (feature_t *i = &__start_features, *e = &__stop_features; i < e; ++i) {
    if (!strcmp(name, i->name)) return i;
  }
  return NULL;
}

static void resolve_feature(feature_t *m) {
  if (m->state >= FEATURE_RESOLVED) {
    return;
  }

  for (feature_prereq_t *p = m->required; p != NULL && p->name != NULL; ++p) {
    p->feature = find_feature(p->name);
  }

  for (feature_prereq_t *p = m->load_after; p != NULL && p->name != NULL; ++p) {
    p->feature = find_feature(p->name);
  }

  m->state = FEATURE_RESOLVED;
}

static void init_feature(feature_t *m) {
  if (m->state >= FEATURE_INIT) {
    return;
  }
  
  m->state = FEATURE_INIT;

  if (m->required) {
    for (feature_prereq_t *p = m->required; p != NULL && p->name != NULL; ++p) {
      if (!p->feature) {
        panic("Feature %s not found (Required by %s) ", p->name, m->name);
      } else {
        init_feature(p->feature);
      }
    }
  }

  if (m->load_after) {
    for (feature_prereq_t *p = m->load_after; p != NULL && p->name != NULL; ++p) {
      if (p->feature) {
        init_feature(p->feature);
      }
    }
  }
  
  printk("Starting %s... ", m->name);
  if (m->init) {
    if (m->init()) {
      greybox("OK", make_color(COLOR_LIGHT_GREEN, COLOR_BLACK), true);
    } else {
      panic("Feature %s failed to start", m->name);
    }
  }
}

static void init_features() {
  for (feature_t *m = &__start_features, *e = &__stop_features; m < e; ++m) {
    m->state = FEATURE_UNINIT;
  }

  for (feature_t *m = &__start_features, *e = &__stop_features; m < e; ++m) {
    resolve_feature(m);
  }

  for (feature_t *m = &__start_features, *e = &__stop_features; m < e; ++m) {
    init_feature(m);
  }
}

