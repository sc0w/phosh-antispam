/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/* main.c
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtk/gtk.h>
#include <libintl.h>
#include <locale.h>

#include "aspam-utils.h"
#include "aspam-application.h"
#include "aspam-log.h"

int
main (int   argc,
      char *argv[])
{
  g_autoptr(ASpamApplication) application = NULL;

  g_assert (ASPAM_IS_MAIN_THREAD ());

  aspam_log_init ();

  g_set_prgname (PACKAGE_ID);
  application = aspam_application_new ();
  setlocale (LC_ALL, "");
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  return g_application_run (G_APPLICATION (application), argc, argv);
}
