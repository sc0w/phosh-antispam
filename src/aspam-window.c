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

#include "aspam-window.h"
#include "aspam-log.h"

struct _ASpamWindow
{
  GtkApplicationWindow parent_instance;

  ASpamSettings *settings;

  GtkWidget   *menu_button;
};

G_DEFINE_TYPE (ASpamWindow, aspam_window, GTK_TYPE_APPLICATION_WINDOW)


enum {
  PROP_0,
  PROP_SETTINGS,
  N_PROPS
};

static GParamSpec *properties[N_PROPS];

static void
aspam_window_show_about (ASpamWindow *self)
{
  const char *authors[] = {
    "Chris Talbot https://www.sadiqpk.org",
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

  gtk_widget_class_bind_template_callback (widget_class, aspam_window_show_about);
}

static void
aspam_window_init (ASpamWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
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
