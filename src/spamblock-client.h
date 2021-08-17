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

#pragma once

#include <gio/gio.h>


#define CALLS_SERVICE	                     "org.gnome.Calls"
#define CALLS_PATH	                     "/org/gnome/Calls"
#define FREEDESKTOP_OBJECT_MANAGER_INTERFACE "org.freedesktop.DBus.ObjectManager"

G_BEGIN_DECLS

#define SPAM_TYPE_SPAMD (spam_block_get_type ())

G_DECLARE_FINAL_TYPE (SpamBlock, spam_block, SPAM, SPAMD, GObject)

/* From Calls calls-call.h */
typedef enum
{
  CALLS_CALL_STATE_ACTIVE = 1,
  CALLS_CALL_STATE_HELD,
  CALLS_CALL_STATE_DIALING,
  CALLS_CALL_STATE_ALERTING,
  CALLS_CALL_STATE_INCOMING,
  CALLS_CALL_STATE_WAITING,
  CALLS_CALL_STATE_DISCONNECTED
} CallsCallState;

SpamBlock *spam_block_get_default (void);

G_END_DECLS
