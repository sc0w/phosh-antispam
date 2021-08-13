/* main.c
 *
 * Copyright 2021 Chris Talbot
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
 */

#include "phoshspamblock-config.h"
#include "spamblock-client.h"

#include <glib.h>
#include <gio/gio.h>
#include <stdlib.h>

static GMainLoop *main_loop = NULL;

static volatile sig_atomic_t __terminated = 0;

static void sig_term (int sig)
{
	if (__terminated > 0)
		return;

	__terminated = 1;

	g_print ("Terminating\n");

	g_main_loop_quit (main_loop);
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_autoptr(GOptionContext) context = NULL;
  g_autoptr(GError) error = NULL;
  struct sigaction sa;
  gboolean version = FALSE;
  SpamBlock *backend;

  GOptionEntry main_entries[] = {
    { "version", 0, 0, G_OPTION_ARG_NONE, &version, "Show program version" },
    { NULL }
  };

  context = g_option_context_new ("- my command line tool");
  g_option_context_add_main_entries (context, main_entries, NULL);

  if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      g_printerr ("%s\n", error->message);
      return EXIT_FAILURE;
    }

  main_loop = g_main_loop_new (NULL, FALSE);

  if (version)
    {
      g_printerr ("%s\n", PACKAGE_VERSION);
      return EXIT_SUCCESS;
    }

  sa.sa_handler = sig_term;
  sigaction (SIGINT, &sa, NULL);
  sigaction (SIGTERM, &sa, NULL);

  g_debug ("Making object");

  backend = spam_block_get_default ();

  g_main_loop_run (main_loop);

  g_object_unref (backend);

  g_main_loop_unref (main_loop);

  return EXIT_SUCCESS;
}
