/* font.c -- font module.
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
    @addtogroup m17nFont
    @brief Font object.

    The m17n GUI API represents a font by an object of the type @c
    MFont.  A font can have @e font @e properties.  Like other types
    of properties, a font property consists of a key and a value.  The
    key of a font property must be one of the following symbols:

    @c Mfoundry, @c Mfamily, @c Mweight, @c Mstyle, @c Mstretch,
    @c Madstyle, @c Mregistry, @c Msize, @c Mresolution, @c Mspacing.

    When the key of a font property is @c Msize or @c Mresolution, its
    value is an integer.  Otherwise the value is a symbol.  

    The notation "xxx property of F" means the font property that
    belongs to font F and whose key is @c Mxxx.

    The value of a foundry property is a symbol representing font
    foundry information, e.g. adobe, misc, etc.

    The value of a family property is a symbol representing font family
    information, e.g. times, helvetica, etc.

    The value of a weight property is a symbol representing weight
    information, e.g. normal, bold, etc.

    The value of a style property is a symbol representing slant
    information, e.g. normal, italic, etc.

    The value of a stretch property is a symbol representing width
    information, e.g. normal, semicondensed, etc.

    The value of an adstyle property is a symbol representing abstract
    font family information, e.g. serif, sans-serif, etc.

    The value of a registry property is a symbol representing registry
    information, e.g. iso10646-1, iso8895-1, etc.

    The value of a size property is an integer representing design
    size in the unit of 1/10 point.

    The value of a resolution property is an integer representing
    assumed device resolution in the unit of dots per inch (dpi).

    The value of a type property is a symbol indicating a font driver;
    currently Mx or Mfreetype.

    The m17n library uses font objects for two purposes: to receive
    font specification from an application program, and to present
    available fonts to an application program.  When the m17n library
    presents an available font to an application program, all font
    properties have a concrete value.

    The m17n library supports three kinds of fonts: Window system fonts,
    FreeType fonts, and OpenType fonts.

    <ul>

    <li> Window system fonts

    The m17n-X library supports all fonts handled by an X server and
    an X font server.  The correspondence between XLFD fields and font
    properties are shown below.

@verbatim
    XLFD field                                  property
    ---------------                             --------
    FOUNDRY                                     foundry
    FAMILY_NAME                                 family
    WEIGHT_NAME                                 weight
    SLANT                                       style
    SETWIDTH_NAME                               stretch
    ADD_STYLE_NAME                              adstyle
    PIXEL_SIZE                                  size
    RESOLUTION_Y                                resolution
    CHARSET_REGISTRY-CHARSET_ENCODING           registry
@endverbatim

    XLFD fields not listed in the above table are ignored.

    <li> FreeType fonts

    The m17n library, if configured to use the FreeType library,
    supports all fonts that can be handled by the FreeType library.
    The variable #mfont_freetype_path is initialized properly according
    to the configuration of the m17n library and the environment
    variable @c M17NDIR.  See the documentation of the variable for
    details.

    If the m17n library is configured to use the fontconfig library,
    in addition to #mfont_freetype_path, all fonts available via
    fontconfig are supported.

    The family name of a FreeType font corresponds to the family
    property.  Style names of FreeType fonts correspond to the weight,
    style, and stretch properties as below.

@verbatim
    style name          weight  style   stretch
    ----------          ------  -----   -------
    Regular             medium  r       normal
    Italic              medium  i       normal
    Bold                bold    r       normal
    Bold Italic         bold    i       normal
    Narrow              medium  r       condensed
    Narrow Italic       medium  i       condensed
    Narrow Bold         bold    r       condensed
    Narrow Bold Italic  bold    i       condensed
    Black               black   r       normal
    Black Italic        black   i       normal
    Oblique             medium  o       normal
    BoldOblique         bold    o       normal
@endverbatim

    Style names not listed in the above table are treated as
    "Regular".

    Combination of a platform ID and an encoding ID corresponds to the
    registry property.  For example, if a font has the combination (1
    1), the registry property is 1-1.  Some frequent combinations have
    a predefined registry property as below.

@verbatim
    platform ID         encoding ID     registry property
    -----------         -----------     -----------------
    0                   3               unicode-bmp
    0                   4               unicode-full
    1                   0               apple-roman
    3                   1               unicode-bmp
    3                   1               unicode-full
@endverbatim

    Thus, a font that has two combinations (1 0) and (3 1) corresponds
    to four font objects whose registries are 1-0, apple-roman, 3-1,
    and unicode-bmp.

    <li> OpenType fonts 

    The m17n library, if configured to use both the FreeType library
    and the OTF library, supports any OpenType fonts.  The list of
    actually available fonts is created in the same way as in the case
    of FreeType fonts.  If a fontset instructs to use an OpenType font
    via an FLT (Font Layout Table), and the FLT has an OTF-related
    command (e.g. otf:deva), the OTF library converts a character
    sequence to a glyph code sequence according to the OpenType layout
    tables of the font, and the FreeType library gives a bitmap image
    for each glyph.

    </ul>

  */

/***ja
    @addtogroup m17nFont
    @brief �ե���ȥ��֥�������.

    m17n GUI API �ϥե���Ȥ� @c MFont ���Υ��֥������ȤȤ���ɽ�����롣
    �ե���Ȥ� @e �ե���ȥץ�ѥƥ� ����Ĥ��Ȥ��Ǥ��롣¾�Υ����פΥ�
    ��ѥƥ�Ʊ�͡��ե���ȥץ�ѥƥ��ϥ������ͤ���ʤꡢ�����ϰʲ��Υ�
    ��ܥ�Τ����줫�Ǥ��롣

    @c Mfoundry, @c Mfamily, @c Mweight, @c Mstyle, @c Mstretch,
    @c Madstyle, @c Mregistry, @c Msize, @c Mresolution, @c Mspacing

    �ե���ȥץ�ѥƥ��Υ����� @c Msize ���뤤�� @c Mresolution 
    �ξ�硢�ͤ������ͤǤ��ꡢ����������ʳ��ξ�硢�ͤϥ���ܥ�Ǥ��롣

    �֥ե���� F �Υե���ȥץ�ѥƥ��Τ��������� @c Mxxx 
    �Ǥ����ΡפΤ��Ȥ��ñ�ˡ�F �� xxx �ץ�ѥƥ��פȸƤ֤��Ȥ����롣

    foundry �ץ�ѥƥ����ͤϡ�adobe, misc 
    ���Υե���Ȥγ�ȯ������򼨤�����ܥ�Ǥ��롣

    family �ץ�ѥƥ����ͤϡ�times, helvetica 
    ���Υե���ȥե��ߥ꡼�򼨤�����ܥ�Ǥ��롣

    weight �ץ�ѥƥ����ͤϡ�normal, bold ���������˴ؤ������򼨤�����ܥ�Ǥ��롣

    style �ץ�ѥƥ����ͤϡ�normal, italic 
    ���Υ�������˴ؤ������򼨤�����ܥ�Ǥ��롣

    stretch �ץ�ѥƥ����ͤϡ�normal, semicondensed 
    ����ʸ�����˴ؤ������򼨤�����ܥ�Ǥ��롣

    adstyle �ץ�ѥƥ����ͤϡ�serif, sans-serif
    �������Ū�ʥե���ȥե��ߥ꡼�˴ؤ������򼨤�����ܥ�Ǥ��롣

    registry �ץ�ѥƥ����ͤϡ�iso10646, iso8895-1
    ���Υ쥸���ȥ����򼨤�����ܥ�Ǥ��롣

    size �ץ�ѥƥ����ͤϡ��ե���ȤΥǥ����󥵥�����ɽ�魯�����ͤǤ��ꡢ
    ñ�̤�1/10 �ݥ���ȤǤ��롣

    resolution �ץ�ѥƥ����ͤϡ����ꤵ��Ƥ���ǥХ����β����٤�ɽ�魯
    �����ͤǤ��ꡢñ�̤�dots per inch (dpi) �Ǥ��롣

    type �ץ�ѥƥ����ͤϡ��ե���ȥɥ饤�Ф�ؼ��������� Mx �⤷����
    Mfreetype �Ǥ��롣

    m17n �饤�֥��ϥե���ȥ��֥������Ȥ򣲤Ĥ���Ū���Ѥ��Ƥ��롣����
    �ꥱ�������ץ���फ��ե���Ȥλ������������Ū�ȡ����ץꥱ��
    �����ץ��������Ѳ�ǽ�ʥե���Ȥ��󼨤�����Ū�Ǥ��롣���ץꥱ��
    �����ץ������Ф����󼨤�Ԥ��ݤˤϡ��ե���ȥץ�ѥƥ��Ϥ���
    �ƶ���Ū���ͤ���ġ�

    m17n �饤�֥��� Window �����ƥ�ե���ȡ�FreeType�ե���ȡ�
    OpenType�ե���ȤΣ�����򥵥ݡ��Ȥ��Ƥ��롣

    <ul>

    <li> Window �����ƥ�ե����

    m17n X �饤�֥��ϡ�X �����Ф� X �ե���ȥ����Ф���갷�����ƤΥե���Ȥ򥵥ݡ��Ȥ��롣
    XLFD �γƥե�����ɤȥե���ȥץ�ѥƥ����б��ϰʲ����̤ꡣ����ɽ�ˤʤ��ե�����ɤ�̵�뤵��롣

@verbatim
    XLFD �ե������                             �ץ�ѥƥ�
    ---------------                             --------
    FOUNDRY                                     foundry
    FAMILY_NAME                                 family
    WEIGHT_NAME                                 weight
    SLANT                                       style
    SETWIDTH_NAME                               stretch
    ADD_STYLE_NAME                              adstyle
    PIXEL_SIZE                                  size
    RESOLUTION_Y                                resolution
    CHARSET_REGISTRY-CHARSET_ENCODING           registry
@endverbatim

    <li> FreeType fonts

    m17n �饤�֥��ϡ�FreeType �饤�֥���Ȥ��褦�����ꤵ�줿���ˤϡ�
    FreeType ���������٤ƤΥե���Ȥ򥵥ݡ��Ȥ��롣�ѿ� 
    #mfont_freetype_path �� m17n �饤�֥�������ȴĶ��ѿ� @c M17NDIR 
    �˱����ƽ��������롣�ܺ٤��ѿ��������򻲾ȤΤ��ȡ�

    �⤷ m17n �饤�֥�꤬ fontconfig �饤�֥���Ȥ��褦�����ꤵ�줿���ˤϡ�
    #mfont_freetype_path �˲ä��ơ� fontconfig �ǻ��Ѳ�ǽ�ʥե���Ȥ⤹�٤ƥ��ݡ��Ȥ���롣

    FreeType �ե���ȤΥե��ߥ�̾�� family �ץ�ѥƥ����б����롣
    FreeType �ե���ȤΥ�������̾�ϡ�����ɽ�Τ褦�� weight, style,
    stretch �ץ�ѥƥ����б����롣

@verbatim
    ��������̾          weight  style   stretch
    ----------          ------  -----   -------
    Regular             medium  r       normal
    Italic              medium  i       normal
    Bold                bold    r       normal
    Bold Italic         bold    i       normal
    Narrow              medium  r       condensed
    Narrow Italic       medium  i       condensed
    Narrow Bold         bold    r       condensed
    Narrow Bold Italic  bold    i       condensed
    Black               black   r       normal
    Black Italic        black   i       normal
    Oblique             medium  o       normal
    BoldOblique         bold    o       normal
@endverbatim

    ���ɽ�˸����ʤ���������̾�� "Regular" �Ȥ��ư����롣

    platform ID �� encoding ID ���Ȥ߹�碌�� registry 
    �ץ�ѥƥ����б����롣���Ȥ��Ф���ե���Ȥ� (1 1) �Ȥ��� ID ���ȹ礻����ƤС�
    registry �ץ�ѥƥ��� 1-1 �Ȥʤ롣���ˤˤ�������ȹ礻�ˤϰʲ��Τ褦������Ѥ�
    registry �ץ�ѥƥ� ��Ϳ�����Ƥ��롣

@verbatim
    platform ID         encoding ID     registry �ץ�ѥƥ�
    -----------         -----------     -----------------
    0                   3               unicode-bmp
    0                   4               unicode-full
    1                   0               apple-roman
    3                   1               unicode-bmp
    3                   1               unicode-full
@endverbatim

    �������äơ���Ĥ��ȹ礻 (1 0) ��(3 1) ����ĥե���Ȥϡ����줾��
    registry �ץ�ѥƥ��� 1-0, apple-roman, 3-1, unicode-bmp
    �Ǥ��룴�ĤΥե���ȥ��֥������Ȥ��б����롣

    <li> OpenType �ե����

    m17n �饤�֥��ϡ�FreeType �饤�֥��� OTF 
    �饤�֥�����Ѥ���褦�����ꤹ��С����٤Ƥ� OpenType 
    �ե���Ȥ򥵥ݡ��Ȥ��롣�ºݤ����ѤǤ���ե���ȤΥꥹ�Ȥ� FreeType
    �ե���Ȥξ���Ʊ�ͤ˺���롣OpenType �ե���Ȥ� FLT (Font Layout Table)
    ��ͳ�ǻ��Ѥ���褦�ե���ȥ��åȤ˻��ꤵ��Ƥ��ꡢFLT �� OTF 
    ��Ϣ�Υ��ޥ�� (���Ȥ��� otf:deva) ������С�OTF �饤�֥�꤬�ե���Ȥ� OpenType
    �쥤�����ȥơ��֥�˽��ä�ʸ����򥰥�ե���������Ѵ�����FreeType
    �饤�֥�꤬�ƥ���դΥӥåȥޥåץ��᡼�����󶡤��롣

    </ul>

*/

/*=*/

#if !defined (FOR_DOXYGEN) || defined (DOXYGEN_INTERNAL_MODULE)
/*** @addtogroup m17nInternal
     @{ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "m17n-gui.h"
#include "m17n-misc.h"
#include "internal.h"
#include "mtext.h"
#include "symbol.h"
#include "plist.h"
#include "charset.h"
#include "language.h"
#include "internal-gui.h"
#include "font.h"
#include "face.h"
#include "fontset.h"

MPlist *mfont__driver_list;

static MSymbol M_font_capability, M_font_list, M_font_list_len;

/** Indices to font properties sorted by their priority.  */
static int font_score_priority[] =
  { MFONT_SIZE,
    MFONT_WEIGHT,
    MFONT_STYLE,
    MFONT_STRETCH,
    MFONT_FAMILY,
    MFONT_ADSTYLE,
    MFONT_FOUNDRY
  };

#define FONT_SCORE_PRIORITY_SIZE	\
  (sizeof font_score_priority / sizeof font_score_priority[0])

/* Indexed by a font property MFONT_XXX, and the value is how many
   bits to shift the difference of property values.  */
static int font_score_shift_bits[MFONT_SIZE + 1];

/** Predefined symbols for each font property.  The order is important
    because the function font_score () decides how well a font matches
    with a spec by checking how close the index is.  */

static char *common_foundry[] =
  { "misc",
    "adobe" };
static char *common_family[] =
  { "fixed",
    "courier",
    "helvetica",
    "times" };
static char *common_weight[] =
  { "thin"
    "ultralight",
    "extralight",
    "light",
    "demilight",
    "book",
    "regular",
    "normal",
    "medium",
    "demibold",
    "semibold",
    "bold",
    "extrabold",
    "ultrabold",
    "black",
    "heavy" };
static char *common_style[] =
  { "o",
    "i",
    "slanted",
    "r",
    "rslanted",
    "ri",
    "ro" };
static char *common_stretch[] =
  { "ultracondensed",
    "extracondensed",
    "condensed",
    "narrow",
    "semicondensed",
    "normal",
    "semiexpanded",
    "expanded",
    "extraexpanded",
    "ultraexpanded" };
static char *common_adstyle[] =
  { "serif",
    "",
    "sans" };
static char *common_registry[] =
  { "iso8859-1" };

static unsigned short font_weight_regular;
static unsigned short font_weight_normal;
static unsigned short font_weight_medium;

/* Table containing all the data above.  */

struct MFontCommonNames
{
  int num;
  char **names;
};

static struct MFontCommonNames font_common_names[] =
  {
    { sizeof (common_foundry) / sizeof (char *), common_foundry},
    { sizeof (common_family) / sizeof (char *), common_family},
    { sizeof (common_weight) / sizeof (char *), common_weight},
    { sizeof (common_style) / sizeof (char *), common_style},
    { sizeof (common_stretch) / sizeof (char *), common_stretch},
    { sizeof (common_adstyle) / sizeof (char *), common_adstyle},
    { sizeof (common_registry) / sizeof (char *), common_registry}
  };


/** Table of available font property names.  */

MFontPropertyTable mfont__property_table[MFONT_REGISTRY + 1];


/** Return the numeric value of SYMBOL as the Nth font property.  */

#define FONT_PROPERTY_NUMERIC(symbol, n)	\
  ((symbol) == Mnil				\
   ? 0						\
   : ((int) msymbol_get ((symbol), mfont__property_table[(n)].property)))


/** Set the numeric value of SYMBOL as the Nth font property to NUMERIC.  */

#define SET_FONT_PROPERTY_NUMERIC(symbol, n, numeric)		\
  msymbol_put((symbol), mfont__property_table[(n)].property,	\
	      (void *) (numeric))


/* Font selector.  */

struct MFontEncoding {
  MFont spec;
  MSymbol encoding_name;
  MCharset *encoding_charset;
  MSymbol repertory_name;
  MCharset *repertory_charset;
};

static MPlist *font_encoding_list;
static MFontEncoding default_encoding;

/** Load font encoding table from the data <font encoding>.
    The data has this form:
	(FONT-SPEC ENCODING) ...
    where FONT-SPEC has this form:
	([FOUNDRY FAMILY [WEIGHT [STYLE [STRETCH [ADSTYLE]]]]] REGISTRY)
    All elements are symbols.  */

static int
load_font_encoding_table ()
{
  MDatabase *mdb;
  MPlist *encoding_list, *plist, *pl, *elt;

  font_encoding_list = pl = mplist ();

  mdb = mdatabase_find (Mfont, msymbol ("encoding"), Mnil, Mnil);
  if (! mdb
      || ! (encoding_list = (MPlist *) mdatabase_load (mdb)))
    MERROR (MERROR_FONT, -1);

  MPLIST_DO (plist, encoding_list)
    {
      MFontEncoding *encoding;
      MSymbol registry;

      MSTRUCT_CALLOC (encoding, MERROR_FONT);

      if (! MPLIST_PLIST_P (plist)
	  || (elt = MPLIST_PLIST (plist), mplist_length (elt) < 2)
	  || ! MPLIST_PLIST_P (elt))
	MWARNING (MERROR_FONT);
      registry = mfont__set_spec_from_plist (&encoding->spec,
					     MPLIST_PLIST (elt));
      elt = MPLIST_NEXT (elt);
      if (! MPLIST_SYMBOL_P (elt))
	MWARNING (MERROR_FONT);
      encoding->encoding_name = MPLIST_SYMBOL (elt);
      elt = MPLIST_NEXT (elt);
      if (MPLIST_TAIL_P (elt))
	encoding->repertory_name = encoding->encoding_name;
      else if (! MPLIST_SYMBOL_P (elt))
	MWARNING (MERROR_FONT);
      else
	encoding->repertory_name = MPLIST_SYMBOL (elt);

      if (registry == Mnil)
	mplist_push (font_encoding_list, Mt, encoding);
      else
	pl = mplist_add (pl, registry, encoding);
      continue;

    warning:
      free (encoding);
    }

  M17N_OBJECT_UNREF (encoding_list);
  return 0;
}

typedef struct {
  MFont spec;
  int resize;
} MFontResize;

static MPlist *font_resize_list;

/** Load font size table from the data <font size>.
    The data has this form:
	(FONT-SPEC RESIZE-FACTOR) ...
    where FONT-SPEC has this form:
	([FOUNDRY FAMILY [WEIGHT [STYLE [STRETCH [ADSTYLE]]]]] REGISTRY)
    All elements of FONT-SPEC are symbols.  */

static int
load_font_resize_table ()
{
  MDatabase *mdb;
  MPlist *size_adjust_list, *plist, *pl, *elt;

  font_resize_list = pl = mplist ();

  mdb = mdatabase_find (Mfont, msymbol ("resize"), Mnil, Mnil);
  if (! mdb)
    return -1;
  if (! (size_adjust_list = (MPlist *) mdatabase_load (mdb)))
    MERROR (MERROR_FONT, -1);

  MPLIST_DO (plist, size_adjust_list)
    {
      MFontResize *resize;
      MSymbol registry;

      MSTRUCT_CALLOC (resize, MERROR_FONT);

      if (! MPLIST_PLIST_P (plist)
	  || (elt = MPLIST_PLIST (plist), mplist_length (elt) != 2)
	  || ! MPLIST_PLIST_P (elt))
	MWARNING (MERROR_FONT);
      registry = mfont__set_spec_from_plist (&resize->spec,
					     MPLIST_PLIST (elt));
      elt = MPLIST_NEXT (elt);
      if (! MPLIST_INTEGER_P (elt))
	MWARNING (MERROR_FONT);
      resize->resize = MPLIST_INTEGER (elt);

      if (registry == Mnil)
	registry = Mt;
      pl = mplist_add (pl, registry, resize);
      continue;

    warning:
      free (resize);
    }

  M17N_OBJECT_UNREF (size_adjust_list);
  return 0;
}

/** Return a font encoding (and repertory) of FONT.  */

static MFontEncoding *
find_encoding (MFont *font)
{
  MSymbol registry = FONT_PROPERTY (font, MFONT_REGISTRY);
  MFontEncoding *encoding = NULL;
  MPlist *plist;

  if (! font_encoding_list)
    load_font_encoding_table ();
  plist = font_encoding_list;
  while (! MPLIST_TAIL_P (plist))
    {
      encoding = (MFontEncoding *) MPLIST_VAL (plist);
      if (mfont__match_p (font, &encoding->spec, MFONT_REGISTRY))
	{
	  if (encoding->encoding_name != Mnil
	      && ! encoding->encoding_charset)
	    {
	      encoding->encoding_charset = MCHARSET (encoding->encoding_name);
	      if (! encoding->encoding_charset)
		{
		  mplist_pop (plist);
		  continue;
		}
	    }
	  if (encoding->repertory_name == encoding->encoding_name)
	    encoding->repertory_charset = encoding->encoding_charset;
	  else if (encoding->repertory_name != Mnil)
	    {
	      encoding->repertory_charset
		= MCHARSET (encoding->repertory_name);
	      if (! encoding->repertory_charset)
		{
		  mplist_pop (plist);
		  continue;
		}
	    }
	  font->encoding = encoding;
	  return encoding;
	}

      if (registry && MPLIST_KEY (plist) != Mt)
	{
	  plist = mplist_find_by_key (plist, registry);
	  if (! plist)
	    break;
	}
      else
	plist = MPLIST_NEXT (plist);
    }
  font->encoding = &default_encoding;
  return &default_encoding;
}

#ifndef HAVE_OTF
static OTF_Tag
OTF_tag (char *name)
{
  unsigned char *p = (unsigned char *) name;

  if (! name)
    return (OTF_Tag) 0;
  return (OTF_Tag) ((p[0] << 24)
		    | (! p[1] ? 0
		       : ((p[1] << 16)
			  | (! p[2] ? 0
			     : (p[2] << 8) | p[3]))));
}

void
OTF_tag_name (OTF_Tag tag, char *name)
{
  name[0] = (char) (tag >> 24);
  name[1] = (char) ((tag >> 16) & 0xFF);
  name[2] = (char) ((tag >> 8) & 0xFF);
  name[3] = (char) (tag & 0xFF);
  name[4] = '\0';
}
#endif	/* not HAVE_OTF */

/* XLFD parser/generator */

/** Indices to each field of split font name.  */

enum xlfd_field_idx
  {
    XLFD_FOUNDRY,
    XLFD_FAMILY,
    XLFD_WEIGHT,
    XLFD_SLANT,
    XLFD_SWIDTH,
    XLFD_ADSTYLE,
    XLFD_PIXEL,
    XLFD_POINT,
    XLFD_RESX,
    XLFD_RESY,
    XLFD_SPACING,
    XLFD_AVGWIDTH,
    XLFD_REGISTRY,		/* This contains ENCODING.  */
    /* anchor */
    XLFD_FIELD_MAX
  };

static int
xlfd_parse_name (const char *name, MFont *font)
{
  char *field[XLFD_FIELD_MAX];
  unsigned short resy, avgwidth;
  unsigned size;
  char copy[513];
  int i;
  char *p;
  MSymbol sym;

  if (name[0] != '-')
    return -1;

  field[0] = copy;
  for (i = 1, p = copy, name++; *name; p++, name++)
    {
      if (p - copy > 512)
	return -1;
      if (*name == '-'
	  && i < XLFD_FIELD_MAX)
	{
	  *p = '\0';
	  if (field[i - 1][0] == '*')
	    field[i - 1] = NULL;
	  field[i++] = p + 1;
	}
      else
	*p = tolower (*name);
    }
  *p = '\0';
  if (field[i - 1][0] == '*')
    field[i - 1] = NULL;
  while (i < XLFD_FIELD_MAX)
    field[i++] = NULL;

  resy = field[XLFD_RESY] ? atoi (field[XLFD_RESY]) : 0;
  avgwidth = ((field[XLFD_AVGWIDTH] && isdigit (field[XLFD_AVGWIDTH][0]))
	      ? atoi (field[XLFD_AVGWIDTH]) : 1);
  if (! avgwidth)
    size = 0;
  else if (! field[XLFD_PIXEL])
    size = field[XLFD_POINT] ? atoi (field[XLFD_POINT]) * resy / 72 : 0;
  else if (field[XLFD_PIXEL][0] == '[')
    {
      /* The pixel size field specifies a transformation matrix of the
	 form "[A B C D]".  The XLFD spec says that the scalar value N
	 for the pixel size is equivalent to D.  */
      char *p0 = field[XLFD_PIXEL] + 1, *p1;
      double d;

      for (i = 0; i < 4; i++, p0 = p1)
	d = strtod (p0, &p1);
      size = d * 10;
    }
  else
    size = atoi (field[XLFD_PIXEL]) * 10;

  if (field[XLFD_FOUNDRY])
    {
      sym = msymbol (field[XLFD_FOUNDRY]);
      if (! sym)
	sym = msymbol ("Nil");
      mfont__set_property (font, MFONT_FOUNDRY, sym);
    }
  if (field[XLFD_FAMILY])
    {
      sym = msymbol (field[XLFD_FAMILY]);
      if (! sym)
	sym = msymbol ("Nil");
      mfont__set_property (font, MFONT_FAMILY, sym);
    }
  if (field[XLFD_WEIGHT])
    mfont__set_property (font, MFONT_WEIGHT, msymbol (field[XLFD_WEIGHT]));
  if (field[XLFD_SLANT])
    mfont__set_property (font, MFONT_STYLE, msymbol (field[XLFD_SLANT]));
  if (field[XLFD_SWIDTH])
    mfont__set_property (font, MFONT_STRETCH, msymbol (field[XLFD_SWIDTH]));
  if (field[XLFD_ADSTYLE])
    mfont__set_property (font, MFONT_ADSTYLE, msymbol (field[XLFD_ADSTYLE]));
  font->property[MFONT_RESY] = resy;
  font->size = size;
  if (field[XLFD_SPACING])
    font->spacing
      = ((field[XLFD_SPACING][0] == 'p' || field[XLFD_SPACING][0] == 'P')
	 ? MFONT_SPACING_PROPORTIONAL
	 : (field[XLFD_SPACING][0] == 'm' || field[XLFD_SPACING][0] == 'M')
	 ? MFONT_SPACING_MONO : MFONT_SPACING_CHARCELL);
  if (field[XLFD_REGISTRY])
    mfont__set_property (font, MFONT_REGISTRY, msymbol (field[XLFD_REGISTRY]));
  font->type = MFONT_TYPE_SPEC;
  font->source = MFONT_SOURCE_X;
  return 0;
}

static char *
xlfd_unparse_name (MFont *font, int full_xlfd)
{
  MSymbol prop[7];
  char name[513];
  char *str[7];
  int len, i;
  char spacing;
  int size, resy;
  int all_nil = 1;

  prop[0] = (MSymbol) mfont_get_prop (font, Mfoundry);
  prop[1] = (MSymbol) mfont_get_prop (font, Mfamily);
  prop[2] = (MSymbol) mfont_get_prop (font, Mweight);
  prop[3] = (MSymbol) mfont_get_prop (font, Mstyle);
  prop[4] = (MSymbol) mfont_get_prop (font, Mstretch);
  prop[5] = (MSymbol) mfont_get_prop (font, Madstyle);
  prop[6] = (MSymbol) mfont_get_prop (font, Mregistry);
  for (len = 0, i = 0; i < 7; i++)
    {
      if (prop[i] != Mnil)
	{
	  str[i] = msymbol_name (prop[i]);
	  len += strlen (str[i]);
	  all_nil = 0;
	}
      else
	{
	  str[i] = "*";
	  len++;
	}
    }
  spacing = (font->spacing == MFONT_SPACING_UNDECIDED ? '*'
	     : font->spacing == MFONT_SPACING_PROPORTIONAL ? 'p'
	     : font->spacing == MFONT_SPACING_MONO ? 'm'
	     : 'c');

  if ((len
       + 13			/* 13 dashes */
       + 2			/* 2 asterisks */
       + 30			/* 3 integers (each 10 digits) */
       + 1			/* 1 spacing char */
       + 1)			/* '\0' terminal */
      > 513)
    return NULL;

  resy = (int) mfont_get_prop (font, Mresolution);
  size = font->size;
  if (size >= 0)
    {
      if (font->multiple_sizes)
	{
	  for (size = 0; size < 24; size++)
	    if (font->size & (1 << size))
	      break;
	  size += 6;
	}
      else if ((size % 10) < 5)
	size /= 10;
      else
	size = size / 10 + 1;
    }
  else
    size = - size;

  if (full_xlfd)
    {
      if (font->size >= 0)
	sprintf (name, "-%s-%s-%s-%s-%s-%s-%d-*-%d-%d-%c-*-%s",
		 str[0], str[1], str[2], str[3], str[4], str[5],
		 size, resy, resy, spacing, str[6]);
      else
	sprintf (name, "-%s-%s-%s-%s-%s-%s-*-%d-%d-%d-%c-*-%s",
		 str[0], str[1], str[2], str[3], str[4], str[5],
		 size, resy, resy, spacing, str[6]);
    }
  else if (all_nil && size == 0)
    sprintf (name, "*");
  else
    {
      char *p = name;

      p += sprintf (p, "-%s", str[0]);
      for (i = 1; i < 6; i++)
	if (p[-1] != '*' || str[i][0] != '*')
	  p += sprintf (p, "-%s", str[i]);
      if (p[-1] != '*' || font->size > 0)
	{
	  if (font->size > 0)
	    p += sprintf (p, "-%d-*", size);
	  else
	    p += sprintf (p, "-*");
	}
      if (str[6][0] != '*')
	sprintf (p, "-%s", str[6]);
    }

  return strdup (name);
}

/* Compare FONT with REQUEST and return how much they differs.  */

static int
font_score (MFont *font, MFont *request)
{
  int score = 0;
  int i = FONT_SCORE_PRIORITY_SIZE;

  while (--i >= 0)
    {
      enum MFontProperty prop = font_score_priority[i];
      int val;

      if (prop == MFONT_SIZE)
	{
	  if (font->size && request->size)
	    {
	      if (font->multiple_sizes)
		{
		  int j, closest = 23;

		  for (j = 23; j >= 0; j--)
		    if (font->size & (1 << j))
		      {
			closest = j;
			if (request->size >= (j + 6) * 10)
			  break;
		      }
		  val = request->size - (closest + 6) * 10;
		}
	      else
		val = font->size - request->size;
	      if (val)
		{
		  if (val < 0)
		    val = - val;
		  if (val >= 0x10000)
		    val = 0xFFFF;
		  score |= (val << font_score_shift_bits[MFONT_SIZE]);
		}
	    }
	}
      else if (font->property[prop] && request->property[prop]
	       && font->property[prop] != request->property[prop])
	{
	  if (prop <= MFONT_FAMILY)
	    val = 1;
	  else if (prop == MFONT_WEIGHT)
	    {
	      unsigned short v1 = font->property[prop];
	      unsigned short v2 = request->property[prop];

	      if (v1 == font_weight_regular || v1 == font_weight_normal)
		v1 = font_weight_medium;
	      if (v2 == font_weight_regular || v2 == font_weight_normal)
		v2 = font_weight_medium;
	      val = v1 > v2 ? v1 - v2 : v2 - v1;
	    }
	  else
	    {
	      val = font->property[prop] - request->property[prop];
	      if (val < 0)
		val = - val;
	      if (val > 3)
		val = 3;
	    }
	  score |= val << font_score_shift_bits[prop];
	}
    }
  if (request->file != Mnil && request->file != font->file)
    score |= 40000000;
  return score;
}

static MSymbol
merge_capability (MSymbol capability, MSymbol key, MSymbol val, int overwrite)
{
  MFontCapability *cap = NULL;
  char *lang = NULL, *script = NULL, *otf = NULL, *buf, *p;
  int lang_len = 0, script_len = 0, otf_len = 0;

  if (key == Mlanguage)
    lang = MSYMBOL_NAME (val), lang_len = MSYMBOL_NAMELEN (val) + 6;
  else if (key == Mscript)
    script = MSYMBOL_NAME (val), script_len = MSYMBOL_NAMELEN (val) + 7;
  else if (key == Motf)
    otf = MSYMBOL_NAME (val), otf_len = MSYMBOL_NAMELEN (val) + 5;
  else
    return capability;

  if (capability != Mnil)
    {
      cap = mfont__get_capability (capability);
      if (! overwrite)
	{
	  if (cap->language)
	    lang = NULL;
	  if (cap->script)
	    script = NULL;
	  if (cap->script_tag)
	    otf = NULL;
	  if (! lang && !script && !otf)
	    return capability;
	}
    }

  if (! lang && cap && cap->language)
    {
      lang_len = MSYMBOL_NAMELEN (cap->language);
      lang = MSYMBOL_NAME (cap->language);
    }
  if (! script && cap && cap->script != Mnil)
    {
      script_len = MSYMBOL_NAMELEN (cap->script);
      script = MSYMBOL_NAME (cap->script);
    }
  if (! otf && cap && cap->script_tag)
    {
      int i;

      otf_len = 4;			/* for script_tag */
      if (cap->langsys_tag)
	otf_len += 5;		/* for "/XXXX */
      for (i = 0; i < MFONT_OTT_MAX; i++)
	if (cap->features[i].str)
	  otf_len += strlen (cap->features[i].str) + 1; /* for "[=+]..." */
      otf = p = alloca (otf_len + 1);
      OTF_tag_name (cap->script_tag, otf);
      p += 4;
      if (cap->langsys_tag)
	{
	  *p++ = '/';
	  OTF_tag_name (cap->langsys_tag, p);
	  p += 4;
	}
      if (cap->features[MFONT_OTT_GSUB].str)
	p += sprintf (p, "=%s", cap->features[MFONT_OTT_GSUB].str);
      if (cap->features[MFONT_OTT_GPOS].str)
	p += sprintf (p, "=%s", cap->features[MFONT_OTT_GSUB].str);
    }
  buf = p = alloca (lang_len + script_len + otf_len + 1);
  if (lang_len)
    p += sprintf (p, ":lang=%s", lang);
  if (script_len)
    p += sprintf (p, ":script=%s", script);
  if (otf_len)
    p += sprintf (p, ":otf=%s", otf);
  return msymbol (buf);
}


/* Internal API */

MSymbol Miso8859_1, Miso10646_1, Municode_bmp, Municode_full, Mapple_roman;

int
mfont__init ()
{
  int i, shift;
  MSymbol regular = msymbol ("regular");
  MSymbol normal = msymbol ("normal");
  MSymbol medium = msymbol ("medium");

  M_font_capability = msymbol_as_managing_key ("  font-capability");
  M_font_list = msymbol_as_managing_key ("  font-list");
  M_font_list_len = msymbol ("  font-list-len");

  Mfoundry = msymbol ("foundry");
  mfont__property_table[MFONT_FOUNDRY].property = Mfoundry;
  Mfamily = msymbol ("family");
  mfont__property_table[MFONT_FAMILY].property = Mfamily;
  Mweight = msymbol ("weight");
  mfont__property_table[MFONT_WEIGHT].property = Mweight;
  Mstyle = msymbol ("style");
  mfont__property_table[MFONT_STYLE].property = Mstyle;
  Mstretch = msymbol ("stretch");
  mfont__property_table[MFONT_STRETCH].property = Mstretch;
  Madstyle = msymbol ("adstyle");
  mfont__property_table[MFONT_ADSTYLE].property = Madstyle;
  Mregistry = msymbol ("registry");
  mfont__property_table[MFONT_REGISTRY].property = Mregistry;

  Mspacing = msymbol ("spacing");
  Msize = msymbol ("size");
  Mresolution = msymbol ("resolution");
  Mmax_advance = msymbol ("max-advance");
  Mfontfile = msymbol ("fontfile");

  Mfontconfig = msymbol ("fontconfig");

  Mx = msymbol ("x");
  Mfreetype = msymbol ("freetype");
  Mxft = msymbol ("xft");

  Miso8859_1 = msymbol ("iso8859-1");
  Miso10646_1 = msymbol ("iso10646-1");
  Municode_bmp = msymbol ("unicode-bmp");
  Municode_full = msymbol ("unicode-full");
  Mapple_roman = msymbol ("apple-roman");

  Motf = msymbol ("otf");

  /* The first entry of each mfont__property_table must be Mnil so
     that actual properties get positive numeric numbers.  */
  for (i = 0; i <= MFONT_REGISTRY; i++)
    {
      MLIST_INIT1 (&mfont__property_table[i], names, 8);
      MLIST_APPEND1 (&mfont__property_table[i], names, Mnil, MERROR_FONT);
    }

  /* Register predefined font property names.  */
  for (i = 0; i <= MFONT_REGISTRY; i++)
    {
      int j;

      for (j = 0; j < font_common_names[i].num; j++)
	{
	  MSymbol sym = msymbol (font_common_names[i].names[j]);

	  if (sym == Mnil)
	    return -1;
	  if (msymbol_put (sym, mfont__property_table[i].property,
			   (void *) (j + 1)) < 0)
	    return -1;
	  MLIST_APPEND1 (&mfont__property_table[i], names, sym,
			 MERROR_FONT);
	  if (i == MFONT_WEIGHT)
	    {
	      if (sym == regular)
		font_weight_regular = j + 1;
	      else if (sym == normal)
		font_weight_normal = j + 1;
	      else if (sym == medium)
		font_weight_medium = j + 1;
	    }
	}
    }

  /* Here, SHIFT starts from 1, not 0.  This is because the lowest bit
     of a score is a flag for a scalable font (see the documentation
     of font_score).  */
  i = FONT_SCORE_PRIORITY_SIZE - 1;
  for (shift = 1; i >= 0; i--)
    {
      font_score_shift_bits[font_score_priority[i]] = shift;
      if (font_score_priority[i] == MFONT_SIZE)
	shift += 16;
      else if (font_score_priority[i] <= MFONT_FAMILY)
	shift++;
      else
	shift += 2;
    }

  MFONT_INIT (&default_encoding.spec);
  default_encoding.encoding_name = Municode_full;
  default_encoding.encoding_charset = mcharset__unicode;
  default_encoding.repertory_name = Mnil;
  default_encoding.repertory_charset = NULL;
  {
    char *path, *buf = NULL;
    int bufsize;
    USE_SAFE_ALLOCA;

    mfont_freetype_path = mplist ();
    bufsize = strlen (M17NDIR) + 7;
    SAFE_ALLOCA (buf, bufsize);
    sprintf (buf, "%s/fonts", M17NDIR);
    mplist_add (mfont_freetype_path, Mstring, strdup (buf));
    path = getenv ("M17NDIR");
    if (path)
      {
	bufsize = strlen (path) + 7;
	SAFE_ALLOCA (buf, bufsize);
	sprintf (buf, "%s/fonts", path);
	mplist_push (mfont_freetype_path, Mstring, strdup (buf));
      }
    SAFE_FREE (buf);
  }

#ifdef HAVE_FREETYPE
  if (mfont__ft_init () < 0)
    return -1;
#endif /* HAVE_FREETYPE */

  return 0;
}

void
mfont__fini ()
{
  MPlist *plist;
  int i;

#ifdef HAVE_FREETYPE
  mfont__ft_fini ();
#endif /* HAVE_FREETYPE */

  MPLIST_DO (plist, mfont_freetype_path)
    free (MPLIST_VAL (plist));
  M17N_OBJECT_UNREF (mfont_freetype_path);

  if (font_resize_list)
    {
      MPLIST_DO (plist, font_resize_list)
	free (MPLIST_VAL (plist));
      M17N_OBJECT_UNREF (font_resize_list);
      font_resize_list = NULL;
    }
  if (font_encoding_list)
    {
      MPLIST_DO (plist, font_encoding_list)
	free (MPLIST_VAL (plist));
      M17N_OBJECT_UNREF (font_encoding_list);
      font_encoding_list = NULL;
    }

  for (i = 0; i <= MFONT_REGISTRY; i++)
    MLIST_FREE1 (&mfont__property_table[i], names);
}


MSymbol
mfont__id (MFont *font)
{
  char *buf = NULL, *p;
  int i;
  int file_len = (font->file == Mnil ? 0 : MSYMBOL_NAMELEN (font->file));
  int capability_len  = (font->capability == Mnil ? 0
			 : MSYMBOL_NAMELEN (font->capability));
  int total_len = MFONT_PROPERTY_MAX * 5 + 9 + file_len + capability_len;
  MSymbol id;
  USE_SAFE_ALLOCA;

  SAFE_ALLOCA (buf, total_len);
  p = buf;
  if (font->property[0])
    p += sprintf (p, "%X", font->property[0]);
  for (i = 1; i < MFONT_PROPERTY_MAX; i++)
    {
      if (font->property[i])
	p += sprintf (p, "-%X", font->property[i]);
      else
	*p++ = '-';
    }
  if (font->size)
    p += sprintf (p, "-%X", font->size);
  if (font->spacing)
    p += sprintf (p, "-%X", font->spacing);
  if (capability_len > 0)
    {
      *p++ = '-';
      memcpy (p, MSYMBOL_NAME (font->capability), capability_len);
      p += capability_len;
    }
  if (file_len > 0)
    {
      *p++ = '-';
      memcpy (p, MSYMBOL_NAME (font->file), file_len);
      p += file_len;
    }      
  id = msymbol__with_len (buf, p - buf);
  SAFE_FREE (buf);
  return id;
}

/** Return 1 iff FONT matches SPEC.  */

int
mfont__match_p (MFont *font, MFont *spec, int prop)
{
  if (spec->capability != font->capability
      && spec->capability != Mnil)
    {
      MRealizedFont *rfont;

      if (font->type != MFONT_TYPE_REALIZED)
	return (font->capability == Mnil);
      rfont = (MRealizedFont *) font;
      return (rfont->driver->check_capability
	      && (rfont->driver->check_capability (rfont, spec->capability)
		  >= 0));
    }
  if (spec->file != font->file
      && spec->file != Mnil && font->file != Mnil)
    return 0;
  for (; prop >= 0; prop--)
    if (spec->property[prop] && font->property[prop]
	&& font->property[prop] != spec->property[prop])
      return 0;
  return 1;
}

/* Merge SRC into DST.  If error_on_conflict is nonzero and a font
   property differs in SRC and DST, return -1.  */

int
mfont__merge (MFont *dst, MFont *src, int error_on_conflict)
{
  int i;

  for (i = 0; i < MFONT_PROPERTY_MAX; i++)
    {
      if (! dst->property[i])
	dst->property[i] = src->property[i];
      else if (error_on_conflict
	       && src->property[i]
	       && dst->property[i] != src->property[i])
	return -1;
    }
  if (! dst->spacing)
    dst->spacing = src->spacing;
  else if (error_on_conflict
	   && src->spacing
	   && dst->spacing != src->spacing)
    return -1;
  if (! dst->size)
    dst->size = src->size;
  else if (error_on_conflict
	   && src->size
	   && dst->size != src->size)
    return -1;
  if (dst->capability == Mnil)
    dst->capability = src->capability;
  else if (error_on_conflict
	   && src->capability
	   && dst->capability != src->capability)
    return -1;
  if (dst->file == Mnil)
    dst->file = src->file;
  else if (error_on_conflict
	   && src->file
	   && dst->file != src->file)
    return -1;
  return 0;
}

void
mfont__set_spec_from_face (MFont *spec, MFace *face)
{
  int i;

  for (i = 0; i <= MFONT_ADSTYLE; i++)
    mfont__set_property (spec, i, face->property[i]);
  spec->property[MFONT_REGISTRY] = 0;
  spec->property[MFONT_RESY] = 0;
  spec->multiple_sizes = 0;
  spec->size = (int) (face->property[MFACE_SIZE]);
  spec->type = MFONT_TYPE_SPEC;
  spec->source = MFONT_SOURCE_UNDECIDED;
  spec->file = spec->capability = Mnil;
  spec->encoding = NULL;
}


extern MSymbol
mfont__set_spec_from_plist (MFont *spec, MPlist *plist)
{
  int i;
  MSymbol spec_list[MFONT_REGISTRY + 1];
  MSymbol registry;
  char *reg;

  MFONT_INIT (spec);
  memset (spec_list, 0, sizeof spec_list);
  for (i = 0; ! MPLIST_TAIL_P (plist); i++, plist = MPLIST_NEXT (plist))
    {
      if (! MPLIST_SYMBOL_P (plist))
	MERROR (MERROR_FONT, Mnil);
      spec_list[i] = MPLIST_SYMBOL (plist);
    }
  if (i == 0)
    MERROR (MERROR_FONT, Mnil);
  registry = spec_list[i - 1];
  if (i > 1 && registry != Mnil)
    {
      reg = MSYMBOL_NAME (registry);
      if (reg[0] == ':')
	{
	  mfont__get_capability (registry);
	  spec->capability = registry;
	  registry = spec_list[i - 2];
	  i--;
	}
    }
  mfont__set_property (spec, MFONT_REGISTRY, registry);
  for (i -= 2; i >= 0; i--)
    mfont__set_property (spec, i, spec_list[i]);
  spec->type = MFONT_TYPE_SPEC;

  return registry;
}


MFont *
mfont__select (MFrame *frame, MFont *font, int max_size)
{
  MFontDriver *driver;

  if (font->type == MFONT_TYPE_FAILURE)
    return NULL;
  if (font->type != MFONT_TYPE_SPEC)
    return font;
  if (font->source == MFONT_SOURCE_UNDECIDED)
    {
      if (font->file != Mnil || font->capability != Mnil)
	font->source = MFONT_SOURCE_FT;
      else if (font->property[MFONT_REGISTRY])
	{
	  MSymbol registry = FONT_PROPERTY (font, MFONT_REGISTRY);
	  char *reg = MSYMBOL_NAME (registry);

	  if (strncmp (reg, "unicode-", 8) == 0
	      || strncmp (reg, "apple-roman", 11) == 0
	      || (reg[0] >= '0' && reg[0] <= '9' && reg[1] == '-'))
	    font->source = MFONT_SOURCE_FT;
	}
    }
  if (font->source != MFONT_SOURCE_FT)
    {
      driver = mplist_get (frame->font_driver_list, Mx);
      if (driver)
	return (driver->select) (frame, font, max_size);
    }
  driver = mplist_get (frame->font_driver_list, Mfreetype);
  if (! driver)
    return NULL;
  return (driver->select) (frame, font, max_size);
}


int
mfont__available (MFrame *frame, MFont *font)
{
  return -1;
}

static int
compare_font_score (const void *e1, const void *e2)
{
  MFontScore *s1 = (MFontScore *) e1, *s2 = (MFontScore *) e2;

  return (s1->font->for_full_width == s2->font->for_full_width
	  ? s1->score > s2->score
	  : s1->font->for_full_width);
}

void
mdebug_dump_font_list (MFontList *font_list)
{
  int i;

  for (i = 0; i < font_list->nfonts; i++)
    {
      fprintf (mdebug__output, "%04X - ", font_list->fonts[i].score);
      mdebug_dump_font (font_list->fonts[i].font);
      fprintf (mdebug__output, "\n");
    }
}

void
mfont__free_realized (MRealizedFont *rfont)
{
  MRealizedFont *next;

  for (; rfont; rfont = next)
    {
      next = rfont->next;
      M17N_OBJECT_UNREF (rfont->info);
      free (rfont);
      rfont = next;
    }
}

MFontList *
mfont__list (MFrame *frame, MFont *spec, MFont *request, int max_size)
{
  MFontList *list;
  MSymbol id = mfont__id (spec);
  MPlist *pl, *p;
  int num, i;

  pl = msymbol_get (id, M_font_list);
  if (pl)
    num = (int) msymbol_get (id, M_font_list_len);
  else
    {
      pl = mplist ();
      num = 0;
      MPLIST_DO (p, frame->font_driver_list)
	{
	  if (spec->source == MFONT_SOURCE_X ? MPLIST_KEY (p) == Mx
	      : spec->source == MFONT_SOURCE_FT ? MPLIST_KEY (p) == Mfreetype
	      : 1)
	    {
	      MFontDriver *driver = MPLIST_VAL (p);
	      num += (driver->list) (frame, pl, spec, 0);
	    }
	}
      msymbol_put (id, M_font_list, pl);
      M17N_OBJECT_UNREF (pl);
      msymbol_put (id, M_font_list_len, (void *) num);
    }
  
  if (num == 0)
    return NULL;

  MSTRUCT_MALLOC (list, MERROR_FONT);
  MTABLE_MALLOC (list->fonts, num, MERROR_FONT);
  for (i = 0; num > 0; num--, pl = MPLIST_NEXT (pl))
    {
      MFont *font = MPLIST_VAL (pl), *adjusted = font;

      if (max_size == 0
	  || font->size == 0
	  || font->size < max_size)
	{
	  list->fonts[i].font = font;
	  if (spec == request)
	    list->fonts[i].score = 0;
	  else
	    {
	      int resize_ratio;
	      MFont resized;

	      if (font->size > 0
		  && (resize_ratio = mfont_resize_ratio (font)) != 100)
		{
		  resized = *font;
		  resized.size = font->size * 100 / resize_ratio;
		  adjusted = &resized;
		}
	      list->fonts[i].score = font_score (adjusted, request);
	    }
	  i++;
	}
    }
  if (i == 0)
    {
      free (list->fonts);
      free (list);
      return NULL;
    }
  list->nfonts = i;
  if (spec != request)
    qsort (list->fonts, i, sizeof (MFontScore), compare_font_score);
  list->object = *spec;
  mfont__merge (&list->object, request, 0);
  list->object.type = MFONT_TYPE_OBJECT;
  return list;
}

/** Open a font specified in FONT.  */

MRealizedFont *
mfont__open (MFrame *frame, MFont *font, MFont *spec)
{
  MFontDriver *driver;
  MRealizedFont *rfont;

  if (font->source == MFONT_SOURCE_UNDECIDED)
    MFATAL (MERROR_FONT);
  if (font->type != MFONT_TYPE_OBJECT)
    MFATAL (MERROR_FONT);
  for (rfont = MPLIST_VAL (frame->realized_font_list); rfont;
       rfont = rfont->next)
    {
      driver = rfont->driver;
      if (rfont->font == font
	  && mplist_find_by_value (frame->font_driver_list, driver))
	break;
    }

  if (! rfont)
    {
      driver = mplist_get (frame->font_driver_list,
			   font->source == MFONT_SOURCE_X ? Mx : Mfreetype);
      if (! driver)
	MFATAL (MERROR_FONT);
    }
  return (driver->open) (frame, font, spec, rfont);
}

int
mfont__has_char (MFrame *frame, MFont *font, MFont *spec, int c)
{
  MFontEncoding *encoding;
  unsigned code;
  MFontDriver *driver;

  if (font->source == MFONT_SOURCE_UNDECIDED)
    MFATAL (MERROR_FONT);
  encoding = (font->encoding ? font->encoding : find_encoding (font));
  if (! encoding->encoding_charset)
    return 0;
  if (encoding->repertory_charset)
    {
      code = ENCODE_CHAR (encoding->repertory_charset, c);
      return (code != MCHAR_INVALID_CODE);
    }
  code = ENCODE_CHAR (encoding->encoding_charset, c);
  if (code == MCHAR_INVALID_CODE)
    return 0;
  if (font->type == MFONT_TYPE_REALIZED)
    driver = ((MRealizedFont *) font)->driver;
  else
    {
      driver = mplist_get (frame->font_driver_list,
			   font->source == MFONT_SOURCE_X ? Mx : Mfreetype);
      if (! driver)
	MFATAL (MERROR_FONT);
    }
  return (driver->has_char) (frame, font, spec, c, code);
}

unsigned
mfont__encode_char (MFrame *frame, MFont *font, MFont *spec, int c)
{
  MFontEncoding *encoding;
  unsigned code;
  MFontDriver *driver;

  if (font->source == MFONT_SOURCE_UNDECIDED)
    MFATAL (MERROR_FONT);
  encoding = (font->encoding ? font->encoding : find_encoding (font));
  if (! encoding->encoding_charset)
    return MCHAR_INVALID_CODE;
  if (font->source == MFONT_SOURCE_X && encoding->repertory_charset)
    return (ENCODE_CHAR (encoding->repertory_charset, c));
  code = ENCODE_CHAR (encoding->encoding_charset, c);
  if (code == MCHAR_INVALID_CODE)
    return MCHAR_INVALID_CODE;
  if (font->type == MFONT_TYPE_REALIZED)
    driver = ((MRealizedFont *) font)->driver;
  else
    {
      driver = mplist_get (frame->font_driver_list,
			   font->source == MFONT_SOURCE_X ? Mx : Mfreetype);
      if (! driver)
	MFATAL (MERROR_FONT);
    }
  return (driver->encode_char) (frame, font, spec, code);
}

void
mfont__get_metric (MGlyphString *gstring, int from, int to)
{
  MGlyph *from_g = MGLYPH (from), *to_g = MGLYPH (to), *g;
  MRealizedFont *rfont = from_g->rface->rfont;

  for (g = from_g; ; g++)
    if (g == to_g || g->rface->rfont != rfont)
      {
	int idx = GLYPH_INDEX (g);

	(rfont->driver->find_metric) (rfont, gstring, from, idx);
	while (from_g < g)
	  {
	    from_g->g.xadv >>= 6;
	    from_g->g.yadv >>= 6;
	    from_g->g.xoff >>= 6;
	    from_g->g.yoff >>= 6;
	    from_g->g.ascent >>= 6;
	    from_g->g.descent >>= 6;
	    from_g->g.lbearing >>= 6;
	    from_g->g.rbearing >>= 6;
	    from_g++;
	  }
	if (g == to_g)
	  break;
	rfont = g->rface->rfont;
	from = idx;
      }
}

int
mfont__get_glyph_id (MFLTFont *font, MFLTGlyphString *gstring,
		     int from, int to)
{
  MFont *mfont = (MFont *) ((MFLTFontForRealized *) font)->rfont;
  MRealizedFont *rfont = ((MFLTFontForRealized *) font)->rfont;
  MFontEncoding *encoding;
  MFontDriver *driver = NULL;
  MGlyph *glyphs = (MGlyph *) gstring->glyphs;
  int result = 0;

  encoding = mfont->encoding ? mfont->encoding : find_encoding (mfont);
  for (; from < to; from++)
    {
      MGlyph *g = glyphs + from;

      if (g->g.encoded)
	continue;
      if (mfont->source == MFONT_SOURCE_X && encoding->repertory_charset)
	g->g.code = ENCODE_CHAR (encoding->repertory_charset, g->g.c);
      else
	{
	  unsigned code;

	  if (encoding->encoding_charset)
	    code = ENCODE_CHAR (encoding->encoding_charset, g->g.c);
	  else
	    code = g->g.code;

	  if (code != MCHAR_INVALID_CODE)
	    {
	      if (! driver)
		{
		  if (mfont->type == MFONT_TYPE_REALIZED)
		    driver = rfont->driver;
		  else
		    {
		      driver = mplist_get (rfont->frame->font_driver_list,
					   mfont->source == MFONT_SOURCE_X
					   ? Mx : Mfreetype);
		      if (! driver)
			MFATAL (MERROR_FONT);
		    }
		}
	      g->g.code = (driver->encode_char) (rfont->frame, rfont->font,
						 mfont, code);
	    }
	}
      g->g.encoded = 1;
      if (g->g.code == MCHAR_INVALID_CODE)
	result = -1;
    }
  return result;
}

int
mfont__get_metrics (MFLTFont *font, MFLTGlyphString *gstring,
		    int from, int to)
{
  MRealizedFont *rfont = ((MFLTFontForRealized *) font)->rfont;
  MGlyphString gstr;

  gstr.glyphs = (MGlyph *) gstring->glyphs;
  (rfont->driver->find_metric) (rfont, &gstr, from, to);
  return 0;
}

/* KEY <= MFONT_REGISTRY */

void
mfont__set_property (MFont *font, enum MFontProperty key, MSymbol val)
{
  int numeric;

  if (val == Mnil)
    numeric = 0;
  else
    {
      numeric = FONT_PROPERTY_NUMERIC (val, key);
      if (! numeric)
	{
	  numeric = mfont__property_table[key].used;
	  MLIST_APPEND1 (mfont__property_table + key, names, val, MERROR_FONT);
	  SET_FONT_PROPERTY_NUMERIC (val, key, numeric);
	}
    }
  font->property[key] = numeric;
}

int
mfont__parse_name_into_font (const char *name, MSymbol format, MFont *font)
{
  int result = -1;

  if (format == Mx || format == Mnil)
    result = xlfd_parse_name (name, font);
#ifdef HAVE_FONTCONFIG
  if (format == Mfontconfig || (result < 0 && format == Mnil))
    result = mfont__ft_parse_name (name, font);
#endif /* HAVE_FONTCONFIG */
  return result;
}

MPlist *
mfont__encoding_list (void)
{
  if (! font_encoding_list)
    load_font_encoding_table ();
  return font_encoding_list;
}

static void
free_font_capability (void *object)
{
  MFontCapability *cap = object;
  
  if (cap->script_tag)
    {
      int i;
      for (i = 0; i < MFONT_OTT_MAX; i++)
	{
	  if (cap->features[i].str)
	    free (cap->features[i].str);
	  if (cap->features[i].tags)
	    free (cap->features[i].tags);
	}
    }
  free (cap);
}

MFontCapability *
mfont__get_capability (MSymbol sym)
{
  MFontCapability *cap = msymbol_get (sym, M_font_capability);
  char *str, *p, *endp;

  if (cap)
    return cap;
  str = MSYMBOL_NAME (sym);
  if (str[0] != ':')
    return NULL;
  M17N_OBJECT (cap, free_font_capability, MERROR_FONT);
  msymbol_put (sym, M_font_capability, cap);
  M17N_OBJECT_UNREF (cap);
  endp = str + MSYMBOL_NAMELEN (sym);
  while (str < endp)
    {
      if (*str++ != ':')
	continue;
      if (str[0] == 'o' && strncmp (str + 1, "tf=", 3) == 0)
	{
	  char *beg;
	  MSymbol sym;
	  int i;

	  str += 4;
	  beg = str;
	  for (i = 0, p = str; i < 4 && p < endp; i++, p++);
	  if (i < 4)
	    break;
	  sym = msymbol__with_len (str, 4);
	  cap->script = mscript__from_otf_tag (sym);
	  if (cap->script == Mnil)
	    break;
	  cap->script_tag = OTF_tag (str);
	  if (*p == '/')
	    {
	      for (i = 0, str = ++p; i < 4 && p < endp; i++, p++);
	      if (i < 4)
		{
		  cap->script = Mnil;
		  cap->script_tag = 0;
		  break;
		}
	      cap->langsys_tag = OTF_tag (str);
	    }
	  else
	    cap->langsys_tag = 0;

	  for (i = 0; i < MFONT_OTT_MAX; i++)
	    cap->features[i].nfeatures = -1;

	  while (*p == '=' || *p == '+')
	    {
	      int idx = *p == '=' ? MFONT_OTT_GSUB : MFONT_OTT_GPOS;

	      str = ++p;
	      while (p < endp && *p != '+') p++;
	      if (str < p)
		{
		  int n;
		  /* We never have more than (p - str) tags.  */
		  OTF_Tag *tags = alloca (sizeof (OTF_Tag) * (p - str));
		  char *p0;

		  cap->features[idx].str = malloc (p - str + 1);
		  for (i = n = 0, p0 = str; str + i < p; i++)
		    {
		      cap->features[idx].str[i] = str[i];
		      if (str[i] == ',' || str + i + 1 == p)
			{
			  if (*p0 == '*')
			    tags[n] = 0;
			  else if (*p0 == '~')
			    tags[n] = OTF_tag (p0 + 1) | 0x80000000;
			  else
			    tags[n] = OTF_tag (p0);
			  n++;
			  p0 = str + i + 1;
			}
		    }
		  cap->features[idx].str[i] = '\0';
		  cap->features[idx].nfeatures = n;
		  if (n > 0)
		    {
		      int size = sizeof (OTF_Tag) * n;

		      cap->features[idx].tags = malloc (size);
		      memcpy (cap->features[idx].tags, tags, size);
		    }
		}
	      else
		{
		  cap->features[idx].str = NULL;
		  cap->features[idx].nfeatures = 0;
		}
	    }

	  for (i = 0; i < MFONT_OTT_MAX; i++)
	    if (cap->features[i].nfeatures < 0)
	      {
		cap->features[i].str = strdup ("*");
		cap->features[i].nfeatures = 1;
		cap->features[i].tags = malloc (sizeof (OTF_Tag));
		cap->features[i].tags[0] = 0;
	      }
	  cap->otf = msymbol__with_len (beg, p - beg);
	  str = p;
	}
      else if (str[0] == 'l' && strncmp (str + 1, "ang=", 4) == 0)
	{
	  str += 5;
	  for (p = str; p < endp && *p != ':'; p++);
	  if (str < p)
	    cap->language = msymbol__with_len (str, p - str);
	  str = p;
	}
      else if (str[0] == 's' && strncmp (str + 1, "cript=", 6) == 0)
	{
	  str += 7;
	  for (p = str; p < endp && *p != ':'; p++);
	  if (str < p)
	    cap->script = msymbol__with_len (str, p - str);
	  str = p;
	}
    }
  return cap;
}

int
mfont__check_capability (MRealizedFont *rfont, MSymbol capability)
{
  return (rfont->driver->check_capability (rfont, capability));
}


/*** @} */
#endif /* !FOR_DOXYGEN || DOXYGEN_INTERNAL_MODULE */



/* External API */

/*** @addtogroup m17nFont */
/*** @{ */
/*=*/

/***en @name Variables: Keys of font property.  */
/***ja @name �ѿ�: �ե���ȥץ�ѥƥ�����ꤹ������Ѥߥ���ܥ� */
/*** @{ */
/*=*/

/***en
    @brief Key of font property specifying foundry.

    The variable #Mfoundry is a symbol of name <tt>"foundry"</tt> and
    is used as a key of font property and face property.  The property
    value must be a symbol whose name is a foundry name of a font.  */
/***ja
    @brief ��ȯ������ꤹ��ե���ȥץ�ѥƥ��Υ���.
    
    �ѿ� #Mfoundry �� <tt>"foundry"</tt> 
    �Ȥ���̾������ĥ���ܥ�Ǥ��ꡢ�ե���ȥץ�ѥƥ��ȥե������ץ�ѥƥ��Υ����Ȥ����Ѥ����롣
    �ͤϡ��ե���Ȥγ�ȯ��̾��̾���Ȥ��ƻ��ĥ���ܥ�Ǥ��롣    */

MSymbol Mfoundry;

/***en
    @brief Key of font property specifying family.

    The variable #Mfamily is a symbol of name <tt>"family"</tt> and is
    used as a key of font property and face property.  The property
    value must be a symbol whose name is a family name of a font.  */ 
/***ja
    @brief �ե��ߥ����ꤹ��ե���ȥץ�ѥƥ��Υ���.
    
    �ѿ� #Mfamily �� <tt>"family"</tt> 
    �Ȥ���̾������ĥ���ܥ�Ǥ��ꡢ�ե���ȥץ�ѥƥ��ȥե������ץ�ѥƥ��Υ����Ȥ����Ѥ����롣
    �ͤϡ��ե���ȤΥե��ߥ�̾��̾���Ȥ��ƻ��ĥ���ܥ�Ǥ��롣    */

MSymbol Mfamily;

/***en
    @brief Key of font property specifying weight.

    The variable #Mweight is a symbol of name <tt>"weight"</tt> and is
    used as a key of font property and face property.  The property
    value must be a symbol whose name is a weight name of a font (e.g
    "medium", "bold").  */ 
/***ja
    @brief ��������ꤹ��ե���ȥץ�ѥƥ��Υ���.
    
    �ѿ� #Mweight �� <tt>"weight"</tt> 
    �Ȥ���̾������ĥ���ܥ�Ǥ��ꡢ�ե���ȥץ�ѥƥ��ȥե������ץ�ѥƥ��Υ����Ȥ����Ѥ����롣
    �ͤϡ��ե���Ȥ�����̾ ( "medium", "bold" ��) ��̾���Ȥ��ƻ��ĥ���ܥ�Ǥ��롣    */

MSymbol Mweight;

/***en
    @brief Key of font property specifying style.

    The variable #Mstyle is a symbol of name <tt>"style"</tt> and is
    used as a key of font property and face property.  The property
    value must be a symbol whose name is a style name of a font (e.g
    "r", "i", "o").  */ 
/***ja
    @brief �����������ꤹ��ե���ȥץ�ѥƥ��Υ���.
    
    �ѿ� #Mstyle �� <tt>"style"</tt> 
    �Ȥ���̾������ĥ���ܥ�Ǥ��ꡢ�ե���ȥץ�ѥƥ��ȥե������ץ�ѥƥ��Υ����Ȥ����Ѥ����롣
    �ͤϡ��ե���ȤΥ�������̾ ("r", "i", "o" ��)��̾���Ȥ��ƻ��ĥ���ܥ�Ǥ��롣    */

MSymbol Mstyle;

/***en
    @brief Key of font property specifying stretch.

    The variable #Mstretch is a symbol of name <tt>"stretch"</tt> and
    is used as a key of font property and face property.  The property
    value must be a symbol whose name is a stretch name of a font (e.g
    "normal", "condensed").  */ 
/***ja
    @brief ������ꤹ��ե���ȥץ�ѥƥ��Υ���.
    
    �ѿ� #Mstretch �� <tt>"stretch"</tt> 
    �Ȥ���̾������ĥ���ܥ�Ǥ��ꡢ�ե���ȥץ�ѥƥ��ȥե������ץ�ѥƥ��Υ����Ȥ����Ѥ����롣
    �ͤϡ��ե���Ȥ�ʸ����̾ ( "normal", "condensed" ��)��̾���Ȥ��ƻ��ĥ���ܥ�Ǥ��롣    */

MSymbol Mstretch;

/***en
    @brief Key of font property specifying additional style.

    The variable #Madstyle is a symbol of name <tt>"adstyle"</tt> and
    is used as a key of font property and face property.  The property
    value must be a symbol whose name is an additional style name of a
    font (e.g "serif", "", "sans").  */ 
/***ja
    @brief adstyle ����ꤹ��ե���ȥץ�ѥƥ��Υ���.
    
    �ѿ� #Madstyle �� <tt>"adstyle"</tt> 
    �Ȥ���̾������ĥ���ܥ�Ǥ��ꡢ�ե���ȥץ�ѥƥ��ȥե������ץ�ѥƥ��Υ����Ȥ����Ѥ����롣
    �ͤϡ��ե���Ȥ� adstyle ̾("serif", "", "sans" ��)��̾���Ȥ��ƻ��ĥ���ܥ�Ǥ��롣    */

MSymbol Madstyle;

/***en
    @brief Key of font property specifying spacing.

    The variable #Madstyle is a symbol of name <tt>"spacing"</tt> and
    is used as a key of font property.  The property value must be a
    symbol whose name specifies the spacing of a font (e.g "p" for
    proportional, "m" for monospaced).  */ 
/***ja
    @brief spacing ����ꤹ��ե���ȥץ�ѥƥ��Υ���.
    
    �ѿ� #Mspacing �� <tt>"spacing"</tt> �Ȥ���̾������ĥ���ܥ�Ǥ��ꡢ
    �ե���ȥץ�ѥƥ��Υ����Ȥ����Ѥ����롣�ͤϡ��ե���Ȥ� spacing
    �����򼨤�̾�� ("p", "m" ��)����ĥ���ܥ�Ǥ��롣  */

MSymbol Mspacing;

/***en
    @brief Key of font property specifying registry.

    The variable #Mregistry is a symbol of name <tt>"registry"</tt>
    and is used as a key of font property.  The property value must be
    a symbol whose name is a registry name a font registry
    (e.g. "iso8859-1", "jisx0208.1983-0").  */ 
/***ja
    @brief �쥸���ȥ����ꤹ��ե���ȥץ�ѥƥ��Υ���.
    
    �ѿ� #Mregistry �� <tt>"registry"</tt> 
    �Ȥ���̾������ĥ���ܥ�Ǥ��ꡢ�ե���ȥץ�ѥƥ��ȥե������ץ�ѥƥ��Υ����Ȥ����Ѥ����롣
    �ͤϡ��ե���ȤΥ쥸���ȥ�̾ ( "iso8859-1", "jisx0208.1983-0" 
    ��) ��̾���Ȥ��ƻ��ĥ���ܥ�Ǥ��롣    */

MSymbol Mregistry;

/***en
    @brief Key of font property specifying size.

    The variable #Msize is a symbol of name <tt>"size"</tt> and is
    used as a key of font property and face property.  The property
    value must be an integer specifying a font design size in the unit
    of 1/10 point (on 100 dpi display).  */ 
/***ja
    @brief ����������ꤹ��ե���ȥץ�ѥƥ��Υ���.
    
    �ѿ� #Msize �� <tt>"size"</tt> 
    �Ȥ���̾������ĥ���ܥ�Ǥ��ꡢ�ե���ȥץ�ѥƥ��ȥե������ץ�ѥƥ��Υ����Ȥ����Ѥ����롣�ͤϡ�
    100 dpi �Υǥ����ץ쥤��ǤΥե���ȤΥǥ����󥵥����� 1/10 
    �ݥ����ñ�̤Ǽ��������ͤǤ��롣
    */

MSymbol Msize;

/***en
    @brief Key of font property specifying file name.

    The variable #Mfontfile is a symbol of name <tt>"fontfile"</tt>
    and is used as a key of font property.  The property value must be
    a symbol whose name is a font file name.  */ 
MSymbol Motf;

/***en
    @brief Key of font property specifying file name.

    The variable #Mfontfile is a symbol of name <tt>"fontfile"</tt>
    and is used as a key of font property.  The property value must be
    a symbol whose name is a font file name.  */ 
/***ja
    @brief �ե���ȥե��������ꤹ��ե���ȥץ�ѥƥ��Υ���.
    
    �ѿ� #Mfontfile �� <tt>"fontfile"</tt> �Ȥ���̾������ĥ���ܥ�Ǥ�
    �ꡢ�ե���ȥץ�ѥƥ��Υ����Ȥ����Ѥ����롣�ͤϡ��ե���ȥե���
    ��̾��̾���Ȥ��ƻ��ĤȤ��륷��ܥ�Ǥ��롣 */

MSymbol Mfontfile;

/***en
    @brief Key of font property specifying resolution.

    The variable #Mresolution is a symbol of name <tt>"resolution"</tt> and
    is used as a key of font property and face property.  The property
    value must be an integer to specifying a font resolution in the
    unit of dots per inch (dpi).  */ 
/***ja
    @brief �����٤���ꤹ��ե���ȥץ�ѥƥ��Υ���.
    
    �ѿ� #Mresolution �� <tt>"resolution"</tt> 
    �Ȥ���̾������ĥ���ܥ�Ǥ��ꡢ�ե���ȥץ�ѥƥ��ȥե������ץ�ѥƥ��Υ����Ȥ����Ѥ����롣
    �ͤϡ��ե���Ȥβ����٤� dots per inch (dpi) ñ�̤Ǽ��������ͤǤ��롣    */

MSymbol Mresolution;

/***en
    @brief Key of font property specifying max advance width.

    The variable #Mmax_advance is a symbol of name
    <tt>"max-advance"</tt> and is used as a key of font property.  The
    property value must be an integer specifying a font's max advance
    value by pixels.  */ 

MSymbol Mmax_advance;


/***en
    @brief Symbol of name "fontconfig".

    The variable #Mfontconfig is to be used as an argument of the
    functions mfont_parse_name () and mfont_unparse_name ().  */
/***ja
    @brief "fontconfig" �Ȥ���̾������ĥ���ܥ�.

    �ѿ� #Mfontconfig �ϴؿ� mfont_parse_name () �� mfont_unparse_name ()
    �ΰ����Ȥ����Ѥ����롣  */

MSymbol Mfontconfig;

/***en
    @brief Symbol of name "x".

    The variable #Mx is to be used for a value of \<type\> member of the
    structure #MDrawGlyph to specify the type of \<fontp\> member is
    actually (XFontStruct *).  */
/***ja
    @brief "x" �Ȥ���̾������ĥ���ܥ�.

    �ѿ� #Mx �Ϲ�¤ #MDrawGlyph �Υ��� \<type\> 
    ���ͤȤ����Ѥ���졢���� \<fontp\> �η����ºݤˤ� (XFontStruct *) �Ǥ��뤳�Ȥ�ɽ��.  */

MSymbol Mx;

/***en
    @brief Symbol of name "freetype".

    The variable #Mfreetype is to be used for a value of \<type\> member
    of the structure #MDrawGlyph to specify the type of \<fontp\> member
    is actually FT_Face.  */
/***ja
    @brief "freetype" �Ȥ���̾������ĥ���ܥ�.

    �ѿ� #Mfreetype �Ϲ�¤ #MDrawGlyph �Υ��� \<type\> 
    ���ͤȤ����Ѥ���졢���� \<fontp\> �η����ºݤˤ� FT_Face �Ǥ��뤳�Ȥ�ɽ����  */

MSymbol Mfreetype;

/***en
    @brief Symbol of name "xft".

    The variable #Mxft is to be used for a value of \<type\> member of the
    structure #MDrawGlyph to specify the type of \<fontp\> member
    is actually (XftFont *).  */
/***ja
    @brief  "xft" �Ȥ���̾������ĥ���ܥ�.

    �ѿ� #Mxft �Ϲ�¤ #MDrawGlyph �Υ��� \<type\> 
    ���ͤȤ����Ѥ���졢���� \<fontp\> �η����ºݤˤ� (XftFont *) �Ǥ��뤳�Ȥ�ɽ����  */

MSymbol Mxft;

/*=*/
/*** @} */
/*=*/

/***en
    @brief List of font files and directories that contain font files.

    The variable @c mfont_freetype_path is a plist of FreeType font
    files and directories that contain FreeType font files.  Key of
    the element is @c Mstring, and the value is a string that
    represents a font file or a directory.

    The macro M17N_INIT () sets up this variable to contain the
    sub-directory "fonts" of the m17n database and the environment
    variable "M17NDIR".  The first call of mframe () creates the
    internal list of the actually available fonts from this variable.
    Thus, an application program, if necessary, must modify the
    variable before calling mframe ().  If it is going to add a new
    element, value must be a string that can be safely freed.

    If the m17n library is not configured to use the FreeType library,
    this variable is not used.  */
/***ja
    @brief �ե���ȥե�����ȥե���ȥե������ޤ�ǥ��쥯�ȥ�Υꥹ��.

    �ѿ� @c mfont_freetype_path �ϡ��ե���ȥե�����ȥե���ȥե������ޤ�ǥ��쥯�ȥ�� 
    plist �Ǥ��롣�����ǤΥ����� @c Mstring 
    �Ǥ��ꡢ�ͤϥե���ȥե����뤫�ǥ��쥯�ȥ�򼨤�ʸ����Ǥ��롣

    �ޥ��� M17N_INIT () �ˤ�äơ������ѿ��� m17n �ǡ����١����ȴĶ��ѿ� 
    "M17NDIR" �����Υ��֥ǥ��쥯�ȥ� "fonts" ��ޤ�褦�����ꤵ��롣
    mframe () �κǽ�θƤӽФ��κݤˡ������ѿ�����ºݤ˻��ѤǤ���ե���Ȥ������ꥹ�Ȥ�����롣
    �����ǥ��ץꥱ�������ץ����ϡ�mframe () 
    ��Ƥ����ˡ�ɬ�פʤ�Сˤ����ѿ����ѹ����ʤ��ƤϤʤ�ʤ���
    ���������Ǥ��ɲä�����ˤϡ������ͤϰ����˳����Ǥ���ʸ����Ǥʤ��ƤϤʤ�ʤ���

    m17n �饤�֥�꤬ FreeType �饤�֥���Ȥ��褦�����ꤵ��Ƥʤ����ˤϡ������ѿ����Ѥ����ʤ��� */

MPlist *mfont_freetype_path;

/*=*/

/***en
    @brief Create a new font.

    The mfont () function creates a new font object that has no
    property.

    @return
    This function returns a pointer to the created font object.  */
/***ja
    @brief �������ե���Ȥ���.

    �ؿ� mfont () �ϥץ�ѥƥ�����ڻ����ʤ��������ե���Ȥ򥪥֥������Ȥ��롣

    @return
    ���δؿ��Ϻ�ä��ե���ȥ��֥������ȤؤΥݥ��󥿤��֤���  */

MFont *
mfont ()
{
  MFont *font;

  MSTRUCT_CALLOC (font, MERROR_FONT);
  return font;
}

/*=*/

/***en
    @brief Create a font by parsing a fontname.

    The mfont_parse_name () function creates a new font object.  The
    properties are extracted fontname $NAME.

    $FORMAT specifies the format of $NAME.  If $FORMAT is #Mx, $NAME
    is parsed as XLFD (X Logical Font Description).  If $FORMAT is
    #Mfontconfig, $NAME is parsed as Fontconfig's textual
    representation of font.  If $FORMAT is #Mnil, $NAME is at first
    parsed as XLFD, and it it fails, parsed as Fontconfig's
    representation.

    @return
    If the operation was successful, this function returns a pointer
    to the created font.  Otherwise it returns @c NULL.  */

/***ja
    @brief �ե����̾����ե���Ȥ���.

    �ؿ� mfont_parse_name () �ϡ��ե����̾ 
    $NAME ������Ф��줿�ץ�ѥƥ�����ġ��������ե���ȥ��֥������Ȥ��롣

    $FORMAT �� $NAME �Υե����ޥåȤ���ꤹ�롣$FORMAT �� #Mx �Ǥ���С�
    $NAME �� XLFD (X Logical Font Description) �˽��äƲ��Ϥ���롣
    $FORMAT �� #Mfontconfig �Ǥ���� $NAME �� Fontfonfig 
    �Υե���ȥƥ�����ɽ���˽��äƲ��Ϥ���롣$FORMAT �� #Mnil �Ǥ���С��ޤ� XLFD 
    �˽��äƲ��Ϥ��졢����˼��Ԥ����� Fontconfig �˽��äƲ��Ϥ���롣

    @return
    ��������������� mfont_parse_name () 
    �Ͽ��������줿�ե���ȤؤΥݥ��󥿤��֤��������Ǥʤ���� @c NULL ���֤���  */

MFont *
mfont_parse_name (const char *name, MSymbol format)
{
  MFont template, *font;
  
  MFONT_INIT (&template);
  if (mfont__parse_name_into_font (name, format, &template) < 0)
    MERROR (MERROR_FONT, NULL);
  MSTRUCT_CALLOC (font, MERROR_FONT);
  *font = template;
  return font;
}

/*=*/

/***en
    @brief Create a fontname from a font.

    The mfont_unparse_name () function creates a fontname string
    from font $FONT according to $FORMAT.

    $FORMAT must be #Mx or #Mfontconfig.  If it is #Mx, the fontname
    is in XLFD (X Logical Font Description) format.  If it is
    #Mfontconfig, the fontname is in the style of Fontconfig's text
    representation.

    @return
    This function returns a newly allocated fontname string, which is
    not freed unless the user explicitly does so by free ().  */

/***ja
    @brief �ե���Ȥ���ե����̾����.

    �ؿ� mfont_unparse_name () �� �ե����̾��ʸ�����ե���� $FONT 
    �򸵤�$FORMAT �˽��äƺ�롣

    $FORMAT �� #Mx �ޤ��� #Mfontconfig �Ǥ��롣
    #Mx �ʤ�Хե����̾�� XLFD (X Logical Font Description) �˽�����
    #Mfontconfig �ʤ�Хե����̾�� Fontconfig �Υե���ȥƥ�����ɽ���˽�����

    @return 
    ���δؿ��Ͽ����˥������Ȥ����ե����̾��ʸ������֤���ʸ����ϡ��桼����
    free () �ˤ�ä�����Ū�˲������ʤ��¤��������ʤ���  */

char *
mfont_unparse_name (MFont *font, MSymbol format)
{
  char *name;

  if (format == Mx)
    name = xlfd_unparse_name (font, 1);
#ifdef HAVE_FONTCONFIG
  else if (format == Mfontconfig)
    name = mfont__ft_unparse_name (font);

#endif /* HAVE_FONTCONFIG */
  else
    MERROR (MERROR_FONT, NULL);
  return name;
}

/*=*/

/***en
    @brief Make a copy of a font.

    The mfont_copy () function returns a new copy of font $FONT.  */
/***ja
    @brief �ե���ȤΥ��ԡ�����.

    �ؿ� Mfont_copy () �ϥե���� $FONT �Υ��ԡ����ꡢ������֤��� */

MFont *
mfont_copy (MFont *font)
{
  MFont *copy;

  MSTRUCT_MALLOC (copy, MERROR_FONT);
  *copy = *font;
  return copy;
}

/*=*/

/***en
    @brief Get a property value of a font.

    The mfont_get_prop () function gets the value of $KEY property of
    font $FONT.  $KEY must be one of the following symbols:

	@c Mfoundry, @c Mfamily, @c Mweight, @c Mstyle, @c Mstretch,
	@c Madstyle, @c Mregistry, @c Msize, @c Mresolution, @c Mspacing.

    If $FONT is a return value of mfont_find (), $KEY can also be one
    of the following symbols:

	@b Mfont_ascent, @b Mfont_descent, #Mmax_advance.

    @return 
    If $KEY is @c Mfoundry, @c Mfamily, @c Mweight, @c Mstyle,
    @c Mstretch, @c Madstyle, @c Mregistry, or @c Mspacing, this
    function returns the corresponding value as a symbol.  If the font
    does not have $KEY property, it returns @c Mnil.  If $KEY is @c
    Msize, @c Mresolution, @b Mfont_ascent, Mfont_descent, or
    #Mmax_advance, this function returns the corresponding value as an
    integer.  If the font does not have $KEY property, it returns 0.
    If $KEY is something else, it returns @c NULL and assigns an error
    code to the external variable #merror_code.  */
 
/***ja
    @brief �ե���ȤΥץ�ѥƥ����ͤ�����.

    �ؿ� mfont_get_prop () �ϥե���� $FONT �Υץ�ѥƥ��Τ�����������
    $KEY �Ǥ����Τ��ͤ��֤���$KEY �ϰʲ��Υ���ܥ�Τ����줫�Ǥʤ���
    �Фʤ�ʤ���

	@c Mfoundry, @c Mfamily, @c Mweight, @c Mstyle, @c Mstretch,
	@c Madstyle, @c Mregistry, @c Msize, @c Mresolution, @c Mspacing.

    @return 
    $KEY �� @c Mfoundry, @c Mfamily, @c Mweight, @c Mstyle, @c
    Mstretch, @c Madstyle, @c Mregistry, @c Mspacing �Τ����줫�Ǥ���С�
    ���������ͤ򥷥�ܥ�Ȥ����֤����ե���Ȥ����Υץ�ѥƥ�������ʤ�
    ���ˤ�@c Mnil ���֤���$KEY �� @c Msize ���뤤�� @c Mresolution ��
    ���ˤϡ����������ͤ�������ͤȤ����֤����ե���Ȥ����Υץ�ѥƥ�
    ������ʤ����ˤ� 0 ���֤���$KEY ������ʳ��Τ�ΤǤ���С�@c
    NULL ���֤��������ѿ� #merror_code �˥��顼�����ɤ����ꤹ�롣  */

void *
mfont_get_prop (MFont *font, MSymbol key)
{
  MRealizedFont *rfont = NULL;

  if (font->type == MFONT_TYPE_REALIZED)
    rfont = (MRealizedFont *) font;

  if (key == Mfoundry)
    return (void *) FONT_PROPERTY (font, MFONT_FOUNDRY);
  if (key == Mfamily)
    return (void *) FONT_PROPERTY (font, MFONT_FAMILY);
  if (key == Mweight)
    return (void *) FONT_PROPERTY (font, MFONT_WEIGHT);
  if (key == Mstyle)
    return (void *) FONT_PROPERTY (font, MFONT_STYLE);
  if (key == Mstretch)
    return (void *) FONT_PROPERTY (font, MFONT_STRETCH);
  if (key == Madstyle)
    return (void *) FONT_PROPERTY (font, MFONT_ADSTYLE);
  if (key == Mregistry)
    return (void *) FONT_PROPERTY (font, MFONT_REGISTRY);
  if (key == Msize)
    {
      int size = font->size;
      return (void *) size;
    }
  if (key == Mresolution)
    {
      int resy = font->property[MFONT_RESY];
      return (void *) resy;
    }
  if (key == Mlanguage || key == Mscript || key == Motf)
    {
      MFontCapability *cap;

      if (! font->capability)
	return NULL;
      cap = mfont__get_capability (font->capability);
      if (key == Mlanguage)
	return cap->language;
      if (key == Mscript)
	return cap->script;
      return cap->otf;
    }

  if (key == Mfontfile)
    return (void *) font->file;
  if (key == Mspacing)
    return (font->spacing == MFONT_SPACING_UNDECIDED ? Mnil
	    : msymbol (font->spacing == MFONT_SPACING_PROPORTIONAL ? "p"
		       : font->spacing == MFONT_SPACING_MONO ? "m" : "c"));
  if (rfont)
    {
      if (key == Mfont_ascent)
	return (void *) rfont->ascent;
      if (key == Mfont_descent)
	return (void *) rfont->descent;
      if (key == Mmax_advance)
	return (void *) rfont->max_advance;
    }
  MERROR (MERROR_FONT, NULL);
}


/*=*/
/***en
    @brief Put a property value to a font.

    The mfont_put_prop () function puts a font property whose key is
    $KEY and value is $VAL to font $FONT.  $KEY must be one of the
    following symbols:

	@c Mfoundry, @c Mfamily, @c Mweight, @c Mstyle, @c Mstretch,
	@c Madstyle, @c Mregistry, @c Msize, @c Mresolution.

    If $KEY is @c Msize or @c Mresolution, $VAL must be an integer.
    Otherwise, $VAL must be a symbol of a property value name.  But,
    if the name is "nil", a symbol of name "Nil" must be
    specified.  */
 /***ja
    @brief �ե���ȤΥץ�ѥƥ����ͤ����ꤹ��.

    �ؿ� mfont_put_prop () �ϡ��ե���� $FONT �Υ�����$KEY �Ǥ���ץ��
    �ƥ����ͤ� $VAL �����ꤹ�롣$KEY �ϰʲ��Υ���ܥ�Τ����줫�Ǥ��롣

	@c Mfoundry, @c Mfamily, @c Mweight, @c Mstyle, @c Mstretch,
	@c Madstyle, @c Mregistry, @c Msize, @c Mresolution.

    $KEY �� @c Msize �� @c Mresolution �Ǥ���� $VAL �������ͤǤʤ��Ƥ�
    ��ʤ�������ʳ��ξ�硢$VAL �ϥץ�ѥƥ��ͤ�̾���Υ���ܥ�Ǥʤ���
    �Ϥʤ�ʤ����������⤷����̾���� "nil" �ξ��ϡ�̾���� "Nil" �Υ�
    ��ܥ�Ǥʤ��ƤϤʤ�ʤ���*/

int
mfont_put_prop (MFont *font, MSymbol key, void *val)
{
  if (key == Mfoundry)
    mfont__set_property (font, MFONT_FOUNDRY, (MSymbol) val);
  else if (key == Mfamily)
    mfont__set_property (font, MFONT_FAMILY, (MSymbol) val);
  else if (key == Mweight)
    mfont__set_property (font, MFONT_WEIGHT, (MSymbol) val);
  else if (key == Mstyle)
    mfont__set_property (font, MFONT_STYLE, (MSymbol) val);
  else if (key == Mstretch)
    mfont__set_property (font, MFONT_STRETCH, (MSymbol) val);
  else if (key == Madstyle)
    mfont__set_property (font, MFONT_ADSTYLE, (MSymbol) val);
  else if (key == Mregistry)
    mfont__set_property (font, MFONT_REGISTRY, (MSymbol) val);
  else if (key == Msize)
    {
      int size = (int) val;
      font->size = size;
    }
  else if (key == Mresolution)
    {
      unsigned resy = (unsigned) val;
      font->property[MFONT_RESY] = resy;
    }
  else if (key == Mlanguage || key == Mscript || key == Motf)
    {
      font->capability = merge_capability (font->capability,
					   key, (MSymbol) val, 1);
    }
  else if (key == Mfontfile)
    {
      font->file = (MSymbol) val;
    }
  else
    MERROR (MERROR_FONT, -1);
  return 0;
}

/*=*/

/***en
    @brief Return the font selection priority.

    The mfont_selection_priority () function returns a newly created
    array of six symbols.  The elements are the following
    keys of font properties ordered by priority.

	@c Mfamily, @c Mweight, @c Mstyle, @c Mstretch,
	@c Madstyle, @c Msize.

   The m17n library selects the best matching font according to the
   order of this array.  A font that has a different value for a
   property of lower priority is preferred to a font that has a
   different value for a property of higher priority.  */
/***ja
    @brief �ե���������ͥ���٤��֤�.

    �ؿ� mfont_selection_priority () �� 6 �ĤΥ���ܥ뤫��ʤ�������ä��֤���
    ��������Ǥϡ��ʲ��Υե���ȥץ�ѥƥ��Υ�����ͥ���ٽ���¤٤���ΤǤ��롣

	@c Mfamily, @c Mweight, @c Mstyle, @c Mstretch,
	@c Madstyle, @c Msize.

   m17n �饤�֥��Ϥ�������˽��äơ��Ǥ���פ���ե���Ȥ����򤹤롣
   ��Ū�Υե���Ȥȡ����줾��㤦�ץ�ѥƥ����ͤ����פ��ʤ��ե���Ȥ����ä���硢ͥ���٤��㤤�ץ�ѥƥ����ͤ����פ��ʤ��ե���ȡ�ͥ���٤ι⤤�ץ�ѥƥ����ͤ����פ��Ƥ���ե���ȡˤ����򤵤�롣

   */

MSymbol *
mfont_selection_priority ()
{
  MSymbol *keys;
  int i;

  MTABLE_MALLOC (keys, FONT_SCORE_PRIORITY_SIZE, MERROR_FONT);
  for (i = 0; i < FONT_SCORE_PRIORITY_SIZE; i++)
    {
      enum MFontProperty prop = font_score_priority[i];

      if (prop == MFONT_SIZE)
	keys[i] = Msize;
      else if (prop == MFONT_ADSTYLE)
	keys[i] = Madstyle;
      else if (prop == MFONT_FAMILY)
	keys[i] = Mfamily;
      else if (prop == MFONT_WEIGHT)
	keys[i] = Mweight;
      else if (prop == MFONT_STYLE)
	keys[i] = Mstyle;
      else if (prop == MFONT_STRETCH)
	keys[i] = Mstretch;
      else
	keys[i] = Mfoundry;
    }
  return keys;
}

/*=*/

/***en
    @brief Set the font selection priority.

    The mfont_set_selection_priority () function sets font selection
    priority according to $KEYS, which is an array of six symbols.
    Each element must be one of the below.  No two elements must be
    the same.

	@c Mfamily, @c Mweight, @c Mstyle, @c Mstretch,
	@c Madstyle, @c Msize.

    See the documentation of the function mfont_selection_priority ()
    for details.  */
/***ja
    @brief �ե��������ͥ���٤����ꤹ��.

    �ؿ� mfont_set_selection_priority () �ϡ�6�ĤΥ���ܥ������ $KEYS 
    �ˤ������äƥե��������ͥ���٤����ꤹ�롣����ϰʲ��γ����Ǥ�Ŭ��
    �ʽ��֤��¤٤���ΤǤ��롣

	@c Mfamily, @c Mweight, @c Mstyle, @c Mstretch,
	@c Madstyle, @c Msize.

    �ܺ٤ϴؿ� mfont_selection_priority () �������򻲾ȤΤ��ȡ�
     */

int
mfont_set_selection_priority (MSymbol *keys)
{
  int priority[FONT_SCORE_PRIORITY_SIZE];
  int i, j, shift;

  for (i = 0; i < FONT_SCORE_PRIORITY_SIZE; i++, keys++)
    {
      enum MFontProperty prop;

      if (*keys == Msize)
	prop = MFONT_SIZE;
      else if (*keys == Madstyle)
	prop = MFONT_ADSTYLE;
      else if (*keys == Mfamily)
	prop = MFONT_FAMILY;
      else if (*keys == Mweight)
	prop = MFONT_WEIGHT;
      else if (*keys == Mstyle)
	prop = MFONT_STYLE;
      else if (*keys == Mstretch)
	prop = MFONT_STRETCH;
      else if (*keys == Mfoundry)
	prop = MFONT_FOUNDRY;
      else
	/* Invalid element.  */
	return -1;
      for (j = 0; j < i; j++)
	if (priority[j] == prop)
	  /* Duplicated element.  */
	  return -1;
      priority[i] = prop;
    }
  for (i = 0; i < FONT_SCORE_PRIORITY_SIZE; i++)
    font_score_priority[i] = priority[i];
  /* Here, SHIFT starts from 1, not 0.  This is because the lowest bit
     of a score is a flag for a scalable font (see the documentation
     of font_score).  */
  i = FONT_SCORE_PRIORITY_SIZE - 1;
  for (shift = 1; i >= 0; i--)
    {
      font_score_shift_bits[font_score_priority[i]] = shift;
      if (font_score_priority[i] == MFONT_SIZE)
	shift += 16;
      else if (font_score_priority[i] <= MFONT_FAMILY)
	shift++;
      else
	shift += 2;
    }
  return 0;
}

/*=*/

/***en
    @brief Find a font.

    The mfont_find () function returns a pointer to the available font
    that matches best the specification $SPEC on frame $FRAME.

    $SCORE, if not NULL, must point to a place to store the score
    value that indicates how well the found font matches to $SPEC.  The
    smaller score means a better match.  */
/***ja
    @brief �ե���Ȥ�õ��.

    �ؿ� mfont_find () �ϡ��ե졼�� $FRAME ��ǥե������� $SPEC 
    �ˤ�äȤ���פ������Ѳ�ǽ�ʥե���ȤؤΥݥ��󥿤��֤���  

    $SCORE �� NULL �Ǥ��뤫�����Ĥ��ä��ե���Ȥ� $SPEC 
    �ˤɤ�ۤɹ�äƤ��뤫�򼨤�����������¸������ؤΥݥ��󥿤Ǥ��롣
    ���������������ۤ��ɤ���äƤ��뤳�Ȥ��̣���롣
    */

MFont *
mfont_find (MFrame *frame, MFont *spec, int *score, int max_size)
{
  MFont spec_copy;
  MFont *best;
  MFontList *list;
  MRealizedFont *rfont;
  MFont adjusted;

  if (spec->size < 0)
    {
      double pt = - spec->size;

      adjusted = *spec;
      adjusted.size = pt * frame->dpi / 72.27 + 0.5;
      spec = &adjusted;
    }
  MFONT_INIT (&spec_copy);
  spec_copy.property[MFONT_FAMILY] = spec->property[MFONT_FAMILY];
  spec_copy.property[MFONT_REGISTRY] = spec->property[MFONT_REGISTRY];
  spec_copy.capability = spec->capability;
  spec_copy.file = spec->file;

  list = mfont__list (frame, &spec_copy, spec, max_size);
  if (! list)
    return NULL;

  best = list->fonts[0].font;
  if (score)
    *score = list->fonts[0].score;
  free (list->fonts);
  free (list);
  spec_copy = *best;
  mfont__merge (&spec_copy, spec, 0);
  rfont = mfont__open (frame, best, spec);
  if (! rfont)
    return NULL;
  return (MFont *) rfont;
}

/*=*/
/***en
    @brief Set encoding of a font.

    The mfont_set_encoding () function sets the encoding information
    of font $FONT.

    $ENCODING_NAME is a symbol representing a charset that has the
    same encoding as the font.

    $REPERTORY_NAME is @c Mnil or a symbol representing a charset that
    has the same repertory as the font.  If it is @c Mnil, whether a
    specific character is supported by the font is asked to each font
    driver.

    @return
    If the operation was successful, this function returns 0.
    Otherwise it returns -1 and assigns an error code to the external
    variable #merror_code.  */
/***ja
    @brief �ե���ȤΥ��󥳡��ǥ��󥰤����ꤹ��.

    �ؿ� mfont_set_encoding () �ϥե���� $FONT �Υ��󥳡��ǥ��󥰾�������ꤹ�롣

    $ENCODING_NAME �ϥե���Ȥ�Ʊ�����󥳡��ǥ��󥰤����ʸ�����åȤ򼨤�����ܥ�Ǥ��롣

    $REPERTORY_NAME �� @c Mnil �Ǥ��뤫���ե���Ȥ�Ʊ�����󥳡��ǥ��󥰤����ʸ�����åȤ򼨤�����ܥ�Ǥ��롣
    @c Mnil �Ǥ���С��ġ���ʸ�������Υե���Ȥǥ��ݡ��Ȥ���Ƥ��뤫�ɤ����ϡ��ơ��Υե���ȥɥ饤�Ф��䤤��碌�롣

    @return
    ��������������Ф��δؿ��� 0 ���֤��������Ǥʤ���� -1 ���֤��������ѿ�
    #merror_code �˥��顼�����ɤ����ꤹ�롣  */


int
mfont_set_encoding (MFont *font, MSymbol encoding_name, MSymbol repertory_name)
{
  MCharset *encoding_charset = MCHARSET (encoding_name);
  MCharset *repertory_charset;
  MSymbol registry;
  MFontEncoding *encoding;
  MPlist *plist;

  if (! encoding_charset)
    MERROR (MERROR_FONT, -1);
  if (repertory_name != Mnil)
    {
      repertory_charset = MCHARSET (repertory_name);
      if (! repertory_charset)
	MERROR (MERROR_FONT, -1);
    }
  else
    repertory_charset = NULL;

  MSTRUCT_CALLOC (encoding, MERROR_FONT);
  encoding->spec = *font;
  encoding->encoding_name = encoding_name;
  encoding->encoding_charset = encoding_charset;
  encoding->repertory_name = repertory_name;
  encoding->repertory_charset = repertory_charset;
  registry = FONT_PROPERTY (font, MFONT_REGISTRY);
  if (registry == Mnil)
    registry = Mt;
  if (! font_encoding_list)
    load_font_encoding_table ();
  mplist_push (font_encoding_list, registry, encoding);
  MPLIST_DO (plist, MPLIST_NEXT (font_encoding_list))
    if (! memcmp (font, &((MFontEncoding *) MPLIST_VAL (plist))->spec,
		  sizeof (MFont)))
      {
	mplist_pop (plist);
	break;
      }
  return 0;
}

/*=*/

/***en
    @brief Create a fontname from a font.

    This function is obsolete.   Use mfont_unparse_name instead. */
/***ja
    @brief �ե����̾����ե���Ȥ���.

    ���δؿ����ѻ�ͽ��Ǥ��롣 mfont_unparse_name () ����ѤΤ��ȡ� */

char *
mfont_name (MFont *font)
{
  return mfont_unparse_name (font, Mx);
}

/*=*/

/***en
    @brief Create a new font from fontname.

    This function is obsolete.  Use mfont_parse_name () instead.  */

/***ja
    @brief �ե���Ȥ���ե����̾����.

    ����ϴؿ����ѻ�ͽ��Ǥ��롣 mfont_parse_name () ����ѤΤ��ȡ�  */

MFont *
mfont_from_name (const char *name)
{
  return mfont_parse_name (name, Mx);
}

/*=*/

/***en
    @brief Get resize information of a font.

    The mfont_resize_ratio () function lookups the m17n database
    \<font, reisize\> and returns a resizing ratio (in percentage) of
    FONT.  For instance, if the return value is 150, that means that
    the m17n library uses an 1.5 time bigger font than a specified
    size.  */

/***ja
    @brief �ե���ȤΥꥵ�������������

    �ؿ� mfont_resize_ratio �� m17n �ǡ����١��� \<font, reisize\> 
    �򸡺������ե���� FONT �Υꥵ��������Ψ�ʥѡ�����ơ�����
    ���֤������Ȥ����֤��ͤ� 150 �Ǥ���С�m17n �饤�֥��ϻ��ꤵ�줿�������� 1.5 
    �ܤΥե���Ȥ���Ѥ��뤳�Ȥ��̣���롣 */

int
mfont_resize_ratio (MFont *font)
{
  MSymbol registry = FONT_PROPERTY (font, MFONT_REGISTRY);
  MFontResize *resize;
  MPlist *plist;

  if (! font_resize_list)
    load_font_resize_table ();
  if (! MPLIST_TAIL_P (font_resize_list))
    while (1)
      {
	plist = font_resize_list;
	while (registry ? (plist = mplist_find_by_key (plist, registry))
	       : plist)
	  {
	    resize = (MFontResize *) MPLIST_VAL (plist);
	    if (mfont__match_p (font, &resize->spec, MFONT_ADSTYLE))
	      return resize->resize;
	    plist = MPLIST_NEXT (plist);
	  }
	if (registry == Mt)
	  break;
	registry = Mt;
      }
  return 100;
}

/*=*/

/***en
    @brief Get a list of fonts.

    The mfont_list () functions returns a list of fonts available on
    frame $FRAME.  $FONT, if not NULL, limits fonts to ones
    that match with $FONT.  $LANGUAGE, if not @c Mnil, limits fonts to
    ones that support $LANGUAGE.  $MAXNUM, if greater than 0, limits
    the number of fonts.

    $LANGUAGE argument exists just for backward compatibility, and the
    use is deprecated.  Use #Mlanguage font property instead.  If
    $FONT already has #Mlanguage property, $LANGUAGE is ignored.

    @return
    This function returns a plist whose keys are family names and
    values are pointers to the object MFont.  The plist must be freed
    by m17n_object_unref ().  If no font is found, it returns
    NULL.  */

/***ja
    @brief �ե���ȤΥꥹ�Ȥ�����

    �ؿ� mfont_list () �ϥե졼�� $FRAME �����Ѳ�ǽ�ʥե���ȤΥꥹ�Ȥ�
    �֤���$FONT �� NULL �Ǥʤ���С�$FONT �ȹ��פ������Ѳ�ǽ�ʥե����
    �Υꥹ�Ȥ��֤���$LANGUAGE �� @c Mnil �Ǥʤ���С�$LANGUAGE �򥵥ݡ�
    �Ȥ������Ѳ�ǽ�ʥե���ȤΥꥹ�Ȥ��֤���$MAXNUM �ϡ�0 ����礭����
    ��ˤϡ��֤��ե���Ȥο��ξ�¤Ǥ��롣

    ������������ $LANGUAGE �ϵ��ǤȤ��������Τ�������ˤ��ꡢ���λ��Ѥ�
    ������ʤ����ե���Ȥ� #Mlanguage �ץ�ѥƥ���Ȥ��٤��Ǥ��롣��
    �� $FONT �����Ǥˤ��Υץ�ѥƥ�����äƤ����顢���� $LANGUAGE ��̵


    @return 
    ���δؿ��ϥ������ե���ȥե��ߥ�̾�Ǥ����ͤ� MFont ���֥������Ȥؤ�
    �ݥ��󥿤Ǥ���褦��plist ���֤���plist �� m17n_object_unref () ��
    ��������ɬ�פ����롣�ե���Ȥ����Ĥ���ʤ����NULL ���֤���  */

MPlist *
mfont_list (MFrame *frame, MFont *font, MSymbol language, int maxnum)
{
  MPlist *plist, *pl;
  MFontList *font_list;
  int i;
  MFont spec;
  
  if (font)
    spec = *font;
  else
    MFONT_INIT (&spec);

  if (spec.size < 0)
    {
      double pt = - spec.size;

      spec.size = pt * frame->dpi / 72.27 + 0.5;
    }

  if (language != Mnil)
    spec.capability = merge_capability (spec.capability, Mlanguage, language,
					0);

  font_list = mfont__list (frame, &spec, &spec, 0);
  if (! font_list)
    return NULL;
  if (font_list->nfonts == 0)
    {
      free (font_list);
      return NULL;
    }

  plist = pl = mplist ();
  for (i = 0; i < font_list->nfonts; i++)
    {
      MSymbol family = FONT_PROPERTY (font_list->fonts[i].font, MFONT_FAMILY);

      if (family != Mnil)
	pl = mplist_add (pl, family, font_list->fonts[i].font);
    }
  free (font_list);
  return plist;
}

/***en
    @brief Get a list of font famiy names.

    The mfont_list_family_names () functions returns a list of font
    family names available on frame $FRAME.

    @return

    This function returns a plist whose keys are #Msymbol and values
    are symbols representing font family names.  The elements are
    sorted by alphabetical order.  The plist must be freed by
    m17n_object_unref ().  If not font is found, it returns NULL.  */

MPlist *
mfont_list_family_names (MFrame *frame)
{
  MPlist *plist = mplist (), *p;

  MPLIST_DO (p, frame->font_driver_list)
    {
      MFontDriver *driver = MPLIST_VAL (p);

      (driver->list_family_names) (frame, plist);
    }
  return plist;
}


/*=*/

/***en
    @brief Check the usability of a font.

    The mfont_check () function checkes if $FONT can be used for
    $SCRIPT and $LANGUAGE in $FONTSET on $FRAME.

    @return 
    If the font is usable, return 1.  Otherwise return 0.
 */

int
mfont_check (MFrame *frame, MFontset *fontset,
	     MSymbol script, MSymbol language, MFont *font)
{
  MRealizedFont *rfont;
  int best, score;

  if (! fontset)
    fontset = frame->face->property[MFACE_FONTSET];
  rfont = mfontset__get_font (frame, fontset, script, language, font, &best);
  if (! rfont || ! best)
    return 0;
  score = font_score (&rfont->spec, font);
  return (score == 0 ? 2 : 1);
}

/*=*/

/***en
    @brief Check is a font matches with a font spec.

    The mfont_match_p () function checks if $FONT matches with the
    font-spec $SPEC.

    @return 
    If the font matches, 1 is returned.  Otherwise 0 is returned.  */

int
mfont_match_p (MFont *font, MFont *spec)
{
  return mfont__match_p (font, spec, MFONT_REGISTRY);
}

/*=*/
/***en
    @brief Open a font.

    The mfont_open () function opens $FONT on $FRAME, and returns a
    realized font.

    @return
    If the font was successfully opened, a realized font is returned.
    Otherwize NULL is returned.

    @seealso
    mfont_close ().  */


MFont *
mfont_open (MFrame *frame, MFont *font)
{
  enum MFontType font_type = font->type;

  if (font_type == MFONT_TYPE_SPEC)
    return mfont_find (frame, font, NULL, 0);
  if (font_type == MFONT_TYPE_OBJECT)
    return (MFont *) mfont__open (frame, font, font);
  if (font_type == MFONT_TYPE_REALIZED)
    return font;
  MERROR (MERROR_FONT, NULL);
}

/*=*/
/***en
    @brief Encapusulate a font.

    The mfont_encapsulate () functions realizes a font by
    encapusulating data $DATA or type $DATA_TYPE on $FRAME.  Currently
    $DATA_TAPE is #Mfontconfig or #Mfreetype, and $DATA points to an
    object of FcPattern or FT_Face respectively.

    @return
    If the operation was successful, a realized font is returned.
    Otherwise NULL is return.

    @seealso
    mfont_close ().  */


MFont *
mfont_encapsulate (MFrame *frame, MSymbol data_type, void *data)
{
  MPlist *p;

  MPLIST_DO (p, frame->font_driver_list)
    {
      MFontDriver *driver = MPLIST_VAL (p);
      MRealizedFont *rfont;

      if (driver->encapsulate
	  && (rfont = driver->encapsulate (frame, data_type, data)))
	return (MFont *) rfont;
    }

  return NULL;
}

/*=*/
/***en
    @brief Close a font.

    The mfont_close () function close a realized font $FONT.  $FONT
    must be opened previously by mfont_open () or mfont_encapsulate
    ().

    @return
    If the operation was successful, 0 is returned.  Otherwise, -1 is
    returned.

    @seealso
    mfont_open (), mfont_encapsulate ().  */

int
mfont_close (MFont *font)
{
  enum MFontType font_type = font->type;
  MRealizedFont *rfont;

  if (font_type != MFONT_TYPE_REALIZED)
    MERROR (MERROR_FONT, -1);
  rfont = (MRealizedFont *) font;
  if (rfont->encapsulating
      && rfont->driver->close)
    rfont->driver->close (rfont);
  return 0;
}

/*** @} */

/*** @addtogroup m17nDebug */
/*=*/
/*** @{ */

/***en
    @brief Dump a font.

    The mdebug_dump_font () function prints font $FONT in a human
    readable way to the stderr or to what specified by the environment
    variable MDEBUG_OUTPUT_FILE.

    @return
    This function returns $FONT.  */
/***ja
    @brief �ե���Ȥ����פ���.

    �ؿ� mdebug_dump_font () �ϥե���� $FONT ��ɸ�२�顼���Ϥ⤷����
    �Ķ��ѿ� MDEBUG_DUMP_FONT �ǻ��ꤵ�줿�ե�����˿ʹ֤˲��ɤʷ��ǽ�
    �Ϥ��롣

    @return
    ���δؿ��� $FONT ���֤���  */

MFont *
mdebug_dump_font (MFont *font)
{
  char *name;
  
  name = xlfd_unparse_name (font, 0);
  if (name)
    {
      fprintf (mdebug__output, "%s", name);
      free (name);
    }
  if (font->file != Mnil)
    {
      char *file = MSYMBOL_NAME (font->file);
      char *lastslash = file, *p;

      for (p = file; *p; p++)
	if (*p == '/')
	  lastslash = p;
      if (name)
	fprintf (mdebug__output, ",");
      fprintf (mdebug__output, "%s", lastslash + 1);
    }
  if (font->capability != Mnil)
    fprintf (mdebug__output, "%s", MSYMBOL_NAME (font->capability));
  return font;
}

/*** @} */

/*
  Local Variables:
  coding: euc-japan
  End:
*/
