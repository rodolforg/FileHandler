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

#ifndef R_GTKMESSAGEDIALOGS_H_
#define R_GTKMESSAGEDIALOGS_H_

#include <gtk/gtk.h>

void showErrorMessage (GtkWindow *parent, const gchar *msg);
void showWarningMessage (GtkWindow *parent, const gchar *msg);
gint showYesNoDialog (GtkWindow *parent, const gchar *msg);
gint showYesNoCancelDialog (GtkWindow *parent, const gchar *msg);

#endif // R_GTKMESSAGEDIALOGS

