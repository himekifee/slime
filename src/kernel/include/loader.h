#pragma once

#include "engine.h"
#include "config.h"

typedef struct {
	const char *name;
	int (*init)(void);

	int (*exit)(void);
} Submodule;

/**
 * Register the submodule. Create and store a struct to .data.slime section to make it accessible for module_init().
 *
 * @param n The name of the submodule, a string.
 * @param i The initializer of the submodule.
 * @param e The de-initializer of the submodule.
 */
#define SUBMODULE(n, i, e)                                                     \
	Submodule __attribute__((unused)) __attribute__((aligned(1)))          \
	__attribute__((section(".data.slime")))                                \
	SUBMODULE_##n = { .name = #n, .init = i, .exit = e }

#define SLIME_FOREACH_SUBMODULE(p)                                             \
	for (p = submodule_tbl; p < submodule_tbl_end; p++)
