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

#ifndef R_FILEHANDLER_H_
#define R_FILEHANDLER_H_

#include "gtk/gtk.h"

// These are Filehandler callbacks
//   Filehandler calls them when the user really wants to do some action.
//   If a function should not be used at all, set it as NULL pointer.
//   user_data is the data passed to the Filehandler structure.
//   If the operations "open", "save" and "save as" can't be done, they should
//   return FALSE and tell it to user. Otherwise, if they're successful, make
//   them return TRUE instead.
typedef struct {
	void (*new)(gpointer user_data);
	gboolean (*open)(const gchar *filename, gpointer user_data);
	gboolean (*save)(gpointer user_data);
	gboolean (*save_as)(const gchar *filename, gpointer user_data);
	void (*close)(gpointer user_data);
	void (*include_in_recents)(const gchar *filename, gpointer user_data);
} FilehandlerCallbacks;

// A set of GtkAction used by the GUI for file handling.
//  Those not NULL will have your sensitiveness set properly.
//  Example: if there isn't a file opened, actions "save" and "close" will
//  be disabled.
//  There isn't an action "new" or "open" here, because they're supposed to be
//  always enabled.
typedef struct {
	GtkAction *save;
	GtkAction *save_as;
	GtkAction *close;
} FilehandlerGtkActions;


typedef struct {
	gboolean file_changes_saved;
	gchar *current_filename;
	gchar *last_dir;
	GtkWidget *main_window;

	gpointer user_data;

	FilehandlerCallbacks callbacks;
	FilehandlerGtkActions actions;
} Filehandler;

// Allocate and initialize a Filehandler structure
Filehandler *filehandler_new(FilehandlerCallbacks *callbacks, GtkWidget *main_window, gpointer user_data);

// Properly desallocate a Filehandler structure
//   It doesn't destroy/free user_data and main_window.
void filehandler_destroy(Filehandler *fh);


// Get the name of the current file
const gchar *filehandler_get_filename(const Filehandler *fh);

// Get the name of the last chosen directory
const gchar *filehandler_get_directory(const Filehandler *fh);

// Tell Filehandler that the file has been changed.
//   You should really use this, as based on this information,
//   the user will be asked or not if he wants to save/close the file
//   when he tries to open another one or close the current one.
void filehandler_file_changed(Filehandler *fh, gboolean changed);

// Filehandler will enable/disable GtkActions "open", "save" and "save as".
void filehandler_update_action_status(const Filehandler *fh);


// Open a file without user choose which one through a file browser dialog,
//   e.g., a file picked on a recent file list.
//   The user will be asked if he accepts close the current file.
// Returns TRUE if the file was opened, FALSE otherwise.
//   If user doesn't allow to close the current file, it will return FALSE.
gboolean filehandler_open_file (Filehandler *fh, const gchar *filename);

// Save the current file.
//   This can be used for autosaving.
// Returns TRUE if the file was saved.
//   If the file has not a name (e.g, a new file), it will return FALSE.
gboolean filehandler_save_file (Filehandler *fh);


// GTK action callbacks that should be used as signal or called by one
//    data must point to a Filehandler structure
//    action value is ignored
void filehandler_on_action_new_activate (GtkAction *action, gpointer data);
void filehandler_on_action_open_activate (GtkAction *action, gpointer data);
void filehandler_on_action_save_as_activate (GtkAction *action, gpointer data);
void filehandler_on_action_save_activate (GtkAction *action, gpointer data);
void filehandler_on_action_close_activate (GtkAction *action, gpointer data);
gboolean filehandler_on_main_window_delete_event (GtkWidget *widget,
		GdkEvent *event, gpointer data);
void filehandler_on_action_quit_activate (GtkAction *action, gpointer data);

#endif //  R_FILEHANDLER_H_

