SimpleNotepad
=============

Description
-------------
This is a really simple text viewer/editor.
It was created as an use case of GtkFileHandler.

Requirements
-------------
* GLib >= 2.8
* GModule >= 2.0
* GTK+ >= 2.14 ( >= 3.0 included)
* Filehandler initial release

How to compile
-------------
If you've just unpacked, here is a quick (and not pretty) command line to do so.
### For GTK+ 2
	$ gcc -o simple-notepad main.c -I ../src ../src/*.c `pkg-config --cflags --libs gtk+-2.0 gmodule-export-2.0`
### For GTK+ 3
	$ gcc -o simple-notepad main.c -I ../src ../src/*.c `pkg-config --cflags --libs gtk+-3.0 gmodule-export-2.0`
	
Author
-------------
Rodolfo Ribeiro Gomes
