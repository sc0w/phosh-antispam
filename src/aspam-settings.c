/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/* aspam-settings.c
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

#define G_LOG_DOMAIN "aspam-settings"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "aspam-settings.h"
#include "aspam-log.h"

/**
 * SECTION: aspam-settings
 * @title: ASpamSettings
 * @short_description: The Application settings
 * @include: "aspam-settings.h"
 *
 * A class that handles application specific settings, and
 * to store them to disk.
 */

struct _ASpamSettings
{
  GObject    parent_instance;

  GSettings *app_settings;
  gboolean   first_run;

  gboolean enable_aspamclient;
  gboolean allow_blocked_numbers;
  gboolean allow_callback;
  gboolean silence;
  guint64 callback_timeout;
  char **match_list;
};

G_DEFINE_TYPE (ASpamSettings, aspam_settings, G_TYPE_OBJECT)

gboolean
aspam_settings_get_silence (ASpamSettings *self)
{
  g_assert (ASPAM_IS_SETTINGS (self));

  return self->silence;
}

void
aspam_settings_set_silence (ASpamSettings *self,
                            gboolean enable)
{
  g_assert (ASPAM_IS_SETTINGS (self));

  self->silence = enable;
  g_settings_set_boolean (self->app_settings, "silence", self->silence);
  g_settings_apply (self->app_settings);
}

gboolean
aspam_settings_get_enable_aspamclient (ASpamSettings *self)
{
  g_assert (ASPAM_IS_SETTINGS (self));

  return self->enable_aspamclient;
}

void
aspam_settings_set_enable_aspamclient (ASpamSettings *self,
                                       gboolean enable)
{
  g_assert (ASPAM_IS_SETTINGS (self));

  self->enable_aspamclient = enable;
  g_settings_set_boolean (self->app_settings, "enable", self->enable_aspamclient);
  g_settings_apply (self->app_settings);
}

gboolean
aspam_settings_get_allow_blocked_numbers (ASpamSettings *self)
{
  g_assert (ASPAM_IS_SETTINGS (self));

  return self->allow_blocked_numbers;
}

void
aspam_settings_set_allow_blocked_numbers (ASpamSettings *self,
                                          gboolean enable)
{
  g_assert (ASPAM_IS_SETTINGS (self));
  self->allow_blocked_numbers = enable;
  g_settings_set_boolean (self->app_settings, "allow-blocked-numbers", self->allow_blocked_numbers);
  g_settings_apply (self->app_settings);
}

gboolean
aspam_settings_get_allow_callback (ASpamSettings *self)
{
  g_assert (ASPAM_IS_SETTINGS (self));

  return self->allow_callback;
}

void
aspam_settings_set_allow_callback (ASpamSettings *self,
                                       gboolean enable)
{
  g_assert (ASPAM_IS_SETTINGS (self));
  self->allow_callback = enable;
  g_settings_set_boolean (self->app_settings, "allow-callback", self->allow_callback);
  g_settings_apply (self->app_settings);
}

guint64
aspam_settings_get_callback_timeout (ASpamSettings *self)
{
  g_assert (ASPAM_IS_SETTINGS (self));

  return self->callback_timeout;
}

void
aspam_settings_set_callback_timeout (ASpamSettings *self,
                                     guint64 timeout)
{
  g_assert (ASPAM_IS_SETTINGS (self));
  self->callback_timeout = timeout;
  g_settings_set_uint64 (self->app_settings, "callback-timeout", self->callback_timeout);
  g_settings_apply (self->app_settings);
}


char **
aspam_settings_get_match_list (ASpamSettings *self)
{
  g_assert (ASPAM_IS_SETTINGS (self));

  return self->match_list;
}

void
aspam_settings_set_match_list (ASpamSettings *self,
                               char **match_list)
{
  g_autoptr(GError) error = NULL;
  g_autofree char *csv = NULL;
  g_autoptr(GFile) whitelist_file = NULL;
  int size = 0;

  g_assert (ASPAM_IS_SETTINGS (self));

  csv = g_strjoinv (",", match_list);

  if (csv)
    size = strlen (csv);
  else
    return;

  whitelist_file = g_file_new_build_filename (g_get_user_config_dir (),
                                              "phoshantispam",
                                              "whitelist.csv",
                                              NULL);

  if (!g_file_replace_contents (whitelist_file, csv, size, NULL, FALSE,
                                G_FILE_CREATE_NONE, NULL, NULL, &error)) {
    g_warning ("Failed to write to file %s: %s",
               g_file_peek_path (whitelist_file), error->message);
    return;
  }

  g_strfreev (self->match_list);
  self->match_list = g_strsplit (csv, ",", -1);
  return;
}

void
aspam_settings_delete_match (ASpamSettings *self,
                             const char *match)
{
  g_auto(GStrv) new_match_list = NULL;
  g_autofree char *new_csv = NULL;
  GString *new_csv_string;
  guint match_list_length;

  g_assert (ASPAM_IS_SETTINGS (self));

  match_list_length = g_strv_length (self->match_list);
  new_csv_string = g_string_new (NULL);
  for (int i = 0; i < match_list_length; i++) {
    if (g_strcmp0 (self->match_list[i], match) != 0) {
      new_csv_string = g_string_append (new_csv_string, self->match_list[i]);
      if (i+1 != match_list_length)
        new_csv_string = g_string_append (new_csv_string, ",");
    }
  }

  new_csv = g_string_free (new_csv_string, FALSE);

  new_match_list = g_strsplit (new_csv, ",", -1);
  aspam_settings_set_match_list (self,
                                 new_match_list);
  return;

}

void
aspam_settings_add_match (ASpamSettings *self,
                          const char *match)
{
  g_autofree char *csv = NULL;
  g_autofree char *new_csv = NULL;
  g_auto(GStrv) new_match_list = NULL;

  g_assert (ASPAM_IS_SETTINGS (self));

  csv = g_strjoinv (",", self->match_list);
  if (!*csv)
    new_csv = g_strdup_printf("%s", match);
  else
    new_csv = g_strdup_printf("%s,%s", csv, match);

  new_match_list = g_strsplit (new_csv, ",", -1);
  aspam_settings_set_match_list (self,
                                 new_match_list);
  return;

}

static void
aspam_settings_dispose (GObject *object)
{
  ASpamSettings *self = (ASpamSettings *)object;

  ASPAM_TRACE_MSG ("disposing settings");

  g_settings_set_string (self->app_settings, "version", PACKAGE_VERSION);
  g_settings_apply (self->app_settings);

  G_OBJECT_CLASS (aspam_settings_parent_class)->dispose (object);
}

static void
aspam_settings_class_init (ASpamSettingsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = aspam_settings_dispose;
}

static void
aspam_settings_init (ASpamSettings *self)
{
  g_autofree char *version = NULL;
  g_autofree char *contents = NULL;
  g_autoptr(GFile) whitelist_file = NULL;
  g_autoptr(GFile) whitelist_dir = NULL;
  g_autoptr(GError) error = NULL;
  gulong length = 0;

  whitelist_dir = g_file_new_build_filename (g_get_user_config_dir (),
                                              "phoshantispam",
                                              NULL);

  if (!g_file_make_directory_with_parents (whitelist_dir, NULL, &error)) {
    if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_EXISTS)) {
      g_debug ("Directory exists, skipping error...");
      g_clear_error (&error);
    } else if (error) {
      g_warning ("Error creating Directory: %s", error->message);
      g_clear_error (&error);
    }
  }

  whitelist_file = g_file_new_build_filename (g_get_user_config_dir (),
                                              "phoshantispam",
                                              "whitelist.csv",
                                              NULL);

  g_file_load_contents (whitelist_file,
                        NULL,
                        &contents,
                        &length,
                        NULL,
                        &error);

  if (error && g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND)) {
    g_auto(GStrv) new_match_list = NULL;
    g_debug ("There is no CSV File! Making a blank one.");
    new_match_list = g_strsplit ("", ",", -1);
    aspam_settings_set_match_list (self, new_match_list);
    g_clear_error (&error);
  } else if (error) {
    g_warning ("Error loading CSV File: %s. Making a blank one.", error->message);
    g_clear_error (&error);
  } else {
    g_debug ("File Contents %s", contents);
    /* Sometimes there are */
    g_strdelimit (contents, "\r\n", ' ');
    g_strstrip (contents);
    g_debug ("File Contents %s", contents);
    self->match_list = g_strsplit (contents, ",", -1);
  }

  self->app_settings = g_settings_new (PACKAGE_ID);
  version = g_settings_get_string (self->app_settings, "version");

  self->enable_aspamclient = g_settings_get_boolean (self->app_settings, "enable");
  self->allow_blocked_numbers = g_settings_get_boolean (self->app_settings, "allow-blocked-numbers");
  self->allow_callback = g_settings_get_boolean (self->app_settings, "allow-callback");
  self->callback_timeout = g_settings_get_uint64 (self->app_settings, "callback-timeout");
  self->silence = g_settings_get_boolean (self->app_settings, "silence");

  if (!g_str_equal (version, PACKAGE_VERSION))
    self->first_run = TRUE;

  g_settings_delay (self->app_settings);
}

/**
 * aspam_settings_get_default:
 *
 * Create a new #ASpamSettings
 *
 * Returns: (transfer full): A #ASpamSettings.
 * Free with g_object_unref().
 */
ASpamSettings *
aspam_settings_get_default (void)
{
  static ASpamSettings *self;

  if (!self) {
    self = g_object_new (ASPAM_TYPE_SETTINGS, NULL);
    g_object_add_weak_pointer (G_OBJECT (self), (gpointer *)&self);
  }
  return self;
}

/**
 * aspam_settings_save:
 * @self: A #ASpamSettings
 *
 * Save modified settings to disk.  By default,
 * the modified settings are saved to disk only
 * when #ASpamSettings is disposed.  Use this
 * to force save to disk.
 */
void
aspam_settings_save (ASpamSettings *self)
{
  g_return_if_fail (ASPAM_IS_SETTINGS (self));

  g_settings_apply (self->app_settings);
}

/**
 * aspam_settings_get_is_first_run:
 * @self: A #ASpamSettings
 *
 * Get if the application has ever launched after install
 * or update.
 *
 * Returns: %TRUE for the first launch of application after
 * install or update.  %FALSE otherwise.
 */
gboolean
aspam_settings_get_is_first_run (ASpamSettings *self)
{
  g_return_val_if_fail (ASPAM_IS_SETTINGS (self), FALSE);

  return self->first_run;
}

/**
 * aspam_settings_get_window_maximized:
 * @self: A #ASpamSettings
 *
 * Get the window maximized state as saved in @self.
 *
 * Returns: %TRUE if maximized.  %FALSE otherwise.
 */
gboolean
aspam_settings_get_window_maximized (ASpamSettings *self)
{
  g_return_val_if_fail (ASPAM_IS_SETTINGS (self), FALSE);

  return g_settings_get_boolean (self->app_settings, "window-maximized");
}

/**
 * aspam_settings_set_window_maximized:
 * @self: A #ASpamSettings
 * @maximized: The window state to save
 *
 * Set the window maximized state in @self.
 */
void
aspam_settings_set_window_maximized (ASpamSettings *self,
                                   gboolean     maximized)
{
  g_return_if_fail (ASPAM_IS_SETTINGS (self));

  g_settings_set_boolean (self->app_settings, "window-maximized", !!maximized);
}

/**
 * aspam_settings_get_window_geometry:
 * @self: A #ASpamSettings
 * @geometry: (out): A #GdkRectangle
 *
 * Get the window geometry as saved in @self.
 */
void
aspam_settings_get_window_geometry (ASpamSettings  *self,
                                  GdkRectangle *geometry)
{
  g_return_if_fail (ASPAM_IS_SETTINGS (self));
  g_return_if_fail (geometry != NULL);

  g_settings_get (self->app_settings, "window-size", "(ii)",
                  &geometry->width, &geometry->height);
  geometry->x = geometry->y = -1;
}

/**
 * aspam_settings_set_window_geometry:
 * @self: A #ASpamSettings
 * @geometry: A #GdkRectangle
 *
 * Set the window geometry in @self.
 */
void
aspam_settings_set_window_geometry (ASpamSettings  *self,
                                  GdkRectangle *geometry)
{
  g_return_if_fail (ASPAM_IS_SETTINGS (self));
  g_return_if_fail (geometry != NULL);

  g_settings_set (self->app_settings, "window-size", "(ii)",
                  geometry->width, geometry->height);
}
