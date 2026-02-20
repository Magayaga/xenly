#ifndef MODULES_H
#define MODULES_H

#include "interpreter.h"

// ─── Module Registry ─────────────────────────────────────────────────────────
// Returns 1 if module was found and populated, 0 otherwise.
int modules_get(const char *name, Module *out);

// ─── Individual module initializers ──────────────────────────────────────────
Module module_math(void);
Module module_string(void);
Module module_io(void);
Module module_array(void);
Module module_os(void);
Module module_type(void);
Module module_crypto(void);
Module module_multiproc(void);
Module module_path(void);

#endif // MODULES_H