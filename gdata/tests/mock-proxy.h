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

#ifndef __GDATA_MOCK_PROXY_RESOLVER_H__
#define __GDATA_MOCK_PROXY_RESOLVER_H__

#include <gio/gio.h>

G_BEGIN_DECLS

#define G_TYPE_DUMMY_PROXY_RESOLVER         (_gdata_mock_proxy_resolver_get_type ())
#define GDATA_MOCK_PROXY_RESOLVER(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_DUMMY_PROXY_RESOLVER, GDataMockProxyResolver))
#define GDATA_MOCK_PROXY_RESOLVER_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_DUMMY_PROXY_RESOLVER, GDataMockProxyResolverClass))
#define G_IS_DUMMY_PROXY_RESOLVER(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), G_TYPE_DUMMY_PROXY_RESOLVER))
#define G_IS_DUMMY_PROXY_RESOLVER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), G_TYPE_DUMMY_PROXY_RESOLVER))
#define GDATA_MOCK_PROXY_RESOLVER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), G_TYPE_DUMMY_PROXY_RESOLVER, GDataMockProxyResolverClass))

typedef struct _GDataMockProxyResolver       GDataMockProxyResolver;
typedef struct _GDataMockProxyResolverClass  GDataMockProxyResolverClass;


struct _GDataMockProxyResolverClass {
	GObjectClass parent_class;
};

GType		_gdata_mock_proxy_resolver_get_type       (void);


G_END_DECLS

#endif /* __GDATA_MOCK_PROXY_RESOLVER_H__ */
