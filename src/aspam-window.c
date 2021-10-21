/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/* aspam-window.c
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

#define G_LOG_DOMAIN "aspam-window"

#ifdef HAVE_CONFIG_H
# include "config.h"
# include "version.h"
#endif

#include <glib/gi18n.h>
#include <handy.h>

#include "aspam-window.h"
#include "aspam-log.h"
#include "aspam-pattern-row.h"

struct _ASpamWindow
{
  GtkApplicationWindow parent_instance;

  ASpamSettings *settings;

  GtkWidget   *menu_button;
  GtkWidget   *enable_aspam_switch;
  GtkWidget   *silence_switch;
  GtkWidget   *allow_callback_switch;
  GtkWidget   *callback_timeout_text;
  GtkWidget   *callback_timeout_button;
  GtkWidget   *allow_blocked_numbers_switch;
  GtkWidget   *whitelist_list_box;
  GtkWidget   *new_whitelist_text;
  GtkWidget   *new_whitelist_button;
};

G_DEFINE_TYPE (ASpamWindow, aspam_window, GTK_TYPE_APPLICATION_WINDOW)


enum {
  PROP_0,
  PROP_SETTINGS,
  N_PROPS
};

static GParamSpec *properties[N_PROPS];

static void
aspam_window_window_populate (ASpamWindow *self)
{
  g_autofree char *callback_timeout_string = NULL;
  ASpamSettings *settings;
  guint64 callback_timeout;
  guint match_list_length;
  char **match_list;
  g_assert (ASPAM_IS_WINDOW (self));
  settings = aspam_settings_get_default ();

  gtk_switch_set_active (GTK_SWITCH (self->enable_aspam_switch),
                         aspam_settings_get_enable_aspamclient (settings));
  gtk_switch_set_active (GTK_SWITCH (self->allow_callback_switch),
                         aspam_settings_get_allow_callback (settings));
  gtk_switch_set_active (GTK_SWITCH (self->allow_blocked_numbers_switch),
                         aspam_settings_get_allow_blocked_numbers (settings));
  gtk_switch_set_active (GTK_SWITCH (self->silence_switch),
                         aspam_settings_get_silence (settings));

  gtk_entry_set_text (GTK_ENTRY (self->new_whitelist_text), "");
  callback_timeout = aspam_settings_get_callback_timeout (settings);
  callback_timeout_string = g_strdup_printf("%li", callback_timeout);
  gtk_entry_set_text (GTK_ENTRY (self->callback_timeout_text),
                      callback_timeout_string);

  match_list = aspam_settings_get_match_list (settings);

  match_list_length = g_strv_length (match_list);

  for (int i = 0; i < match_list_length; i++) {
    ASpamPatternRow *new_row;
    new_row = aspam_pattern_row_new ();

    gtk_list_box_prepend (GTK_LIST_BOX (self->whitelist_list_box),
                          GTK_WIDGET (new_row));

    hdy_action_row_set_subtitle (HDY_ACTION_ROW (new_row),
                                 match_list[i]);

  }
}


static void
enable_aspam_switch_flipped_cb (ASpamWindow *self)
{
  ASpamSettings *settings;
  gboolean enable_aspam;

  g_assert (ASPAM_IS_WINDOW (self));
  settings = aspam_settings_get_default ();
  enable_aspam = gtk_switch_get_active (GTK_SWITCH (self->enable_aspam_switch));
  aspam_settings_set_enable_aspamclient (settings, enable_aspam);
}

static void
silence_switch_flipped_cb (ASpamWindow *self)
{
  ASpamSettings *settings;
  gboolean enable_silence;

  g_assert (ASPAM_IS_WINDOW (self));
  settings = aspam_settings_get_default ();
  enable_silence = gtk_switch_get_active (GTK_SWITCH (self->silence_switch));
  aspam_settings_set_silence (settings, enable_silence);
}

static void
allow_callback_switch_flipped_cb (ASpamWindow *self)
{
  ASpamSettings *settings;
  gboolean allow_callback;

  g_assert (ASPAM_IS_WINDOW (self));
  settings = aspam_settings_get_default ();
  allow_callback = gtk_switch_get_active (GTK_SWITCH (self->allow_callback_switch));
  aspam_settings_set_allow_callback (settings, allow_callback);
}

static void
allow_blocked_numbers_switch_flipped_cb (ASpamWindow *self)
{
  ASpamSettings *settings;
  gboolean allow_blocked_numbers;

  g_assert (ASPAM_IS_WINDOW (self));
  settings = aspam_settings_get_default ();
  allow_blocked_numbers = gtk_switch_get_active (GTK_SWITCH (self->allow_blocked_numbers_switch));
  aspam_settings_set_allow_blocked_numbers (settings, allow_blocked_numbers);
}

static void
callback_timeout_button_clicked_cb (ASpamWindow *self)
{
  g_autoptr(GError) error = NULL;
  ASpamSettings *settings;
  const char *timeout_string;
  guint64 new_timeout;

  g_assert (ASPAM_IS_WINDOW (self));
  settings = aspam_settings_get_default ();

  timeout_string = gtk_entry_get_text (GTK_ENTRY (self->callback_timeout_text));

  g_ascii_string_to_unsigned (timeout_string, 10, 0, 9999, &new_timeout, &error);
  if (error) {
    guint64 callback_timeout;
    g_autofree char *callback_timeout_string = NULL;
    g_warning ("Error converting string to unsigned 64: %s", error->message);
    callback_timeout = aspam_settings_get_callback_timeout (settings);
    callback_timeout_string = g_strdup_printf("%li", callback_timeout);
    gtk_entry_set_text (GTK_ENTRY (self->callback_timeout_text),
                        callback_timeout_string);
  } else {
    aspam_settings_set_callback_timeout (settings, new_timeout);
  }
}

static void
new_whitelist_button_clicked_cb (ASpamWindow *self)
{
  g_autoptr(GError) error = NULL;
  //ASpamSettings *settings;
  const char *new_pattern;
  ASpamPatternRow *new_row;
  g_assert (ASPAM_IS_WINDOW (self));
  //settings = aspam_settings_get_default ();

  new_pattern = gtk_entry_get_text (GTK_ENTRY (self->new_whitelist_text));

  if (!*new_pattern) {
    g_debug ("Empty string");
    return;
  }

  new_row = aspam_pattern_row_new ();

  gtk_list_box_prepend (GTK_LIST_BOX (self->whitelist_list_box),
                        GTK_WIDGET (new_row));

  hdy_action_row_set_subtitle (HDY_ACTION_ROW (new_row),
                               new_pattern);

  //TODO: Append the new pattern

  gtk_entry_set_text (GTK_ENTRY (self->new_whitelist_text), "");
}




static void
aspam_window_show_about (ASpamWindow *self)
{
  const char *authors[] = {
    "Chris Talbot https://www.gitlab.com/kop316/",
    NULL
  };

  g_assert (ASPAM_IS_WINDOW (self));

  /*
   * If “program-name” is not set, it is retrieved from
   * g_get_application_name().
   */
  gtk_show_about_dialog (GTK_WINDOW (self),
                         "website", "https://gitlab.com/kop316/phosh-antispam",
                         "version", PACKAGE_VCS_VERSION,
                         "copyright", "Copyright © 2021 Chris Talbot",
                         "license-type", GTK_LICENSE_GPL_3_0,
                         "authors", authors,
                         "logo-icon-name", PACKAGE_ID,
                         "translator-credits", _("translator-credits"),
                         NULL);
}

static void
aspam_window_unmap (GtkWidget *widget)
{
  ASpamWindow *self = (ASpamWindow *)widget;
  GtkWindow *window = (GtkWindow *)widget;
  GdkRectangle geometry;
  gboolean is_maximized;

  is_maximized = gtk_window_is_maximized (window);
  aspam_settings_set_window_maximized (self->settings, is_maximized);

  if (!is_maximized)
    {
      gtk_window_get_size (window, &geometry.width, &geometry.height);
      aspam_settings_set_window_geometry (self->settings, &geometry);
    }

  GTK_WIDGET_CLASS (aspam_window_parent_class)->unmap (widget);
}

static void
aspam_window_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  ASpamWindow *self = (ASpamWindow *)object;

  switch (prop_id)
    {
    case PROP_SETTINGS:
      self->settings = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
aspam_window_constructed (GObject *object)
{
  ASpamWindow *self = (ASpamWindow *)object;
  GtkWindow *window = (GtkWindow *)object;
  GdkRectangle geometry;

  aspam_settings_get_window_geometry (self->settings, &geometry);
  gtk_window_set_default_size (window, geometry.width, geometry.height);

  if (aspam_settings_get_window_maximized (self->settings))
    gtk_window_maximize (window);

  G_OBJECT_CLASS (aspam_window_parent_class)->constructed (object);
}

static void
aspam_window_finalize (GObject *object)
{
  ASpamWindow *self = (ASpamWindow *)object;

  ASPAM_TRACE_MSG ("finalizing window");

  g_object_unref (self->settings);

  G_OBJECT_CLASS (aspam_window_parent_class)->finalize (object);
}

static void
aspam_window_class_init (ASpamWindowClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->set_property = aspam_window_set_property;
  object_class->constructed  = aspam_window_constructed;
  object_class->finalize     = aspam_window_finalize;

  widget_class->unmap = aspam_window_unmap;

  /**
   * ASpamWindow:settings:
   *
   * The Application Settings
   */
  properties[PROP_SETTINGS] =
    g_param_spec_object ("settings",
                         "Settings",
                         "The Application Settings",
                         ASPAM_TYPE_SETTINGS,
                         G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/kop316/antispam/"
                                               "ui/aspam-window.ui");

  gtk_widget_class_bind_template_child (widget_class, ASpamWindow, menu_button);
  gtk_widget_class_bind_template_child (widget_class, ASpamWindow, enable_aspam_switch);
  gtk_widget_class_bind_template_child (widget_class, ASpamWindow, silence_switch);
  gtk_widget_class_bind_template_child (widget_class, ASpamWindow, allow_callback_switch);
  gtk_widget_class_bind_template_child (widget_class, ASpamWindow, callback_timeout_text);
  gtk_widget_class_bind_template_child (widget_class, ASpamWindow, callback_timeout_button);
  gtk_widget_class_bind_template_child (widget_class, ASpamWindow, allow_blocked_numbers_switch);
  gtk_widget_class_bind_template_child (widget_class, ASpamWindow, whitelist_list_box);
  gtk_widget_class_bind_template_child (widget_class, ASpamWindow, new_whitelist_text);
  gtk_widget_class_bind_template_child (widget_class, ASpamWindow, new_whitelist_button);

  gtk_widget_class_bind_template_callback (widget_class, aspam_window_show_about);
  gtk_widget_class_bind_template_callback (widget_class, allow_callback_switch_flipped_cb);
  gtk_widget_class_bind_template_callback (widget_class, silence_switch_flipped_cb);
  gtk_widget_class_bind_template_callback (widget_class, callback_timeout_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, new_whitelist_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, enable_aspam_switch_flipped_cb);
  gtk_widget_class_bind_template_callback (widget_class, allow_blocked_numbers_switch_flipped_cb);

}

static void
aspam_window_init (ASpamWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  aspam_window_window_populate (self);
}

GtkWidget *
aspam_window_new (GtkApplication *application,
                ASpamSettings    *settings)
{
  g_assert (GTK_IS_APPLICATION (application));
  g_assert (ASPAM_IS_SETTINGS (settings));

  return g_object_new (ASPAM_TYPE_WINDOW,
                       "application", application,
                       "settings", settings,
                       NULL);
}
