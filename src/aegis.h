/*
 * Copyright (c) 2020, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef AEGIS_H
#define AEGIS_H 1

#if !defined(__minix)
# include <pthread.h>

typedef int (*aegis_gorgon_exit_func)(void *args);

int aegis_set_gorgon(aegis_gorgon_exit_func func, void *args);

#endif // !defined(__minix__)

int aegis_has_debugger(void);

#endif
