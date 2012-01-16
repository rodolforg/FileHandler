GtkFileHandler
==============

Description
--------------
If your GTK+ application let the user handle files/documents (create new ones,
open, save, close them), this small code is helpful.

Based on callback functions you provide, it'll automatically ask user if he
really wants to close a not-saved document, remember the last browsed directory
in current app session, if the document is already saved, set up GtkActions'
sensitiveness for those file actions properly, etc.

As a bonus, it also provides some pratical "show message box" functions.

Requirements
-------------
* GLib >= 2.16
* GModule >= 2.0
* GTK+ >= 2.8 ( >= 3.0 included)

TODO
-------------
* Work with URIs
* Help application that allows multiple documents

Author
--------------
Rodolfo Ribeiro Gomes
