/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * GData Client
 * Copyright (C) Thibault Saunier 2009 <saunierthibault@gmail.com>
 *
 * GData Client is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * GData Client is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GData Client.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION:gdata-documents-query
 * @short_description: GData Documents query object
 * @stability: Unstable
 * @include: gdata/services/documents/gdata-documents-query.h
 *
 * #GDataDocumentsQuery represents a collection of query parameters specific to the Google Documents service, which go above and beyond
 * those catered for by #GDataQuery.
 *
 * For more information on the custom GData query parameters supported by #GDataDocumentsQuery, see the <ulink type="http"
 * url="http://code.google.com/apis/documents/docs/2.0/reference.html#Parameters">online documentation</ulink>.
 **/

#include <config.h>
#include <glib.h>
#include <glib/gi18n-lib.h>
#include <string.h>

#include "gd/gdata-gd-email-address.h"
#include "gdata-documents-query.h"
#include "gdata-query.h"

#include <gdata/services/documents/gdata-documents-spreadsheet.h>
#include <gdata/services/documents/gdata-documents-presentation.h>
#include <gdata/services/documents/gdata-documents-text.h>
#include <gdata/services/documents/gdata-documents-folder.h>

static void gdata_documents_query_finalize (GObject *object);
static void gdata_documents_query_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void gdata_documents_query_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void get_query_uri (GDataQuery *self, const gchar *feed_uri, GString *query_uri, gboolean *params_started);

struct _GDataDocumentsQueryPrivate {
	gboolean show_deleted;
	gboolean show_folders;
	gboolean exact_title;
	gchar *folder_id;
	gchar *title;
	GList *collaborator_addresses; /* GDataGDEmailAddress */
	GList *reader_addresses; /* GDataGDEmailAddress */
};

enum {
	PROP_SHOW_DELETED = 1,
	PROP_SHOW_FOLDERS,
	PROP_EXACT_TITLE,
	PROP_FOLDER_ID,
	PROP_TITLE
};

G_DEFINE_TYPE (GDataDocumentsQuery, gdata_documents_query, GDATA_TYPE_QUERY)
#define GDATA_DOCUMENTS_QUERY_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GDATA_TYPE_DOCUMENTS_QUERY, GDataDocumentsQueryPrivate))

static void
gdata_documents_query_class_init (GDataDocumentsQueryClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GDataQueryClass *query_class = GDATA_QUERY_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GDataDocumentsQueryPrivate));

	gobject_class->get_property = gdata_documents_query_get_property;
	gobject_class->set_property = gdata_documents_query_set_property;
	gobject_class->finalize = gdata_documents_query_finalize;

	query_class->get_query_uri = get_query_uri;

	/**
	 * GDataDocumentsQuery:show-deleted:
	 *
	 * A shortcut to request all documents that have been deleted.
	 *
	 * Since: 0.4.0
	 **/
	g_object_class_install_property (gobject_class, PROP_SHOW_DELETED,
				g_param_spec_boolean ("show-deleted",
					"Show deleted?", "A shortcut to request all documents that have been deleted.",
					FALSE,
					G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	/**
	 * GDataDocumentsQuery:show-folders:
	 *
	 * Specifies if the request also returns folders.
	 *
	 * Since: 0.4.0
	 **/
	g_object_class_install_property (gobject_class, PROP_SHOW_FOLDERS,
				g_param_spec_boolean ("show-folders",
					"Show folders?", "Specifies if the request also returns folders.",
					FALSE,
					G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	/**
	 * GDataDocumentsQuery:exact-title:
	 *
	 * Specifies whether the query should search for an exact title match for the #GDataDocumentsQuery:title parameter.
	 *
	 * Since: 0.4.0
	 **/
	g_object_class_install_property (gobject_class, PROP_EXACT_TITLE,
				g_param_spec_boolean ("exact-title",
					"Exact title?", "Specifies whether the query should search for an exact title match.",
					FALSE,
					G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	/**
	 * GDataDocumentsQuery:folder-id:
	 *
	 * Specifies the ID of the folder in which to search.
	 *
	 * Since: 0.4.0
	 **/
	g_object_class_install_property (gobject_class, PROP_FOLDER_ID,
				g_param_spec_string ("folder-id",
					"Folder ID", "Specifies the ID of the folder in which to search.",
					NULL,
					G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	/**
	 * GDataDocumentsQuery:title:
	 *
	 * A title (or title fragment) to be searched for. If #GDataDocumentsQuery:exact-title is %TRUE, an exact
	 * title match will be searched for, otherwise substring matches will also be returned.
	 *
	 * Since: 0.4.0
	 **/
	g_object_class_install_property (gobject_class, PROP_TITLE,
				g_param_spec_string ("title",
					"Title", "A title (or title fragment) to be searched for.",
					NULL,
					G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

static void
gdata_documents_query_init (GDataDocumentsQuery *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GDATA_TYPE_DOCUMENTS_QUERY, GDataDocumentsQueryPrivate);
}

static void
gdata_documents_query_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	GDataDocumentsQueryPrivate *priv = GDATA_DOCUMENTS_QUERY_GET_PRIVATE (object);

	switch (property_id) {
		case PROP_SHOW_DELETED:
			g_value_set_boolean (value, priv->show_deleted);
			break;
		case PROP_SHOW_FOLDERS:
			g_value_set_boolean (value, priv->show_folders);
			break;
		case PROP_FOLDER_ID:
			g_value_set_string (value, priv->folder_id);
			break;
		case PROP_EXACT_TITLE:
			g_value_set_boolean (value, priv->exact_title);
			break;
		case PROP_TITLE:
			g_value_set_string (value, priv->title);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

static void
gdata_documents_query_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	GDataDocumentsQuery *self = GDATA_DOCUMENTS_QUERY (object);

	switch (property_id) {
		case PROP_SHOW_DELETED:
			gdata_documents_query_set_show_deleted (self, g_value_get_boolean (value));
			break;
		case PROP_SHOW_FOLDERS:
			gdata_documents_query_set_show_folders (self, g_value_get_boolean (value));
			break;
		case PROP_FOLDER_ID:
			gdata_documents_query_set_folder_id (self, g_value_get_string (value));
			break;
		case PROP_EXACT_TITLE:
			self->priv->exact_title = g_value_get_boolean (value);
			break;
		case PROP_TITLE:
			gdata_documents_query_set_title (self, g_value_get_string (value), TRUE);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

static void
gdata_documents_query_finalize (GObject *object)
{
	GDataDocumentsQueryPrivate *priv = GDATA_DOCUMENTS_QUERY_GET_PRIVATE (object);

	g_free (priv->folder_id);
	g_free (priv->title);

	G_OBJECT_CLASS (gdata_documents_query_parent_class)->finalize (object);
}

static void
get_query_uri (GDataQuery *self, const gchar *feed_uri, GString *query_uri, gboolean *params_started)
{
	GDataDocumentsQueryPrivate *priv = GDATA_DOCUMENTS_QUERY (self)->priv;
	const gchar *entry_id = gdata_query_get_entry_id (self);

	#define APPEND_SEP g_string_append_c (query_uri, (*params_started == FALSE) ? '?' : '&'); *params_started = TRUE;

	if (entry_id == NULL && priv->folder_id != NULL) {
		g_string_append (query_uri, "/folder%3A");
		g_string_append_uri_escaped (query_uri, priv->folder_id, NULL, TRUE);
	}

	/* Chain up to the parent class */
	GDATA_QUERY_CLASS (gdata_documents_query_parent_class)->get_query_uri (self, feed_uri, query_uri, params_started);

	/* Return if the entry ID has been set, since that's handled in the parent class' get_query_uri() function */
	if (entry_id != NULL)
		return;

	if  (priv->collaborator_addresses != NULL) {
		GList *collaborator_address;
		APPEND_SEP
		collaborator_address = priv->collaborator_addresses;

		g_string_append (query_uri, "writer=");
		g_string_append_uri_escaped (query_uri, gdata_gd_email_address_get_address (collaborator_address->data), NULL, TRUE);
		for (collaborator_address = collaborator_address->next; collaborator_address != NULL; collaborator_address = collaborator_address->next) {
			g_string_append_c (query_uri, ';');
			g_string_append_uri_escaped (query_uri, gdata_gd_email_address_get_address (collaborator_address->data), NULL, TRUE);
		}
	}

	if  (priv->reader_addresses != NULL) {
		GList *reader_address;
		APPEND_SEP
		reader_address = priv->reader_addresses;

		g_string_append (query_uri, "reader=");
		g_string_append_uri_escaped (query_uri, gdata_gd_email_address_get_address (reader_address->data), NULL, TRUE);
		for (reader_address = reader_address->next; reader_address != NULL; reader_address = reader_address->next) {
			g_string_append_c (query_uri, ';');
			g_string_append_uri_escaped (query_uri, gdata_gd_email_address_get_address (reader_address->data), NULL, TRUE);
		}
	}

	if (priv->title != NULL) {
		APPEND_SEP
		g_string_append (query_uri, "title=");
		g_string_append_uri_escaped (query_uri, priv->title, NULL, TRUE);
		if (priv->exact_title == TRUE)
			g_string_append (query_uri, "&title-exact=true");
	}

	APPEND_SEP
	if (priv->show_deleted == TRUE)
		g_string_append (query_uri, "showdeleted=true");
	else
		g_string_append (query_uri, "showdeleted=false");

	if (priv->show_folders == TRUE)
		g_string_append (query_uri, "&showfolders=true");
	else
		g_string_append (query_uri, "&showfolders=false");
}

/**
 * gdata_documents_query_new:
 * @q: a query string
 *
 * Creates a new #GDataDocumentsQuery with its #GDataQuery:q property set to @q.
 *
 * Return value: a new #GDataDocumentsQuery
 *
 * Since: 0.4.0
 **/
GDataDocumentsQuery *
gdata_documents_query_new (const gchar *q)
{
	return g_object_new (GDATA_TYPE_DOCUMENTS_QUERY, "q", q, NULL);
}

/**
 * gdata_documents_query_new_with_limits:
 * @q: a query string
 * @start_index: a one-based start index for the results
 * @max_results: the maximum number of results to return
 *
 * Creates a new #GDataDocumentsQuery with its #GDataQuery:q property set to @q, and the limits @start_index and @max_results
 * applied.
 *
 * Return value: a new #GDataDocumentsQuery
 *
 * Since: 0.4.0
 **/
GDataDocumentsQuery *
gdata_documents_query_new_with_limits (const gchar *q, gint start_index, gint max_results)
{
	return g_object_new (GDATA_TYPE_DOCUMENTS_QUERY,
			     "q", q,
			     "start-index", start_index,
			     "max-results", max_results,
			     NULL);
}

/**
 * gdata_documents_query_show_deleted:
 * @self: a #GDataDocumentsQuery
 *
 * Gets the #GDataDocumentsQuery:show_deleted property.
 *
 * Return value: %TRUE if the request should return deleted entries, %FALSE otherwise
 *
 * Since: 0.4.0
 **/
gboolean
gdata_documents_query_show_deleted (GDataDocumentsQuery *self)
{
	g_return_val_if_fail (GDATA_IS_DOCUMENTS_QUERY (self), FALSE);
	return self->priv->show_deleted;
}

/**
 * gdata_documents_query_set_show_deleted:
 * @self: a #GDataDocumentsQuery
 * @show_deleted: %TRUE if the request should return deleted entries, %FALSE otherwise
 *
 * Sets the #GDataDocumentsQuery:show_deleted property to @show_deleted.
 *
 * Since: 0.4.0
 **/
void
gdata_documents_query_set_show_deleted (GDataDocumentsQuery *self, gboolean show_deleted)
{
	g_return_if_fail (GDATA_IS_DOCUMENTS_QUERY (self));
	self->priv->show_deleted = show_deleted;
	g_object_notify (G_OBJECT (self), "show-deleted");
}

/**
 * gdata_documents_query_show_folders:
 * @self: a #GDataDocumentsQuery
 *
 * Gets the #GDataDocumentsQuery:show-folders property.
 *
 * Return value: %TRUE if the request should return folders, %FALSE otherwise
 *
 * Since: 0.4.0
 **/
gboolean
gdata_documents_query_show_folders (GDataDocumentsQuery *self)
{
	g_return_val_if_fail (GDATA_IS_DOCUMENTS_QUERY (self), FALSE);
	return self->priv->show_folders;
}

/**
 * gdata_documents_query_set_show_folders:
 * @self: a #GDataDocumentsQuery
 * @show_folders: %TRUE if the request should return folders, %FALSE otherwise
 *
 * Sets the #GDataDocumentsQuery:show-folders property to show_folders.
 *
 * Since: 0.4.0
 **/
void
gdata_documents_query_set_show_folders (GDataDocumentsQuery *self, gboolean show_folders)
{
	g_return_if_fail (GDATA_IS_DOCUMENTS_QUERY (self));
	self->priv->show_folders = show_folders;
	g_object_notify (G_OBJECT (self), "show-folders");
}

/**
 * gdata_documents_query_get_folder_id:
 * @self: a #GDataDocumentsQuery
 *
 * Gets the #GDataDocumentsQuery:folder-id property.
 *
 * Return value: the ID of the folder to be queried, or %NULL
 *
 * Since: 0.4.0
 **/
const gchar *
gdata_documents_query_get_folder_id (GDataDocumentsQuery *self)
{
	g_return_val_if_fail (GDATA_IS_DOCUMENTS_QUERY (self), NULL);
	return self->priv->folder_id;
}

/**
 * gdata_documents_query_set_folder_id:
 * @self: a #GDataDocumentsQuery
 * @folder_id: the ID of the folder to be queried, or %NULL
 *
 * Sets the #GDataDocumentsQuery:folder-id property to @folder_id.
 *
 * Set @folder_id to %NULL to unset the property in the query URI.
 *
 * Since: 0.4.0
 **/
void
gdata_documents_query_set_folder_id (GDataDocumentsQuery *self, const gchar *folder_id)
{
	g_return_if_fail (GDATA_IS_DOCUMENTS_QUERY (self));

	g_free (self->priv->folder_id);
	self->priv->folder_id = g_strdup (folder_id);
	g_object_notify (G_OBJECT (self), "folder-id");
}

/**
 * gdata_documents_query_get_title:
 * @self: a #GDataDocumentsQuery
 *
 * Gets the #GDataDocumentsQuery:title property.
 *
 * Return value: the title (or title fragment) being queried for, or %NULL
 *
 * Since: 0.4.0
 **/
const gchar *
gdata_documents_query_get_title (GDataDocumentsQuery *self)
{
	g_return_val_if_fail (GDATA_IS_DOCUMENTS_QUERY (self), NULL);
	return self->priv->title;
}

/**
 * gdata_documents_query_get_exact_title:
 * @self: a #GDataDocumentsQuery
 *
 * Gets the #GDataDocumentsQuery:exact-title property.
 *
 * Return value: %TRUE if the query matches the exact title of documents with #GDataDocumentsQuery:title, %FALSE otherwise
 *
 * Since: 0.4.0
 **/
gboolean
gdata_documents_query_get_exact_title (GDataDocumentsQuery *self)
{
	g_return_val_if_fail (GDATA_IS_DOCUMENTS_QUERY (self), FALSE);
	return self->priv->exact_title;
}

/**
 * gdata_documents_query_set_title:
 * @self: a #GDataDocumentsQuery
 * @title: the title (or title fragment) to query for, or %NULL
 * @exact_title: %TRUE if the query should match the exact @title, %FALSE otherwise
 *
 * Sets the #GDataDocumentsQuery:title property to @title.
 *
 * Set @title to %NULL to unset the property in the query URI.
 *
 * Since: 0.4.0
 **/
void
gdata_documents_query_set_title (GDataDocumentsQuery *self, const gchar *title, gboolean exact_title)
{
	g_return_if_fail (GDATA_IS_DOCUMENTS_QUERY (self));

	g_free (self->priv->title);
	self->priv->title = g_strdup (title);
	self->priv->exact_title = exact_title;

	g_object_freeze_notify (G_OBJECT (self));
	g_object_notify (G_OBJECT (self), "exact-title");
	g_object_notify (G_OBJECT (self), "title");
	g_object_thaw_notify (G_OBJECT (self));
}

/**
 * gdata_documents_query_get_collaborator_addresses:
 * @self: a #GDataDocumentsQuery
 *
 * Gets a list of #GDataGDEmailAddress<!-- -->es of the document collaborators whose documents will be queried.
 *
 * Return value: a list of #GDataGDEmailAddress<!-- -->es of the collaborators concerned by the query, or %NULL
 *
 * Since: 0.4.0
 **/
GList *
gdata_documents_query_get_collaborator_addresses (GDataDocumentsQuery *self)
{
	g_return_val_if_fail (GDATA_IS_DOCUMENTS_QUERY (self), NULL);
	return self->priv->collaborator_addresses;
}

/**
 * gdata_documents_query_get_reader_addresses:
 * @self: a #GDataDocumentsQuery
 *
 * Gets a list of #GDataGDEmailAddress<!-- -->es of the document readers whose documents will be queried.
 *
 * Return value: a list of #GDataGDEmailAddress<!-- -->es of the readers concerned by the query, or %NULL
 *
 * Since: 0.4.0
 **/
GList *
gdata_documents_query_get_reader_addresses (GDataDocumentsQuery *self)
{
	g_return_val_if_fail (GDATA_IS_DOCUMENTS_QUERY (self), NULL);
	return self->priv->reader_addresses;
}

/**
 * gdata_documents_query_add_reader:
 * @self: a #GDataDocumentsQuery
 * @email_address: the e-mail address of the reader to add
 *
 * Add @email_address as a #GDataGDEmailAddress to the list of readers, the documents readable by whom will be queried.
 *
 * Since: 0.4.0
 **/
void
gdata_documents_query_add_reader (GDataDocumentsQuery *self, const gchar *email_address)
{
	GDataGDEmailAddress *address;

	g_return_if_fail (GDATA_IS_DOCUMENTS_QUERY (self));
	g_return_if_fail (email_address != NULL && *email_address != '\0');

	address = gdata_gd_email_address_new (email_address, "reader", NULL, FALSE);
	self->priv->reader_addresses = g_list_append (self->priv->reader_addresses, address);
}

/**
 * gdata_documents_query_add_collaborator:
 * @self: a #GDataDocumentsQuery
 * @email_address: the e-mail address of the collaborator to add
 *
 * Add @email_address as a #GDataGDEmailAddress to the list of collaborators whose edited documents will be queried.
 *
 * Since: 0.4.0
 **/
void
gdata_documents_query_add_collaborator (GDataDocumentsQuery *self, const gchar *email_address)
{
	GDataGDEmailAddress *address;

	g_return_if_fail (GDATA_IS_DOCUMENTS_QUERY (self));
	g_return_if_fail (email_address != NULL && *email_address != '\0');

	address = gdata_gd_email_address_new (email_address, "collaborator", NULL, FALSE);
	self->priv->collaborator_addresses = g_list_append (self->priv->collaborator_addresses, address);
}