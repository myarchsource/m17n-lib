/* input-gui.c -- gui-based input method module.
   Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
     National Institute of Advanced Industrial Science and Technology (AIST)
     Registration Number H15PRO112

   This file is part of the m17n library.

   The m17n library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   The m17n library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the m17n library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301 USA.  */

/***en
    @addtogroup m17nInputMethodWin
    @brief Input method support on window systems.

    The input driver @c minput_gui_driver is provided for internal
    input methods that is useful on window systems.  It displays
    preedit text and status text at the inputting spot.  See the
    documentation of @c minput_gui_driver for more details.

    In the m17n-X library, the foreign input method of name @c Mxim is
    provided.  It uses XIM (X Input Method) as a background input
    engine.  The symbol @c Mxim has a property @c Minput_driver whose
    value is a pointer to the input driver @c minput_xim_driver.  See
    the documentation of @c minput_xim_driver for more details.  */

/***ja
    @addtogroup m17nInputMethodWin
    @brief ������ɥ������ƥ������ϥ᥽�åɤΥ��ݡ���.

    ���ϥɥ饤�� @c minput_gui_driver �ϡ�
    ������ɥ������ƥ����Ѥ������������ϥ᥽�å��ѤΥɥ饤�ФǤ��롣
    ���Υɥ饤�Ф����ϥ��ݥåȤ� preedit �ƥ����Ȥ� status 
    �ƥ����Ȥ�ɽ�����롣�ܺ٤ˤĤ��Ƥ� @c minput_gui_driver �������򻲾ȤΤ��ȡ�

    m17n-X �饤�֥��ϡ�@c Mxim �ȸ���̾������ĳ������ϥ᥽�åɤ��󶡤��Ƥ��롣����� 
    XIM (X Input Method) ��Хå����饦��ɤ����ϥ��󥸥�Ȥ������Ѥ��롣����ܥ�
    @c Mxim �� @c Minput_driver �Ȥ����ץ�ѥƥ�����äƤ��ꡢ�����ͤ����ϥɥ饤�� 
    @c minput_xim_driver �ؤΥݥ��󥿤Ǥ��롣 �ܺ٤ˤĤ��Ƥ� 
    @c minput_xim_driver �������򻲾ȤΤ��ȡ�  */

/*=*/

#if !defined (FOR_DOXYGEN) || defined (DOXYGEN_INTERNAL_MODULE)
/*** @addtogroup m17nInternal
     @{ */

#include <string.h>
#include <ctype.h>

#include "config.h"
#include "m17n-gui.h"
#include "m17n-misc.h"
#include "internal.h"
#include "internal-gui.h"
#include "input.h"

typedef struct
{
  MDrawWindow win;
  MDrawMetric geometry;
  MDrawControl control;
  int mapped;
} MInputGUIWinInfo;

typedef struct
{
  MInputContextInfo *ic_info;

  MFrame *frame;
  /* <geometry>.x and <geometry>.y are not used.  */
  MInputGUIWinInfo client;
  /* In the following members, <geometry> is relative to <client>.  */
  MInputGUIWinInfo focus;
  MInputGUIWinInfo preedit;
  MInputGUIWinInfo status;
  MInputGUIWinInfo candidates;
} MInputGUIContextInfo;

static MFace *status_face;
static MFaceBoxProp face_box_prop;

static int
win_create_ic (MInputContext *ic)
{
  MInputGUIContextInfo *win_ic_info;
  MInputGUIArgIC *win_info = (MInputGUIArgIC *) ic->arg;
  MFrame *frame = win_info->frame;

  if ((*minput_default_driver.create_ic) (ic) < 0)
    return -1;

  MSTRUCT_CALLOC (win_ic_info, MERROR_IM);
  win_ic_info->ic_info = (MInputContextInfo *) ic->info;
  win_ic_info->frame = frame;
  win_ic_info->client.win = win_info->client;
  (*frame->driver->window_geometry) (frame, win_info->client, win_info->client,
			 &win_ic_info->client.geometry);
  win_ic_info->focus.win = win_info->focus;
  (*frame->driver->window_geometry) (frame, win_info->focus, win_info->client,
			 &win_ic_info->focus.geometry);

  win_ic_info->preedit.win = (*frame->driver->create_window) (frame, win_info->client);
  win_ic_info->preedit.control.two_dimensional = 1;
  win_ic_info->preedit.control.as_image = 0;
  win_ic_info->preedit.control.with_cursor = 1;
  win_ic_info->preedit.control.cursor_width = 1;
  win_ic_info->preedit.control.enable_bidi = 1;
  win_ic_info->preedit.geometry.x = -1;
  win_ic_info->preedit.geometry.y = -1;

  win_ic_info->status.win = (*frame->driver->create_window) (frame, win_info->client);
  win_ic_info->status.control.as_image = 1;
  win_ic_info->status.control.enable_bidi = 1;

  win_ic_info->candidates.win = (*frame->driver->create_window) (frame, win_info->client);
  win_ic_info->candidates.control.as_image = 1;

  ic->info = win_ic_info;

  return 0;
}

static void
win_destroy_ic (MInputContext *ic)
{
  MInputGUIContextInfo *win_ic_info = (MInputGUIContextInfo *) ic->info;
  MInputContextInfo *ic_info = (MInputContextInfo *) win_ic_info->ic_info;
  MFrame *frame = win_ic_info->frame;

  (*frame->driver->destroy_window) (frame, win_ic_info->preedit.win);
  (*frame->driver->destroy_window) (frame, win_ic_info->status.win);
  (*frame->driver->destroy_window) (frame, win_ic_info->candidates.win);
  ic->info = ic_info;
  (*minput_default_driver.destroy_ic) (ic);
  free (win_ic_info);
}

static int
win_filter (MInputContext *ic, MSymbol key, void *arg)
{
  MInputGUIContextInfo *win_ic_info = (MInputGUIContextInfo *) ic->info;
  int ret;

  if (! ic
      || ! ic->active)
    return 0;

  if (key == Mnil && arg)
    {
      key = minput_event_to_key (win_ic_info->frame, arg);
      if (key == Mnil)
	return 1;
    }
  ic->info = win_ic_info->ic_info;
  ret = (*minput_default_driver.filter) (ic, key, arg);
  win_ic_info->ic_info = (MInputContextInfo *) ic->info;
  ic->info = win_ic_info;
  return ret;
}

static void
adjust_window_and_draw (MFrame *frame, MInputContext *ic, MText *mt, int type)
{
  MInputGUIContextInfo *win_ic_info = (MInputGUIContextInfo *) ic->info;
  MDrawControl *control;
  MDrawWindow win;
  MDrawMetric *geometry, physical, logical;
  int xoff = win_ic_info->focus.geometry.x;
  int yoff = win_ic_info->focus.geometry.y;
  int x0, x1, y0, y1;
  int len = mtext_nchars (mt);

  if (type == 0)
    {
      win = win_ic_info->preedit.win;
      control = &win_ic_info->preedit.control;
      geometry = &win_ic_info->preedit.geometry;
      len++;
    }
  else if (type == 1)
    {
      win = win_ic_info->status.win;
      control = &win_ic_info->status.control;
      geometry = &win_ic_info->status.geometry;
    }
  else
    {
      win = win_ic_info->candidates.win;
      control = &win_ic_info->candidates.control;
      geometry = &win_ic_info->candidates.geometry;
    }

  mdraw_text_extents (frame, mt, 0, len, control, &physical, &logical, NULL);
  x0 = physical.x, x1 = x0 + physical.width;
  y0 = physical.y, y1 = y0 + physical.height;
  if (x0 > logical.x)
    x0 = logical.x;
  if (x1 < logical.x + logical.width)
    x1 = logical.x + logical.width;
  if (y0 > logical.y)
    y0 = logical.y;
  if (y1 < logical.y + logical.height)
    y1 = logical.y + logical.height;
  physical.width = x1 - x0;
  physical.height = y1 - y0;
  physical.x = xoff + ic->spot.x;
  if (physical.x + physical.width > win_ic_info->client.geometry.width)
    physical.x = win_ic_info->client.geometry.width - physical.width;
  if (type == 0)
    {
      if (len <= 1)
	{
	  physical.height = physical.width = 1;
	  physical.x = physical.y = -1;
	}
      else
	{
	  if (y0 > - ic->spot.ascent)
	    {
	      physical.height += y0 + ic->spot.ascent;
	      y0 = - ic->spot.ascent;
	    }
	  if (y1 < ic->spot.descent)
	    {
	      physical.height += ic->spot.descent - y1;
	    }
	  physical.y = yoff + ic->spot.y + y0;
	}
    }
  else if (type == 1)
    {
      physical.y = yoff + ic->spot.y + ic->spot.descent + 2;
      if (physical.y + physical.height > win_ic_info->client.geometry.height
	  && yoff + ic->spot.y - ic->spot.ascent - 2 - physical.height >= 0)
	physical.y = yoff + ic->spot.y - ic->spot.ascent - 2 - physical.height;
    }
  else
    {
      if (win_ic_info->status.mapped)
	{
	  /* We assume that status is already drawn.  */
	  if (win_ic_info->status.geometry.y < yoff + ic->spot.y)
	    /* As there was no lower room for status, candidates must also
	       be drawn upper.  */
	    physical.y = win_ic_info->status.geometry.y - 1 - physical.height;
	  else
	    {
	      /* There was a lower room for status.  */
	      physical.y = (win_ic_info->status.geometry.y
			    + win_ic_info->status.geometry.height
			    + 1);
	      if (physical.y + physical.height
		  > win_ic_info->client.geometry.height)
		/* But not for candidates.  */
		physical.y = (yoff + ic->spot.y - ic->spot.ascent - 1
			      - physical.height);
	    }
	}
      else
	{
	  physical.y = yoff + ic->spot.y + ic->spot.descent + 2;
	  if ((physical.y + physical.height
	       > win_ic_info->client.geometry.height)
	      && (yoff + ic->spot.y - ic->spot.ascent - 2 - physical.height
		  >= 0))
	    physical.y = (yoff + ic->spot.y - ic->spot.ascent - 2
			  - physical.height);
	}
    }

  (*frame->driver->adjust_window) (frame, win, geometry, &physical);
  mdraw_text_with_control (frame, win, -x0, -y0, mt, 0, len, control);
}

static void
win_callback (MInputContext *ic, MSymbol command)
{
  MInputGUIContextInfo *win_ic_info = (MInputGUIContextInfo *) ic->info;
  MFrame *frame = win_ic_info->frame;

  if (command == Minput_preedit_draw)
    {
      MText *mt;
      MFace *face = mface ();

      if (! win_ic_info->preedit.mapped)
	{
	  (*frame->driver->map_window) (frame, win_ic_info->preedit.win);
	  win_ic_info->preedit.mapped = 1;
	}
      win_ic_info->preedit.control.cursor_pos = ic->cursor_pos;
      if (ic->spot.fontsize)
	mface_put_prop (face, Msize, (void *) ic->spot.fontsize);
      mface_merge (face, mface_underline);
      mtext_push_prop (ic->preedit, 0, mtext_nchars (ic->preedit),
		       Mface, face);
      M17N_OBJECT_UNREF (face);
      if (ic->im->language != Mnil)
	mtext_put_prop (ic->preedit, 0, mtext_nchars (ic->preedit), Mlanguage,
			ic->im->language);
      if (ic->candidate_list && ic->candidate_show)
	mtext_push_prop (ic->preedit, ic->candidate_from, ic->candidate_to,
			 Mface, mface_reverse_video);
      if (mtext_nchars (ic->produced) == 0)
	mt = ic->preedit;
      else
	{
	  mt = mtext_dup (ic->produced);
	  mtext_cat (mt, ic->preedit);
	  win_ic_info->preedit.control.cursor_pos
	    += mtext_nchars (ic->produced);
	}
      adjust_window_and_draw (frame, ic, mt, 0);
      if (ic->candidate_list)
	mtext_pop_prop (ic->preedit, 0, mtext_nchars (ic->preedit), Mface);
      mtext_pop_prop (ic->preedit, 0, mtext_nchars (ic->preedit), Mface);
      if (mtext_nchars (ic->produced) != 0)
	M17N_OBJECT_UNREF (mt);
    }
  else if (command == Minput_status_draw)
    {
      if (! win_ic_info->client.win)
	return;
      mtext_put_prop (ic->status, 0, mtext_nchars (ic->status), Mface,
		      status_face);
      if (ic->im->language != Mnil)
	mtext_put_prop (ic->status, 0, mtext_nchars (ic->status), Mlanguage,
			ic->im->language);
      adjust_window_and_draw (frame, ic, ic->status, 1);
    }
  else if (command == Minput_candidates_draw)
    {
      MPlist *group;
      MText *mt;
      int i, len;
      int from, to;

      if (! ic->candidate_list || ! ic->candidate_show)
	{
	  if (win_ic_info->candidates.mapped)
	    {
	      (*frame->driver->unmap_window) (frame, win_ic_info->candidates.win);
	      win_ic_info->candidates.mapped = 0;
	    }
	  return;
	}

      if (! win_ic_info->candidates.mapped)
	{
	  (*frame->driver->map_window) (frame, win_ic_info->candidates.win);
	  win_ic_info->candidates.mapped = 1;
	}

      i = 0;
      group = ic->candidate_list;
      while (1)
	{
	  if (mplist_key (group) == Mtext)
	    len = mtext_len (mplist_value (group));
	  else
	    len = mplist_length (mplist_value (group));
	  if (i + len > ic->candidate_index)
	    break;
	  i += len;
	  group = mplist_next (group);
	}

      mt = mtext ();
      if (mplist_key (group) == Mtext)
	{
	  MText *candidates = (MText *) mplist_value (group);

	  from = (ic->candidate_index - i) * 2 + 1;
	  to = from + 1;
	  for (i = 0; i < len; i++)
	    {
	      mtext_cat_char (mt, ' ');
	      mtext_cat_char (mt, mtext_ref_char (candidates, i));
	    }
	}
      else
	{
	  MPlist *pl;

	  for (pl = (MPlist *) mplist_value (group);
	       i < ic->candidate_index && mplist_key (pl) != Mnil;
	       i++, pl = mplist_next (pl))
	    {
	      mtext_cat_char (mt, ' ');
	      mtext_cat (mt, (MText *) mplist_value (pl));
	    }
	  from = mtext_nchars (mt) + 1;
	  to = from + mtext_nchars ((MText *) mplist_value (pl));
	  for (; mplist_key (pl) != Mnil; pl = mplist_next (pl))
	    {
	      mtext_cat_char (mt, ' ');
	      mtext_cat (mt, (MText *) mplist_value (pl));
	    }
	}
      mtext_cat_char (mt, ' ');
      mtext_push_prop (mt, 0, mtext_nchars (mt), Mface, status_face);
      mtext_push_prop (mt, from, to, Mface, mface_reverse_video);
      if (ic->im->language != Mnil)
	mtext_put_prop (mt, 0, mtext_nchars (mt), Mlanguage, ic->im->language);
      adjust_window_and_draw (frame, ic, mt, 2);
      M17N_OBJECT_UNREF (mt);
    }
  else if (command == Minput_set_spot)
    {
      minput_callback (ic, Minput_preedit_draw);
      minput_callback (ic, Minput_status_draw);
      minput_callback (ic, Minput_candidates_draw);
    }
  else if (command == Minput_toggle)
    {
      if (ic->active)
	{
	  minput_callback (ic, Minput_preedit_done);
	  minput_callback (ic, Minput_status_done);
	  minput_callback (ic, Minput_candidates_done);
	}
      else
	{
	  minput_callback (ic, Minput_preedit_start);
	  minput_callback (ic, Minput_status_start);
	  minput_callback (ic, Minput_candidates_start);
	}
    }
  else if (command == Minput_preedit_start)
    {
    }
  else if (command == Minput_preedit_done)
    {
      if (win_ic_info->preedit.mapped)
	{
	  (*frame->driver->unmap_window) (frame, win_ic_info->preedit.win);
	  win_ic_info->preedit.mapped = 0;
	}
    }
  else if (command == Minput_status_start)
    {
      if (! win_ic_info->status.mapped)
	{
	  (*frame->driver->map_window) (frame, win_ic_info->status.win);
	  win_ic_info->status.mapped = 1;
	}
    }
  else if (command == Minput_status_done)
    {
      if (win_ic_info->status.mapped)
	{
	  (*frame->driver->unmap_window) (frame, win_ic_info->status.win);
	  win_ic_info->status.mapped = 0;
	}
    }
  else if (command == Minput_candidates_start)
    {
      if (! win_ic_info->candidates.mapped)
	{
	  (*frame->driver->map_window) (frame, win_ic_info->candidates.win);
	  win_ic_info->candidates.mapped = 1;
	}
    }
  else if (command == Minput_candidates_done)
    {
      if (win_ic_info->candidates.mapped)
	{
	  (*frame->driver->unmap_window) (frame, win_ic_info->candidates.win);
	  win_ic_info->candidates.mapped = 0;
	}
    }
  else if (command == Minput_reset)
    {
      MInputCallbackFunc func;

      if (minput_default_driver.callback_list
	  && (func = ((MInputCallbackFunc)
		      mplist_get_func (minput_default_driver.callback_list,
				       Minput_reset))))
	{
	  MInputContextInfo *ic_info
	    = (MInputContextInfo *) win_ic_info->ic_info;
	  ic->info = ic_info;
	  (func) (ic, Minput_reset);
	  ic->info = win_ic_info;
	}
      if (ic->preedit_changed)
	minput_callback (ic, Minput_preedit_draw);
      if (ic->status_changed)
	minput_callback (ic, Minput_status_draw);
      if (ic->candidates_changed)
	minput_callback (ic, Minput_candidates_draw);
    }
}

static int
win_lookup (MInputContext *ic, MSymbol key, void *arg, MText *mt)
{
  MInputGUIContextInfo *win_ic_info = (MInputGUIContextInfo *) ic->info;
  MInputContextInfo *ic_info = (MInputContextInfo *) win_ic_info->ic_info;
  int ret;

  ic->info = ic_info;
  ret = (*minput_default_driver.lookup) (ic, key, arg, mt);
  ic->info = win_ic_info;
  return ret;
}



int
minput__win_init ()
{
  minput_gui_driver = minput_default_driver;

  minput_gui_driver.create_ic = win_create_ic;
  minput_gui_driver.destroy_ic = win_destroy_ic;
  minput_gui_driver.filter = win_filter;
  minput_gui_driver.lookup = win_lookup;
  {
    MPlist *plist = mplist ();

    minput_gui_driver.callback_list = plist;
    mplist_put_func (plist, Minput_preedit_start, M17N_FUNC (win_callback));
    mplist_put_func (plist, Minput_preedit_draw, M17N_FUNC (win_callback));
    mplist_put_func (plist, Minput_preedit_done, M17N_FUNC (win_callback));
    mplist_put_func (plist, Minput_status_start, M17N_FUNC (win_callback));
    mplist_put_func (plist, Minput_status_draw, M17N_FUNC (win_callback));
    mplist_put_func (plist, Minput_status_done, M17N_FUNC (win_callback));
    mplist_put_func (plist, Minput_candidates_start, M17N_FUNC (win_callback));
    mplist_put_func (plist, Minput_candidates_draw, M17N_FUNC (win_callback));
    mplist_put_func (plist, Minput_candidates_done, M17N_FUNC (win_callback));
    mplist_put_func (plist, Minput_set_spot, M17N_FUNC (win_callback));
    mplist_put_func (plist, Minput_toggle, M17N_FUNC (win_callback));
    mplist_put_func (plist, Minput_reset, M17N_FUNC (win_callback));
  }
#if 0
  /* This will make the caller of minput_method_open() pazzled.  */
  minput_driver = &minput_gui_driver;
#endif

  face_box_prop.width = 1;
  face_box_prop.color_top = face_box_prop.color_left
    = face_box_prop.color_bottom = face_box_prop.color_right
    = msymbol ("black");
  face_box_prop.inner_hmargin = face_box_prop.inner_vmargin = 2;
  face_box_prop.outer_hmargin = face_box_prop.outer_vmargin = 1;
  status_face = mface ();
  mface_put_prop (status_face, Mbox, &face_box_prop);

  return 0;
}

void
minput__win_fini ()
{
  M17N_OBJECT_UNREF (status_face);
  if (minput_gui_driver.callback_list)
    {
      M17N_OBJECT_UNREF (minput_gui_driver.callback_list);
      minput_gui_driver.callback_list = NULL;
    }
}

/*** @} */
#endif /* !FOR_DOXYGEN || DOXYGEN_INTERNAL_MODULE */


/* External API */

/*** @addtogroup m17nInputMethodWin */
/*** @{ */

/*=*/
/***en
    @brief Input driver for internal input methods on window systems.

    The input driver @c minput_gui_driver is for internal input
    methods to be used on window systems.

    It creates sub-windows for a preedit text and a status text, and
    displays them at the input spot set by the function
    minput_set_spot ().

    The macro M17N_INIT () set the variable @c minput_driver to the
    pointer to this driver so that all internal input methods use it.

    Therefore, unless @c minput_driver is changed from the default,
    the driver dependent arguments to the functions whose name begin
    with minput_ must are treated as follows.

    The argument $ARG of the function minput_open_im () is ignored.

    The argument $ARG of the function minput_create_ic () must be a
    pointer to the structure @c MInputGUIArgIC.  See the documentation
    of @c MInputGUIArgIC for more details.

    If the argument $KEY of function minput_filter () is @c Mnil, the
    argument $ARG must be a pointer to the object of type @c XEvent.
    In that case, $KEY is generated from $ARG.

    The argument $ARG of the function minput_lookup () must be the
    same one as that of the function minput_filter (). */

/***ja
    @brief ������ɥ������ƥ���������ϥ᥽�å������ϥɥ饤��.

    ���ϥɥ饤�� @c minput_gui_driver
    �ϡ�������ɥ������ƥ����Ѥ��������ϥ᥽�å��ѥɥ饤�ФǤ��롣

    ���Υɥ饤�Фϡ��ؿ� minput_set_spot () �ˤ�ä����ꤵ�줿���ϥ��ݥåȤ�
    preedit �ƥ������ѤΥ��֥�����ɥ��� status 
    �ƥ������ѤΥ��֥�����ɥ����ꡢ���줾���ɽ�����롣

    �ޥ��� M17N_INIT () ���ѿ� @c minput_driver 
    �򤳤Υɥ饤�ФؤΥݥ��󥿤����ꤷ�����Ƥ��������ϥ᥽�åɤ����Υɥ饤�Ф�Ȥ��褦�ˤ��롣

    �������äơ�@c minput_driver ���ǥե�����ͤΤޤޤǤ���С�minput_ 
    �ǻϤޤ�̾������Ĵؿ��ΰ����Τ����ɥ饤�а�¸�Τ�Τϰʲ��Τ褦�ˤʤ롣

    �ؿ� minput_open_im () �ΰ��� $ARG ��̵�뤵��롣

    �ؿ� minput_create_ic () �ΰ��� $ARG �Ϲ�¤�� @c MInputGUIArgIC 
    �ؤΥݥ��󥿤Ǥʤ��ƤϤʤ�ʤ����ܺ٤ˤĤ��Ƥ� @c MInputGUIArgIC 
    �������򻲾ȤΤ��ȡ�

    �ؿ� minput_filter () �ΰ��� $ARG �� @c Mnil �ξ�硢 $ARG �� @c
    XEvent ���Υ��֥������ȤؤΥݥ��󥿤Ǥʤ��ƤϤʤ�ʤ������ξ�� 
    $KEY �� $ARG ������������롣

    �ؿ� minput_lookup () �ΰ��� $ARG �ϴؿ� minput_filter () �ΰ��� 
    $ARG ��Ʊ���Ǥʤ��ƤϤʤ�ʤ��� */

MInputDriver minput_gui_driver;

/*=*/

/***en
    @brief Symbol of the name "xim".

    The variable Mxim is a symbol of name "xim".  It is a name of the
    input method driver #minput_xim_driver.  */ 
/***ja
    @brief "xim"��̾���Ȥ��ƻ��ĥ���ܥ� .

    �ѿ� Mxim ��"xim"��̾���Ȥ��ƻ��ĥ���ܥ�Ǥ��롣"xim" 
    �����ϥ᥽�åɥɥ饤�� #minput_xim_driver ��̾���Ǥ��롣  */ 

MSymbol Mxim;

/*=*/

/***en
    @brief Convert an event to an input key.

    The minput_event_to_key () function returns the input key
    corresponding to event $EVENT on $FRAME by a window system
    dependent manner.

    In the m17n-X library, $EVENT must be a pointer to the structure
    @c XKeyEvent, and it is handled as below.

    At first, the keysym name of $EVENT is acquired by the function @c
    XKeysymToString.  Then, the name is modified as below.

    If the name is one of "a" .. "z" and $EVENT has a Shift modifier,
    the name is converted to "A" .. "Z" respectively, and the Shift
    modifier is cleared.

    If the name is one byte length and $EVENT has a Control modifier,
    the byte is bitwise anded by 0x1F and the Control modifier is
    cleared.

    If $EVENT still has modifiers, the name is preceded by "S-"
    (Shift), "C-" (Control), "M-" (Meta), "A-" (Alt), "G-" (AltGr),
    "s-" (Super), and "H-" (Hyper) in this order.

    For instance, if the keysym name is "a" and the event has Shift,
    Meta, and Hyper modifiers, the resulting name is "M-H-A".

    At last, a symbol who has the name is returned.  */

/***ja
    @brief ���٥�Ȥ����ϥ������Ѵ�����.

    �ؿ� minput_event_to_key () �ϡ�$FRAME �Υ��٥�� $EVENT 
    ���б��������ϥ������֤��������ǤΡ��б��פϥ�����ɥ������ƥ�˰�¸���롣

    m17n-X �饤�֥��ξ��ˤϡ�$EVENT �� ��¤�� @c XKeyEvent 
    �ؤΥݥ��󥿤Ǥ��ꡢ���Τ褦�˽�������롣

    �ޤ����ؿ� @c XKeysymToString �ˤ�äơ�$EVENT �� keysym 
    ̾��������������ǰʲ����ѹ���ä��롣

    ̾���� "a" .. "z" �Τ����줫�Ǥ��ä� $EVENT �� Shift 
    ��ǥ��ե�����������С�̾���Ϥ��줾�� "A" .. "Z" ���Ѵ����졢Shift 
    ��ǥ��ե������ϼ�������롣

    ̾�������Х���Ĺ�� $EVENT �� Control ��ǥ��ե�����������С�̾����
    0x1F �Ȥ�ӥå�ñ�̤� and �黻����Control ��ǥ��ե������ϼ�������롣

    ����Ǥ� $EVENT �ˤޤ���ǥ��ե�����������С�̾�������ˤ��줾��
    "S-" (Shift), "C-" (Control), "M-" (Meta), "A-" (Alt), , "G-" (AltGr),
    "s-" (Super), "H-" (Hyper)�����ν��֤��դ���
    
    ���Ȥ��С�keysym ̾�� "a" �ǥ��٥�Ȥ� Shift, Meta, and Hyper 
    ��ǥ��ե���������ƤС�������̾���� "M-H-A" �Ǥ��롣

    �Ǹ�ˤ���̾������ĥ���ܥ���֤���*/


MSymbol
minput_event_to_key (MFrame *frame, void *event)
{
  int modifiers;
  MSymbol key;
  char *name, *str;

  M_CHECK_READABLE (frame, MERROR_IM, Mnil);
  key = (*frame->driver->parse_event) (frame, event, &modifiers);
  if (! modifiers)
    return key;

  name = msymbol_name (key);
  str = alloca (strlen (name) + 2 * 8 + 1);
  str[0] = '\0';
  if (modifiers & MINPUT_KEY_SHIFT_MODIFIER)
    strcat (str, "S-");
  if (modifiers & MINPUT_KEY_CONTROL_MODIFIER)
    strcat (str, "C-");
  if (modifiers & MINPUT_KEY_META_MODIFIER)
    strcat (str, "M-");
  if (modifiers & MINPUT_KEY_ALT_MODIFIER)
    strcat (str, "A-");
  if (modifiers & MINPUT_KEY_ALTGR_MODIFIER)
    strcat (str, "G-");
  if (modifiers & MINPUT_KEY_SUPER_MODIFIER)
    strcat (str, "s-");
  if (modifiers & MINPUT_KEY_HYPER_MODIFIER)
    strcat (str, "H-");
  strcat (str, name);

  return msymbol (str);
}

/*** @} */

/*
  Local Variables:
  coding: euc-japan
  End:
*/
