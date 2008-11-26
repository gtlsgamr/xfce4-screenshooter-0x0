/*  $Id$
 *
 *  Copyright © 2004 German Poo-Caaman~o <gpoo@ubiobio.cl>
 *  Copyright © 2005,2006 Daniel Bobadilla Leal <dbobadil@dcc.uchile.cl>
 *  Copyright © 2005 Jasper Huijsmans <jasper@xfce.org>
 *  Copyright © 2006 Jani Monoses <jani@ubuntu.com>
 *  Copyright © 2008 Jérôme Guelfucci <jerome.guelfucci@gmail.com>
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
t */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include <libxfcegui4/libxfcegui4.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4panel/xfce-panel-convenience.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <X11/Xatom.h>

#include "libscreenshooter.h"

#define SCREENSHOT_ICON_NAME  "applets-screenshooter"

/* Struct containing all panel plugin data */
typedef struct
{
  XfcePanelPlugin *plugin;

  GtkWidget *button;
  GtkWidget *image;
  
  int style_id;
  ScreenshotData *sd;
}
PluginData;



/* Protoypes */
                                   
static void 
screenshooter_plugin_construct       (XfcePanelPlugin      *plugin);

static void 
screenshooter_plugin_read_rc_file    (XfcePanelPlugin      *plugin, 
                                      PluginData           *pd);  
              
static void 
screenshooter_plugin_write_rc_file   (XfcePanelPlugin      *plugin, 
                                      PluginData           *pd);

static gboolean 
cb_set_size                          (XfcePanelPlugin      *plugin, 
                                      int                   size, 
                                      PluginData           *pd);
                                   
static void 
cb_properties_dialog                 (XfcePanelPlugin      *plugin,
                                      PluginData           *pd);
                                      
static void
cb_dialog_response                   (GtkWidget            *dlg, 
                                      int                   reponse,
                                      PluginData           *pd);
                                   
static void 
cb_free_data                         (XfcePanelPlugin      *plugin, 
                                      PluginData           *pd);
                                  
static void 
cb_button_clicked                    (GtkWidget            *button, 
                                      PluginData           *pd);

static void
cb_style_set                         (XfcePanelPlugin      *plugin, 
                                      gpointer              ignored,
                                      PluginData           *pd);
                                       
                                              

/* Register the panel plugin */
XFCE_PANEL_PLUGIN_REGISTER_INTERNAL (screenshooter_plugin_construct);



/* Internal functions */



/* Modify the size of the panel button 
Returns TRUE if succesful. 
*/
static gboolean
cb_set_size (XfcePanelPlugin *plugin, int size, PluginData *pd)
{
  GdkPixbuf *pb;
  int width = size - 2 - 2 * MAX (pd->button->style->xthickness,
                                    pd->button->style->ythickness);

  pb = xfce_themed_icon_load (SCREENSHOT_ICON_NAME, width);
  gtk_image_set_from_pixbuf (GTK_IMAGE (pd->image), pb);
  g_object_unref (pb);
  gtk_widget_set_size_request (GTK_WIDGET (plugin), size, size);

  return TRUE;
}



/* Free the panel plugin data stored in pd
plugin: a XfcePanelPlugin (a screenshooter one).
pd: the associated PluginData. 
*/
static void
cb_free_data (XfcePanelPlugin *plugin, PluginData *pd)
{
  if (pd->style_id)
  	g_signal_handler_disconnect (plugin, pd->style_id);

  pd->style_id = 0;
  g_free (pd->sd->screenshot_dir);
  #ifdef HAVE_GIO
  g_free (pd->sd->app);
  #endif
  g_free (pd->sd);
  g_free (pd);
}



/* Take the screenshot when the button is clicked.
button: the panel button.
pd: the PluginData storing the options for taking the screenshot.
*/
static void
cb_button_clicked (GtkWidget *button, PluginData *pd)
{
  /* Make the button unclickable so that the user does not press it while 
	another screenshot is in progress */
	gtk_widget_set_sensitive (GTK_WIDGET (pd->button), FALSE);

  screenshooter_take_and_output_screenshot (pd->sd);
  
  /* Make the panel button clickable */
	gtk_widget_set_sensitive (GTK_WIDGET (pd->button), TRUE);
}



/* Set the style of the panel plugin.
plugin: a XfcePanelPlugin (a screenshooter one).
pd: the associated PluginData.
*/
static void
cb_style_set (XfcePanelPlugin *plugin, gpointer ignored,
                       PluginData *pd)
{
  cb_set_size (plugin, xfce_panel_plugin_get_size (plugin), pd);
}



/* Read the rc file associated to the panel plugin and store the options in pd.
plugin: a XfcePanelPlugin (a screenshooter one).
pd: the associated PluginData.
*/
static void
screenshooter_plugin_read_rc_file (XfcePanelPlugin *plugin, PluginData *pd)
{
  screenshooter_read_rc_file (xfce_panel_plugin_lookup_rc_file (plugin), 
                              pd->sd, 
                              FALSE);
}



/* Write the pd options in the rc file associated to plugin
plugin: a XfcePanelPlugin (a screenshooter one).
pd: the associated PluginData.
*/
static void
screenshooter_plugin_write_rc_file (XfcePanelPlugin *plugin, PluginData *pd)
{
  screenshooter_write_rc_file (xfce_panel_plugin_save_location (plugin, TRUE), 
                               pd->sd);  
}



/* Callback for dialog response:
   Update the tooltips if using gtk >= 2.12.
   Unblock the plugin contextual menu.
   Save the options in the rc file.*/
static void
cb_dialog_response (GtkWidget *dlg, int reponse,
                    PluginData *pd)
{
  g_object_set_data (G_OBJECT (pd->plugin), "dialog", NULL);

  gtk_widget_destroy (dlg);
  
  /* Update tooltips according to the chosen option */
  #if GTK_CHECK_VERSION(2,12,0)
  if (pd->sd->mode == FULLSCREEN)
  {
    gtk_widget_set_tooltip_text (GTK_WIDGET (pd->button),
                                 _("Take a screenshot of the entire screen"));
  }
  else
  {
    gtk_widget_set_tooltip_text (GTK_WIDGET (pd->button),
                                 _("Take a screenshot of the active window"));
  }
  #endif
  
  /* Unblock the menu and save options */
  xfce_panel_plugin_unblock_menu (pd->plugin);
  screenshooter_plugin_write_rc_file (pd->plugin, pd);
}



/* Properties dialog to set the plugin options */
static void
cb_properties_dialog (XfcePanelPlugin *plugin, PluginData *pd)
{
  GtkWidget *dlg;
  
  dlg = screenshooter_dialog_new (pd->sd, TRUE);
        
  /* Block the menu to prevent the user from launching several dialogs at
  the same time */
  xfce_panel_plugin_block_menu (plugin);
  
  g_object_set_data (G_OBJECT (plugin), "dialog", dlg);
  
  g_signal_connect (dlg, "response", G_CALLBACK (cb_dialog_response),
                    pd);

  gtk_widget_show (dlg);
}



/* Create the plugin button */
static void
screenshooter_plugin_construct (XfcePanelPlugin *plugin)
{
  /* Initialise the data structs */
  PluginData *pd = g_new0 (PluginData, 1);
  ScreenshotData *sd = g_new0 (ScreenshotData, 1);

  pd->sd = sd;
  
  xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  pd->plugin = plugin;

  /* Read the options */
  screenshooter_plugin_read_rc_file (plugin, pd);
  
  /* Create the panel button */
  pd->button = xfce_create_panel_button ();

  pd->image = gtk_image_new ();

  gtk_container_add (GTK_CONTAINER (pd->button), GTK_WIDGET (pd->image));
  
  /* Set the tooltips if available */
  #if GTK_CHECK_VERSION(2,12,0)
  if (pd->sd->mode == FULLSCREEN)
  {
    gtk_widget_set_tooltip_text (GTK_WIDGET (pd->button),
                                 _("Take a screenshot of the entire screen"));
  }
  else
  {
    gtk_widget_set_tooltip_text (GTK_WIDGET (pd->button),
                                 _("Take a screenshot of the active window"));
  }
  #endif
    
  gtk_widget_show_all (pd->button);
  
  gtk_container_add (GTK_CONTAINER (plugin), pd->button);
  xfce_panel_plugin_add_action_widget (plugin, pd->button);
  
  /* Set the callbacks */
  g_signal_connect (pd->button, "clicked",
                    G_CALLBACK (cb_button_clicked), pd);

  g_signal_connect (plugin, "free-data",
                    G_CALLBACK (cb_free_data), pd);

  g_signal_connect (plugin, "size-changed",
                    G_CALLBACK (cb_set_size), pd);

  pd->style_id =
      g_signal_connect (plugin, "style-set",
                        G_CALLBACK (cb_style_set), pd);

  xfce_panel_plugin_menu_show_configure (plugin);
  g_signal_connect (plugin, "configure-plugin",
                    G_CALLBACK (cb_properties_dialog), pd);
}
XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL (screenshooter_plugin_construct);
