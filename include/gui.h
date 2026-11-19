// Some defaults for the allegro gui
// $Id: gui.h,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $
// Richard Reeve
#ifndef GUI_H
#define GUI_H

#include <allegro.h>

extern int normalColour;
extern int errorColour;
extern int highlightColour;

#define LEFT_MOUSE 1
#define RIGHT_MOUSE 2
#define MIDDLE_MOUSE 4

#define COLOUR_DEPTH 24

#define FONT_FILE "%sallegro/font_times.dat"

#define MSG_UPDATE MSG_USER

// a yield dialog that can accept the focus...
int d_active_yield_proc(int msg, DIALOG *d, int c);
// a general edit dialog
int d_name_proc(int msg, DIALOG *d, int c);
// an edit dialog for numbers derived from d_name_proc
int d_num_proc(int msg, DIALOG *d, int c);
// make a dialog from args
void make_dialog(DIALOG *dialog, int (*proc)(int, DIALOG *, int), int x, int y,
		 int w, int h, int fg, int bg, int key, int flags, 
		 int d1, int d2, void *dp, void *dp2, void *dp3);

#endif //ndef GUI_H
