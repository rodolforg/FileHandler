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
 
#include "filehandler.h"
#include "message_dialogs.h"

#include <glib/gi18n.h>

///////////////////////////////////
// Contructors & destructors
///////////////////////////////////

// Allocate and initialize a Filehandler structure
Filehandler *filehandler_new(FilehandlerCallbacks *callbacks, GtkWidget *main_window, gpointer user_data)
{
	Filehandler *fh = g_new0(Filehandler, 1);
	if (fh == NULL)
		return NULL;
	fh->file_changes_saved = TRUE;

	if (callbacks != NULL)
		fh->callbacks = *callbacks;

	fh->main_window = main_window;
	fh->user_data = user_data;

	return fh;
}

// Properly desallocate a Filehandler structure
//   It doesn't destroy/free user_data and main_window.
void filehandler_destroy(Filehandler *fh)
{
	if (fh == NULL)
		return;
	g_free(fh->current_filename);
	g_free(fh->last_dir);
	g_free(fh);
}

///////////////////////////////////
// For internal use
///////////////////////////////////

// The default name for new files
static gchar * const FILENAME_NOT_SAVED="";

// Checks if there are no opened files
#define IS_CLOSED(fh) (fh->current_filename == NULL)

// If the file is saved, close it.
//   Otherwise, ask the user what to do: discard changes, save them or don't close it.
static gboolean try_close_file(Filehandler *fh);

// Checks if the name of current file is not empty or a default for new ones.
static gboolean is_file_named(Filehandler *fh)
{
	return !IS_CLOSED(fh) && g_strcmp0(fh->current_filename, FILENAME_NOT_SAVED) != 0;
}

///////////////////////////////////
// Accessors & mutators
///////////////////////////////////

// Get the name of the current file
const gchar *filehandler_get_filename(const Filehandler *fh)
{
	return fh != NULL? fh->current_filename : NULL;
}

// Get the name of the last chosen directory
const gchar *filehandler_get_directory(const Filehandler *fh)
{
	return fh != NULL? fh->last_dir : NULL;
}

// Tell Filehandler that the file has been changed.
//   You should really use this, as based on this information,
//   the user will be asked or not if he wants to save/close the file
//   when he tries to open another one or close the current one.
void filehandler_file_changed(Filehandler *fh, gboolean changed)
{
	if (fh != NULL && !IS_CLOSED(fh))
	{
		fh->file_changes_saved = !changed;
		if (fh->actions.save != NULL)
			gtk_action_set_sensitive(fh->actions.save, !fh->file_changes_saved);
	}
}

// Filehandler will enable/disable GtkActions "open", "save" and "save as".
void filehandler_update_action_status(const Filehandler *fh)
{
	if (fh == NULL)
		return;

	if (fh->actions.save != NULL)
		gtk_action_set_sensitive(fh->actions.save, (!IS_CLOSED(fh)) && !fh->file_changes_saved);
	if (fh->actions.save_as != NULL)
		gtk_action_set_sensitive(fh->actions.save_as, !IS_CLOSED(fh));
	if (fh->actions.close != NULL)
		gtk_action_set_sensitive(fh->actions.close, !IS_CLOSED(fh));

}

///////////////////////////////////
// New file
///////////////////////////////////

G_MODULE_EXPORT
void filehandler_on_action_new_activate(GtkAction *action, gpointer data)
{
	if (data == NULL)
		return;

	Filehandler *fh = data;

	if (fh->callbacks.new == NULL)
	{
		showWarningMessage(GTK_WINDOW(fh->main_window), _("You aren't allowed to create a new file."));
		return;
	}

	if (!try_close_file(fh))
		return;

	fh->callbacks.new(fh->user_data);

	fh->current_filename = FILENAME_NOT_SAVED;
	fh->file_changes_saved = TRUE;

	if (fh->actions.save != NULL)
		gtk_action_set_sensitive(fh->actions.save, TRUE);
	if (fh->actions.save_as != NULL)
		gtk_action_set_sensitive(fh->actions.save_as, TRUE);
	if (fh->actions.close != NULL)
		gtk_action_set_sensitive(fh->actions.close, TRUE);
}

///////////////////////////////////
// Open file
///////////////////////////////////

// Open a file called filename.
//   No file must be opened on filehandler.
static gboolean do_open_file(Filehandler *fh, const gchar *filename)
{
	if (! fh->callbacks.open(filename, fh->user_data))
		return FALSE;

	if (is_file_named(fh))
		g_free(fh->current_filename);
	fh->current_filename = g_strdup(filename);
	g_free(fh->last_dir);
	fh->last_dir = g_path_get_dirname(filename);

	fh->file_changes_saved = TRUE;

	if (fh->actions.save != NULL)
		gtk_action_set_sensitive(fh->actions.save, TRUE);
	if (fh->actions.save_as != NULL)
		gtk_action_set_sensitive(fh->actions.save_as, TRUE);
	if (fh->actions.close != NULL)
		gtk_action_set_sensitive(fh->actions.close, TRUE);


	// Add to recent files list
	if (fh->callbacks.include_in_recents != NULL)
		fh->callbacks.include_in_recents(filename, fh->user_data);

	return TRUE;
}

G_MODULE_EXPORT
void filehandler_on_action_open_activate (GtkAction *action, gpointer data)
{
	if (data == NULL)
		return;

	Filehandler *fh = data;

	GtkWidget * dialog = gtk_file_chooser_dialog_new(_("Open file..."),
			GTK_WINDOW(fh->main_window), GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_OPEN, GTK_RESPONSE_OK, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, NULL);

	if (fh->last_dir != NULL)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
				fh->last_dir);

	gint result = gtk_dialog_run(GTK_DIALOG(dialog));

	if (result != GTK_RESPONSE_OK)
	{
		gtk_widget_destroy(dialog);
		return;
	}

	gchar *filename;
	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	gtk_widget_destroy(dialog);

	if (!try_close_file(fh))
		return;

	do_open_file(fh, filename);
	g_free(filename);
}

// Open a file without user choose which one through a file browser dialog,
//   e.g., a file picked on a recent file list.
//   The user will be asked if he accepts close the current file.
// Returns TRUE if the file was opened, FALSE otherwise.
//   If user doesn't allow to close the current file, it will return FALSE.
gboolean filehandler_open_file (Filehandler *fh, const gchar *filename)
{
	if (fh == NULL || filename == NULL)
		return FALSE;
	if (!try_close_file(fh))
		return FALSE;

	return do_open_file(fh, filename);
}

///////////////////////////////////
// Save file
///////////////////////////////////

static gboolean do_save_file(Filehandler *fh)
{
	if (fh->callbacks.save == NULL)
	{
		showWarningMessage(GTK_WINDOW(fh->main_window), _("You aren't allowed to save a file."));
		return FALSE;
	}

	// Save
	if (! fh->callbacks.save(fh->user_data))
	{
		return FALSE;
	}

	fh->file_changes_saved = TRUE;
	if (fh->actions.save != NULL)
		gtk_action_set_sensitive(fh->actions.save, FALSE);

	return TRUE;
}

static gboolean do_save_as_file(Filehandler *fh)
{
	if (fh->callbacks.save_as == NULL)
	{
		showWarningMessage(GTK_WINDOW(fh->main_window), _("You aren't allowed to save as another file."));
		return FALSE;
	}

	// Let user choose the file name
	GtkWidget * dialog = gtk_file_chooser_dialog_new(_("Save as..."),
			GTK_WINDOW(fh->main_window),
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_SAVE, GTK_RESPONSE_OK, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, NULL);

	if (fh->last_dir != NULL)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
				fh->last_dir);

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	gint result = gtk_dialog_run(GTK_DIALOG(dialog));

	// User gave up?
	if (result != GTK_RESPONSE_OK)
	{
		gtk_widget_destroy(dialog);
		return FALSE;
	}

	gchar *filename;
	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	gtk_widget_destroy(dialog);


	// It's an overwrite?
	if (g_strcmp0(filename, fh->current_filename) == 0)
	{
		// The file name is the same as the current. Just save it.
		g_free(filename);
		do_save_file(fh);
		return FALSE;
	}

	// It's a real Save As
	// Finally save
	if ( !fh->callbacks.save_as(filename, fh->user_data))
	{
		g_free(filename);
		return FALSE;
	}

	// Update current file name and current directory
	if (is_file_named(fh))
		g_free(fh->current_filename);
	fh->current_filename = filename;

	g_free(fh->last_dir);
	fh->last_dir = g_path_get_dirname(fh->current_filename);

	// Add to recent files list
	if (fh->callbacks.include_in_recents != NULL)
		fh->callbacks.include_in_recents(fh->current_filename, fh->user_data);

	// Update status
	fh->file_changes_saved = TRUE;

	if (fh->actions.save != NULL)
		gtk_action_set_sensitive(fh->actions.save, FALSE);

	return TRUE;
}

G_MODULE_EXPORT
void filehandler_on_action_save_as_activate(GtkAction *action, gpointer data)
{
	if (data == NULL)
		return;

	Filehandler *fh = data;

	// Is there a file to save?
	if (IS_CLOSED(fh))
		return;

	do_save_as_file(fh);

}

G_MODULE_EXPORT
void filehandler_on_action_save_activate(GtkAction *action, gpointer data)
{
	if (data == NULL)
		return;

	Filehandler *fh = data;

	// If it's a new file, make user choose its name
	if (!is_file_named(fh))
	{
		filehandler_on_action_save_as_activate(action, data);
		return;
	}

	do_save_file(fh);
}

// Save the current file.
//   This can be used for autosaving.
// Returns TRUE if the file was saved.
//   If the file has not a name (e.g, a new file), it will return FALSE.
gboolean filehandler_save_file (Filehandler *fh)
{
	if (fh == NULL)
		return FALSE;
	if (IS_CLOSED(fh))
		return FALSE;
	if (!is_file_named(fh))
		return FALSE;

	do_save_file(fh);

	return TRUE;
}


///////////////////////////////////
// Close file
///////////////////////////////////

G_MODULE_EXPORT
void filehandler_on_action_close_activate(GtkAction *action, gpointer data)
{
	if (data == NULL)
		return;

	Filehandler *fh = data;
	try_close_file(fh);
}

static void do_close_file(Filehandler *fh)
{
	fh->callbacks.close(fh->user_data);
	// FIXME: And if it should support multiple files (tabs or windows)?
	//          close() callback shouldn't destroy Filehandler structure?
	if (is_file_named(fh))
		g_free(fh->current_filename);
	fh->current_filename = NULL;

	fh->file_changes_saved = TRUE;
	if (fh->actions.save != NULL)
		gtk_action_set_sensitive(fh->actions.save, FALSE);
	if (fh->actions.save_as != NULL)
		gtk_action_set_sensitive(fh->actions.save_as, FALSE);
	if (fh->actions.close != NULL)
		gtk_action_set_sensitive(fh->actions.close, FALSE);

}

// If the file is saved, close it.
//   Otherwise, ask the user what to do: discard changes, save them or don't close it.
static gboolean try_close_file(Filehandler *fh)
{
	if (!fh->file_changes_saved)
	{
		// Ask if it should be saved, discarded or cancelled
		gboolean (*save_func) (Filehandler*) = NULL;
		if (is_file_named(fh) && fh->callbacks.save != NULL)
			save_func = do_save_file;
		else if (fh->callbacks.save_as != NULL)
			save_func = do_save_as_file;

		if (save_func != NULL)
		{
			gint result = showYesNoCancelDialog(GTK_WINDOW(fh->main_window),
					_("There are unsaved changes.\nDo you want to save them before close this file?"));
			if (result == GTK_RESPONSE_CANCEL)
				return FALSE;
			if (result == GTK_RESPONSE_YES)
			{
				if (!save_func(fh))
					return FALSE;
			}
			do_close_file(fh);
			return TRUE;
		}
	}

	if (! IS_CLOSED(fh))
		do_close_file(fh);
	return TRUE;
}

///////////////////////////////////
// Exit
///////////////////////////////////

static void exit_program(Filehandler *fh)
{
	gtk_widget_destroy(fh->main_window);
	gtk_main_quit();
}

G_MODULE_EXPORT
gboolean filehandler_on_main_window_delete_event(GtkWidget *widget,
		GdkEvent *event, gpointer data)
{
	if (data == NULL)
		return FALSE;

	Filehandler *fh = data;

	if (try_close_file(fh))
	{
		exit_program(fh);
		return FALSE;
	}

	return TRUE;
}

G_MODULE_EXPORT
void filehandler_on_action_quit_activate(GtkAction *action, gpointer data)
{
	if (data == NULL)
		return;

	Filehandler *fh = data;

	if (try_close_file(fh))
		exit_program(fh);
}

