/* aspam-client.c
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

#define G_LOG_DOMAIN "aspam-client"

#include "aspam-client.h"
#include "aspam-settings.h"

#define SETTING_NAME     "gnome_calls_spam_options"
#define SETTINGS_GROUP_SPAM_BLOCK "Spam Block"

struct _ASpamClient
{
  GObject     parent_instance;

  /* dbus connection */
  GDBusConnection  *connection;
  unsigned int      calls_watch_id;

  /* dbus proxy */
  GDBusProxy       *calls_phone_proxy;
  unsigned int      calls_phone_signal_added_id;
  unsigned int      calls_phone_signal_removed_id;

  char *allow_callback_number;
};

static gboolean
aspamclient_remove_callback_number (gpointer user_data)
{
  ASpamClient *self = user_data;
  g_debug("Removing callback number");
  g_clear_pointer(&self->allow_callback_number, g_free);

  return FALSE;
}

static void
hang_up_call (ASpamClient *self,
              const char *interface,
              const char *objectpath,
              const char *id,
              gboolean    allow_callback)
{

  g_autoptr(GError) error = NULL;
  GDBusProxy *hangup_proxy;
  ASpamSettings *settings = aspam_settings_get_default ();

  hangup_proxy = g_dbus_proxy_new_sync(
		self->connection,
		G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
		NULL,
		CALLS_SERVICE,
		objectpath,
		interface,
		NULL,
		&error);

  if (error != NULL) {
    g_warning ("Error making proxy: %s\n", error->message);
    return;
  }
  if (aspam_settings_get_silence (settings)) {
    g_warning ("This requires Calls 41.1 and above!");
    g_dbus_proxy_call_sync(hangup_proxy,
                           "Silence",
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           &error);
  }
  if (!aspam_settings_get_silence (settings) || error) {
    if (error) {
      g_warning ("Error Silencing call: %s", error->message);
      g_clear_error (&error);
      g_warning ("Attempting to hang up instead");
    }
    g_dbus_proxy_call_sync(hangup_proxy,
                           "Hangup",
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           &error);
  }

  if (error != NULL) {
    g_warning ("Error making proxy call: %s\n", error->message);
    return;
  }

  if (allow_callback) {
    guint64 callback_timeout;
    g_clear_pointer(&self->allow_callback_number, g_free);
    if (!*id) {
      g_debug ("Caller was anonymous, not allowing callback");
      return;
    }
    g_debug ("Allowing callback");
    self->allow_callback_number = g_strdup(id);
    callback_timeout = 60*aspam_settings_get_callback_timeout (settings);
    //Setting timeout to user defined timeout;
    g_timeout_add_seconds (callback_timeout, aspamclient_remove_callback_number, self);
  } else {
    g_debug ("Not allowing callback");
  }

}

static void
call_added_cb (GDBusConnection *connection,
               const char      *sender_name,
               const char      *object_path,
               const char      *interface_name,
               const char      *signal_name,
               GVariant        *parameters,
               gpointer         user_data)
{
  ASpamClient *self = user_data;
  GVariant *properties, *properties_container, *interface_variant, *dict_variant;
  g_autofree char *objectpath = NULL;
  g_autofree char *interface = NULL;
  g_autofree char *id = NULL;
  g_autofree char *displayname = NULL;
  g_autofree char *protocol = NULL;
  gboolean encrypted, inbound;
  unsigned int match_allow_length;
  unsigned int match_allow_length_counter;
  guint32 state;
  GVariantDict dict;
  ASpamSettings *settings;
  char **match_allow;

  g_debug ("Call Added!");
  settings = aspam_settings_get_default ();
  g_variant_get (parameters, "(o@a{?*})", &objectpath, &properties_container);
  properties = g_variant_get_child_value (properties_container, 0);
  interface_variant = g_variant_get_child_value (properties, 0);
  dict_variant = g_variant_get_child_value (properties, 1);
  g_variant_get (interface_variant, "s", &interface);

  g_variant_dict_init (&dict, dict_variant);
  g_variant_dict_lookup (&dict, "Inbound", "b", &inbound);
  g_variant_dict_lookup (&dict, "State", "u", &state);
  g_variant_dict_lookup (&dict, "Id", "s", &id);
  g_variant_dict_lookup (&dict, "DisplayName", "s", &displayname);
  g_variant_dict_lookup (&dict, "Protocol", "s", &protocol);
  g_variant_dict_lookup (&dict, "Encrypted", "b", &encrypted);

  g_variant_unref (properties);
  g_variant_unref (interface_variant);
  g_variant_unref (dict_variant);

  if (!aspam_settings_get_enable_aspamclient (settings)) {
    g_debug ("Spam filtering is disabled!");
    return;
  }

  if (g_strcmp0 (protocol, "tel") != 0) {
    g_debug ("Not a phone call, ignoring");
    return;
  }
  if (state != CALLS_CALL_STATE_INCOMING) {
    g_debug ("State is not INCOMING: %u", state);
    return;
  }
  if (inbound == FALSE) {
    g_debug ("Inbound is not INCOMING: %d", inbound);
    return;
  }

  if (strlen(displayname) > 0) {
    g_autofree char *spam_test = NULL;
    spam_test = g_utf8_strdown (displayname, -1);
    if (g_strcmp0 (spam_test, "spam") != 0) {
      return;
    } else {
      g_warning ("Contact Marked as spam");
      hang_up_call (self, interface, objectpath, id, FALSE);
      return;
    }
  }

  if (g_strcmp0 (id, self->allow_callback_number) == 0) {
    g_debug ("Number calling back, allowing");
    return;
  }

  match_allow = aspam_settings_get_match_list (settings);
  match_allow_length = g_strv_length (match_allow);
  for (match_allow_length_counter = 0; match_allow_length_counter < match_allow_length; match_allow_length_counter++) {
    if (strstr(id, match_allow[match_allow_length_counter]) != NULL) {
       g_debug ("Number is allowed to call, matches pattern %s", match_allow[match_allow_length_counter]);
       return;
    }
  }
  g_warning ("Number does not match allowed list");
  if (!(strlen(id) > 0)) {
    g_debug("Id is blank, this is a blocked number");
    if (aspam_settings_get_allow_blocked_numbers (settings)) {
      g_debug ("Allowing blocked number");
      return;
    } else {
      g_warning ("Not allowing blocked number");
    }
  }
  hang_up_call (self, interface, objectpath, id, aspam_settings_get_allow_callback (settings));

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
  //ASpamClient *self = user_data;
  g_debug("Call Removed!");
}

 /* gdbus monitor --session -d org.gnome.Calls  to observe behavior */

static void
get_calls_object_manager_interface_cb (GObject      *manager,
                                       GAsyncResult *res,
                                       gpointer      user_data)
{
  ASpamClient *self = user_data;
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
  ASpamClient *self = user_data;
  g_debug("Calls Appeared");

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
  ASpamClient *self = user_data;
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

G_DEFINE_TYPE (ASpamClient, aspam_client, G_TYPE_OBJECT)

static void
aspam_client_constructed (GObject *object)
{
  //ASpamClient *self = (ASpamClient *)object;

  G_OBJECT_CLASS (aspam_client_parent_class)->constructed (object);

}

static void
aspam_client_finalize (GObject *object)
{
  ASpamClient *self = (ASpamClient *)object;

  g_clear_pointer(&self->allow_callback_number, g_free);

  G_OBJECT_CLASS (aspam_client_parent_class)->finalize (object);
}

static void
aspam_client_class_init (ASpamClientClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = aspam_client_constructed;
  object_class->finalize = aspam_client_finalize;
}

static void
aspam_client_init (ASpamClient *self)
{
}

ASpamClient *
aspam_client_get_default (void)
{
  static ASpamClient *self;

  if (!self) {
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
