/*
 * Copyright © 2011 Rodolfo Ribeiro Gomes <rodolforg arr0ba gmail.com>
 *
 * A really simple text editor
 *

    SimpleNotepad is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SimpleNotepad is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SimpleNotepad.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>

#include "filehandler.h"
#include "message_dialogs.h"

#include <glib/gi18n.h>

#define GETTEXT_PACKAGE "simple-notepad"
#define LOCALEDIR "mo"

struct GUI_widgets
{
	GtkWidget *main_window;
	GtkWidget *textview;
	GtkWidget *statusbar;
	Filehandler *fh;
};

// Filehandler callbacks
static void notepad_new(gpointer data);
static gboolean notepad_open(const gchar *filename, gpointer data);
static gboolean notepad_save(gpointer data);
static gboolean notepad_save_as(const gchar *filename, gpointer data);
static void notepad_close(gpointer data);

// Load GUI stuff
static gboolean load_gui(Filehandler *fh, struct GUI_widgets *widgets)
{
	GtkBuilder * builder;
	GError *err = NULL;

	builder = gtk_builder_new();
	if (builder == NULL)
		return FALSE;

	gtk_builder_add_from_file(builder, "simple-notepad.ui", &err);
	if (err != NULL)
	{
		g_error_free(err);
		g_object_unref(builder);
		g_print(_("Couldn't load interface file.\n"));
		return FALSE;
	}

	gtk_builder_connect_signals(builder, fh);

	widgets->main_window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
	if (widgets->main_window == NULL)
	{
		g_print(_("Corrupted interface file?"));
		g_object_unref(builder);
		return FALSE;
	}

	widgets->textview = GTK_WIDGET(gtk_builder_get_object(builder, "textview1"));
	widgets->statusbar = GTK_WIDGET(gtk_builder_get_object(builder, "statusbar1"));
	
	fh->actions.save = GTK_ACTION(gtk_builder_get_object(builder, "action_save"));
	fh->actions.save_as = GTK_ACTION(gtk_builder_get_object(builder, "action_save_as"));
	fh->actions.close = GTK_ACTION(gtk_builder_get_object(builder, "action_close"));

	g_object_unref(builder);


	gtk_window_set_icon_name(GTK_WINDOW(widgets->main_window), GTK_STOCK_JUSTIFY_LEFT);

	return TRUE;
}

int main(int argc, char *argv[])
{
	Filehandler *fh;
	FilehandlerCallbacks cb;
	struct GUI_widgets widgets;
	
	// Internationalization stuff
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");	
	textdomain (GETTEXT_PACKAGE);
	
	// Initialize GTK
	gtk_init(&argc, &argv);
	
	// Create the document file handler callbacks
	cb.new = notepad_new;
	cb.open = notepad_open;
	cb.save = notepad_save;
	cb.save_as = notepad_save_as;
	cb.close = notepad_close;
	cb.include_in_recents = NULL;

	// Create the document file handler
	fh = filehandler_new(&cb, NULL, &widgets);
	if (fh == NULL)
	{
		showErrorMessage(NULL, _("Couldn't allocate memory!"));
		return 1;
	}

	// Load GUI
	if (!load_gui(fh, &widgets))
	{
		showErrorMessage(NULL, _("Couldn't open the window!"));
		filehandler_destroy(fh);
		return 1;
	}
	
	widgets.fh = fh;

	fh->main_window = widgets.main_window;
	filehandler_update_action_status(fh);

	// Display the window
	gtk_widget_show_all(fh->main_window);

	// Start the GTK event loop
	gtk_main();
	
	// Destroy the handler
	filehandler_destroy(fh);
	
	// Return 0 if exit is successful
	return 0;
}

// Callback for document change
//  As it is a GTK callback, data contains a Filehandler pointer
//    since it was set by gtk_builder_connect_signals
G_MODULE_EXPORT
void on_textbuffer1_changed(GtkTextBuffer *buffer, gpointer data)
{
	Filehandler *fh = data;

	filehandler_file_changed(fh, TRUE);
}

// Filehandler callbacks
//   Those callbacks contains "user" data - that loaded into Filehandler structure

static void notepad_new(gpointer data)
{
	struct GUI_widgets *widgets = data;
	
	gtk_widget_set_sensitive(widgets->textview, TRUE);
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->textview))), "", -1);
}

static gboolean notepad_open(const gchar *filename, gpointer data)
{
	struct GUI_widgets *widgets = data;
	
	GError *error = NULL;
	gchar *contents;
	gsize length;
	
	GtkTextBuffer *buffer;
	
	if (!g_file_get_contents(filename, &contents, &length, &error))
	{
		showErrorMessage(GTK_WINDOW(widgets->main_window), error->message);
		g_error_free(error);
		return FALSE;
	}
	
	buffer = GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->textview)));
	gtk_text_buffer_set_text(buffer, contents, length);
	g_free(contents);
	
	gtk_widget_set_sensitive(widgets->textview, TRUE);
	
	return TRUE;
}

static gboolean notepad_save(gpointer data)
{
	struct GUI_widgets *widgets = data;
	
	return notepad_save_as(filehandler_get_filename(widgets->fh), data);
}

static gboolean notepad_save_as(const gchar *filename, gpointer data)
{
	struct GUI_widgets *widgets = data;
	
	GError *error = NULL;
	gchar *contents;
	
	GtkTextBuffer *buffer;
	
	GtkTextIter start, end;

	buffer = GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->textview)));
	
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);
	
	contents = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
	
	if (!g_file_set_contents(filename, contents, -1, &error))
	{
		g_free(contents);
		showErrorMessage(GTK_WINDOW(widgets->main_window), error->message);
		g_error_free(error);
		return FALSE;
	}
	
	g_free(contents);
	
	return TRUE;
}

static void notepad_close(gpointer data)
{
	struct GUI_widgets *widgets = data;
	gtk_widget_set_sensitive(widgets->textview, FALSE);
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->textview))), "", -1);
}

