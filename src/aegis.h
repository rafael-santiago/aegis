/*
 *                          Copyright (C) 2020 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
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
