/*
 * Copyright © 2011 Rodolfo Ribeiro Gomes <rodolforg arr0ba gmail.com>
 *
 * A file handler for GTK+
 *
    This file is part of GtkFileHandler.

    GtkFileHandler is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GtkFileHandler is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GtkFileHandler.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "message_dialogs.h"

#include <glib/gi18n.h>

void showErrorMessage (GtkWindow *parent, const gchar *msg)
{
	const gchar *appname = g_get_application_name();
	
	gchar *msg1 = g_strdup_printf(_("%s - Error!"), appname);
	GtkWidget *dialog = gtk_message_dialog_new(parent,
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, msg1);
	g_free(msg1);
	
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), msg);
	
	gtk_window_set_title(GTK_WINDOW(dialog), appname);
	if (parent)
	{
		GdkPixbuf *icon = gtk_window_get_icon(parent);
		gtk_window_set_icon(GTK_WINDOW(dialog), icon);
	}
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void showWarningMessage (GtkWindow *parent, const gchar *msg)
{
	const gchar *appname = g_get_application_name();
	gchar *msg1 = g_strdup_printf(_("%s - Warning!"), appname);
	GtkWidget *dialog = gtk_message_dialog_new(parent,
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
			GTK_BUTTONS_CLOSE, msg1);
	g_free(msg1);
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), msg);
	gtk_window_set_title(GTK_WINDOW(dialog), appname);
	if (parent)
	{
		GdkPixbuf *icon = gtk_window_get_icon(parent);
		gtk_window_set_icon(GTK_WINDOW(dialog), icon);
	}
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

gint showYesNoDialog (GtkWindow *parent, const gchar *msg)
{
	GtkWidget *dialog = gtk_message_dialog_new(parent,
			GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
			msg);
	gtk_window_set_title(GTK_WINDOW(dialog), g_get_application_name());
	if (parent)
	{
		GdkPixbuf *icon = gtk_window_get_icon(parent);
		gtk_window_set_icon(GTK_WINDOW(dialog), icon);
	}
	gint result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return result;
}

gint showYesNoCancelDialog (GtkWindow *parent, const gchar *msg)
{
	GtkWidget *dialog = gtk_message_dialog_new(parent,
			GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
			msg);
	gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_YES, GTK_RESPONSE_YES,
			GTK_STOCK_NO, GTK_RESPONSE_NO,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			NULL);

	gtk_window_set_title(GTK_WINDOW(dialog), g_get_application_name());
	if (parent)
	{
		GdkPixbuf * icon = gtk_window_get_icon(parent);
		gtk_window_set_icon(GTK_WINDOW(dialog), icon);
	}

	gint result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return result;
}
