/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/* aspam-settings.h
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

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ASPAM_TYPE_SETTINGS (aspam_settings_get_type ())

G_DECLARE_FINAL_TYPE (ASpamSettings, aspam_settings, ASPAM, SETTINGS, GObject)

ASpamSettings *aspam_settings_new                  (void);
void         aspam_settings_save                 (ASpamSettings *self);

gboolean     aspam_settings_get_is_first_run     (ASpamSettings *self);
gboolean     aspam_settings_get_window_maximized (ASpamSettings *self);
void         aspam_settings_set_window_maximized (ASpamSettings *self,
                                                gboolean     maximized);
void         aspam_settings_get_window_geometry  (ASpamSettings  *self,
                                                GdkRectangle *geometry);
void         aspam_settings_set_window_geometry  (ASpamSettings  *self,
                                                GdkRectangle *geometry);
gboolean     aspam_settings_get_enable_aspamclient (ASpamSettings *self);
void         aspam_settings_set_enable_aspamclient (ASpamSettings *self,
                                                    gboolean enable);
gboolean     aspam_settings_get_allow_blocked_numbers (ASpamSettings *self);
void         aspam_settings_set_allow_blocked_numbers (ASpamSettings *self,
                                                       gboolean enable);
gboolean     aspam_settings_get_allow_callback (ASpamSettings *self);
void         aspam_settings_set_allow_callback (ASpamSettings *self,
                                                gboolean enable);
int          aspam_settings_get_callback_timeout (ASpamSettings *self);
void         aspam_settings_set_callback_timeout (ASpamSettings *self,
                                                  int timeout);
const char **aspam_settings_get_match_list (ASpamSettings *self);
void         aspam_settings_set_match_list (ASpamSettings *self,
                                           char **match_list);
G_END_DECLS
