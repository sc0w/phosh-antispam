/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/* aspam-log.h
 *
 * Copyright 2021 Chris Talbot <chris@talbothome.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author(s):
 *   Chris Talbot <chris@talbothome.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <glib.h>

#ifndef ASPAM_LOG_LEVEL_TRACE
# define ASPAM_LOG_LEVEL_TRACE ((GLogLevelFlags)(1 << G_LOG_LEVEL_USER_SHIFT))
# define ASPAM_LOG_DETAILED ((GLogLevelFlags)(1 << (G_LOG_LEVEL_USER_SHIFT + 1)))
#endif

#define ASPAM_TRACE_MSG(fmt, ...)                         \
  aspam_log (G_LOG_DOMAIN, ASPAM_LOG_LEVEL_TRACE,           \
           NULL, __FILE__, G_STRINGIFY (__LINE__),      \
           G_STRFUNC, fmt, ##__VA_ARGS__)
#define ASPAM_TRACE(value, fmt, ...)                      \
  aspam_log (G_LOG_DOMAIN,                                \
           ASPAM_LOG_LEVEL_TRACE,                         \
           value, __FILE__, G_STRINGIFY (__LINE__),     \
           G_STRFUNC, fmt, ##__VA_ARGS__)
#define ASPAM_TRACE_DETAILED(value, fmt, ...)             \
  aspam_log (G_LOG_DOMAIN,                                \
           ASPAM_LOG_LEVEL_TRACE | ASPAM_LOG_DETAILED,      \
           value, __FILE__, G_STRINGIFY (__LINE__),     \
           G_STRFUNC, fmt, ##__VA_ARGS__)
#define ASPAM_DEBUG_MSG(fmt, ...)                         \
  aspam_log (G_LOG_DOMAIN,                                \
           G_LOG_LEVEL_DEBUG | ASPAM_LOG_DETAILED,        \
           NULL, __FILE__, G_STRINGIFY (__LINE__),      \
           G_STRFUNC, fmt, ##__VA_ARGS__)
#define ASPAM_DEBUG(value, fmt, ...)                      \
  aspam_log (G_LOG_DOMAIN,                                \
           G_LOG_LEVEL_DEBUG | ASPAM_LOG_DETAILED,        \
           value, __FILE__, G_STRINGIFY (__LINE__),     \
           G_STRFUNC, fmt, ##__VA_ARGS__)
#define ASPAM_WARNING(value, fmt, ...)                    \
  aspam_log (G_LOG_DOMAIN,                                \
           G_LOG_LEVEL_WARNING | ASPAM_LOG_DETAILED,      \
           value, __FILE__, G_STRINGIFY (__LINE__),     \
           G_STRFUNC, fmt, ##__VA_ARGS__)

void aspam_log_init               (void);
void aspam_log_increase_verbosity (void);
int  aspam_log_get_verbosity      (void);
void aspam_log                    (const char     *domain,
                                 GLogLevelFlags  log_level,
                                 const char     *value,
                                 const char     *file,
                                 const char     *line,
                                 const char     *func,
                                 const char     *message_format,
                                 ...) G_GNUC_PRINTF (7, 8);
void aspam_log_anonymize_value    (GString        *str,
                                 const char     *value);
