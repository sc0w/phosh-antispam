/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/* aspam-application.c
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

#define G_LOG_DOMAIN "aspam-application"

#ifdef HAVE_CONFIG_H
# include "config.h"
# include "version.h"
#endif

#include <glib/gi18n.h>
#include <handy.h>

#include "aspam-window.h"
#include "aspam-application.h"
#include "aspam-log.h"
#include "aspam-client.h"

/**
 * SECTION: aspam-application
 * @title: ASpamApplication
 * @short_description: Base Application class
 * @include: "aspam-application.h"
 */

struct _ASpamApplication
{
  GtkApplication  parent_instance;

  gboolean        daemon;
  gboolean        daemon_running;
  GtkWindow      *main_window;

  ASpamSettings  *settings;
  ASpamClient    *client;
};

G_DEFINE_TYPE (ASpamApplication, aspam_application, GTK_TYPE_APPLICATION)


static gboolean
cmd_verbose_cb (const char *option_name,
                const char *value,
                gpointer    data,
                GError    **error);

static GOptionEntry cmd_options[] = {
  {
    "quit", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, NULL,
    N_("Quit all running instances of the application"), NULL
  },
  {
    "verbose", 'v', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, cmd_verbose_cb,
    N_("Show verbose logs"), NULL
  },
  {
    "daemon", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, NULL,
    N_("Whether to present the main window on startup"),
    NULL
  },
  {
    "version", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, NULL,
    N_("Show release version"), NULL
  },
  { NULL }
};

static gboolean
cmd_verbose_cb (const char  *option_name,
                const char  *value,
                gpointer     data,
                GError     **error)
{
  aspam_log_increase_verbosity ();

  return TRUE;
}

static void
aspam_application_show_help (GSimpleAction *action,
                           GVariant      *parameter,
                           gpointer       user_data)
{
  GtkWindow *window;
  g_autoptr(GError) error = NULL;

  window = gtk_application_get_active_window (GTK_APPLICATION (user_data));

  if (!gtk_show_uri_on_window (window, PACKAGE_HELP_URL, GDK_CURRENT_TIME, &error))
    g_warning ("Failed to launch help: %s", error->message);
}

static int
aspam_application_handle_local_options (GApplication *application,
                                      GVariantDict *options)
{
  ASpamApplication *self = (ASpamApplication *)application;
  if (g_variant_dict_contains (options, "version"))
    {
      g_print ("%s %s\n", PACKAGE_NAME, PACKAGE_VCS_VERSION);
      return 0;
    }

  self->daemon = FALSE;
  if (g_variant_dict_contains (options, "daemon"))
    {
      /* Hold application only the first time daemon mode is set */
      if (!self->daemon)
        g_application_hold (application);

      self->daemon = TRUE;
      g_debug ("Application marked as daemon");
    }

  return -1;
}

static void
aspam_application_add_actions (ASpamApplication *self)
{
  static const GActionEntry app_entries[] = {
    { "help", aspam_application_show_help },
  };

  struct
  {
    const char *action;
    const char *accel[2];
  } accels[] = {
    { "app.help", { "F1", NULL } },
  };

  g_assert (ASPAM_IS_APPLICATION (self));

  g_action_map_add_action_entries (G_ACTION_MAP (self), app_entries,
                                   G_N_ELEMENTS (app_entries), self);

  for (gsize i = 0; i < G_N_ELEMENTS (accels); i++)
    gtk_application_set_accels_for_action (GTK_APPLICATION (self),
                                           accels[i].action, accels[i].accel);
}

static void
aspam_application_startup (GApplication *application)
{
  ASpamApplication *self = (ASpamApplication *)application;
  g_autoptr(GtkCssProvider) css_provider = NULL;

  g_info ("%s %s, git version: %s", PACKAGE_NAME,
          PACKAGE_VERSION, PACKAGE_VCS_VERSION);

  G_APPLICATION_CLASS (aspam_application_parent_class)->startup (application);

  hdy_init ();

  aspam_application_add_actions (self);
  g_set_application_name (_("Anti-Spam"));
  gtk_window_set_default_icon_name (PACKAGE_ID);

  self->settings = aspam_settings_new ();
  self->client = aspam_client_get_default ();
  css_provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_resource (css_provider,
                                       "/org/kop316/antispam/css/gtk.css");

  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                             GTK_STYLE_PROVIDER (css_provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

static int
aspam_application_command_line (GApplication            *application,
                              GApplicationCommandLine *command_line)
{
  GVariantDict *options;

  options = g_application_command_line_get_options_dict (command_line);

  /*
   * NOTE: g_application_quit() is almost always a bad idea.
   * This simply kills the application.  So active process like
   * Saving file will get stopped midst the process.  If you
   * find it bad, find your luck with g_application_release()
   */
  if (g_variant_dict_contains (options, "quit"))
    {
      g_application_quit (application);
      return 0;
    }

  g_application_activate (application);

  return 0;
}

gboolean
on_widget_deleted(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  ASpamApplication *self = (ASpamApplication *)data;
  gboolean hide_on_del = FALSE;

  /* If we are not running in daemon mode, hide the window and reset speaker button */
  if (self->daemon) {
    hide_on_del = gtk_widget_hide_on_delete (widget);
  }

  return hide_on_del;
}

static void
aspam_application_activate (GApplication *application)
{
  ASpamApplication *self = (ASpamApplication *)application;

  if (!self->main_window) {
    self->main_window = GTK_WINDOW (aspam_window_new (GTK_APPLICATION (self), self->settings));
    g_object_add_weak_pointer (G_OBJECT (self->main_window), (gpointer *)&self->main_window);

    g_signal_connect(G_OBJECT(self->main_window),
        "delete-event", G_CALLBACK(on_widget_deleted), application);
  }

  if ((!self->daemon && !self->daemon_running) || (self->daemon_running)) {
    self->main_window = gtk_application_get_active_window (GTK_APPLICATION (self));
    gtk_window_present (GTK_WINDOW (self->main_window));
  } else {
    self->daemon_running = TRUE;
  }
}

static void
aspam_application_finalize (GObject *object)
{
  ASpamApplication *self = (ASpamApplication *)object;

  ASPAM_TRACE_MSG ("disposing application");

  g_clear_object (&self->settings);

  G_OBJECT_CLASS (aspam_application_parent_class)->finalize (object);
}

static void
aspam_application_class_init (ASpamApplicationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GApplicationClass *application_class = G_APPLICATION_CLASS (klass);

  object_class->finalize = aspam_application_finalize;

  application_class->handle_local_options = aspam_application_handle_local_options;
  application_class->startup = aspam_application_startup;
  application_class->command_line = aspam_application_command_line;
  application_class->activate = aspam_application_activate;
}

static void
aspam_application_init (ASpamApplication *self)
{
  g_application_add_main_option_entries (G_APPLICATION (self), cmd_options);
}

ASpamApplication *
aspam_application_new (void)
{
  return g_object_new (ASPAM_TYPE_APPLICATION,
                       "application-id", PACKAGE_ID,
                       "flags", G_APPLICATION_HANDLES_COMMAND_LINE,
                       NULL);
}
