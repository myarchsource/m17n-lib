/* m17n-X.h -- header file for the GUI API on X Windows.
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

#ifndef _M17N_X_H_
#define _M17N_X_H_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* For inputting.  */

extern MInputDriver minput_xim_driver;
extern MSymbol Mxim;

/*** @ingroup m17nInputMethodWin */
/***en
    @brief Structure pointed to by the argument $ARG of the function minput_open_im ().

    The type #MInputXIMArgIM is the structure pointed to by the
    argument $ARG of the function minput_open_im () for the foreign
    input method of name #Mxim.  */

/***ja
    @brief �ؿ� minput_open_im () �ΰ��� $ARG �ˤ�äƻؤ���빽¤��.

    #MInputXIMArgIM ���ϡ��ؿ� minput_open_im () ��̾�� #Mxim 
    ����ĳ������ϥ᥽�åɤ���������ݤ˰��� $ARG �ˤ�äƻؤ���빽¤�ΤǤ��롣  */

typedef struct
{
  /***en The meaning of the following four members are the same as
      arguments to XOpenIM ().  */
  /***ja �ʲ��Σ��ĤΥ��Фΰ�̣�ϡ�XOpenIM () �ΰ����ΰ�̣��Ʊ���Ǥ���.  */

  /***en Display of the client.  */
  /***ja ���饤����ȤΥǥ����ץ쥤.  */
  Display *display;

  /***en Pointer to the X resource database.  */
  /***ja X �꥽�������ǡ����١����ؤΥݥ���.  */
  XrmDatabase db;

  /***en Full class name of the application.  */
  /***ja ���ץꥱ�������δ����ʥ��饹̾.  */
  char *res_class;

  /***en Full resource name of the application.  */
  /***ja ���ץꥱ�������δ����ʥ꥽����̾.  */
  char *res_name;

  /***en Locale name under which an XIM is opened.  */
  /***ja XIM�������ץ󤵤줿������̾.  */
  char *locale;

  /***en Arguments to XSetLocaleModifiers ().  */
  /***ja XSetLocaleModifiers () �ΰ���.  */
  char *modifier_list;
} MInputXIMArgIM;

/*=*/

/*** @ingroup m17nInputMethodWin */
/***en
    @brief Structure pointed to by the argument $ARG of the function minput_create_ic ().

    The type #MInputXIMArgIC is the structure pointed to by the
    argument $ARG of the function minput_create_ic () for the foreign
    input method of name #Mxim.  */

/***ja
    @brief �ؿ� minput_create_ic () �ΰ��� $ARG �ˤ�äƻؤ���빽¤��.

    #MInputXIMArgIC ���ϡ��ؿ� minput_create_ic () ��̾�� #Mxim 
    ����ĳ������ϥ᥽�å��Ѥ˸ƤФ��ݤˡ����� $ARG �ˤ�äƻؤ���빽¤�ΤǤ��롣 */

typedef struct
{
  /***en Used as the arguments of @c XCreateIC following @c
      XNInputStyle.  If this is zero, ( @c XIMPreeditNothing | @c
      XIMStatusNothing) is used, and \<preedit_attrs\> and
      \<status_attrs\> are set to @c NULL.  */
  /***ja @c XCreateIC �� @c XNInputStyle ��³�������Ȥ����Ѥ����롣
      ����ʤ�С� ( @c XIMPreeditNothing | @c XIMStatusNothing) ����
      ����졢 \<preedit_attrs\> �� \<status_attrs\> �� @c NULL 
      �����ꤵ��롣 */

  XIMStyle input_style;
  /***en Used as the argument of @c XCreateIC following @c XNClientWindow.  */
  /***ja @c XCreateIC �� @c XNClientWindow ��³�������Ȥ����Ѥ����롣  */

  Window client_win;
  /***en Used as the argument of @c XCreateIC following @c XNFocusWindow.  */
  /***ja @c XCreateIC �� @c XNFocusWindow ��³�������Ȥ����Ѥ����롣  */

  Window focus_win;
  /***en If non- @c NULL, used as the argument of @c XCreateIC following
      @c XNPreeditAttributes.  */
  /***ja @c NULL �Ǥʤ���С� @c XCreateIC following ��@c
      XNPreeditAttributes ��³�������Ȥ����Ѥ����롣  */

  XVaNestedList preedit_attrs;
  /***en If non- @c NULL, used as the argument of @c XCreateIC following
      @c XNStatusAttributes.  */ 
  /***ja @c NULL �Ǥʤ���С� @c XCreateIC following �� @c
      XNStatusAttributes ��³�������Ȥ����Ѥ����롣  */

  XVaNestedList status_attrs;
} MInputXIMArgIC;
/*=*/

#ifdef __cplusplus
}
#endif

#endif /* not _M17N_X_H_ */

/*
  Local Variables:
  coding: euc-japan
  End:
*/
