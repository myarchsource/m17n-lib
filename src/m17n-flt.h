/* m17n-flt.h -- header file for the FLT API of the m17n library.
   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
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

#ifndef _M17N_FLT_H_
#define _M17N_FLT_H_

#ifndef _M17N_CORE_H_
#include <m17n-core.h>
#endif

M17N_BEGIN_HEADER

#if !defined (FOR_DOXYGEN) || defined (DOXYGEN_INTERNAL_MODULE)

extern void m17n_init_flt (void);
#undef M17N_INIT
#ifdef _M17N_H_
#define M17N_INIT()			\
  do {					\
    m17n_init ();			\
    if (merror_code == MERROR_NONE)	\
      m17n_init_flt ();			\
  } while (0)
#else  /* not _M17N_H_ */
#define M17N_INIT() m17n_init_flt ()
#endif	/* not _M17N_H_ */

extern void m17n_fini_flt (void);
#undef M17N_FINI
#ifdef _M17N_H_
#define M17N_FINI()	\
  do {			\
    m17n_fini_flt ();	\
    m17n_fini ();	\
  } while (0)
#else  /* not _M17N_H_ */
#define M17N_FINI() m17n_fini_flt ()
#endif	/* not _M17N_H_ */

#endif

/***en @defgroup m17nFLT FLT API
    @brief API provided by libm17n-flt.so */
/***ja @defgroup m17nFLT FLT API
    @brief libm17n-flt.so ���󶡤��� API */
/*=*/

/*** @addtogroup m17nFLT */
/*** @{ */
/*=*/

/***en
    @brief Type of information about a glyph.

    The type #MFLTGlyph is the structure that contains information
    about a glyph.  The members @ref c and @ref encoded are the
    members to be set appropriately before calling the functions
    mflt_find () and mflt_run ().  And, if @ref encoded is set to 1,
    the member @ref code should also be set.  */

/***ja
    @brief ����դ˴ؤ������η�.

    �� #MFLTGlyph �ϡ�����դ˴ؤ��������Ǽ���빽¤�ΤǤ��롣�ؿ�
    mflt_find () �� mflt_run () ��Ƥ����ˤϥ��С� \<c\> ��
    \<encoded\> ��Ŭ�ڤ����ꤷ�Ƥ����ͤФʤ餺���⤷ \<encoded\> �� 1
    �Ȥ������� \<code\> �����ꤷ�Ƥ����ͤФʤ�ʤ���  */

typedef struct
{
  /***en Character code (Unicode) of the glyph.  */
  /***ja ����դ� (Unicode �ˤ�����) ʸ�������ɡ�  */
  int c;
  /***en Glyph ID of the glyph in the font.  */
  /***ja �ե������ˤ����뤽�Υ���դ� ID��  */
  unsigned int code;
  /***en Starting index of the run in #MFLTGlyphString that is
      replaced by this glyph.  */
  /***ja #MFLTGlyphString
      ����ǡ����Υ���դˤ�ä��֤�����������ʬ����Ƭ�Υ���ǥ�����  */
  int from;
  /***en Ending index of the run in #MFLTGlyphString that is
      replaced by this glyph.  */
  /***ja #MFLTGlyphString
      ����ǡ����Υ���դˤ�ä��֤�����������ʬ�������Υ���ǥ�����  */
  int to;
  /***en Advance width for horizontal layout expressed in 26.6
      fractional pixel format.  */
  /***ja ���񤭻����������� 26.6 fractional pixel format ��ɽ��������Ρ�  */
  int xadv;
  /***en Advance height for vertical layout expressed in 26.6
      fractional pixel format.  */
  /***ja �Ľ񤭻��������� 26.6 fractional pixel format ��ɽ��������Ρ�  */
  int yadv;
  /* @{ */
  /***en Ink metrics of the glyph expressed in 26.6 fractional pixel
      format.  */
  /***ja ���Υ���դΥ��󥯥�ȥ�å��� 26.6 fractional pixel format
      ��ɽ��������Ρ�  */
  int ascent, descent, lbearing, rbearing;
  /* @} */
  /* @{ */
  /***en Horizontal and vertical adjustments for the glyph positioning
      expressed in 26.6 fractional pixel format.  */
  /***ja ����հ��ַ��κݤο�ʿ����ľĴ���ͤ�
      26.6 fractional pixel format ��ɽ��������Ρ�  */
  int xoff, yoff;
  /* @} */
  /***en Flag to tell whether the member \<code\> has already been set
      to a glyph ID in the font.  */
  /***ja ���С� \<code\> �˴��˥���� ID
      �����åȤ���Ƥ��뤫�ݤ��򼨤��ե饰��  */
  unsigned encoded : 1;
  /***en Flag to tell if the metrics of the glyph (members \<xadv\> thru
      \<rbearing\>) are already calculated.  */
  /***ja ���С� \<xadv\> ���� \<rbearing\>
      �ޤǤγƥ�ȥ�å������˷׻��Ѥ��ݤ��򼨤��ե饰��  */
  unsigned measured : 1;
  /***en Flag to tell if the metrics of the glyph is adjusted,
      i.e. \<xadv\> or \<yadv\> is different from the normal size, or
      \<xoff\> or \<yoff\> is nonzero.  */
  /***ja ����դΥ�ȥ�å���Ĵ���Ѥߤ��ݤ���
      ���ʤ���ʲ��Τ���1�İʾ夬��Ω���Ƥ��뤳�Ȥ򼨤��ե饰��
      \<xadv\> ��ɸ����ͤȰۤʤ롢
      \<yadv\> ��ɸ����ͤȰۤʤ롢
      \<xoff\> ������Ǥʤ���
      \<yoff\> ������Ǥʤ���  */
  unsigned adjusted : 1;
  /***en For m17n-lib's internal use only.  */
  /***ja m17n-lib ��������ѡ�  */
  unsigned internal : 30;

  /* Arbitrary data can follow.  */
} MFLTGlyph;

/*=*/

/***en
    @brief Type of information about a glyph position adjustment.

    The type #MFLTGlyphAdjustment is the structure to store
    information about a glyph metrics/position adjustment.  It is
    given to the callback function @b drive_otf of #MFLTFont.  */

/***ja
    @brief ����հ���Ĵ������Τ���η�.

    �� #MFLTGlyphAdjustment
    �ϡ�����դΥ�ȥ�å�/���֤�Ĵ���˴ؤ��������Ǽ���뤿��ι�¤�ΤǤ��ꡢ
    #MFLTFont �� callback �ؿ� @b drive_otf ���Ϥ���롣  */

typedef struct
{
  /* @{ */
  /***en Adjustments for advance width for horizontal layout and
      advance height for vertical layout expressed in 26.6 fractional
      pixel format.  */
  /***ja ��ʿ����ľ�����������̤�Ĵ���ͤ� 26.6 fractional pixel format
      ��ɽ��������Ρ�  */
  int xadv, yadv;
  /* @} */
  /* @{ */
  /***en Horizontal and vertical adjustments for glyph positioning
      expressed in 26.6 fractional pixel format.  */
  /***ja ����հ��ַ�᤿��ο�ʿ����ľĴ���ͤ� 26.6 fractional pixel
      format ��ɽ��������Ρ�  */
  int xoff, yoff;
  /* @} */
  /***en Number of glyphs to go back for drawing a glyph.  */
  /***ja ���������Τ�������٤�����տ���  */
  short back;
  /***en If nonzero, the member \<xadv\> and \<yadv\> are absolute, i.e.,
      they should not be added to a glyph's origianl advance width and
      height.  */
  /***ja �󥼥�ΤȤ������С� \<xadv\> �� \<yadv\> �������ͤǤ��롣
      ���ʤ�������ͤ򥰥��������������˲û����ƤϤʤ�ʤ���  */
  unsigned advance_is_absolute : 1;
  /***en Should be set to 1 iff at least one of the other members has
      a nonzero value.  */
  /***ja ¾�Υ��С��Τ�������1�Ĥ��󥼥�ΤȤ��Τߡ�1�˥��åȤ���롣  */
  unsigned set : 1;
} MFLTGlyphAdjustment;

/*=*/

/***en
    @brief Type of information about a glyph sequence.

    The type #MFLTGlyphString is the structure that contains
    information about a sequence of glyphs.  */

/***ja
    @brief �������ξ���Τ���η�.

    �� #MFLTGlyphString �ϡ��������ξ�����Ǽ���뤿��ι�¤�ΤǤ��롣  */

typedef struct
{
  /***en The actual byte size of elements of the array pointed by the
      member #glyphs.  It must be equal to or greater than "sizeof
      (MFLTGlyph)".  */
  /***ja ���С� #glyphs �λؤ���������Ǥ�����¥Х��ȿ���
      �����ͤ� "sizeof (MFLTGlyph)" �ʾ�Ǥʤ���Фʤ�ʤ���  */
  int glyph_size;
  /***en Array of glyphs.  */
  /***ja ����դ�����  */
  MFLTGlyph *glyphs;
  /***en Number of elements allocated in #glyphs.  */
  /***ja #glyphs ������֤���Ƥ������Ǥο���  */
  int allocated;
  /***en Number of elements in #glyphs in use.  */
  /***ja #glyphs ��ǻ���������Ǥο���  */
  int used;
  /***en Flag to tell if the glyphs should be drawn from right-to-left
      or not.  */
  /***ja ����դ������麸�ؤ��������٤����ݤ��򼨤��ե饰��  */
  unsigned int r2l;
} MFLTGlyphString;

/*=*/

/***en
    @brief Type of specification of GSUB and GPOS OpenType tables.

    The type #MFLTOtfSpec is the structure that contains information
    about the GSUB and GPOS features of a specific script and language
    system.  The information is used to select which features to
    apply to a glyph string, or to check if a specific FLT is usable
    for a specific font.  */

/***ja
    @brief GSUB ����� GPOS OpenType �ơ��֥�λ��ͤΤ���η�.

    �� #MFLTOtfSpec �ϡ�GSUB ����� GPOS�ե������㡼�ξ�����Ǽ���뤿
    ��ι�¤�ΤǤ��롣�����ե������㡼������Υ�����ץȤ���Ӹ��쥷
    ���ƥ�Τ�ΤǤ��롣���ξ���ϡ��ɤΥե������㡼�򥰥�����Ŭ�Ѥ�
    �뤫�����뤤������� FLT ������Υե���Ȥ��Ф���ͭ�����ɤ����η���
    �˻��Ѥ���롣  */

typedef struct
{
  /***en Unique symbol representing the spec.  This is the same as the
      @ref OTF-SPEC of the FLT.  */
  /***ja ���λ��ͤ�ɽ�魯��ˡ����ʥ���ܥ롣
      FLT �� @ref OTF-SPEC ��Ʊ����ͤǤ��롣  */
  MSymbol sym;

  /* @{ */
  /***en Tags for script and language system.  */
  /***ja ������ץȤ���Ӹ��쥷���ƥ�Υ�����  */
  unsigned int script, langsys;
  /* @} */

  /***en Array of GSUB (1st element) and GPOS (2nd element) feature
      tag arrays.  Each array is terminated by 0.  It may be NULL if
      there is no feature to specify.

      (1) The case of using this information for selecting which
      features to apply to a glyph string.  If the array is NULL,
      apply no feature.  If the first element is 0xFFFFFFFF, apply all
      available features except for what appear in the second and
      following elements (if any).  Otherwise, apply all listed
      features.

      (2) The case of using this information for checking if a a font
      can be drived by a specific FLT.  If the array is NULL, the font
      should not have any features.  Otherwize, the font should have
      all features before 0xFFFFFFFF element (if any) and should not
      have any features after that element.  */
  /***ja GSUB �ե������㡼�������������1���ǡ�GPOS �ե������㡼������
      �������2���ǤȤ������󡣳������������0�Ǽ�����롣�ե������㡼
      �λ��꤬1�Ĥ�ʤ����Ϥ�����������Ǥ� NULL �Ǥ�褤��

      (1) ���ξ��󤬥�������Ŭ�Ѥ��٤��ե������㡼������˻Ȥ����
      �硣�⤷���󼫿Ȥ�NULL�ʤ顢�ɤΥե������㡼��Ŭ�Ѥ��ʤ����⤷��
      ������Ǥ� 0xFFFFFFFF �ʤ顢�����ܰʹߤΥե������㡼�ʤ⤷����
      �Сˤ�������٤Ƥ�Ŭ�Ѳ�ǽ�ʥե������㡼��Ŭ�Ѥ��롣����ʳ��ξ�
      ��ꥹ�Ȥ��줿���٤ƤΥե������㡼��Ŭ�Ѥ��롣

      (2) ���ξ�������� FLT ������Υե���Ȥ�ͭ�����ɤ����η���˻�
      �����硣�⤷���󼫿Ȥ�NULL�ʤ顢�ե���Ȥϥե������㡼����
      ����äƤ��ƤϤ����ʤ����⤷�ǽ�����Ǥ�0xFFFFFFFF�ʤ顢�ե����
      �ϣ����ܤ����ǰʹߤΥե���Ȥ���äƤ��ƤϤ����ʤ�������ʳ��ξ�
      �硢�ե���Ȥ�0xFFFFFFFF �����Τ��٤ƤΥե������㡼�����������
      0xFFFFFFFF �ʹߤΥե������㡼�ϰ�Ĥ���äƤ��ƤϤ����ʤ���*/
  unsigned int *features[2];
} MFLTOtfSpec;

/*=*/

/***en
    @brief Type of font to be used by the FLT driver.

    The type #MFLTFont is the structure that contains information
    about a font used by the FLT driver.  Usually, an application
    should prepare a bigger structure whose first element is MFLTFont
    and has more information about the font that is used by callback
    funcitons, and give that structure to mflt functions by coercing
    it to MFLTFont.  It is assured that callback functions can safely
    coerce MFLTFont back to the original structure.  */

/***ja
    @brief FLT �ɥ饤�Ф��Ȥ��ե���Ȥη�.

    �� #MFLTFont �ϡ�FLT�ɥ饤�Ф��Ȥ��ե���Ȥ˴ؤ��������Ǽ���뤿
    ��ι�¤�ΤǤ��롣�̾異�ץꥱ�������Ϻǽ�����Ǥ� MFLTFont �ǡ�
    �Ĥ�����Ǥ�callback�ؿ������Ѥ���ե���Ⱦ������ä�������礭��
    ��¤�Τ��Ѱդ�������� MFLTFont �� coerce ���� mflt �γƴؿ����Ϥ���
    ��callback�ؿ��� MFLTFont �򸵤ι�¤�Τ� coerce ��ľ�����Ȥ��Ǥ���
    ���Ȥ��ݾڤ���Ƥ��롣 */

typedef struct _MFLTFont
{
  /***en Family name of the font.  It may be #Mnil if the family name
     is not important in finding a Font Layout Table suitable for the
     font (for instance, in the case that the font is an OpenType
     font).  */
  /***ja �ե���ȤΥե��ߥ꡼̾���ե���Ȥ�Ŭ���� FLT��õ���ݤ˽��פǤ�
      ����� (���Ȥ��� OpenType�ե���Ȥξ��ʤ�) �ϡ�#Mnil �Ǥ褤�� */
  MSymbol family;

  /***en Horizontal font sizes in pixels per EM.  */
  /***ja �ե���Ȥο�ʿ�������� pixels per EM ��ɽ��������Ρ�  */
  int x_ppem;
  /***en Vertical font sizes in pixels per EM.  */
  /***ja �ե���Ȥο�ľ�������� pixels per EM ��ɽ��������Ρ�  */
  int y_ppem;

  /***en Callback function to get glyph IDs for glyphs between FROM
     (inclusive) and TO (exclusive) of GSTRING.  If the member \<encoded\>
     of a glyph is zero, the member \<code\> of that glyph is a character
     code.  The function must convert it to the glyph ID of FONT.  */
  /***ja GSTRING ��� FROM ���� TO ľ���ޤǤγƥ���դ��б����륰���
      ID��������뤿��� callback �ؿ����⤷���륰��դΥ��С�
      \<encoded\>������ʤ�С����Υ���դΥ��С� \<code\> ��ʸ�������ɤ�
      ���롣���δؿ��Ϥ���ʸ�������ɤ� FONT �Υ���� ID���Ѵ����ʤ��Ƥ�
      �ʤ�ʤ���  */
  int (*get_glyph_id) (struct _MFLTFont *font, MFLTGlyphString *gstring,
		       int from, int to);

  /***en Callback function to get metrics of glyphs between FROM
     (inclusive) and TO (exclusive) of GSTRING.  If the member \<measured\>
     of a glyph is zero, the function must set the members \<xadv\>, \<yadv\>,
     \<ascent\>, \<descent\>, \<lbearing\>, and \<rbearing\> of the glyph.  */
  /***ja GSTRING ��� FROM ���� TOľ���ޤǤγƥ���դ��б������ȥ��
      ����������뤿��� callback �ؿ����⤷���륰��դΥ��С�
      \<measured\>������ʤ�С����δؿ��Ϥ��Υ���դΥ��С� \<xadv\>,
      \<yadv\>, \<ascent\>, \<descent\>, \<lbearing\>, ����� \<rbearing\>�򥻥�
      �Ȥ��ʤ���Фʤ�ʤ���  */
  int (*get_metrics) (struct _MFLTFont *font, MFLTGlyphString *gstring,
		     int from, int to);

  /***en Callback function to check if the font has OpenType GSUB/GPOS
     features for a specific script/language.  The function must
     return 1, if the font satisfies SPEC, or 0.  It must be
     NULL if the font does not have OpenType tables.  */
  /***ja �ե���Ȥ���������Υ�����ץ�/������Ф��� GSUB/GPOS
      OpenType�ե������㡼����Ĥ��ݤ���Ĵ�٤� callback �ؿ������δؿ�
      �ϥե���Ȥ�SPEC ���������Ȥ��� 1 �򡢤����Ǥʤ��Ȥ��� 0���֤���
      ����Фʤ�ʤ����ե���Ȥ� OpenType �ơ��֥������ʤ��Ȥ���NULL
      �Ǥʤ���Фʤ�ʤ���  */
  int (*check_otf) (struct _MFLTFont *font, MFLTOtfSpec *spec);

  /***en Callback function to apply OpenType features in SPEC to glyphs
     between FROM (inclusive) and TO (exclusive) of IN.  The resulting
     glyphs are appended to the tail of OUT.  If OUT does not
     have a room to store all the resulting glyphs, it must return -2.
     It must be NULL if the font does not have OpenType tables.  */
  /***ja IN ��� FROM ���� TO ľ���ޤǤγƥ���դ� SPEC��γ� OpenType
      �ե������㡼��Ŭ�Ѥ��뤿��� callback �ؿ���Ŭ�ѷ�̤Υ�������
      OUT ���������ɲä���롣OUT ��û���᤮�Ʒ�̤��ɲä��ڤ�ʤ����
      �� -2 ���֤��ʤ��ƤϤʤ�ʤ����ե���Ȥ� OpenType �ơ��֥�����
      �ʤ����� NULL�Ǥʤ���Фʤ�ʤ���  */
  int (*drive_otf) (struct _MFLTFont *font, MFLTOtfSpec *spec,
		    MFLTGlyphString *in, int from, int to,
		    MFLTGlyphString *out, MFLTGlyphAdjustment *adjustment);

  /***en For m17n-lib's internal use only.  It should be initialized
      to NULL.  */
  /***ja m17n-lib ����������ѡ�NULL �˽��Ͳ�����롣  */
  void *internal;
} MFLTFont;

/*=*/

/***en
    @brief Type of FLT (Font Layout Table).

    The type #MFLT is for an FLT object.  Its internal structure is
    concealed from application programs.  */

/***ja
    @brief FLT (Font Layout Table) �η�.

    �� #MFLT �� FLT ���֥������ȤΤ���η��Ǥ��롣
    ����������¤�ϡ����ץꥱ�������ץ���फ��ϱ��ä���Ƥ��롣  */

typedef struct _MFLT MFLT;

extern MFLT *mflt_get (MSymbol name);

extern MFLT *mflt_find (int c, MFLTFont *font);

extern const char *mflt_name (MFLT *flt);

extern MCharTable *mflt_coverage (MFLT *flt);

extern int mflt_run (MFLTGlyphString *gstring, int from, int to,
		     MFLTFont *font, MFLT *flt);

/*=*/
/*** @} */

extern int mflt_enable_new_feature;

extern MSymbol (*mflt_font_id) (MFLTFont *font);

extern int (*mflt_iterate_otf_feature) (MFLTFont *font,
					MFLTOtfSpec *spec,
					int from, int to,
					unsigned char *table);

extern int (*mflt_try_otf) (struct _MFLTFont *font, MFLTOtfSpec *spec,
			    MFLTGlyphString *gstring, int from, int to);

M17N_END_HEADER

#endif /* _M17N_FLT_H_ */

/*
  Local Variables:
  coding: euc-japan
  End:
*/
