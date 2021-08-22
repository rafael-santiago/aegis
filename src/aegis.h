/*
 * Copyright (c) 2020, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef AEGIS_H
#define AEGIS_H 1

#define AEGIS_VERSION "v2"

#if !defined(CGO)
typedef int (*aegis_gorgon_exit_test_func)(void *args);
typedef void (*aegis_gorgon_on_debugger_func)(void *args);

void aegis_default_on_debugger(void *args);

int aegis_set_gorgon(aegis_gorgon_exit_test_func exit_test, void *exit_test_args,
                     aegis_gorgon_on_debugger_func on_debugger, void *on_debugger_args);
#endif // !defined(CGO)

int aegis_has_debugger(void);

#endif
