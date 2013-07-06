/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright (C) 2010 Collabora, Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Nicolas Dufresne <nicolas.dufresne@collabora.co.uk>
 */

#include "config.h"

#include "mock-proxy.h"

#include <glib.h>
#include <gio/gio.h>

struct _GDataMockProxyResolver {
  GObject parent_instance;
};

static void gdata_mock_proxy_resolver_iface_init (GProxyResolverInterface *iface);

#define gdata_mock_proxy_resolver_get_type _gdata_mock_proxy_resolver_get_type
G_DEFINE_TYPE_WITH_CODE (GDataMockProxyResolver, gdata_mock_proxy_resolver, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (G_TYPE_PROXY_RESOLVER,
						gdata_mock_proxy_resolver_iface_init)
			       { GIOExtensionPoint *ep = g_io_extension_point_register (G_PROXY_RESOLVER_EXTENSION_POINT_NAME);
      g_io_extension_point_set_required_type (ep, G_TYPE_PROXY_RESOLVER); /* HACK to ensure the extension point's registered */ }
			 g_io_extension_point_implement (G_PROXY_RESOLVER_EXTENSION_POINT_NAME,
							 g_define_type_id,
							 "gdata-mock",
							 1000))

static void
gdata_mock_proxy_resolver_finalize (GObject *object)
{
  /* must chain up */
  G_OBJECT_CLASS (gdata_mock_proxy_resolver_parent_class)->finalize (object);
}

static void
gdata_mock_proxy_resolver_init (GDataMockProxyResolver *resolver)
{
}

static gboolean
gdata_mock_proxy_resolver_is_supported (GProxyResolver *resolver)
{
  return TRUE;
}

static gchar **
gdata_mock_proxy_resolver_lookup (GProxyResolver  *resolver,
			       const gchar     *uri,
			       GCancellable    *cancellable,
			       GError         **error)
{
  gchar **proxies;

  if (g_cancellable_set_error_if_cancelled (cancellable, error))
    return NULL;

  proxies = g_new0 (gchar *, 2);
  proxies[0] = g_strdup ("https://127.0.0.1:443");

  return proxies;
}

static void
gdata_mock_proxy_resolver_lookup_async (GProxyResolver      *resolver,
				     const gchar         *uri,
				     GCancellable        *cancellable,
				     GAsyncReadyCallback  callback,
				     gpointer             user_data)
{
  GError *error = NULL;
  GTask *task;
  gchar **proxies;

  task = g_task_new (resolver, cancellable, callback, user_data);

  proxies = gdata_mock_proxy_resolver_lookup (resolver, uri, cancellable, &error);
  if (proxies)
    g_task_return_pointer (task, proxies, (GDestroyNotify) g_strfreev);
  else
    g_task_return_error (task, error);
  g_object_unref (task);
}

static gchar **
gdata_mock_proxy_resolver_lookup_finish (GProxyResolver     *resolver,
				      GAsyncResult       *result,
				      GError            **error)
{
  g_return_val_if_fail (g_task_is_valid (result, resolver), NULL);

  return g_task_propagate_pointer (G_TASK (result), error);
}

static void
gdata_mock_proxy_resolver_class_init (GDataMockProxyResolverClass *resolver_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (resolver_class);
  object_class->finalize = gdata_mock_proxy_resolver_finalize;
}

static void
gdata_mock_proxy_resolver_iface_init (GProxyResolverInterface *iface)
{
  iface->is_supported = gdata_mock_proxy_resolver_is_supported;
  iface->lookup = gdata_mock_proxy_resolver_lookup;
  iface->lookup_async = gdata_mock_proxy_resolver_lookup_async;
  iface->lookup_finish = gdata_mock_proxy_resolver_lookup_finish;
}
