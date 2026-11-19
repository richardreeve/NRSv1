// $Id: dialog.c,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#include <allegro.h>
#include <stdio.h>
#include "gui.h"

void make_dialog(DIALOG *dialog, int (*proc)(int, DIALOG *, int), int x, int y,
		 int w, int h, int fg, int bg, int key, int flags, 
		 int d1, int d2, void *dp, void *dp2, void *dp3)
{
  dialog->proc=proc;
  dialog->x=x;
  dialog->y=y;
  dialog->w=w;
  dialog->h=h;
  dialog->fg=fg;
  dialog->bg=bg;
  dialog->key=key;
  dialog->flags=flags;
  dialog->d1=d1;
  dialog->d2=d2;
  dialog->dp=dp;
  dialog->dp2=dp2;
  dialog->dp3=dp3;
}

/*  d_my_yield_proc:
 *  Simple dialog procedure which just yields the timeslice when the dialog
 *  is idle.
 */
int d_active_yield_proc(int msg, DIALOG *d, int c)
{
   if (msg == MSG_IDLE)
      yield_timeslice();
   else if (msg == MSG_WANTFOCUS)
     return D_WANTFOCUS;

   return D_O_K;
}

/* d_name_proc: An editable text object (the 1st dp field points to
 *  the string, 2nd to the callback). When it has
 *  the input focus (obtained by clicking on it with the mouse), text
 *  can be typed into this object. The d1 field specifies the maximum
 *  number of characters that it will accept, and d2 is the text
 *  cursor position within the string. */

int d_name_proc(int msg, DIALOG *d, int c)
{
  static int ignore_next_uchar = FALSE;
  int f, l, p, w, x, fg, bg, b, scroll;
  char buf[16];
  char *s;
  int (*proc)();

  proc=d->dp2;
  
  s = d->dp;
  l = ustrlen(s);
  if (d->d2 > l) 
    d->d2 = l;
  
  /* calculate maximal number of displayable characters */
  if (d->d2 == l)
    {
      usetc(buf+usetc(buf, ' '), 0);
      x = text_length(font, buf);
    }
  else
    x = 0;
  
  b = 0;
  
  for (p=d->d2; p>=0; p--) 
    {
      usetc(buf+usetc(buf, ugetat(s, p)), 0);
      x += text_length(font, buf);
      b++;
      if (x > d->w) 
	break;
    }
  
  if (x <= d->w) 
    {
      b = l; 
      scroll = FALSE;
    }
  else 
    {
      b--; 
      scroll = TRUE;
    }
  
  switch (msg) 
    {
    case MSG_START:
      d->d2 = l;
      break;
      
    case MSG_GOTMOUSE:
    case MSG_LOSTMOUSE:
      return D_REDRAWME;
      break;
    case MSG_DRAW:
      fg = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;
      fg = (d->flags & D_GOTFOCUS) ? 0xffffff : fg;
      bg = (d->flags & D_GOTMOUSE) ? 0x808080 : d->bg;
      bg = (d->flags & D_GOTFOCUS) ? 0x000000 : bg;
      x = 0;
      
      if (scroll) 
	{
	  p = d->d2-b+1; 
	  b = d->d2; 
	}
      else 
	p = 0; 
      
      for (; p<=b; p++)
	{
	  f = ugetat(s, p);
	  usetc(buf+usetc(buf, (f) ? f : ' '), 0);
	  w = text_length(font, buf);
	  if (x+w > d->w) 
	    break;
	  f = ((p == d->d2) && (d->flags & D_GOTFOCUS));
	  text_mode((f) ? fg : bg);
	  textout(screen, font, buf, d->x+x, d->y, (f) ? bg : fg);
	  x += w;
	}
      if (x < d->w)
	rectfill(screen, d->x+x, d->y,
		 d->x+d->w-1, d->y+text_height(font)-1, bg);
      break;
      
    case MSG_CLICK:
      x = d->x;
      
      if (scroll) 
	{
	  p = d->d2-b+1; 
	  b = d->d2; 
	}
      else
	p = 0; 
      
      for (; p<b; p++)
	{
	  usetc(buf+usetc(buf, ugetat(s, p)), 0);
	  x += text_length(font, buf);
	  if (x > gui_mouse_x()) 
	    break;
	}
      d->d2 = MID(0, p, l);
      scare_mouse();
      SEND_MESSAGE(d, MSG_DRAW, 0);
      unscare_mouse();
      break;
      
    case MSG_LOSTFOCUS:
      if (proc!=NULL)
	{
	  scare_mouse();
	  if (!(*proc)())
	    {
	      unscare_mouse();
	      return D_REDRAW;
	    }
	  unscare_mouse();
	}
      return D_O_K;
      break;
    case MSG_WANTFOCUS:
    case MSG_KEY:
      return D_WANTFOCUS;
      break;
    case MSG_CHAR:
      ignore_next_uchar = FALSE;
      
      if ((c >> 8) == KEY_LEFT) 
	{
	  if (d->d2 > 0) d->d2--;
	}
      else if ((c >> 8) == KEY_RIGHT) 
	{
	  if (d->d2 < l) d->d2++;
	}
      else if ((c >> 8) == KEY_HOME) 
	{
	  d->d2 = 0;
	}
      else if ((c >> 8) == KEY_END) 
	{
	  d->d2 = l;
	}
      else if ((c >> 8) == KEY_DEL)
	{
	  if (d->d2 < l)
	    uremove(s, d->d2);
	}
      else if ((c >> 8) == KEY_BACKSPACE)
	{
	  if (d->d2 > 0)
	    {
	      d->d2--;
	      uremove(s, d->d2);
	    }
	}
      else if ((c >> 8) == KEY_ENTER)
	{
	  if (d->flags & D_EXIT)
	    {
	      scare_mouse();
	      SEND_MESSAGE(d, MSG_DRAW, 0);
	      unscare_mouse();
	      return D_CLOSE;
	    }
	  else
	    {
	      if (proc!=NULL)
		{
		  scare_mouse();
		  if (!(*proc)())
		    {
		      unscare_mouse();
		      return D_REDRAW;
		    }
		  // broadcast_dialog_message(MSG_DRAW, 0);
		  unscare_mouse();
		}
	      return D_USED_CHAR;
	    }
	}
      else if ((c >> 8) == KEY_TAB)
	{
	  ignore_next_uchar = TRUE;
	  return D_O_K;
	}
      else
	{
	  /* don't process regular keys here: MSG_UCHAR will do that */
	  break;
	}
      scare_mouse();
      SEND_MESSAGE(d, MSG_DRAW, 0);
      unscare_mouse();
      return D_USED_CHAR;
      
    case MSG_UCHAR:
      if ((c >= ' ') && (uisok(c)) && (!ignore_next_uchar))
	{
	  if (l < d->d1) 
	    {
	      uinsert(s, d->d2, c);
	      d->d2++;
	      scare_mouse();
	      SEND_MESSAGE(d, MSG_DRAW, 0);
	      unscare_mouse();
	    }
	  return D_USED_CHAR;
	}
      break;
    case MSG_IDLE:
      break;
    default:
      broadcast_dialog_message(MSG_IDLE, 0);
      break;
    } 
  return D_O_K;
}

/* d_num_proc: Based on d_name proc, but dp3 points to number */
int d_num_proc(int msg, DIALOG *d, int c)
{
  int retval;
  int (*proc)();
  int tmp_bg;

  proc=d->dp2;

  switch (msg)
    {
    case MSG_START:
      sprintf((char *)d->dp, "%d", *(int *)d->dp3);
      break;
    case MSG_LOSTFOCUS:
      *(int *)d->dp3=atoi((char *)d->dp);
      if (proc!=NULL)
	{
	  scare_mouse();
	  if (!(*proc)())
	    {
	      unscare_mouse();
	      return D_REDRAW;
	    }
	  unscare_mouse();
	}
      return D_O_K;
      break;
    case MSG_CHAR:
      if ((c >> 8) == KEY_ENTER)
	{
	  if (d->flags & D_EXIT)
	    {
	      scare_mouse();
	      SEND_MESSAGE(d, MSG_DRAW, 0);
	      unscare_mouse();
	      return D_CLOSE;
	    }
	  else
	    {
	      *(int *)d->dp3=atoi((char *)d->dp);
	      if (proc!=NULL)
		{
		  scare_mouse();
		  if (!(*proc)())
		    {
		      unscare_mouse();
		      broadcast_dialog_message(MSG_UPDATE, 0);
		    }
		  else
		    unscare_mouse();
		}
	      return D_USED_CHAR;
	    }
	}
      break;
    case MSG_UPDATE:
      if (*(int *)d->dp3!=atoi((char *)d->dp))
	{
	  sprintf((char *)d->dp, "%d", *(int *)d->dp3);
	  tmp_bg=d->bg;
	  d->bg=0xff0000;
	  retval = d_name_proc(MSG_DRAW, d, c);
	  d->bg=tmp_bg;
	  return retval;
	}
      break;
    }

  retval = d_name_proc(msg, d, c);
  
  return retval;
}

