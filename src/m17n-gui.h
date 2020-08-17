/* m17n-gui.h -- header file for the GUI API of the m17n library.
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

#ifndef _M17N_GUI_H_
#define _M17N_GUI_H_

#ifndef _M17N_FLT_H_
#include <m17n-flt.h>
#endif

#ifndef _M17N_H_
#include <m17n.h>
#endif

M17N_BEGIN_HEADER

#if !defined (FOR_DOXYGEN) || defined (DOXYGEN_INTERNAL_MODULE)

extern void m17n_init_win (void);
#undef M17N_INIT
#define M17N_INIT() m17n_init_win ()

extern void m17n_fini_win (void);
#undef M17N_FINI
#define M17N_FINI() m17n_fini_win ()

#endif

/***en @defgroup m17nGUI GUI API
    @brief API provided by libm17n-gui.so */
/***ja @defgroup m17nGUI GUI API
    @brief libm17n-gui.so ���󶡤��� API */
/*=*/

/*** @ingroup m17nGUI */
/***en @defgroup m17nFrame Frame */
/***ja @defgroup m17nFrame �ե졼�� */
/*=*/

/*** @ingroup m17nFrame */
/***en
    @brief Type of frames.

    The type #MFrame is for a @e frame object.  Each frame holds
    various information about the corresponding physical display/input
    device.

    The internal structure of the type #MFrame is concealed from an
    application program, and its contents depend on the window system
    in use.  In the m17n-X library, it contains the information about
    @e display and @e screen in the X Window System.  */

/***ja
    @brief �ե졼��η����.

    #MFrame �ϡ�@e �ե졼�� ���֥��������Ѥη��Ǥ��롣
    �ġ��Υե졼��ϡ����줬�б�����ʪ��Ū��ɽ�������ϥǥХ����γƼ������ݻ����롣

    #MFrame ����������¤�ϡ����ץꥱ�������ץ���फ��ϸ����ʤ���
    �ޤ��������Ƥϻ��Ѥ��륦����ɥ������ƥ�˰�¸���롣�ޤ� m17n-X 
    �饤�֥��ˤ�����ե졼��ϡ�X ������ɥ��� @e display �� @e screen 
    �˴ؤ���������ġ�      */

typedef struct MFrame MFrame;

/*=*/

extern MSymbol Mdevice;

extern MSymbol Mfont;
extern MSymbol Mfont_width;
extern MSymbol Mfont_ascent;
extern MSymbol Mfont_descent;
extern MFrame *mframe_default;

extern MSymbol Mdisplay;
extern MSymbol Mscreen;
extern MSymbol Mdrawable;
extern MSymbol Mwidget;
extern MSymbol Mdepth;
extern MSymbol Mcolormap;

extern MFrame *mframe (MPlist *plist);

extern void *mframe_get_prop (MFrame *frame, MSymbol key);

/* end of frame module */
/*=*/

/*** @ingroup m17nGUI  */
/***en @defgroup m17nFont Font */
/***ja @defgroup m17nFont �ե���� */
/*=*/

/*** @ingroup m17nFont */
/***en
    @brief Type of fonts.

    The type #MFont is the structure defining fonts.  It contains
    information about the following properties of a font: foundry,
    family, weight, style, stretch, adstyle, registry, size, and
    resolution.

    This structure is used both for specifying a font in a fontset
    and for storing information about available system fonts.

    The internal structure is concealed from an application program.  */

/***ja
    @brief �ե���Ȥη����.

    #MFont ���ϥե���Ȼ����Ѥι�¤�ΤǤ��ꡢ�ե���ȤΥץ�ѥƥ��Ǥ���
    foundry, family, weight, style, stretch, adstyle, registry,
    size, resolution �˴ؤ�������ޤࡣ

    ���ι�¤�Τϥե���ȥ��å���Υե���Ȥ���ꤹ��ݤȡ����Ѳ�ǽ�ʥ����ƥ�ե���Ȥξ�����Ǽ����ݤ�ξ�����Ѥ����롣

    ������¤�ϥ��ץꥱ�������ץ���फ��ϸ����ʤ���  */

/***
    @seealso
    mfont (), mfont_from_name (), mfont_find ().  */

typedef struct MFont MFont;

/*=*/

extern MSymbol Mx, Mfreetype, Mxft;

extern MPlist *mfont_freetype_path;

extern MFont *mfont ();

extern MFont *mfont_copy (MFont *font);

extern MFont *mfont_parse_name (const char *name, MSymbol format);

extern char *mfont_unparse_name (MFont *font, MSymbol format);

/* These two are obsolete (from 1.1.0).  */
extern char *mfont_name (MFont *font);
extern MFont *mfont_from_name (const char *name);

extern MSymbol Mfoundry;
extern MSymbol Mfamily;
extern MSymbol Mweight;
extern MSymbol Mstyle;
extern MSymbol Mstretch;
extern MSymbol Madstyle;
extern MSymbol Mspacing;
extern MSymbol Mregistry;
extern MSymbol Msize;
extern MSymbol Mresolution;
extern MSymbol Mmax_advance;
extern MSymbol Motf;
extern MSymbol Mfontfile;

extern MSymbol Mfontconfig;

extern void *mfont_get_prop (MFont *font, MSymbol key);

extern int mfont_put_prop (MFont *font, MSymbol key, void *val);

extern int mfont_set_encoding (MFont *font,
			       MSymbol encoding_name, MSymbol repertory_name);


/*=*/

/***en
    @brief Find a font.

    The mfont_find () function returns a pointer to the available font
    that matches best with the specification $SPEC in frame $FRAME.

    $SCORE, if not NULL, must point to a place to store the score
    value which indicates how well the found font matches $SPEC.  The
    smaller score means a better match.

    $LIMITED_SIZE, if nonzero, forces the font selector to find a
    font not greater than the #Msize property of $SPEC.  */

/***ja
    @brief �ե���Ȥ�õ��.

    �ؿ� mfont_find () �ϡ��ե졼�� $FRAME ��ǥե������� $SPEC 
    �ˤ�äȤ���פ�����Ѳ�ǽ�ʥե���ȤؤΥݥ��󥿤��֤���  

    $SCORE �� NULL �Ǥ��뤫�����Ĥ��ä��ե���Ȥ� $SPEC 
    �ˤɤ�ۤɹ�äƤ��뤫�򼨤�����������¸������ؤΥݥ��󥿤Ǥ��롣
    ���������������ۤ��ɤ���äƤ��뤳�Ȥ��̣���롣

    $LIMITED_SIZE �� 0 �Ǥʤ���С�$SPEC �Υץ�ѥƥ� #Msize 
    ����礭���ʤ��ե���Ȥ�����õ����롣
*/

extern MFont *mfont_find (MFrame *frame, MFont *spec,
			  int *score, int limited_size);
extern MSymbol *mfont_selection_priority ();

extern int mfont_set_selection_priority (MSymbol *keys);

extern int mfont_resize_ratio (MFont *font);

extern MPlist *mfont_list (MFrame *frame, MFont *font, MSymbol language,
			   int maxnum);
extern MPlist *mfont_list_family_names (MFrame *frame);

typedef struct MFontset MFontset;

extern int mfont_check (MFrame *frame, MFontset *fontset,
			MSymbol script, MSymbol language, MFont *font);

extern int mfont_match_p (MFont *font, MFont *spec);

extern MFont *mfont_open (MFrame *frame, MFont *font);

extern MFont *mfont_encapsulate (MFrame *frame, MSymbol data_type, void *data);

extern int mfont_close (MFont *font);

/* end of font module */
/*=*/

/*** @ingroup m17nGUI  */
/***en @defgroup m17nFontset Fontset */
/***ja @defgroup m17nFontset �ե���ȥ��å� */
/*=*/
/*** @addtogroup m17nFontset
     @{   */
extern MFontset *mfontset (char *name);

extern MSymbol mfontset_name (MFontset *fontset);

extern MFontset *mfontset_copy (MFontset *fontset, char *name);

extern int mfontset_modify_entry (MFontset *fontset,
				  MSymbol language, MSymbol script,
				  MSymbol charset,
				  MFont *spec, MSymbol layouter_name,
				  int how);

extern MPlist *mfontset_lookup (MFontset *fontset, MSymbol script,
				MSymbol language, MSymbol charset);
/*** @}   */
/* end of fontset module */
/*=*/

/*** @ingroup m17nGUI */
/***en @defgroup m17nFace Face */
/***ja @defgroup m17nFace �ե����� */
/*=*/

/*** @ingroup m17nFace */
/***en
    @brief Type of faces.

    The type #MFace is the structure of face objects.  The internal
    structure is concealed from an application program.  */

/***ja
    @brief �ե������η����.

    #MFace ���ϥե��������֥������ȤΤ���ι�¤�ΤǤ��롣
    ������¤�ϥ��ץꥱ�������ץ���फ��ϸ����ʤ���  */

typedef struct MFace MFace;
/*=*/

extern MSymbol Mforeground;
extern MSymbol Mbackground;
extern MSymbol Mvideomode;
extern MSymbol Mnormal;
extern MSymbol Mreverse;
extern MSymbol Mhline;
extern MSymbol Mbox;
extern MSymbol Mfontset;
extern MSymbol Mratio;
extern MSymbol Mhook_func;
extern MSymbol Mhook_arg;

/* Predefined faces.  */
extern MFace *mface_normal_video;
extern MFace *mface_reverse_video;
extern MFace *mface_underline;
extern MFace *mface_medium;
extern MFace *mface_bold;
extern MFace *mface_italic;
extern MFace *mface_bold_italic;
extern MFace *mface_xx_small;
extern MFace *mface_x_small;
extern MFace *mface_small;
extern MFace *mface_normalsize;
extern MFace *mface_large;
extern MFace *mface_x_large;
extern MFace *mface_xx_large;
extern MFace *mface_black;
extern MFace *mface_white;
extern MFace *mface_red;
extern MFace *mface_green;
extern MFace *mface_blue;
extern MFace *mface_cyan;
extern MFace *mface_yellow;
extern MFace *mface_magenta;

/* etc */
extern MSymbol Mface;

extern MFace *mface ();

extern int mface_equal (MFace *face1, MFace *face2);

extern MFace *mface_copy (MFace *face);

extern MFace *mface_merge (MFace *dst, MFace *src);

extern MFace *mface_from_font (MFont *font);

/*=*/

/*** @ingroup m17nFace */
/***en
    @brief Type of horizontal line spec of face.

    The type #MFaceHLineProp is to specify the detail of #Mhline
    property of a face.  The value of the property must be a pointer
    to an object of this type.  */
/***ja
    @brief �ե������ο�ʿ�������ѷ����.

    #MFaceHLineProp �ϥե������� #Mhline 
    �ץ�ѥƥ��ξܺ٤���ꤹ�뷿�Ǥ��롣���Υץ�ѥƥ����ͤϤ��η��Υ��֥������ȤǤʤ��ƤϤʤ�ʤ���
      */

typedef struct
{
  /***en Type of the horizontal line.  */
  /***ja ��ʿ���Υ�����.  */
  enum MFaceHLineType
    {
      MFACE_HLINE_BOTTOM,      
      MFACE_HLINE_UNDER,
      MFACE_HLINE_STRIKE_THROUGH,
      MFACE_HLINE_OVER,
      MFACE_HLINE_TOP
    } type;

  /***en Width of the line in pixels.  */
  /***ja �����ʥԥ�����ñ�̡�.  */
  unsigned width;

  /***en Color of the line.  If the value is Mnil, foreground color of
      a merged face is used.  */
  /***ja ���ο�.  Mnil �ʤ�С����礷���ե����������ʿ����Ȥ��롣  */
  
  MSymbol color;
} MFaceHLineProp;
/*=*/

/*** @ingroup m17nFace */
/***en
    @brief Type of box spec of face.

    The type #MFaceBoxProp is to specify the detail of #Mbox property
    of a face.  The value of the property must be a pointer to an
    object of this type.  */
/***ja
    @brief �ե������ΰϤ��Ȼ����ѷ����.

    #MFaceBoxProp �ϥե������� #Mbox �ץ�ѥƥ��ξܺ٤���ꤹ�뷿�Ǥ��롣
    ���Υץ�ѥƥ����ͤϤ��η��Υ��֥������ȤؤΥݥ��󥿤Ǥʤ��ƤϤʤ�ʤ���
      */

typedef struct
{
  /***en Width of the box line in pixels.  */
  /***ja �����ʥԥ�����ñ�̡�.  */
  unsigned width;

  /* @{ */
  /*** Colors of borders.  */
  MSymbol color_top;
  MSymbol color_bottom;
  MSymbol color_left;
  MSymbol color_right;
  /* @} */

  /* @{ */
  /*** Margins  */
  unsigned inner_hmargin;
  unsigned inner_vmargin;
  unsigned outer_hmargin;
  unsigned outer_vmargin;
  /* @} */

} MFaceBoxProp;
/*=*/

/*** @ingroup m17nFace */
/***en
    @brief Type of hook function of face.

    #MFaceHookFunc is a type of a hook function of a face.  */
/***ja
    @brief �ե������Υեå��ؿ��η����.

    #MFaceHookFunc �ϥե������Υեå��ؿ��η��Ǥ��롣*/
typedef void (*MFaceHookFunc) (MFace *face, void *arg, void *info);
/*=*/

extern void *mface_get_prop (MFace *face, MSymbol key);

extern int mface_put_prop (MFace *face, MSymbol key, void *val);

extern MFaceHookFunc mface_get_hook (MFace *face);

extern int mface_put_hook (MFace *face, MFaceHookFunc func);

extern void mface_update (MFrame *frame, MFace *face);

/* end of face module */
/*=*/

/*** @ingroup m17nGUI */
/***en @defgroup m17nDraw Drawing */
/***ja @defgroup m17nDraw ɽ�� */
/*=*/

/*** @ingroup m17nDraw */
/***en
    @brief Window system dependent type for a window.

    The type #MDrawWindow is for a window; a rectangular area that
    works in several ways like a miniature screen.

    What it actually points depends on a window system.  A program
    that uses the m17n-X library must coerce the type @c Drawable to
    this type.  */
/***ja 
    @brief ������ɥ������ƥ�˰�¸���롢������ɥ��η����.

    #MDrawWindow �ϥ�����ɥ������ʤ�����Ĥ������ǥ����꡼��Υߥ˥��奢�Ȥ���Ư������ΰ��Ѥη��Ǥ��롣

    �ºݤ˲���ؤ����ϥ�����ɥ������ƥ�˰�¸���롣 m17n X 
    �饤�֥������Ѥ���ץ����� @c Drawable ���򤳤η����Ѵ����ʤ��ƤϤʤ�ʤ��� */

typedef void *MDrawWindow;
/*=*/

/*** @ingroup m17nDraw */
/***en
    @brief Window system dependent type for a region.

    The type #MDrawRegion is for a region; an arbitrary set of pixels
    on the screen (typically a rectangular area).

    What it actually points depends on a window system.  A program
    that uses the m17n-X library must coerce the type @c Region to
    this type.  */
/***ja
    @brief ������ɥ������ƥ�˰�¸���롢�ΰ�η����.

    #MDrawRegion ���ΰ衢���ʤ�������꡼����Ǥ�դΥԥ�����ν����ŵ��Ū�ˤ϶���ΰ���Ѥη��Ǥ��롣

    �ºݤ˲���ؤ����ϥ�����ɥ������ƥ�˰�¸���롣 m17n X 
    �饤�֥������Ѥ���ץ����� @c Region ���򤳤η����Ѵ����ʤ��ƤϤʤ�ʤ���  */

typedef void *MDrawRegion;
/*=*/

/*** @ingroup m17nDraw */
/***en
    @brief Type of a text drawing control.

    The type #MDrawControl is the structure that controls how to draw
    an M-text.  */
/***ja
    @brief �ƥ�����ɽ������η����.

    #MDrawControl ���ϡ�M-text ��ɤ�ɽ�����뤫�����椹�빽¤�ΤǤ��롣
      */


typedef struct
{
  /***en If nonzero, draw an M-text as image, i.e. with background
      filled with background colors of faces put on the M-text.
      Otherwise, the background is not changed.  */
  /***ja 0 �Ǥʤ���С� M-text ������Ȥ��ơ����ʤ���طʤ� M-text 
      �Υե������ǻ��ꤵ��Ƥ����طʿ�������ɽ�����롣�����Ǥʤ�����طʤ��Ѥ��ʤ���  */
  unsigned as_image : 1;

  /***en If nonzero and the first glyph of each line has negative
      lbearing, shift glyphs horizontally to right so that no pixel is
      drawn to the left of the specified position.  */
  /***ja 0 �Ǥʤ����ƹԤκǽ�Υ���դ� lbearing 
      ����ʤ�С�����դ��ʿ�˱��ˤ��餷�ơ����ꤷ�����֤�꺸�˥ԥ����뤬������ʤ��褦�ˤ��롣  */
  unsigned align_head : 1;

  /***en If nonzero, draw an M-text two-dimensionally, i.e., newlines
      in M-text breaks lines and the following characters are drawn in
      the next line.  If \<format\> is non-NULL, and the function
      returns nonzero line width, a line longer than that width is
      also broken.  */
  /***ja 0 �Ǥʤ���С�M-text �򣲼���Ū�ˡ����ʤ�� M-text ��� 
      newline �ǲ��Ԥ���³��ʸ���ϼ��ιԤ�ɽ�����롣�⤷ \<format\> �� 
      NULL �Ǥʤ������δؿ��� 0 �Ǥʤ��������֤��С����������Ĺ���Ԥ���Ԥ���롣  */
  unsigned two_dimensional : 1;

  /***en If nonzero, draw an M-text to the right of a specified
      position.  */
  /***ja 0 �Ǥʤ���С�M-text ����ꤷ�����֤α���ɽ�����롣  */
  unsigned orientation_reversed : 1;

  /***en If nonzero, reorder glyphs correctly for bidi text.  */ 
  /***ja 0 �ʤ���С�bidi �ƥ������Ѥ˥���դ����������󤹤롣  */
  unsigned enable_bidi : 1;

  /***en If nonzero, don't draw characters whose general category (in
      Unicode) is Cf (Other, format).  */
  /***ja 0 �Ǥʤ���С���˥����ɤ��֤�����̥��ƥ��꤬ Cf (Other,
      format) �Ǥ���ʸ����ɽ�����ʤ���  */
  unsigned ignore_formatting_char : 1;

  /***en If nonzero, draw glyphs suitable for a terminal.  Not yet
      implemented.  */
  /***ja 0 �Ǥʤ���С�ü���ѤΥ���դ�ɽ�����롣̤������  */
  unsigned fixed_width : 1;

  /***en If nonzero, draw glyphs with anti-aliasing if a backend font
      driver supports it.  */
  /***ja 0 �Ǥʤ���С�����������ꥢ���ǥ���դ�ɽ�����롣
      �ʥХå�����ɤΥե���ȥɥ饤�Ф�����������ꥢ����ǽ����ľ��Τߡ��� */
  unsigned anti_alias : 1;

  /***en If nonzero, disable the adjustment of glyph positions to
      avoid horizontal overlapping at font boundary.  */
  /***ja 0 �Ǥʤ���С��ե���ȶ����Ǥο�ʿ�����Υ���դνŤʤ���򤱤뤿��Υ���հ��֤�Ĵ����̵���ˤ��롣  */
  unsigned disable_overlapping_adjustment : 1;

  /***en If nonzero, the values are minimum line ascent pixels.  */
  /***ja 0 �Ǥʤ���С��ͤϹԤ� ascent �κǾ��ͤ򼨤���  */
  unsigned int min_line_ascent;
  /***en If nonzero, the values are minimum line descent pixels.  */
  /***ja 0 �Ǥʤ���С��ͤϹԤ� descent �κǾ��ͤ򼨤���  */
  unsigned int min_line_descent;

  /***en If nonzero, the values are maximum line ascent pixels.  */
  /***ja 0 �Ǥʤ���С��ͤϹԤ� ascent �κ����ͤ򼨤���  */
  unsigned int max_line_ascent;
  /***en If nonzero, the values are maximum line descent pixels.  */
  /***ja 0 �Ǥʤ���С��ͤϹԤ� descent �κ����ͤ򼨤���  */
  unsigned int max_line_descent;

  /***en If nonzero, the value specifies how many pixels each line can
      occupy on the display.  The value zero means that there is no
      limit.  It is ignored if \<format\> is non-NULL.  */
  /***ja 0 �Ǥʤ���С��ͤϤ��Υǥ����ץ쥤��ǳƹԤ����뤳�ȤΤǤ���ԥ�������򼨤���
      0 �ϸ��ꤵ��ʤ����Ȥ��̣���롣\<format\> �� NULL �Ǥʤ����̵�뤵��롣   */
  unsigned int max_line_width;

  /***en If nonzero, the value specifies the distance between tab
      stops in columns (the width of one column is the width of a
      space in the default font of the frame).  The value zero means
      8.  */
  /***ja 0 �Ǥʤ���С��ͤϥ��֥��ȥå״֤ε�Υ�򥳥��ñ��
      �ʥ����ϥե졼��Υǥե���ȥե���Ȥˤ��������ʸ�������Ǥ���ˤǼ����� 
      0 �� 8 ���̣���롣 */
  unsigned int tab_width;

  /***en If non-NULL, the value is a function that calculates the
      indentation and width limit of each line based on the line
      number LINE and the coordinate Y.  The function store the
      indentation and width limit at the place pointed by INDENT and
      WIDTH respectively.

      The indentation specifies how many pixels the first glyph of
      each line is shifted to the right (if the member
      \<orientation_reversed\> is zero) or to the left (otherwise).  If
      the value is negative, each line is shifted to the reverse
      direction.

      The width limit specifies how many pixels each line can occupy
      on the display.  The value 0 means that there is no limit.

      LINE and Y are reset to 0 when a line is broken by a newline
      character, and incremented each time when a long line is broken
      because of the width limit.

      This has an effect only when \<two_dimensional\> is nonzero.  */
  /***ja 0 �Ǥʤ���С��ͤϴؿ��Ǥ��ꡢ���δؿ��Ϲ��ֹ� LINE �Ⱥ�ɸ Y 
      �˴�Ť��ƳƹԤΥ���ǥ�ȤȺ�������׻��������줾���INDENT ��
      WIDTH �ǻؤ���������¸���롣

      ����ǥ�Ȥϡ��ƹԤκǽ�Υ���դ򱦡ʥ��� 
      \<orientation_reversed\> �� 0 
      �λ��ˤ��뤤�Ϻ��ʤ���ʳ��λ��ˤ˲��ԥ����뤺�餹������ꤹ�롣�ͤ���ʤ�е������ˤ��餹��

      �������ϡ��ƹԤ��ǥ����ץ쥤������뤳�ȤΤǤ���ԥ�������κ����ͤǤ��롣�ͤ�
      0 �ξ������¤�����ʤ����Ȥ��̣���롣

      LINE �� Y �ϲ���ʸ���ˤ�äƹԤ����ޤä��ݤˤ� 0 
      �˥ꥻ�åȤ��졢Ĺ���Ԥ������������¤ˤ�äƲ��Ԥ���뤿�Ӥ� 1 ���䤵��롣

      ����� \<two_dimensional\> �� 0 �Ǥʤ����ˤΤ�ͭ���Ǥ��롣  */
  void (*format) (int line, int y, int *indent, int *width);

  /***en If non-NULL, the value is a function that calculates a line
      breaking position when a line is too long to fit within the
      width limit.  POS is the position of the character next to the
      last one that fits within the limit.  FROM is the position of the
      first character of the line, and TO is the position of the last
      character displayed on the line if there were not width limit.
      LINE and Y are the same as the arguments to \<format\>.

      The function must return a character position to break the
      line.

      The function should not modify MT.

      The mdraw_default_line_break () function is useful for such a
      script that uses SPACE as a word separator.  */
  /***ja NULL �Ǥʤ���С��ͤϹԤ���������˼��ޤ�ʤ����˹Ԥ�������֤�׻�����ؿ��Ǥ��롣
      POS �Ϻ������˼��ޤ�Ǹ��ʸ���μ���ʸ���ΰ��֤Ǥ��롣FROM
      �ϹԤκǽ��ʸ���ΰ��֡�TO 
      �Ϻ����������ꤵ��Ƥ��ʤ���Ф��ιԤ�ɽ�������Ǹ��ʸ���ΰ��֤Ǥ��롣LINE 
      �� Y �� \<format\> �ΰ�����Ʊ�ͤǤ��롣

      ���δؿ��ϹԤ�����ʸ�����֤��֤��ʤ��ƤϤʤ�ʤ����ޤ� MT ���ѹ����ƤϤʤ�ʤ���

      �ؿ� mdraw_default_line_break ()
      �ϡ�������ζ��ڤ�Ȥ����Ѥ��륹����ץ��ѤȤ���ͭ�ѤǤ��롣  */
  int (*line_break) (MText *mt, int pos, int from, int to, int line, int y);

  /***en If nonzero, show the cursor according to \<cursor_width\>.  */
  /***ja ����Ǥʤ���� \<cursor_width\> �ˤ������äƥ��������ɽ�����롣 */
  int with_cursor;

  /***en Specifies the character position to display a cursor.  If it
      is greater than the maximum character position, the cursor is
      displayed next to the last character of an M-text.  If the value
      is negative, even if \<cursor_width\> is nonzero, cursor is not
      displayed.  */
  /***ja ���������ɽ������ʸ�����֤򼨤��������ʸ�����֤���礭����С���������� 
      M-text �κǸ��ʸ�����٤�ɽ������롣��ʤ�С�
      \<cursor_width\> �� 0 �Ǥʤ��Ƥ⥫�������ɽ������ʤ���
        */
  int cursor_pos;

  /***en If nonzero, display a cursor at the character position
      \<cursor_pos\>.  If the value is positive, it is the pixel width
      of the cursor.  If the value is negative, the cursor width is
      the same as the underlining glyph(s).  */
  /***ja 0 �Ǥʤ���С�\<cursor_pos\> �˥��������ɽ�����롣
      �ͤ����ʤ�С�������������Ϥ����͡ʥԥ�����ñ�̡ˤǤ��롣
      ��ʤ�С���������Τ��륰��դ�Ʊ�����Ǥ��롣  */
  int cursor_width;

  /***en If nonzero and \<cursor_width\> is also nonzero, display double
      bar cursors; at the character position \<cursor_pos\> and at the
      logically previous character.  Both cursors have one pixel width
      with horizontal fringes at upper or lower positions.  */
  /***ja If 0 �Ǥʤ������� \<cursor_width\> �� 0 �Ǥʤ���С��С����������ʸ������
      \<cursor_pos\> ������Ū�ˤ�������ˤ���ʸ���Σ������ɽ�����롣
      �����Ȥ⣱�ԥ��������ǡ��夫���˿�ʿ�ξ��꤬�Ĥ���*/
  int cursor_bidi;

  /***en If nonzero, on drawing partial text, pixels of surrounding
      texts that intrude into the drawing area are also drawn.  For
      instance, some CVC sequence of Thai text (C is consonant, V is
      upper vowel) is drawn so that V is placed over the middle of two
      Cs.  If this CVC sequence is already drawn and only the last C
      is drawn again (for instance by updating cursor position), the
      right half of V is erased if this member is zero.  By setting
      this member to nonzero, even with such a drawing, we can keep
      this CVC sequence correctly displayed.  */
  /***ja 0 �Ǥʤ���С��ƥ����Ȥΰ���ʬ��ɽ������ݤˡ�����Υƥ����ȤΤ�������ɽ���ΰ�˿���������ʬ��ɽ�����롣
      ���Ȥ��С�������ƥ����� �Ҳ�-�첻-�Ҳ� 
      �Ȥ������������󥹤Τ����Ĥ��ϡ��첻����ĤλҲ��δ֤˾�ˤΤ�褦��������롣
      ���Τ褦�ʥ��������󥹤����Ǥ�������Ƥ��ꡢ�Ǹ�λҲ�����������ľ�����
      �ʤ��Ȥ��С�����������֤򹹿�����ݤʤɡˤ��Υ��Ф� 0 
      �Ǥ���С��첻�α�Ⱦʬ���ä���Ƥ��ޤ�������� 0 �ʳ��ˤ��뤳�Ȥˤ�äơ����Τ褦�ʺݤˤ�
      �Ҳ�-�첻-�Ҳ� �Υ��������󥹤�������ɽ����³���뤳�Ȥ��Ǥ��롣  */
  int partial_update;

  /***en If nonzero, don't cache the result of any drawing information
      of an M-text.  */
  /***ja 0 �Ǥʤ���С�M-text ��ɽ���˴ؤ������򥭥�å��夷�ʤ���
       */
  int disable_caching;

  /***en If non-NULL, limit the drawing effect to the specified region.  */
  /***ja NULL �Ǥʤ����ɽ�����ꥢ����ꤵ�줿�ΰ�˸��ꤹ�롣 */
  MDrawRegion clip_region;

} MDrawControl;

extern int mdraw_line_break_option;

/*=*/

/*** @ingroup m17nDraw */
/***en
    @brief Type of metric for glyphs and texts.

    The type #MDrawMetric is for a metric of a glyph and a drawn text.
    It is also used to represent a rectangle area of a graphic
    device.  */
/***ja
    @brief ����դȥƥ����Ȥ���ˡ�η����.

    #MDrawMetric �ϥ���դ�ɽ�����줿�ƥ����Ȥ���ˡ�Ѥη��Ǥ��롣
    �ޤ���ɽ���ǥХ����ζ���ΰ��ɽ���Τˤ��Ѥ����롣 */

typedef struct {
  /*** X coordinates of a glyph or a text.  */
  int x;
  /*** Y coordinates of a glyph or a text.  */
  int y;
  /*** Pixel width of a glyph or a text.  */
  unsigned int width;
  /*** Pixel height of a glyph or a text.  */
  unsigned int height;
} MDrawMetric;

/*=*/

/*** @ingroup m17nDraw */
/***en
    @brief Type of information about a glyph.

    The type #MDrawGlyphInfo is the structure that contains
    information about a glyph.  It is used by mdraw_glyph_info ().  */
/***ja
    @brief ����դ˴ؤ������η����.

    #MDrawGlyphInfo ���ϥ���դ˴ؤ�������ޤ๽¤�ΤǤ��롣
    mdraw_glyph_info () �Ϥ�����Ѥ��롣  */

typedef struct
{
  /***en Start position of character range corresponding to the glyph.  */
  /***ja ����դ��б�����ʸ�����ϰϤγ��ϰ���.  */
  int from;

  /***en End position of character range corresponding to the glyph.  */
  /***ja ����դ��б�����ʸ�����ϰϤν�λ����.  */
  int to;

  /***en Start position of character range corresponding to the line of the glyph.  */
  /***ja ��ԤΥ���դ�����б�����ʸ�����ϰϤγ��ϰ���.  */
  int line_from;
  /***en End position of character range corresponding to the line of the glyph.  */
  /***ja ��ԤΥ���դ�����б�����ʸ�����ϰϤν�λ����.  */
  int line_to;

  /***en X coordinates of the glyph.  */
  /***ja ����դ� X ��ɸ.  */
  int x;
  /***en Y coordinates of the glyph.  */
  /***ja ����դ� Y ��ɸ.  */
  int y;

  /***en Metric of the glyph.  */
  /***ja ����դ���ˡ.  */
  MDrawMetric metrics;

  /***en Font used for the glyph.  Set to NULL if no font is found for
      the glyph.  */
  /***ja ����դ˻Ȥ���ե���ȡ����Ĥ���ʤ���� NULL�� */
      
  MFont *font;

  /***en Character ranges corresponding to logically previous glyphs.
      Note that we do not need the members prev_to because it must
      be the same as the member \<from\>.  */
  /***ja ����Ū�����Υ���դ��б�����ʸ�����ϰϡ����� prev_to �ϡ���
      ��� from ��Ʊ���Ǥ���Ϥ��ʤΤ����פǤ��롣  */
  int prev_from;
  /***en Character ranges corresponding to logically next glyphs.
      Note that we do not need the members next_from because it must
      be the same as the member \<to\> respectively.  */
  /***ja ����Ū�ʸ�Υ���դ��б�����ʸ�����ϰϡ����� next_from ��
      ���� to ��Ʊ���Ǥ���Ϥ��ʤΤ����פǤ��롣  */
  int next_to;

  /***en Start position of character ranges corresponding to visually
      left glyphs. */
  /***ja ɽ����κ��Υ���դ��б�����ʸ�����ϰϤγ��ϰ��֡�  */
  int left_from;
  /***en End position of character ranges corresponding to visually
      left glyphs. */
  /***ja ɽ����κ��Υ���դ��б�����ʸ�����ϰϤν�λ���֡�  */
  int left_to;
  /***en Start position of character ranges corresponding to visually
      right glyphs. */
  /***ja ɽ����α��Υ���դ��б�����ʸ�����ϰϤγ��ϰ��֡�  */
  int right_from;
  /***en End position of character ranges corresponding to visually
      left glyphs. */
  /***ja ɽ����α��Υ���դ��б�����ʸ�����ϰϤν�λ���֡�  */
  int right_to;

  /***en Logical width of the glyph.  Nominal distance to the next
      glyph.  */
  /***ja ����դ�����Ū�������Υ���դȤ�̾�ܾ�ε�Υ��  */
  int logical_width;
} MDrawGlyphInfo;

/*=*/

/*** @ingroup m17nDraw */
/***en
    @brief Type of information about a glyph metric and font.

    The type #MDrawGlyph is the structure that contains information
    about a glyph metric and font.  It is used by the function
    mdraw_glyph_list ().  */
/***ja
    @brief ����դ���ˡ�ȥե���Ȥ˴ؤ������η����.

    #MDrawGlyph ���ϥ���դ���ˡ�ȥե���Ȥ˴ؤ�������ޤ๽¤�ΤǤ��롣
    mdraw_glyph_list () �Ϥ�����Ѥ��롣  */

typedef struct
{
  /* @{ */
  /***en Character range corresponding to the glyph.  */
  /***ja ����դ��б�����ʸ�����ϰ�.  */
  int from, to;
  /* @} */

  /***en Font glyph code of the glyph.  */
  /***ja �ե������Υ���ե����ɡ�  */
  int glyph_code;

  /***en Logical width of the glyph.  Nominal distance to the next
      glyph.  */
  /***ja ����դ�����Ū�������Υ���դȤ�̾�ܾ�ε�Υ��  */
  int x_advance;
  /***en Logical height of the glyph.  Nominal distance to the next
      glyph.  */
  /***ja ����դ�����Ū�⤵�����Υ���դȤ�̾�ܾ�ε�Υ��  */
  int y_advance;

  /***en X offset relative to the glyph position.  */
  /***ja ����դΰ��֤��Ф��� X ���ե��å�.  */
  int x_off;
  /***en Y offset relative to the glyph position.  */
  /***ja ����դΰ��֤��Ф��� Y ���ե��å�.  */
  int y_off;

  /***en Metric of the glyph (left-bearing).  */
  /***ja ����դ���ˡ (left-bearing).  */
  int lbearing;
  /***en Metric of the glyph (right-bearing).  */
  /***ja ����դ���ˡ (right-bearing).  */
  int rbearing;
  /***en Metric of the glyph (ascent).  */
  /***ja ����դ���ˡ (ascent).  */
  int ascent;
  /***en Metric of the glyph (descent).  */
  /***ja ����դ���ˡ (descent).  */
  int descent;

  /***en Font used for the glyph.  Set to NULL if no font is found for
      the glyph.  */
  /***ja ����դ˻Ȥ���ե���ȡ����Ĥ���ʤ���� NULL��  */
  MFont *font;

  /***en Type of the font.  One of Mx, Mfreetype, Mxft.  */
  /***ja �ե���ȤΥ����ס�Mx��Mfreetype��Mxft �Τ����줫��  */
  MSymbol font_type;

  /***en Pointer to the font structure.  The actual type is
      (XFontStruct *) if \<font_type\> member is Mx, FT_Face if
      \<font_type\> member is Mfreetype, and (XftFont *) if \<font_type\>
      member is Mxft.  */
  /***ja �ե���Ȥι�¤�ΤؤΥݥ��󥿡��ºݤη��� \<font_type\> ���Ф�
      Mx �ʤ� (XFontStruct *)�� Mfreetype �ʤ� FT_Face��Mxft 
      �ʤ� (XftFont *)�� */
  void *fontp;

} MDrawGlyph;

/*=*/

/***en
    @brief Type of textitems.

    The type #MDrawTextItem is for @e textitem objects.
    Each textitem contains an M-text and some other information to
    control the drawing of the M-text.  */

/***ja
    @brief textitem �η����.

    #MDrawTextItem �� @e �ƥ����ȥ����ƥ� ���֥��������Ѥη��Ǥ��롣
    �ƥƥ����ȥ����ƥ�ϡ� 1 �Ĥ� M-text �ȡ�����ɽ�������椹�뤿��ξ����ޤ�Ǥ��롣

    @latexonly \IPAlabel{MTextItem} @endlatexonly  */

typedef struct
{
  /***en M-text. */
  /***ja M-text. */
  MText *mt;                      

  /***en Optional change in the position (in the unit of pixel) along
      the X-axis before the M-text is drawn.  */
  /***ja M-text ɽ�����˹Ԥʤ�X�������ΰ���Ĵ�� (�ԥ�����ñ��) */
  int delta;                     

  /***en Pointer to a face object.  Each property of the face, if not
      Mnil, overrides the same property of face(s) specified as a text
      property in \<mt\>.  */
  /***ja �ե��������֥������ȤؤΥݥ��󥿡��ե������γƥץ�ѥƥ��� 
      Mnil �Ǥʤ���� \<mt\> �ǻ��ꤵ�줿�ե�������Ʊ���ץ�ѥƥ���ͥ�褹��*/
  MFace *face;

  /***en Pointer to a draw control object.  The M-text \<mt\> is drawn
      by mdraw_text_with_control () with this control object.  */
  /***ja ɽ�����楪�֥������ȤؤΥݥ��󥿡� mdraw_text_with_control () 
      �Ϥ��Υ��֥������Ȥ��Ѥ��� M-text \<mt\> ��ɽ�����롣  */
  MDrawControl *control;

} MDrawTextItem;

/*=*/

extern int mdraw_text (MFrame *frame, MDrawWindow win, int x, int y,
		       MText *mt, int from, int to);

extern int mdraw_image_text (MFrame *frame, MDrawWindow win, int x, int y,
			     MText *mt, int from, int to);

extern int mdraw_text_with_control (MFrame *frame, MDrawWindow win,
				    int x, int y, MText *mt, int from, int to,
				    MDrawControl *control);

extern int mdraw_coordinates_position (MFrame *frame,
				       MText *mt, int from, int to,
				       int x, int y, MDrawControl *control);

extern int mdraw_text_extents (MFrame *frame,
			       MText *mt, int from, int to,
			       MDrawControl *control,
			       MDrawMetric *overall_ink_return,
			       MDrawMetric *overall_logical_return,
			       MDrawMetric *overall_line_return);

extern int mdraw_text_per_char_extents (MFrame *frame,
					MText *mt, int from, int to,
					MDrawControl *control,
					MDrawMetric *ink_array_return,
					MDrawMetric *logical_array_return,
					int array_size,
					int *num_chars_return,
					MDrawMetric *overall_ink_return,
					MDrawMetric *overall_logical_return);

extern int mdraw_glyph_info (MFrame *frame, MText *mt, int from, int pos,
			     MDrawControl *control, MDrawGlyphInfo *info);

extern int mdraw_glyph_list (MFrame *frame, MText *mt, int from, int to,
			     MDrawControl *control, MDrawGlyph *glyphs,
			     int array_size, int *num_glyphs_return);

extern void mdraw_text_items (MFrame *frame, MDrawWindow win, int x, int y,
			      MDrawTextItem *items, int nitems);

extern void mdraw_per_char_extents (MFrame *frame, MText *mt,
				    MDrawMetric *array_return,
				    MDrawMetric *overall_return);

extern int mdraw_default_line_break (MText *mt, int pos,
				     int from, int to, int line, int y);

extern void mdraw_clear_cache (MText *mt);

/* end of drawing module */
/*=*/

/*** @ingroup m17nGUI */
/***en @defgroup m17nInputMethodWin Input Method (GUI) */
/***ja @defgroup m17nInputMethodWin ���ϥ᥽�å� (GUI) */
/*=*/

extern MInputDriver minput_gui_driver;

/*=*/
/*** @ingroup m17nInputMethodWin */
/***en 
    @brief Type of the argument to the function minput_create_ic ().

    The type #MInputGUIArgIC is for the argument $ARG of the function
    minput_create_ic () to create an input context of an internal
    input method.  */

/***ja
    @brief �ؿ� minput_create_ic () �ΰ����η����.

    #MInputGUIArgIC �ϡ��ؿ� minput_create_ic () 
    ���������ϥ᥽�åɤ����ϥ���ƥ����Ȥ���������ݤΡ����� $ARG �Ѥη��Ǥ��롣  */

typedef struct
{
  /***en Frame of the client.  */
  /***ja ���饤����ȤΥե졼��  */
  MFrame *frame;

  /***en Window on which to display the preedit and status text.  */
  /***ja preedit �ƥ����Ȥ� status �ƥ����Ȥ�ɽ�����륦����ɥ�  */
  MDrawWindow client;

  /***en Window that the input context has a focus on.  */
  /***ja ���ϥ���ƥ����Ȥ��ե��������򤪤��Ƥ��륦����ɥ�  */
  MDrawWindow focus;
} MInputGUIArgIC;

/*=*/

extern MSymbol minput_event_to_key (MFrame *frame, void *event);

/* end of input module */
/*=*/
/* end of window modules */
/*=*/

extern MFace *mdebug_dump_face (MFace *face, int indent);
extern MFont *mdebug_dump_font (MFont *font);
extern MFontset *mdebug_dump_fontset (MFontset *fontset, int indent);

M17N_END_HEADER

#endif /* _M17N_GUI_H_ */

/*
  Local Variables:
  coding: euc-japan
  End:
*/
