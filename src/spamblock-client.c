/* spamblock-client.c
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

#include "spamblock-client.h"

struct _SpamBlock
{
  GObject     parent_instance;

  GDBusConnection  *connection;
  unsigned int      calls_watch_id;

  GDBusProxy       *calls_phone_proxy;
  unsigned int      calls_phone_signal_added_id;
  unsigned int      calls_phone_signal_removed_id;
};

static void
call_added_cb (GDBusConnection *connection,
                              const char      *sender_name,
                              const char      *object_path,
                              const char      *interface_name,
                              const char      *signal_name,
                              GVariant        *parameters,
                              gpointer         user_data)
{
  //SpamBlock *self = user_data;
  g_debug("Call Added!");
}

static void
call_removed_cb (GDBusConnection *connection,
                              const char      *sender_name,
                              const char      *object_path,
                              const char      *interface_name,
                              const char      *signal_name,
                              GVariant        *parameters,
                              gpointer         user_data)
{
  //SpamBlock *self = user_data;
  g_debug("Call Removed!");
}

 /* gdbus monitor --session -d org.gnome.Calls  to observe behavior */

static void
get_calls_object_manager_interface_cb (GObject      *manager,
                             GAsyncResult *res,
                             gpointer      user_data)
{
  SpamBlock *self = user_data;
  g_autoptr (GError) error = NULL;

  self->calls_phone_proxy = g_dbus_proxy_new_finish (res, &error);

  if (self->calls_phone_proxy == NULL)
    g_warning ("Proxy is NULL");

  if (error != NULL) {
    g_warning ("Error in Calls Phone Proxy call: %s\n", error->message);
  } else {
    g_debug ("Got Calls Phone Proxy");

    self->calls_phone_signal_added_id = g_dbus_connection_signal_subscribe (self->connection,
                                        CALLS_SERVICE,
                                        FREEDESKTOP_OBJECT_MANAGER_INTERFACE,
                                        "InterfacesAdded",
                                        CALLS_PATH,
                                        NULL,
                                        G_DBUS_SIGNAL_FLAGS_NONE,
                                        (GDBusSignalCallback)call_added_cb,
                                        self,
                                        NULL);

    self->calls_phone_signal_removed_id = g_dbus_connection_signal_subscribe (self->connection,
                                        CALLS_SERVICE,
                                        FREEDESKTOP_OBJECT_MANAGER_INTERFACE,
                                        "InterfacesRemoved",
                                        CALLS_PATH,
                                        NULL,
                                        G_DBUS_SIGNAL_FLAGS_NONE,
                                        (GDBusSignalCallback)call_removed_cb,
                                        self,
                                        NULL);


  }
}

static void
calls_appeared_cb (GDBusConnection *connection,
                  const char      *name,
                  const char      *name_owner,
                  gpointer         user_data)
{
  SpamBlock *self = user_data;
  g_debug("VVMD Appeared");

  self->connection = connection;

  g_dbus_proxy_new (self->connection,
                    G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                    NULL,
	            CALLS_SERVICE,
	            CALLS_PATH,
	            FREEDESKTOP_OBJECT_MANAGER_INTERFACE,
	            NULL,
	            get_calls_object_manager_interface_cb,
	            self);

}

static void
calls_vanished_cb (GDBusConnection *connection,
                  const char      *name,
                  gpointer        user_data)
{
  SpamBlock *self = user_data;
  g_debug("Calls Vanished");

  if (G_IS_DBUS_CONNECTION (self->connection)) {
    g_dbus_connection_unregister_object (self->connection,
                                         self->calls_watch_id);

  }
  if (G_IS_OBJECT (self->calls_phone_proxy)) {
    g_object_unref (self->calls_phone_proxy);
    g_dbus_connection_signal_unsubscribe (self->connection,
                                          self->calls_phone_signal_added_id);
    g_dbus_connection_signal_unsubscribe (self->connection,
                                          self->calls_phone_signal_removed_id);
  }

}

G_DEFINE_TYPE (SpamBlock, spam_block, G_TYPE_OBJECT)

static void
spam_block_constructed (GObject *object)
{
  //SpamBlock *self = (SpamBlock *)object;

  G_OBJECT_CLASS (spam_block_parent_class)->constructed (object);

}

static void
spam_block_finalize (GObject *object)
{
  SpamBlock *self = (SpamBlock *)object;

  calls_vanished_cb (NULL, NULL, object);
  g_bus_unwatch_name (self->calls_watch_id);

  G_OBJECT_CLASS (spam_block_parent_class)->finalize (object);
}

static void
spam_block_class_init (SpamBlockClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = spam_block_constructed;
  object_class->finalize = spam_block_finalize;
}

static void
spam_block_init (SpamBlock *self)
{
}

SpamBlock *
spam_block_get_default (void)
{
  static SpamBlock *self;

  if (!self)
    {
      self = g_object_new (SPAM_TYPE_SPAMD, NULL);
      g_object_add_weak_pointer (G_OBJECT (self), (gpointer *)&self);

      self->calls_watch_id = g_bus_watch_name (G_BUS_TYPE_SESSION,
                                               CALLS_SERVICE,
                                               G_BUS_NAME_WATCHER_FLAGS_AUTO_START,
                                               (GBusNameAppearedCallback)calls_appeared_cb,
                                               (GBusNameVanishedCallback)calls_vanished_cb,
                                               self, NULL);


    }
  return self;
}
