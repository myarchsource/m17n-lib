/* m17n-gui.c -- body of the GUI API.
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
    @addtogroup m17nGUI
    @brief GUI support for a window system.

    This section defines the m17n GUI API concerning M-text drawing
    and inputting under a window system.

    All the definitions here are independent of window systems.  An
    actual library file, however, can depend on a specific window
    system.  For instance, the library file m17n-X.so is an example of
    implementation of the m17n GUI API for the X Window System.

    Actually the GUI API is mainly for toolkit libraries or to
    implement XOM, not for direct use from application programs.
*/

/***ja
    @addtogroup m17nGUI
    @brief ������ɥ������ƥ��� GUI ���ݡ���.

    ���Υ��������ϥ�����ɥ������ƥ�Τ�ȤǤ� M-text ��ɽ�������Ϥˤ������
    m17n GUI API ��������롣

    �����ǤΤ��٤Ƥ�����ϥ�����ɥ������ƥ�Ȥ���Ω�Ǥ��롣
    ���������ºݤΥ饤�֥��ե�����ϸ��̤Υ�����ɥ������ƥ�˰�¸�����礬���롣
    ���Ȥ��Х饤�֥��ե����� m17n-X.so �ϡ�m17n GUI API �� X 
    ������ɥ��Ѥμ�����Ǥ��롣

    ���¤ˤϡ�GUI API �ϼ�˥ġ��륭�åȥ饤�֥������Ǥ��뤫���ޤ��� 
    XOM ��������뤿����Ѥ����Ƥ��ꡢ���ץꥱ�������ץ���फ���ľ�ܤ����Ѥ�ǰƬ�ˤ�������ΤǤϤʤ���
*/

/*=*/

#if !defined (FOR_DOXYGEN) || defined (DOXYGEN_INTERNAL_MODULE)
/*** @addtogroup m17nInternal
     @{ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#include "m17n-gui.h"
#include "m17n-misc.h"
#include "internal.h"
#include "plist.h"
#include "internal-gui.h"
#include "font.h"
#include "fontset.h"
#include "face.h"

#ifndef DLOPEN_SHLIB_EXT
#define DLOPEN_SHLIB_EXT ".so"
#endif

/** Information about a dynamic library supporting a specific graphic
    device.  */
typedef struct
{
  /** Name of the dynamic library (e.g. "libm17n-X.so").  */
  char *library;
  /** Handle of the dynamic library.  */
  void *handle;
  /** Function to call just after loading the library.  */
  int (*init) ();
  /** Function to call to open a frame on the graphic device.  */
  int (*open) (MFrame *frame, MPlist *param);
  /** Function to call just before unloading the library.  */
  int (*fini) ();
} MDeviceLibraryInterface;


/** Plist of device symbol vs MDeviceLibraryInterface.  */

static MPlist *device_library_list;

/** Close MFrame and free it.  */

static void
free_frame (void *object)
{
  MFrame *frame = (MFrame *) object;

  (*frame->driver->close) (frame);
  M17N_OBJECT_UNREF (frame->face);
  M17N_OBJECT_UNREF (frame->font_driver_list);
  free (object);
}


/** Register a dynamic library of name LIB by a key NAME.  */

static int
register_device_library (MSymbol name, char *lib)
{
  MDeviceLibraryInterface *interface;

  MSTRUCT_CALLOC (interface, MERROR_WIN);
  interface->library = malloc (strlen (M17N_MODULE_DIR) + 1
			       + strlen (lib) 
			       + strlen (DLOPEN_SHLIB_EXT) + 1);
  sprintf (interface->library, "%s/%s%s", M17N_MODULE_DIR, lib,
	   DLOPEN_SHLIB_EXT);
  if (! device_library_list)
    device_library_list = mplist ();
  mplist_add (device_library_list, name, interface);
  return 0;
}


#ifdef HAVE_FREETYPE
/** Null device support.  */

static struct {
  MPlist *realized_fontset_list;
  MPlist *realized_font_list;
  MPlist *realized_face_list;
} null_device;

static void
null_device_close (MFrame *frame)
{
}

static void *
null_device_get_prop (MFrame *frame, MSymbol key)
{
  return NULL;
}

static void
null_device_realize_face (MRealizedFace *rface)
{
  rface->info = NULL;
}

static void
null_device_free_realized_face (MRealizedFace *rface)
{
}

static MDeviceDriver null_driver =
  {
    null_device_close,
    null_device_get_prop,
    null_device_realize_face,
    null_device_free_realized_face
  };

static int
null_device_init ()
{
  null_device.realized_fontset_list = mplist ();
  null_device.realized_font_list = mplist ();
  null_device.realized_face_list = mplist ();  
  return 0;
}

static int
null_device_fini ()
{
  MPlist *plist;

  MPLIST_DO (plist, null_device.realized_fontset_list)
    mfont__free_realized_fontset ((MRealizedFontset *) MPLIST_VAL (plist));
  M17N_OBJECT_UNREF (null_device.realized_fontset_list);

  MPLIST_DO (plist, null_device.realized_face_list)
    mface__free_realized ((MRealizedFace *) MPLIST_VAL (plist));
  M17N_OBJECT_UNREF (null_device.realized_face_list);

  if (MPLIST_VAL (null_device.realized_font_list))
    mfont__free_realized (MPLIST_VAL (null_device.realized_font_list));
  M17N_OBJECT_UNREF (null_device.realized_font_list);
  return 0;
}

static int
null_device_open (MFrame *frame, MPlist *param)
{
  MFace *face;

  frame->device = NULL;
  frame->device_type = 0;
  frame->dpi = (int) mplist_get (param, Mresolution);
  if (frame->dpi == 0)
    frame->dpi = 100;
  frame->driver = &null_driver;
  frame->font_driver_list = mplist ();
  mplist_add (frame->font_driver_list, Mfreetype, &mfont__ft_driver);
  frame->realized_font_list = null_device.realized_font_list;
  frame->realized_face_list = null_device.realized_face_list;
  frame->realized_fontset_list = null_device.realized_fontset_list;
  face = mface_copy (mface__default);
  mplist_push (param, Mface, face);
  M17N_OBJECT_UNREF (face);
  return 0;
}

static MDeviceLibraryInterface null_interface =
  { NULL, NULL, null_device_init, null_device_open, null_device_fini };

#endif

/* Internal API */


/* External API */

void
m17n_init_win (void)
{
  int mdebug_flag = MDEBUG_INIT;

  merror_code = MERROR_NONE;
  if (m17n__gui_initialized++)
    return;
  m17n_init ();
  m17n_init_flt ();
  if (merror_code != MERROR_NONE)
    {
      m17n__gui_initialized--;
      return;
    }

  MDEBUG_PUSH_TIME ();

  Mgd = msymbol ("gd");

  Mfont = msymbol ("font");
  Mfont_width = msymbol ("font-width");
  Mfont_ascent = msymbol ("font-ascent");
  Mfont_descent = msymbol ("font-descent");
  Mdevice = msymbol ("device");

  Mdisplay = msymbol ("display");
  Mscreen = msymbol ("screen");
  Mdrawable = msymbol ("drawable");
  Mdepth = msymbol ("depth");
  Mwidget = msymbol ("widget");
  Mcolormap = msymbol ("colormap");

  MDEBUG_PUSH_TIME ();
  if (mfont__init () < 0)
    goto err;
  MDEBUG_PRINT_TIME ("INIT", (mdebug__output, " to initialize font module."));
  if (mfont__fontset_init () < 0)
    goto err;
  MDEBUG_PRINT_TIME ("INIT", (mdebug__output, " to initialize fontset module."));
  if (mface__init () < 0)
    goto err;
  MDEBUG_PRINT_TIME ("INIT", (mdebug__output, " to initialize face module."));
  if (mdraw__init () < 0)
    goto err;
  MDEBUG_PRINT_TIME ("INIT", (mdebug__output, " to initialize draw module."));
  if (minput__win_init () < 0)
    goto err;
  MDEBUG_PRINT_TIME ("INIT",
		     (mdebug__output, " to initialize input-win module."));
  mframe_default = NULL;

  register_device_library (Mx, "libm17n-X");
  register_device_library (Mgd, "libm17n-gd");
  return;

 err:
  MDEBUG_POP_TIME ();
  MDEBUG_PRINT_TIME ("INIT",
		     (mdebug__output, " to initialize the m17n GUI module."));
  MDEBUG_POP_TIME ();
}

void
m17n_fini_win (void)
{
  int mdebug_flag = MDEBUG_FINI;
  MPlist *plist;

  if (m17n__gui_initialized == 0
      || --m17n__gui_initialized > 0)
    return;

  MDEBUG_PUSH_TIME ();
  MDEBUG_PUSH_TIME ();
  MDEBUG_PRINT_TIME ("FINI", (mdebug__output, " to finalize device modules."));
  MPLIST_DO (plist, device_library_list)
    {
      MDeviceLibraryInterface *interface = MPLIST_VAL (plist);

      if (interface->handle && interface->fini)
	{
	  (*interface->fini) ();
	  dlclose (interface->handle);
	}
      free (interface->library);
      free (interface);
    }
#ifdef HAVE_FREETYPE
  if (null_interface.handle)
    {
      (*null_interface.fini) ();
      null_interface.handle = NULL;
    }
#endif	/* not HAVE_FREETYPE */
  M17N_OBJECT_UNREF (device_library_list);
  minput__win_fini ();
  MDEBUG_PRINT_TIME ("FINI",
		     (mdebug__output, " to finalize input-gui module."));
  mdraw__fini ();
  MDEBUG_PRINT_TIME ("FINI", (mdebug__output, " to finalize draw module."));
  mface__fini ();
  MDEBUG_PRINT_TIME ("FINI", (mdebug__output, " to finalize face module."));
  mfont__fontset_fini ();
  MDEBUG_PRINT_TIME ("FINI", (mdebug__output, " to finalize fontset module."));
  mfont__fini ();
  MDEBUG_PRINT_TIME ("FINI", (mdebug__output, " to finalize font module."));
  mframe_default = NULL;
  MDEBUG_POP_TIME ();
  MDEBUG_PRINT_TIME ("FINI", (mdebug__output, " to finalize the gui modules."));
  MDEBUG_POP_TIME ();
  m17n_fini_flt ();
  m17n_fini ();
}

/*** @} */ 
#endif /* !FOR_DOXYGEN || DOXYGEN_INTERNAL_MODULE */

/*** @addtogroup m17nFrame */
/***en
    @brief A @e frame is an object corresponding to the graphic device.

    A @e frame is an object of the type #MFrame to hold various
    information about each display/input device.  Almost all m17n GUI
    functions require a pointer to a frame as an argument.  */
/***ja
    @brief @e �ե졼�� �Ȥϥ���ե��å��ǥХ������б����륪�֥������ȤǤ���.

    @e �ե졼�� �Ȥ� #MFrame 
    ���Υ��֥������ȤǤ��ꡢ�ġ���ɽ�������ϥǥХ����ξ�����Ǽ���뤿����Ѥ����롣
    �ۤȤ�ɤ��٤Ƥ� m17n GUI�ؿ��ϡ������Ȥ��ƥե졼��ؤΥݥ��󥿤��׵᤹�롣  */

/*** @{ */
/*=*/

/***en
    @name Variables: Keys of frame parameter

    These are the symbols to use in a parameter to create a frame.  See
    the function mframe () for details.

    @b Mdevice, @b Mdisplay, @b Mscreen, @b Mdrawable, @b Mdepth, and
    @b Mcolormap are also keys of a frame property.  */

/***ja
    @name �ѿ��� �ե졼��ѥ�᡼���ѥ���

    �ե졼�����������ݤΥѥ�᡼�����Ѥ��륷��ܥ롣�ܤ����ϴؿ�
    mframe () ���������ȡ�

    @b Mdevice�� @b Mdisplay�� @b Mscreen�� @b Mdrawable�� @b Mdepth��
    @b Mcolormap �ϥե졼��ץ�ѥƥ��Υ����Ǥ⤢�롣  */

/*=*/

MSymbol Mdevice, Mdisplay, Mscreen, Mdrawable, Mdepth, Mcolormap, Mwidget; 

MSymbol Mgd;

/*=*/

/***en
    @name Variables: Keys of frame property

    These are the symbols to use as an argument to the function
    mframe_get_prop ().  */
/***ja
    @name �ѿ��� �ե졼��ץ�ѥƥ��Υ���

    �ؿ� mframe_get_prop () �ΰ������Ѥ����륷��ܥ롣  */ 
/*** @{ */ 
/*=*/
MSymbol Mfont;
MSymbol Mfont_width;
MSymbol Mfont_ascent;
MSymbol Mfont_descent;

/*=*/

/*** @} */ 
/*=*/

/***en
    @brief Create a new frame.

    The mframe () function creates a new frame with parameters listed
    in $PLIST which may be @c NULL.

    The recognized keys in $PLIST are window system dependent.

    The following key is always recognized.

    <ul>

    <li> @b Mdevice, the value must be one of #Mx, @b Mgd, and #Mnil.

    If the value is #Mx, the frame is for X Window System.  The
    argument #MDrawWindow specified together with the frame must be of
    type @c Window.  The frame is both readable and writable, thus all
    GUI functions can be used.

    If the value is @b Mgd, the frame is for an image object of GD
    library.  The argument #MDrawWindow specified together with the
    frame must be of type @c gdImagePtr.  The frame is writable
    only, thus functions minput_XXX can't be used for the frame.

    If the value is #Mnil, the frame is for a null device.  The frame
    is not writable nor readable, thus functions mdraw_XXX that
    require the argument #MDrawWindow and functions minput_XXX can't
    be used for the frame.

    <li> #Mface, the value must be a pointer to #MFace.

    The value is used as the default face of the frame.

    </ul>

    In addition, if the value of the key @b Mdevice is #Mx, the
    following keys are recognized.  They are to specify the root
    window and the depth of drawables that can be used with the frame.

    <ul>

    <li> @b Mdrawable, the value type must be <tt>Drawable</tt>.

    A parameter of key @b Mdisplay must also be specified.  The
    created frame can be used for drawables whose root window and
    depth are the same as those of the specified drawable on the
    specified display.

    When this parameter is specified, the parameter of key @b Mscreen
    is ignored.

    <li> @b Mwidget, the value type must be <tt>Widget</tt>.

    The created frame can be used for drawables whose root window and
    depth are the same as those of the specified widget.

    If a parameter of key #Mface is not specified, the default face
    is created from the resources of the widget.

    When this parameter is specified, the parameters of key @b Mdisplay,
    @b Mscreen, @b Mdrawable, @b Mdepth are ignored.

    <li> @b Mdepth, the value type must be <tt>unsigned</tt>.

    The created frame can be used for drawables of the specified
    depth.

    <li> @b Mscreen, the value type must be <tt>(Screen *)</tt>.

    The created frame can be used for drawables whose root window is
    the same as the root window of the specified screen, and depth is
    the same at the default depth of the screen.

    When this parameter is specified, parameter of key @b Mdisplay is
    ignored.

    <li> @b Mdisplay, the value type must be <tt>(Display *)</tt>.

    The created frame can be used for drawables whose root window is
    the same as the root window for the default screen of the display,
    and depth is the same as the default depth of the screen.

    <li> @b Mcolormap, the value type must be <tt>(Colormap)</tt>.

    The created frame uses the specified colormap.

    <li> @b Mfont, the value must be #Mx, #Mfreetype, or #Mxft.

    The created frame uses the specified font backend.  The value #Mx
    instructs to use X core fonts, #Mfreetype to use local fonts
    supported by FreeType fonts, and #Mxft to use local fonts via Xft
    library.  You can specify this parameter more than once with
    different values if you want to use multiple font backends.  This
    is ignored if the specified font backend is not supported on the
    device.
    
    When this parameter is not specified, all font backend supported
    on the device are used.

    </ul>

    @return
    If the operation was successful, mframe () returns a pointer to a
    newly created frame.  Otherwise, it returns @c NULL.  */

/***ja
    @brief �������ե졼�����.

    �ؿ� mframe () �� $PLIST ��Υѥ�᡼������Ŀ������ե졼����롣
    $PLIST �� @c NULL �Ǥ��ɤ���

    $PLIST �˸����륭���Τ����ɤ줬ǧ������뤫�ϥ�����ɥ������ƥ�˰�¸���롣

    �ʲ��Υ����Ͼ��ǧ������롣

    <ul>

    <li> @b Mdevice. �ͤ� #Mx, @b Mgd, #Mnil �Τ����줫�Ǥʤ��ƤϤʤ�ʤ���

    �ͤ� #Mx �ʤ�С��������ե졼��� X ������ɥ������ƥ��ѤǤ��롣
    ���Υե졼��ȶ��˻��ꤵ�줿���� #MDrawWindow �ϡ� @c Window
    ���Ǥʤ��ƤϤʤ�ʤ����ե졼����ɤ߽񤭤Ȥ�˲�ǽ�Ǥ��ꡢ���٤Ƥ�GUI 
    �ؿ������ѤǤ��롣

    �ͤ� @b Mgd �ʤ�С��������ե졼��� GD 
    �饤�֥��Υ��᡼�����֥��������ѤǤ��롣���Υե졼��ȶ��˻��ꤵ�줿����
    #MDrawWindow �ϡ� @c gdImagePtr ���Ǥʤ��ƤϤʤ�ʤ����ե졼��Ͻ񤭽Ф����ѤǤ��ꡢ
    minput_ �ǻϤޤ�̾���δؿ��ϻ��ѤǤ��ʤ���

    �ͤ� #Mnil �ʤ�С��������ե졼���, null 
    �ǥХ����ѤǤ��롣���Υե졼����ɤ߽񤭤Ǥ��ʤ��Τǡ����� #MDrawWindow 
    ��ɬ�פȤ���mdraw_ �ǻϤޤ�̾���δؿ��䡢minput_ �ǻϤޤ�̾���δؿ��ϻ��ѤǤ��ʤ���

    <li> #Mface. �ͤ� #MFace �ؤΥݥ��󥿤Ǥʤ��ƤϤʤ�ʤ���

    �����ͤϥե졼��Υǥե���ȤΥե������Ȥ����Ѥ����롣

    </ul>

    �����Υ����˲ä���@b Mdevice �Υ����� #Mx 
    �Ǥ�����˸¤�ʲ��Υ�����ǧ������롣�ʲ��Υ����ϥ롼�ȥ�����ɥ��ȡ��ե졼������ѤǤ��� 
    drawable �ο�������ꤹ�롣

    <ul>

    <li> @b Mdrawable. �ͤ� <tt>Drawable</tt> ���Ǥʤ��ƤϤʤ�ʤ���

    ���� @b Mdisplay ����ĥѥ�᡼������ꤵ��Ƥ���ɬ�פ����롣
    �������줿�ե졼��ϡ����ꤵ�줿�ǥ����ץ쥤��λ��ꤵ�줿 drawable 
    ��Ʊ���롼�ȥ�����ɥ��ȿ�������� drawable ���Ѥ����롣

    ���Υѥ�᡼����������ˤϡ�@b Mscreen �򥭡��Ȥ���ѥ�᡼����̵�뤵��롣

    <li> @b Mwidget. �ͤ� <tt>Widget</tt> ���Ǥʤ��ƤϤʤ�ʤ���

    �������줿�ե졼��ϡ����ꤷ�����������åȤ�Ʊ���롼�ȥ�����ɥ��ȿ��������
    drawable ���Ѥ����롣

    ���� #Mface ����ĥѥ�᡼�����ʤ���С��ǥե���ȤΥե������Ϥ���
    ���������åȤΥ꥽�����������롣

    ���Υѥ�᡼����������ˤϡ�@b Mdisplay, @b Mscreen, @b Mdrawable,
    @b Mdepth �򥭡��Ȥ���ѥ�᡼����̵�뤵��롣

    <li> @b Mdepth. �ͤ� <tt>unsigned</tt>  ���Ǥʤ��ƤϤʤ�ʤ���

    �������줿�ե졼��ϡ����ꤷ�������� drawable ���Ѥ����롣

    <li> @b Mscreen. �ͤ� <tt>(Screen *)</tt> ���Ǥʤ��ƤϤʤ�ʤ���

    ���������ե졼��ϡ����ꤷ�������꡼���Ʊ���롼�ȥ�����ɥ�������������꡼��Υǥե���Ȥο�����Ʊ����������� drawable ���Ѥ����롣

    ���Υѥ�᡼����������ˤϡ�@b Mdisplay �򥭡��Ȥ���ѥ�᡼����̵�뤵��롣

    <li> @b Mdisplay. �ͤ� <tt>(Display *)</tt> ���Ǥʤ��ƤϤʤ�ʤ���

    �������줿�ե졼��ϡ����ꤷ���ǥ����ץ쥤�Υǥե���ȥ����꡼���Ʊ���롼�ȥ�����ɥ���Ʊ�����������drawables ���Ѥ����롣

    <li> @b Mcolormap. �ͤ� <tt>(Colormap)</tt> ���Ǥʤ��ƤϤʤ�ʤ���

    �������줿�ե졼��ϡ����ꤷ�����顼�ޥåפ���Ѥ��롣

    <li> #Mfont. �ͤϡ�#Mx, #Mfreetype, #Mxft �Τ����줫��

    �������줿�ե졼��ϻ��ꤷ���ե���ȥХå�����ɤ���Ѥ��롣�ͤ� 
    #Mx �Ǥ���� X �Υ����ե���ȡ�#Mfreetype �Ǥ���� FreeType 
    �ǥ��ݡ��Ȥ���Ƥ��������ե���ȡ�#Mxft �Ǥ���� Xft 
    �饤�֥���ͳ���Ѥ��������ե���Ȥ���Ѥ��롣
    ʣ���Υե���ȥХå�����ɤ���Ѥ��������ˤϡ����Υѥ�᡼����ʣ���󡢰ۤʤ��ͤǻ��ꤹ�뤳�Ȥ��Ǥ��롣
    ���ꤷ���Хå�����ɤ����ݡ��Ȥ���Ƥ��ʤ��ǥХ����Ǥϡ����Υѥ�᡼����̵�뤵��롣

    ���Υѥ�᡼����̵�����ˤϡ��ǥХ����ǥ��ݡ��Ȥ���Ƥ��뤹�٤ƤΥե���ȥХå�����ɤ����Ѥ��롣

    </ul>

    @return
    ��������� mframe() �Ͽ������ե졼��ؤΥݥ��󥿤��֤��������Ǥʤ����
    @c NULL ���֤���  */

MFrame *
mframe (MPlist *plist)
{
  MFrame *frame;
  int plist_created = 0;
  MPlist *pl;
  MSymbol device;
  MDeviceLibraryInterface *interface;

  if (plist)
    {
      pl = mplist_find_by_key (plist, Mdevice);
      if (pl)
	device = MPLIST_VAL (pl);
      else
	device = Mx;
    }
  else
    {
      plist = mplist ();
      plist_created = 1;
      device = Mx;
    }

  if (device == Mnil)
    {
#ifdef HAVE_FREETYPE
      interface = &null_interface;
      if (! interface->handle)
	{
	  (*interface->init) ();
	  interface->handle = (void *) 1;
	}
#else  /* not HAVE_FREETYPE */
      MERROR (MERROR_WIN, NULL);
#endif	/* not HAVE_FREETYPE */
    }
  else
    {
      interface = mplist_get (device_library_list, device);
      if (! interface)
	MERROR (MERROR_WIN, NULL);
      if (! interface->handle)
	{
	  if (! (interface->handle = dlopen (interface->library, RTLD_NOW))
	      || ! (interface->init
		    = (int (*) ()) dlsym (interface->handle, "device_init"))
	      || ! (interface->open
		    = (int (*) (MFrame *, MPlist *)) dlsym (interface->handle,
							    "device_open"))
	      || ! (interface->fini
		    = (int (*) ()) dlsym (interface->handle, "device_fini"))
	      || (*interface->init) () < 0)
	    {
	      fprintf (stderr, "%s\n", (char *) dlerror ());
	      if (interface->handle)
		dlclose (interface->handle);
	      MERROR (MERROR_WIN, NULL);
	    }
	}
    }

  M17N_OBJECT (frame, free_frame, MERROR_FRAME);
  if ((*interface->open) (frame, plist) < 0)
    {
      free (frame);
      MERROR (MERROR_WIN, NULL);
    }

  if (! mframe_default)
    mframe_default = frame;

  frame->face = mface ();
  MPLIST_DO (pl, plist)
    if (MPLIST_KEY (pl) == Mface)
      mface_merge (frame->face, (MFace *) MPLIST_VAL (pl));
  mface__update_frame_face (frame);
  frame->font
    = frame->rface->rfont ? (MFont *) frame->rface->rfont : NULL;
  if (plist_created)
    M17N_OBJECT_UNREF (plist);
  return frame;
}

/*=*/

/***en
    @brief Return property value of frame.

    The mframe_get_prop () function returns a value of property $KEY
    of frame $FRAME.  The valid keys and the corresponding return
    values are as follows.

@verbatim

        key             type of value   meaning of value
        ---             -------------   ----------------
        Mface           MFace *         The default face.

        Mfont           MFont *         The default font.

        Mfont_width     int             Width of the default font.

        Mfont_ascent    int             Ascent of the default font.

        Mfont_descent   int             Descent of the default font.

@endverbatim

    In the m17n-X library, the followings are also accepted.

@verbatim

        key             type of value   meaning of value
        ---             -------------   ----------------
        Mdisplay        Display *       Display associated with the frame.

        Mscreen         int             Screen number of a screen associated
                                        with the frame.

        Mcolormap       Colormap        Colormap of the frame.

        Mdepth          unsigned        Depth of the frame.
@endverbatim
*/

/***ja
    @brief �ե졼��Υץ�ѥƥ����ͤ��֤�.

    �ؿ� mframe_get_prop () �ϥե졼�� $FRAME �Υ��� $KEY 
    ����ĥץ�ѥƥ����ͤ��֤���ͭ���ʥ����Ȥ����ͤϰʲ����̤ꡣ

@verbatim

        ����            �ͤη�          �ͤΰ�̣
        ---             -------------   ----------------
        Mface           MFace *         �ǥե���ȤΥե�����

        Mfont           MFont *         �ǥե���ȤΥե����

        Mfont_width     int             �ǥե���ȤΥե���Ȥ���

        Mfont_ascent    int             �ǥե���ȤΥե���Ȥ� ascent

        Mfont_descent   int             �ǥե���ȤΥե���Ȥ� descent

@endverbatim

     m17n-X �饤�֥��Ǥϡ��ʲ��Υ�������ѤǤ��롣

@verbatim

        ����            �ͤη�          �ͤΰ�̣
        ---             -------------   ----------------
        Mdisplay        Display *       �ե졼��ȴ�Ϣ�դ���줿�ǥ����ץ쥤

        Mscreen         int             �ե졼��ȴ�Ϣ�դ���줿�����꡼��
                                        �Υ����꡼��ʥ��

        Mcolormap       Colormap        �ե졼��Υ��顼�ޥå�

        Mdepth          unsigned        �ե졼��ο���
@endverbatim

*/

void *
mframe_get_prop (MFrame *frame, MSymbol key)
{
  if (key == Mface)
    return frame->face;
  if (key == Mfont)
    return frame->font;
  if (key == Mfont_width)
    return (void *) (frame->average_width);
  if (key == Mfont_ascent)
    return (void *) (frame->ascent);
  if (key == Mfont_descent)
    return (void *) (frame->descent);
  return (*frame->driver->get_prop) (frame, key);
}

/*=*/

/***en
    @brief The default frame.

    The external variable #mframe_default contains a pointer to the
    default frame that is created by the first call of mframe ().  */

/***ja
    @brief �ǥե���ȤΥե졼��.

    �����ѿ� #mframe_default �ϡ��ǥե���ȤΥե졼��ؤΥݥ��󥿤���ġ�
    �ǥե���ȤΥե졼��ϡ��ǽ�� mframe () ���ƤӽФ��줿�Ȥ��˺���롣  */

MFrame *mframe_default;

/*** @} */

/*
  Local Variables:
  coding: euc-japan
  End:
*/
