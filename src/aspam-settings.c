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
};

G_DEFINE_TYPE (ASpamSettings, aspam_settings, G_TYPE_OBJECT)


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

  self->app_settings = g_settings_new (PACKAGE_ID);
  version = g_settings_get_string (self->app_settings, "version");

  if (!g_str_equal (version, PACKAGE_VERSION))
    self->first_run = TRUE;

  g_settings_delay (self->app_settings);
}

/**
 * aspam_settings_new:
 *
 * Create a new #ASpamSettings
 *
 * Returns: (transfer full): A #ASpamSettings.
 * Free with g_object_unref().
 */
ASpamSettings *
aspam_settings_new (void)
{
  return g_object_new (ASPAM_TYPE_SETTINGS, NULL);
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
