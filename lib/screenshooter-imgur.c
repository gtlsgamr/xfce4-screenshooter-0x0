/*  $Id$
 *
 *  Copyright © 2009-2010 Sebastian Waisbrot <seppo0010@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * */


#include "screenshooter-imgur.h"
#include "screenshooter-job-callbacks.h"
#include <string.h>
#include <stdlib.h>
#include <libsoup/soup.h>
#include <libxml/parser.h>

static gboolean          imgur_upload_job          (ScreenshooterJob  *job,
                                                    GArray            *param_values,
                                                    GError           **error);

static gboolean
imgur_upload_job (ScreenshooterJob *job, GArray *param_values, GError **error)
{
  const gchar *image_path, *title;
  guchar *online_file_name = NULL;
  guchar *delete_hash = NULL;
  const gchar *proxy_uri;
  GUri *soup_proxy_uri;
#ifdef DEBUG
  SoupLogger *log;
#endif
  SoupSession *session;
  SoupMessage *msg;
  GBytes *buf, *response;
  GMappedFile *mapping;
  SoupMultipart *mp;
  xmlDoc *doc;
  xmlNode *root_node, *child_node;

  /*const gchar *upload_url = "https://api.imgur.com/3/upload.xml";*/
  const gchar *upload_url = "https://0.vern.cc";

  GError *tmp_error = NULL;

  g_return_val_if_fail (SCREENSHOOTER_IS_JOB (job), FALSE);
  g_return_val_if_fail (param_values != NULL, FALSE);
  g_return_val_if_fail (param_values->len == 2, FALSE);
  g_return_val_if_fail ((G_VALUE_HOLDS_STRING (&g_array_index(param_values, GValue, 0))), FALSE);
  g_return_val_if_fail ((G_VALUE_HOLDS_STRING (&g_array_index(param_values, GValue, 1))), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_object_set_data (G_OBJECT (job), "jobtype", "imgur");
  if (exo_job_set_error_if_cancelled (EXO_JOB (job), error))
    return FALSE;


  image_path = g_value_get_string (&g_array_index (param_values, GValue, 0));
  title = g_value_get_string (&g_array_index (param_values, GValue, 1));

  session = soup_session_new ();
#ifdef DEBUG
  log = soup_logger_new (SOUP_LOGGER_LOG_HEADERS);
  soup_session_add_feature (session, (SoupSessionFeature *)log);
#endif

  /* Set the proxy URI if any */
  proxy_uri = g_getenv ("http_proxy");

  if (proxy_uri != NULL)
    {
      soup_proxy_uri = g_uri_parse (proxy_uri, G_URI_FLAGS_NONE, NULL);
      g_object_set (session, "proxy-uri", soup_proxy_uri, NULL);
      g_uri_unref (soup_proxy_uri);
    }

  mapping = g_mapped_file_new (image_path, FALSE, NULL);
  if (!mapping) {
    g_object_unref (session);
    return FALSE;
  }

  mp = soup_multipart_new(SOUP_FORM_MIME_TYPE_MULTIPART);
  buf = g_mapped_file_get_bytes (mapping);

  soup_multipart_append_form_file (mp, "file", title, NULL, buf);
  soup_multipart_append_form_string (mp, "name", title);
  soup_multipart_append_form_string (mp, "title", title);
  msg = soup_message_new_from_multipart (upload_url, mp);

  /* for v3 API - key registered *only* for xfce4-screenshooter! 
  soup_message_headers_append (soup_message_get_request_headers (msg), "Authorization", "Client-ID 66ab680b597e293");*/
  exo_job_info_message (EXO_JOB (job), _("Upload the screenshot..."));

  response = soup_session_send_and_read (session, msg, NULL, &tmp_error);

  g_mapped_file_unref (mapping);
  g_bytes_unref (buf);
  g_object_unref (session);
  g_object_unref (msg);
#ifdef DEBUG
  g_object_unref (log);
#endif

  if (!response)
    {
      TRACE ("Error during the POST exchange: %s\n", tmp_error->message);
      g_propagate_error (error, tmp_error);
      return FALSE;
    }

  TRACE("response was %s\n", (gchar*) g_bytes_get_data (response, NULL));
  /* returned XML is like <data type="array" success="1" status="200"><id>xxxxxx</id> 
  doc = xmlParseMemory (g_bytes_get_data (response, NULL), g_bytes_get_size (response));

  root_node = xmlDocGetRootElement(doc);
  for (child_node = root_node->children; child_node; child_node = child_node->next)
  {
    if (xmlStrEqual(child_node->name, (const xmlChar *) "id"))
      online_file_name = xmlNodeGetContent(child_node);
    else if (xmlStrEqual (child_node->name, (const xmlChar *) "deletehash"))
      delete_hash = xmlNodeGetContent (child_node);
  }*/

  char *abc = (guchar*) g_bytes_get_data (response, NULL);

  printf("-------------------%s",abc+17);
  TRACE("found picture id %s\n", abc);
  xmlFreeDoc(doc);
  g_bytes_unref (response);

  screenshooter_job_image_uploaded (job,
                                    (const gchar*) abc+17,
                                    (const gchar*) abc+17);

  g_free (online_file_name);
  //g_free (abc);
  g_free (delete_hash);

  return TRUE;
}


/* Public */



/**
 * screenshooter_upload_to_imgur:
 * @image_path: the local path of the image that should be uploaded to
 * imgur.com.
 *
 * Uploads the image whose path is @image_path
 *
 **/

gboolean screenshooter_upload_to_imgur   (const gchar  *image_path,
                                          const gchar  *title)
{
  ScreenshooterJob *job;
  GtkWidget *dialog, *label;

  g_return_val_if_fail (image_path != NULL, TRUE);

  dialog = create_spinner_dialog(_("Imgur"), &label);

  job = screenshooter_simple_job_launch (imgur_upload_job, 2,
                                          G_TYPE_STRING, image_path,
                                          G_TYPE_STRING, title);

  /* dismiss the spinner dialog after success or error */
  g_signal_connect_swapped (job, "error", G_CALLBACK (gtk_widget_hide), dialog);
  g_signal_connect_swapped (job, "image-uploaded", G_CALLBACK (gtk_widget_hide), dialog);

  g_signal_connect (job, "ask", G_CALLBACK (cb_ask_for_information), NULL);
  g_signal_connect (job, "image-uploaded", G_CALLBACK (cb_image_uploaded), NULL);
  g_signal_connect (job, "error", G_CALLBACK (cb_error), dialog);
  g_signal_connect (job, "finished", G_CALLBACK (cb_finished), dialog);
  g_signal_connect (job, "info-message", G_CALLBACK (cb_update_info), label);

  return gtk_dialog_run (GTK_DIALOG (dialog)) != DIALOG_RESPONSE_ERROR;
}
