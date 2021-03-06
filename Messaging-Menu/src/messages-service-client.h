/* Generated by dbus-binding-tool; do not edit! */

#include <glib.h>
#include <dbus/dbus-glib.h>

G_BEGIN_DECLS

#ifndef _DBUS_GLIB_ASYNC_DATA_FREE
#define _DBUS_GLIB_ASYNC_DATA_FREE
static
#ifdef G_HAVE_INLINE
inline
#endif
void
_dbus_glib_async_data_free (gpointer stuff)
{
	g_slice_free (DBusGAsyncData, stuff);
}
#endif

#ifndef DBUS_GLIB_CLIENT_WRAPPERS_org_ayatana_indicator_messages_service
#define DBUS_GLIB_CLIENT_WRAPPERS_org_ayatana_indicator_messages_service

static
#ifdef G_HAVE_INLINE
inline
#endif
gboolean
org_ayatana_indicator_messages_service_watch (DBusGProxy *proxy, GError **error)

{
  return dbus_g_proxy_call (proxy, "Watch", error, G_TYPE_INVALID, G_TYPE_INVALID);
}

typedef void (*org_ayatana_indicator_messages_service_watch_reply) (DBusGProxy *proxy, GError *error, gpointer userdata);

static void
org_ayatana_indicator_messages_service_watch_async_callback (DBusGProxy *proxy, DBusGProxyCall *call, void *user_data)
{
  DBusGAsyncData *data = (DBusGAsyncData*) user_data;
  GError *error = NULL;
  dbus_g_proxy_end_call (proxy, call, &error, G_TYPE_INVALID);
  (*(org_ayatana_indicator_messages_service_watch_reply)data->cb) (proxy, error, data->userdata);
  return;
}

static
#ifdef G_HAVE_INLINE
inline
#endif
DBusGProxyCall*
org_ayatana_indicator_messages_service_watch_async (DBusGProxy *proxy, org_ayatana_indicator_messages_service_watch_reply callback, gpointer userdata)

{
  DBusGAsyncData *stuff;
  stuff = g_slice_new (DBusGAsyncData);
  stuff->cb = G_CALLBACK (callback);
  stuff->userdata = userdata;
  return dbus_g_proxy_begin_call (proxy, "Watch", org_ayatana_indicator_messages_service_watch_async_callback, stuff, _dbus_glib_async_data_free, G_TYPE_INVALID);
}
static
#ifdef G_HAVE_INLINE
inline
#endif
gboolean
org_ayatana_indicator_messages_service_attention_requested (DBusGProxy *proxy, gboolean* OUT_dot, GError **error)

{
  return dbus_g_proxy_call (proxy, "AttentionRequested", error, G_TYPE_INVALID, G_TYPE_BOOLEAN, OUT_dot, G_TYPE_INVALID);
}

typedef void (*org_ayatana_indicator_messages_service_attention_requested_reply) (DBusGProxy *proxy, gboolean OUT_dot, GError *error, gpointer userdata);

static void
org_ayatana_indicator_messages_service_attention_requested_async_callback (DBusGProxy *proxy, DBusGProxyCall *call, void *user_data)
{
  DBusGAsyncData *data = (DBusGAsyncData*) user_data;
  GError *error = NULL;
  gboolean OUT_dot;
  dbus_g_proxy_end_call (proxy, call, &error, G_TYPE_BOOLEAN, &OUT_dot, G_TYPE_INVALID);
  (*(org_ayatana_indicator_messages_service_attention_requested_reply)data->cb) (proxy, OUT_dot, error, data->userdata);
  return;
}

static
#ifdef G_HAVE_INLINE
inline
#endif
DBusGProxyCall*
org_ayatana_indicator_messages_service_attention_requested_async (DBusGProxy *proxy, org_ayatana_indicator_messages_service_attention_requested_reply callback, gpointer userdata)

{
  DBusGAsyncData *stuff;
  stuff = g_slice_new (DBusGAsyncData);
  stuff->cb = G_CALLBACK (callback);
  stuff->userdata = userdata;
  return dbus_g_proxy_begin_call (proxy, "AttentionRequested", org_ayatana_indicator_messages_service_attention_requested_async_callback, stuff, _dbus_glib_async_data_free, G_TYPE_INVALID);
}
static
#ifdef G_HAVE_INLINE
inline
#endif
gboolean
org_ayatana_indicator_messages_service_icon_shown (DBusGProxy *proxy, gboolean* OUT_hidden, GError **error)

{
  return dbus_g_proxy_call (proxy, "IconShown", error, G_TYPE_INVALID, G_TYPE_BOOLEAN, OUT_hidden, G_TYPE_INVALID);
}

typedef void (*org_ayatana_indicator_messages_service_icon_shown_reply) (DBusGProxy *proxy, gboolean OUT_hidden, GError *error, gpointer userdata);

static void
org_ayatana_indicator_messages_service_icon_shown_async_callback (DBusGProxy *proxy, DBusGProxyCall *call, void *user_data)
{
  DBusGAsyncData *data = (DBusGAsyncData*) user_data;
  GError *error = NULL;
  gboolean OUT_hidden;
  dbus_g_proxy_end_call (proxy, call, &error, G_TYPE_BOOLEAN, &OUT_hidden, G_TYPE_INVALID);
  (*(org_ayatana_indicator_messages_service_icon_shown_reply)data->cb) (proxy, OUT_hidden, error, data->userdata);
  return;
}

static
#ifdef G_HAVE_INLINE
inline
#endif
DBusGProxyCall*
org_ayatana_indicator_messages_service_icon_shown_async (DBusGProxy *proxy, org_ayatana_indicator_messages_service_icon_shown_reply callback, gpointer userdata)

{
  DBusGAsyncData *stuff;
  stuff = g_slice_new (DBusGAsyncData);
  stuff->cb = G_CALLBACK (callback);
  stuff->userdata = userdata;
  return dbus_g_proxy_begin_call (proxy, "IconShown", org_ayatana_indicator_messages_service_icon_shown_async_callback, stuff, _dbus_glib_async_data_free, G_TYPE_INVALID);
}
#endif /* defined DBUS_GLIB_CLIENT_WRAPPERS_org_ayatana_indicator_messages_service */

G_END_DECLS
