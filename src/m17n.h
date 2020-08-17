/* m17n.h -- header file for the SHELL API of the m17n library.
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

#ifndef _M17N_H_
#define _M17N_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef _M17N_CORE_H_
#include <m17n-core.h>
#endif

M17N_BEGIN_HEADER

#if !defined (FOR_DOXYGEN) || defined (DOXYGEN_INTERNAL_MODULE)
extern void m17n_init (void);
#undef M17N_INIT
#ifdef _M17N_FLT_H_
#define M17N_INIT()	\
  do {			\
    m17n_init ();	\
    m17n_init_flt ();	\
  } while (0)
#else  /* not _M17N_FLT_H_ */
#define M17N_INIT() m17n_init ()
#endif	/* not _M17N_FLT_H_ */

extern void m17n_fini (void);
#undef M17N_FINI
#ifdef _M17N_FLT_H_
#define M17N_FINI()	\
  do {			\
    m17n_fini_flt ();	\
    m17n_fini ();	\
  } while (0)
#else  /* not _M17N_FLT_H_ */
#define M17N_FINI() m17n_fini ()
#endif	/* not _M17N_FLT_H_ */

#endif

/***en @defgroup m17nShell SHELL API
    @brief API provided by libm17n.so */
/***ja @defgroup m17nShell ������ API
    @brief libm17n.so ���󶡤��� API */
/*=*/

/*
 *  (11) Functions related to the m17n database
 */
/*=*/

/*=*/
/* (S2) Charset staffs */
/*=*/

/*** @ingroup m17nShell */
/***en @defgroup m17nCharset Charset */
/***ja @defgroup m17nCharset ʸ�����å� */
/*=*/

#if !defined (FOR_DOXYGEN) || defined (DOXYGEN_INTERNAL_MODULE)
#define MCHAR_INVALID_CODE 0xFFFFFFFF
#endif

/* Predefined charsets */ 
extern MSymbol Mcharset_ascii;
extern MSymbol Mcharset_iso_8859_1;
extern MSymbol Mcharset_unicode;
extern MSymbol Mcharset_m17n;
extern MSymbol Mcharset_binary;

/* Predefined keys for mchar_define_charset ().  */ 
extern MSymbol Mmethod;
extern MSymbol Mdimension;
extern MSymbol Mmin_range;
extern MSymbol Mmax_range;
extern MSymbol Mmin_code;
extern MSymbol Mmax_code;
extern MSymbol Mascii_compatible;
extern MSymbol Mfinal_byte;
extern MSymbol Mrevision;
extern MSymbol Mmin_char;
extern MSymbol Mmapfile;
extern MSymbol Mparents;
extern MSymbol Msubset_offset;
extern MSymbol Mdefine_coding;
extern MSymbol Maliases;

/* Methods of a charset.  */
extern MSymbol Moffset;
extern MSymbol Mmap;
extern MSymbol Munify;
extern MSymbol Msubset;
extern MSymbol Msuperset;

/* etc. */
extern MSymbol mchar_define_charset (const char *name, MPlist *plist);

extern MSymbol mchar_resolve_charset (MSymbol symbol);

extern int mchar_list_charset (MSymbol **symbols);

extern int mchar_decode (MSymbol charset_name, unsigned code);

extern unsigned mchar_encode (MSymbol charset_name, int c);

extern int mchar_map_charset (MSymbol charset_name,
			      void (*func) (int from, int to, void *arg),
			      void *func_arg);

/*=*/

/* (S3) code conversion */
/*=*/
/*** @ingroup m17nShell */
/***en @defgroup m17nConv Code Conversion */
/***ja @defgroup m17nConv �������Ѵ� */
/*=*/

/* Predefined coding systems */
extern MSymbol Mcoding_us_ascii;
extern MSymbol Mcoding_iso_8859_1;
extern MSymbol Mcoding_utf_8;
extern MSymbol Mcoding_utf_8_full;
extern MSymbol Mcoding_utf_16;
extern MSymbol Mcoding_utf_16be;
extern MSymbol Mcoding_utf_16le;
extern MSymbol Mcoding_utf_32;
extern MSymbol Mcoding_utf_32be;
extern MSymbol Mcoding_utf_32le;
extern MSymbol Mcoding_sjis;

/* Parameter keys for mconv_define_coding ().  */
extern MSymbol Mtype;
extern MSymbol Mcharsets;
extern MSymbol Mflags;
extern MSymbol Mdesignation;
extern MSymbol Minvocation;
extern MSymbol Mcode_unit;
extern MSymbol Mbom;
extern MSymbol Mlittle_endian;

/* Symbols representing coding system type.  */
extern MSymbol Mutf;
extern MSymbol Miso_2022;

/* Symbols appearing in the value of Mfrag parameter.  */
extern MSymbol Mreset_at_eol;
extern MSymbol Mreset_at_cntl;
extern MSymbol Meight_bit;
extern MSymbol Mlong_form;
extern MSymbol Mdesignation_g0;
extern MSymbol Mdesignation_g1;
extern MSymbol Mdesignation_ctext;
extern MSymbol Mdesignation_ctext_ext;
extern MSymbol Mlocking_shift;
extern MSymbol Msingle_shift;
extern MSymbol Msingle_shift_7;
extern MSymbol Meuc_tw_shift;
extern MSymbol Miso_6429;
extern MSymbol Mrevision_number;
extern MSymbol Mfull_support;

/* etc */
extern MSymbol Mcoding;
extern MSymbol Mmaybe;

/*** @ingroup m17nConv */
/***en
    @brief Codes that represent the result of code conversion.

    One of these values is set in @c MConverter-\>result.   */

/***ja
    @brief �������Ѵ��η�̤򼨤�������.

    �������ͤΤ�����Ĥ� @c MConverter-\>result �����ꤵ��롣  */

enum MConversionResult
  {
    /***en Code conversion is successful. */
    /***ja �������Ѵ�������. */
    MCONVERSION_RESULT_SUCCESS,

    /***en On decoding, the source contains an invalid byte. */
    /***ja �ǥ����ɻ����������������ʥХ��Ȥ��ޤޤ�Ƥ���. */
    MCONVERSION_RESULT_INVALID_BYTE,

    /***en On encoding, the source contains a character that cannot be
	encoded by the specified coding system. */
    /***ja ���󥳡��ɻ�������Υ����ɷϤǥ��󥳡��ɤǤ��ʤ�ʸ�����������˴ޤޤ�Ƥ���. */
    MCONVERSION_RESULT_INVALID_CHAR,

    /***en On decoding, the source ends with an incomplete byte sequence. */
    /***ja �ǥ����ɻ����Դ����ʥХ�����ǥ�����������äƤ��롣*/
    MCONVERSION_RESULT_INSUFFICIENT_SRC,

    /***en On encoding, the destination is too short to store the result. */
    /***ja ���󥳡��ɻ�����̤��Ǽ�����ΰ褬û�������롣 */
    MCONVERSION_RESULT_INSUFFICIENT_DST,

    /***en An I/O error occurred in the conversion.  */
    /***ja �������Ѵ���� I/O ���顼�������ä���  */
    MCONVERSION_RESULT_IO_ERROR
  };
/*=*/

/*** @ingroup m17nConv */
/***en
    @brief Structure to be used in code conversion.

    Structure to be used in code conversion.  The first three members
    are to control the conversion.  */

/***ja
    @brief �������Ѵ����Ѥ����빽¤��.

    �������Ѵ����Ѥ����빽¤�Ρ��ǽ�Σ��ĤΥ��Ф��Ѵ������椹�롣

    @latexonly \IPAlabel{MConverter} @endlatexonly  
*/

typedef struct
{
  /***en
      Set the value to nonzero if the conversion should be lenient.
      By default, the conversion is strict (i.e. not lenient).

      If the conversion is strict, the converter stops at the first
      invalid byte (on decoding) or at the first character not
      supported by the coding system (on encoding).  If this happens,
      @c MConverter-\>result is set to @c
      MCONVERSION_RESULT_INVALID_BYTE or @c
      MCONVERSION_RESULT_INVALID_CHAR accordingly.

      If the conversion is lenient, on decoding, an invalid byte is
      kept per se, and on encoding, an invalid character is replaced
      with "<U+XXXX>" (if the character is a Unicode character) or
      with "<M+XXXXXX>" (otherwise).  */

  /***ja 
      ��̩���Ѵ���ɬ�פǤʤ������ͤ� 0 �ʳ��ˤ��롣
      �ǥե���ȤǤϡ��Ѵ��ϸ�̩�Ǥ��롣

      �Ѵ�����̩�Ȥϡ��ǥ����ɤκݤˤϺǽ�������ʥХ��Ȥǥ���С������ߤޤ뤳�ȡ�
      ���󥳡��ɤκݤˤ��Ѵ�����륳���ɷϤǥ��ݡ��Ȥ���ʤ��ǽ��ʸ���ǥ���С������ߤޤ뤳�Ȥ�ؤ���
      �����ξ�硢@c MConverter-\>result �Ϥ��줾�� @c
      MCONVERSION_RESULT_INVALID_BYTE ��@c
      MCONVERSION_RESULT_INVALID_CHAR �Ȥʤ롣

      �Ѵ�����̩�Ǥʤ����ˤϡ��ǥ����ɤκݤ������ʥХ��ȤϤ��ΥХ��ȤΤޤ޻Ĥ롣
      �ޤ����󥳡��ɤκݤˤϡ�������ʸ���� Unicode ʸ���ξ��ˤ� "<U+XXXX>"
      �ˡ������Ǥʤ����ˤ� "<M+XXXXXX>" ���֤��������롣  */

  int lenient;

  /***en
      Set the value to nonzero before decoding or encoding the last
      block of the byte sequence or the character sequence
      respectively.  The value influences the conversion as below.

      On decoding, in the case that the last few bytes are too short
      to form a valid byte sequence:

	If the value is nonzero, the conversion terminates by error
	(MCONVERSION_RESULT_INVALID_BYTE) at the first byte of the
	sequence.

	If the value is zero, the conversion terminates successfully.
	Those bytes are stored in the converter as carryover and are
	prepended to the byte sequence of the further conversion.

      On encoding, in the case that the coding system is context
      dependent:

	If the value is nonzero, the conversion may produce a byte
	sequence at the end to reset the context to the initial state
	even if the source characters are zero.

	If the value is zero, the conversion never produce such a byte
	sequence at the end.  */

/***ja 

     �Х�����ν�ü�Υ֥�å���ǥ����ɤ���ݡ��ޤ���ʸ����ν�ü�Υ֥�å��򥨥󥳡��ɤ���ݤϡ��ͤ�
     0 �ʳ��ˤ��롣�����ͤϰʲ��Τ褦���Ѵ��˱ƶ����롣

     �ǥ����ǥ��󥰤κݤ˺Ǹ�ο��Х��Ȥ��������Х��ȥ��������󥹤��������ˤ�û�������硧

       �ͤ� 0 �Ǥʤ���С��Ѵ��Ϥ��Υ��������󥹤κǽ�ΥХ��Ȥˤ����ơ����顼
       (MCONVERSION_RESULT_INVALID_BYTE) �ǽ��롣

       �ͤ� 0 �ʤ�С��Ѵ����������ƽ��롣
       ����ο��Х��Ȥϥ���꡼�����ФȤ��ƥ���С������ݻ����졢�Ѵ���³����Ԥ��ݤ��Ѵ�����Х�����������դ����롣

     ���󥳡��ǥ��󥰤κݤ˥����ɷϤ�ʸ̮��¸�ξ�硢

       �ͤ� 0 �Ǥʤ���С�����ƥ����Ȥ�ǽ���᤹����ΥХ����󤬥�������ʸ���Ȥ������ʤ��Ѵ��η����������뤳�Ȥ����롣

       �ͤ� 0 �ʤ�С����Τ褦�ʥХ��������������ʤ���

	 */

  int last_block;

  /***en
      If the value is nonzero, it specifies at most how many
      characters to convert.  */
  /***ja
      0 �Ǥʤ���С��Ѵ����������ʸ��������ꤹ�롣
      */

  unsigned at_most;

  /***en
      The following three members are to report the result of the
      conversion.  */
  /***ja
      �ʲ��Σ��ĤΥ��Ф��Ѵ��η�̤�ɽ������Τ�ΤǤ��롣  */

  /***en
      Number of characters most recently decoded or encoded. */
  /***ja
      �Ƕ�˥ǥ�����/���󥳡��ɤ��줿ʸ������ */

  int nchars;

  /***en
      Number of bytes recently decoded or encoded. */

  /***ja
      �Ƕ�˥ǥ�����/���󥳡��ɤ��줿�Х��ȿ��� */

  int nbytes;

  /***en
      Result code of the conversion. */

  /***ja
      �������Ѵ��η�̤򼨤������ɡ� */

  enum MConversionResult result;

  /***en
      Various information about the status of code conversion.  The
      contents depend on the type of coding system.  It is assured
      that @c status is aligned so that any type of casting is safe
      and at least 256 bytes of memory space can be used.  */

  /***ja
      �������Ѵ��ξ����˴ؤ����ξ������Ƥϥ����ɷϤΥ����פˤ�äưۤʤ롣
      @c status �ϤɤΤ褦�ʷ��ؤΥ��㥹�Ȥ��Ф��Ƥ�����ʤ褦�˥��ꥢ�饤�󤵤�Ƥ��ꡢ�ޤ�����256�Х��ȤΥ����ΰ褬�Ȥ���褦�ˤʤäƤ��롣  */

  union {
    void *ptr;
    double dbl;
    char c[256];
  } status;

  /***en
      This member is for internally use only.  An application program
      should never touch it.  */
  /***ja ���Υ��Ф�����Ū�˻��Ѥ��졢���ץꥱ�������ץ����Ͽ���ƤϤʤ�ʤ��� */
  void *internal_info;
} MConverter;
/*=*/

/*** @ingroup m17nConv */
/***en 
    @brief Types of coding system.  */
/***ja 
    @brief �����ɷϤΥ�����.  */

enum MCodingType
  {
    /***en
	A coding system of this type supports charsets directly.
	The dimension of each charset defines the length of bytes to
	represent a single character of the charset, and a byte
	sequence directly represents the code-point of a character.
	The m17n library provides the default decoding and encoding
	routines of this type.  */

    /***ja
	���Υ����פΥ����ɷϤ�ʸ�����åȤ�ľ�ܥ��ݡ��Ȥ��롣
	��ʸ�����åȤμ����Ȥϡ�����ʸ�����åȤǰ�ʸ����ɽ�����뤿���ɬ�פʥХ��ȿ��Ǥ��ꡢ�Х������ʸ���Υ����ɥݥ���Ȥ�ľ��ɽ����
	m17n �饤�֥��Ϥ��Υ������ѤΥǥե���ȤΥ��󥳡��ɡ��ǥ����ɥ롼�ƥ�����󶡤��롣  */

    MCODING_TYPE_CHARSET,

    /***en
	A coding system of this type supports byte sequences of a
	UTF (UTF-8, UTF-16, UTF-32) like structure.
	The m17n library provides the default decoding and encoding
	routines of this type.  */

    /***ja
	���Υ����פΥ����ɷϤϡ�UTF �� (UTF-8, UTF-16, UTF-32) �ΥХ�����򥵥ݡ��Ȥ��롣
	m17n �饤�֥��Ϥ��Υ������ѤΥǥե���ȤΥ��󥳡��ɡ��ǥ����ɥ롼�ƥ�����󶡤��롣  */

    MCODING_TYPE_UTF,

    /***en
	A coding system of this type supports byte sequences of an
	ISO-2022 like structure.  The details of each structure are
	specified by @c MCodingInfoISO2022 .
	The m17n library provides decoding and encoding routines of
	this type.  */

    /***ja 
        ���Υ����פΥ����ɷϤϡ�ISO-2022 �ϤΥХ�����򥵥ݡ��Ȥ��롣
	�ƥ����ɷϤι�¤�ξܺ٤� @c MCodingInfoISO2022 �ǻ��ꤵ��롣
	m17n �饤�֥��Ϥ��Υ������ѤΥǥե���ȤΥ��󥳡��ɡ��ǥ����ɥ롼�ƥ�����󶡤��롣  */

    MCODING_TYPE_ISO_2022,

    /***en
	A coding system of this type is for byte sequences of
	miscellaneous structures.
	The m17n library does not provide decoding and encoding
	routines of this type.  They must be provided by the
	application program.  */

    /***ja
	���Υ����פΥ����ɷϤϡ�����¾�ι�¤�ΥХ�����Τ���Τ�ΤǤ��롣
	m17n �饤�֥��Ϥ��Υ������ѤΥ��󥳡��ɡ��ǥ����ɥ롼�ƥ�����󶡤��ʤ��Τǡ����ץꥱ�������ץ����¦�ǽ�������ɬ�פ����롣  */

    MCODING_TYPE_MISC
  };
/*=*/

/*** @ingroup m17nConv */
/***en 
    @brief  Bit-masks to specify the detail of coding system whose type is MCODING_TYPE_ISO_2022.  */
/***ja 
    @brief MCODING_TYPE_ISO_2022 �����פΥ����ɷϤξܺ٤�ɽ�魯�ӥåȥޥ���.  */

enum MCodingFlagISO2022
  {
    /***en
	On encoding, reset the invocation and designation status to
	initial at end of line.  */
    /***ja ���󥳡��ɤκݡ������ǸƤӽФ� (invocation) �Ȼؼ�
	(designation) �ξ��֤����ͤ��᤹��   */
    MCODING_ISO_RESET_AT_EOL =		0x1,

    /***en
	On encoding, reset the invocation and designation status to
	initial before any control codes.  */
    /***ja
	���󥳡��ɤκݡ����٤Ƥ�����ʸ�������ǡ��ƤӽФ�
	(invocation) �Ȼؼ� (designation) �ξ��֤����ͤ��᤹��        */
    MCODING_ISO_RESET_AT_CNTL =		0x2,

    /***en
	Use the right graphic plane.  */
    /***ja
	�޷�ʸ������α�¦��Ȥ���  */
    MCODING_ISO_EIGHT_BIT =		0x4,

    /***en
	Use the non-standard 4 bytes format for designation sequence
	for charsets JISX0208-1978, GB2312, and JISX0208-1983.  */
    /***ja
	JISX0208-1978, GB2312, JISX0208-1983 
	��ʸ��������Ф���ؼ����������󥹤Ȥ��ơ���ɸ���4�Х��ȷ������Ѥ��롣 */

    MCODING_ISO_LONG_FORM =		0x8,

    /***en
	On encoding, unless explicitly specified, designate charsets
	to G0.  */
    /***ja 
        ���󥳡��ɤκݡ��ä˻��ꤵ��ʤ��¤ꡢʸ������� G0 
	�˻ؼ����롣*/
    MCODING_ISO_DESIGNATION_G0 =		0x10,

    /***en
	On encoding, unless explicitly specified, designate charsets
	except for ASCII to G1.  */
    /***ja
	���󥳡��ɤκݡ��ä˻��ꤵ��ʤ��¤ꡢASCII �ʳ���ʸ������� 
	G1 �˻ؼ����롣*/
    MCODING_ISO_DESIGNATION_G1 =		0x20,

    /***en
	On encoding, unless explicitly specified, designate 94-chars
	charsets to G0, 96-chars charsets to G1.  */
    /***ja
	���󥳡��ɤκݡ��ä˻��ꤵ��ʤ��¤ꡢ94ʸ������� G0 
	�ˡ�96ʸ������� G1 �˻ؼ����롣*/
    MCODING_ISO_DESIGNATION_CTEXT =	0x40,

    /***en
	On encoding, encode such charsets not conforming to ISO-2022
	by ESC % / ..., and encode non-supported Unicode characters by
	ESC % G ... ESC % @@ .  On decoding, handle those escape
	sequences.  */
    /***ja
	���󥳡��ɤκݡ�ISO-2022 �˹��פ��ʤ�ʸ������� ESC % / ... 
	�ǥ��󥳡��ɤ��롣���ݡ��Ȥ���Ƥ��ʤ� Unicode ʸ���� ESC % G ...
	ESC % @@ �ǥ��󥳡��ɤ��롣
	�ǥ����ɤκݡ������Υ��������ס��������󥹤��᤹�롣  */
    MCODING_ISO_DESIGNATION_CTEXT_EXT =	0x80,

    /***en
	Use locking shift.  */
    /***ja
	��å��󥰥��եȤ�Ȥ���  */
    MCODING_ISO_LOCKING_SHIFT =	0x100,

    /***en
	Use single shift (SS2 (0x8E or ESC N), SS3 (0x8F or ESC O)).  */
    /***ja
	���󥰥륷�ե� (SS2 (0x8E or ESC N), SS3 (0x8F or ESC O)) ��Ȥ���  */
    MCODING_ISO_SINGLE_SHIFT =	0x200,

    /***en
	Use 7-bit single shift 2 (SS2 (0x19)).  */
    /***ja
	7�ӥåȥ��󥰥륷�ե� 2 (SS2 (0x19)) ��Ȥ���  */
    MCODING_ISO_SINGLE_SHIFT_7 =	0x400,

    /***en
	Use EUC-TW like special shifting.  */
    /***ja
	EUC-TW �������̤ʥ��եȤ�Ȥ���  */
    MCODING_ISO_EUC_TW_SHIFT =	0x800,

    /***en
	Use ISO-6429 escape sequences to indicate direction.
	Not yet implemented.  */
    /***ja
	ISO-6429 �Υ��������ץ��������󥹤�������ؼ����롣̤������ */
    MCODING_ISO_ISO6429 =		0x1000,

    /***en
	On encoding, if a charset has revision number, produce escape
	sequences to specify the number.  */
    /***ja
	���󥳡��ɤκݡ�ʸ�����åȤ� revision number 
	������Ф����ɽ�魯���������ץ��������󥹤��������롣        */
    MCODING_ISO_REVISION_NUMBER =	0x2000,

    /***en
	Support all ISO-2022 charsets.  */
    /***ja
	ISO-2022 ����ʸ������򥵥ݡ��Ȥ��롣  */
    MCODING_ISO_FULL_SUPPORT =		0x3000,

    MCODING_ISO_FLAG_MAX
  };
/*=*/

/*** @ingroup m17nConv */
/***en
    @brief Structure for a coding system of type #MCODING_TYPE_ISO_2022.

    Structure for extra information about a coding system of type
    MCODING_TYPE_ISO_2022.  */

/***ja 
    @brief #MCODING_TYPE_ISO_2022 �����פΥ����ɷϤ�ɬ�פ��ղþ����ѹ�¤��.

    MCODING_TYPE_ISO_2022 �����פΥ����ɷϤ�ɬ�פ��ղþ����Ѥ��ݻ����뤿��ι�¤�Ρ�

    @latexonly \IPAlabel{MCodingInfoISO2022} @endlatexonly  */

typedef struct
{
  /***en
      Table of numbers of an ISO2022 code extension element invoked
      to each graphic plane (Graphic Left and Graphic Right).  -1
      means no code extension element is invoked to that plane.  */

  /***ja
      �ƿ޷�ʸ���ΰ� (Graphic Left �� Graphic Right) �˸ƤӽФ���Ƥ��롢
      ISO2022 ����ĥ���Ǥ��ֹ�Υơ��֥롣-1 
      �Ϥ����ΰ�ˤɤ�����ĥ���Ǥ�ƤӽФ���Ƥ��ʤ����Ȥ򼨤���   */

  int initial_invocation[2];

  /***en
      Table of code extension elements.  The Nth element corresponds
      to the Nth charset in $CHARSET_NAMES, which is an argument given
      to the mconv_define_coding () function.

      If an element value is 0..3, it specifies a graphic register
      number to designate the corresponds charset.  In addition, the
      charset is initially designated to that graphic register.

      If the value is -4..-1, it specifies a graphic register number
      0..3 respectively to designate the corresponds charset.
      Initially, the charset is not designated to any graphic
      register.  */

  /***ja
      ����ĥ���ǤΥơ��֥롣N���ܤ����Ǥϡ�$CHARSET_NAMES �� N 
      ���ܤ�ʸ�����åȤ��б����롣$CHARSET_NAMES �ϴؿ� 
      mconv_define_coding () �ΰ����Ȥʤ롣

      �ͤ� 0..3 ���ä��顢�б�����ʸ�����åȤ� G0..G3 
      �Τ��줾��˻ؼ����뤳�ȡ��ޤ�������֤Ǥ��Ǥ� G0..G3
      �˻ؼ�����Ƥ��뤳�Ȥ��̣���롣

      �ͤ� -4..-1 ���ä��顢�б�����ʸ�����åȤ� G0..G3 
      �Τ��줾��˻ؼ����뤳�ȡ�������������֤ǤϤɤ��ˤ�ؼ�����Ƥ��ʤ����Ȥ��̣���롣
        */

  char designations[32];

  /***en
      Bitwise OR of @c enum @c MCodingFlagISO2022 .  */

  /***ja
      @c enum @c MCodingFlagISO2022 �Υӥå�ñ�̤Ǥ����� OR  */

  unsigned flags;

} MCodingInfoISO2022;
/*=*/

/*** @ingroup m17nConv */
/***en
    @brief Structure for extra information about a coding system of type #MCODING_TYPE_UTF.
    */

/***ja
    @brief #MCODING_TYPE_UTF �����פΥ����ɷϤ�ɬ�פ��ղþ����Ѥι�¤��.

    @latexonly \IPApage{MCodingInfoUTF} @endlatexonly

    @latexonly \IPAlabel{MCodingInfoUTF} @endlatexonly  */

typedef struct
{
  /***en
      Specify bits of a code unit.  The value must be 8, 16, or 32.  */
  /***ja
      ������Ĺ�ʥӥåȿ��ˤλ��ꡣ�ͤ� 8, 16, 32 �Τ����줫��  */
  int code_unit_bits;

  /***en
      Specify how to handle the heading BOM (byte order mark).  The
      value must be 0, 1, or 2.  The meanings are as follows:

      0: On decoding, check the first two byte.  If they are BOM,
      decide endian by them. If not, decide endian by the member @c
      endian.  On encoding, produce byte sequence according to
      @c endian with heading BOM.

      1: On decoding, do not handle the first two bytes as BOM, and
      decide endian by @c endian.  On encoding, produce byte sequence
      according to @c endian without BOM.

      2: On decoding, handle the first two bytes as BOM and decide
      ending by them.  On encoding, produce byte sequence according to
      @c endian with heading BOM.

      If \<code_unit_bits\> is 8, the value has no meaning.  */

  /***ja
      ��Ƭ�� BOM (�Х��ȥ��������ޡ���) �μ�갷������ꤹ�롣�ͤ� 0,
      1, 2 �Τ����줫�Ǥ��ꡢ���줾��ΰ�̣�ϰʲ��Τ褦�ˤʤ롣

      0: �ǥ����ɤκݤ˺ǽ�� 2 �Х��Ȥ�Ĵ�٤롣�⤷���줬 BOM 
      �Ǥ���С�����ǥ�����򤽤��Ƚ�ꤹ�롣�����Ǥʤ���С����� @c
      endian �˽��äƥ���ǥ��������ꤹ�롣���󥳡��ɤκݤˤ� @c
      endian �˽��ä��Х��������Ƭ�� BOM �դ��������롣

      1: �ǥ����ɤκݡ��ǽ�� 2 �Х��Ȥ� BOM �Ȥ��ư��鷺������ǥ������
      @c endian ��Ƚ�ꤹ�롣���󥳡��ɤκݤˤϡ�BOM ����Ϥ�����
      @c endian �˱������Х�������������롣

      2: �ǥ����ɤκݤ˺ǽ��2�Х��Ȥ� BOM�Ȥ��ư���������˽��äƥ���ǥ������Ƚ�ꤹ�롣
      ���󥳡��ɤκݤˤ� @c endian �˱������Х��������Ƭ�� BOM �դ����������롣  */
  int bom;

  /***en
      Specify the endian type.  The value must be 0 or 1.  0 means
      little endian, and 1 means big endian.

      If \<code_unit_bits\> is 8, the value has no meaning.  */
  /***ja
      ����ǥ�����Υ����פ���ꤹ�롣�ͤ� 0 �� 1 �Ǥ��ꡢ0 
      �ʤ�Х�ȥ륨��ǥ�����1 �ʤ�Хӥå�����ǥ�����Ǥ��롣

      \<code_unit_bits\> �� 8 �ξ��ˤϡ������ͤϰ�̣������ʤ���
  */
  int endian;
} MCodingInfoUTF;
/*=*/

extern MSymbol mconv_define_coding (const char *name, MPlist *plist,
				    int (*resetter) (MConverter *),
				    int (*decoder) (const unsigned char *, int,
						    MText *, MConverter *),
				    int (*encoder) (MText *, int, int,
						    unsigned char *, int,
						    MConverter *),
				    void *extra_info);

extern MSymbol mconv_resolve_coding (MSymbol symbol);

extern int mconv_list_codings (MSymbol **symbols);

extern MConverter *mconv_buffer_converter (MSymbol coding,
					   const unsigned char *buf,
					   int n);

extern MConverter *mconv_stream_converter (MSymbol coding, FILE *fp);

extern int mconv_reset_converter (MConverter *converter);

extern void mconv_free_converter (MConverter *converter);

extern MConverter *mconv_rebind_buffer (MConverter *converter,
					const unsigned char *buf, int n);

extern MConverter *mconv_rebind_stream (MConverter *converter, FILE *fp);

extern MText *mconv_decode (MConverter *converter, MText *mt);

MText *mconv_decode_buffer (MSymbol name, const unsigned char *buf, int n);

MText *mconv_decode_stream (MSymbol name, FILE *fp);   

extern int mconv_encode (MConverter *converter, MText *mt);

extern int mconv_encode_range (MConverter *converter, MText *mt,
			       int from, int to);

extern int mconv_encode_buffer (MSymbol name, MText *mt,
				unsigned char *buf, int n);

extern int mconv_encode_stream (MSymbol name, MText *mt, FILE *fp);

extern int mconv_getc (MConverter *converter);

extern int mconv_ungetc (MConverter *converter, int c);

extern int mconv_putc (MConverter *converter, int c);

extern MText *mconv_gets (MConverter *converter, MText *mt);

/* (S4) Locale related functions corresponding to libc functions */
/*=*/
/*** @ingroup m17nShell */
/***en @defgroup m17nLocale Locale */
/***ja @defgroup m17nLocale ������ */
/*=*/

/*** @ingroup m17nLocale */
/***en
    @brief @c struct @c MLocale.

    The structure @c MLocale is used to hold information about name,
    language, territory, modifier, codeset, and the corresponding
    coding system of locales.

    The contents of this structure are implementation dependent.  Its
    internal structure is concealed from application programs.  */

/***ja
    @brief @c MLocale ��¤��.

    @c MLocale ��¤�Τϡ��������̾�������졢�ϰ衢��ǥ��ե������������ɥ��åȡ�������б����륳���ɷϤ˴ؤ��������ݻ����뤿����Ѥ����롣

    ���ι�¤�Τ����Ƥϼ����˰�¸���롣 
    ������¤�ϥ��ץꥱ�������ץ���फ��ϸ����ʤ���  */

/***
    @seealso
    mlocale_get_prop () */

typedef struct MLocale MLocale;

/*=*/

extern MSymbol Miso639_1, Miso639_2;
extern MSymbol Mterritory;
extern MSymbol Mmodifier;
extern MSymbol Mcodeset;

extern MPlist *mlanguage_list (void);

extern MSymbol mlanguage_code (MSymbol language, int len);

extern MPlist *mlanguage_name_list (MSymbol language, MSymbol target,
				    MSymbol script, MSymbol territory);

extern MText *mlanguage_text (MSymbol language);

extern MPlist *mscript_list (void);

extern MPlist *mscript_language_list (MSymbol script);

extern MSymbol mlanguage_name (MSymbol language);

extern MLocale *mlocale_set (int category, const char *locale);

extern MSymbol mlocale_get_prop (MLocale *locale, MSymbol key);

extern int mtext_ftime (MText *mt, const char *format, const struct tm *tm,
			MLocale *locale);

extern MText *mtext_getenv (const char *name);

extern int mtext_putenv (MText *mt);

extern int mtext_coll (MText *mt1, MText *mt2);

/*
 *  (9) Miscellaneous functions of libc level (not yet implemented)
 */

/*
extern int mtext_width (MText *mt, int n);
extern MText *mtext_tolower (MText *mt);
extern MText *mtext_toupper (MText *mt);
*/

/*
 *  (10) Input method
 */
/*=*/
/*** @ingroup m17nShell */
/***en @defgroup m17nInputMethod Input Method (basic) */
/***ja @defgroup m17nInputMethod ���ϥ᥽�å� (������ʬ) */
/*=*/

/*** @addtogroup m17nInputMethod 
     @{ */
/*=*/
/* Struct forward declaration.  */
/***
    @brief See struct MInputMethod */
typedef struct MInputMethod MInputMethod;
/*=*/
/***
    @brief See struct MInputContext */
typedef struct MInputContext MInputContext;
/*=*/

/***en
    @brief Type of input method callback functions.

    This is the type of callback functions called from input method
    drivers.  $IC is a pointer to an input context, $COMMAND is a name
    of callback for which the function is called.   */
/***ja
    @brief ���ϥ᥽�åɥ�����Хå��ؿ��η����.

    ���ϥ᥽�åɤ���ƤФ�륳����Хå��ؿ��η��Ǥ��롣$IC 
    �����ϥ���ƥ����ȤؤΥݥ��󥿡�$COMMAND �ϴؿ����ƤФ�륳����Хå���̾���Ǥ��롣  */

typedef void (*MInputCallbackFunc) (MInputContext *ic, MSymbol command);
/*=*/

/***en
    @brief Structure of input method driver.

    The type @c MInputDriver is the structure of an input method driver that
    contains several functions to handle an input method.  */

/***ja
    @brief ���ϥɥ饤���ѹ�¤��.

    @c MInputDriver �ϡ����ϥ᥽�åɤ��갷���ؿ���ޤ����ϥ᥽�åɥɥ饤�Фι�¤�Τη��Ǥ��롣  */

typedef struct MInputDriver
{
  /***en
      @brief Open an input method.

      This function opens the input method $IM.  It is called from the
      function minput_open_im () after all member of $IM but \<info\>
      set.  If opening $IM succeeds, it returns 0.  Otherwise, it
      returns -1.  The function can setup $IM->info to keep various
      information that is referred by the other driver functions.  */

  /***ja
      @brief ���ϥ᥽�åɤ򥪡��ץ󤹤�.

      ���δؿ��ϡ����ϥ᥽�å� $IM �򥪡��ץ󤹤롣$IM �� \<info\> 
      �ʳ��������С������åȤ��줿��ǡ��ؿ� minput_open_im () 
      ����ƤФ�롣$IM �򥪡��ץ�Ǥ���� 0 �򡢤Ǥ��ʤ���� -1���֤���
      ���δؿ��� $IM->info 
      �����ꤷ�ơ�¾�Υɥ饤�дؿ����黲�Ȥ���������ݻ����뤳�Ȥ��Ǥ��롣
      */

  int (*open_im) (MInputMethod *im);

  /***en
      @brief Close an input method.

      This function closes the input method $IM.  It is called from
      the function minput_close_im ().  It frees all memory allocated
      for $IM->info (if any) after finishing all the tasks of closing
      the input method.  But, the other members of $IM should not be
      touched.  */

  /***ja
      @brief ���ϥ᥽�åɤ򥯥�������.

      ���δؿ��ϡ����ϥ᥽�å� $IM �򥯥������롣�ؿ� 
      minput_close_im () ����ƤФ�롣
      ���ϥ᥽�åɤΥ����������٤ƽ�λ���������ǡ����δؿ���$IM->info 
      �˳�����Ƥ��Ƥ�������(�����)���٤Ƴ������롣
      ��������$IM ��¾�Υ��Ф˱ƶ���Ϳ���ƤϤʤ�ʤ���
        */

  void (*close_im) (MInputMethod *im);

  /***en
      @brief Create an input context.

      This function creates the input context $IC.  It is called from
      the function minput_create_ic () after all members of $IC but
      \<info\> are set.  If creating $IC succeeds, it returns 0.
      Otherwise, it returns -1.  The function can setup $IC->info to
      keep various information that is referred by the other driver
      functions.  */

  /***ja
      @brief ���ϥ���ƥ����Ȥ���������.

      ���δؿ������ϥ���ƥ����� $IC ���������롣
      $IC �� \<info\> �ʳ��������С������åȤ��줿��ǡ��ؿ�
      minput_create_ic () ����ƤФ�롣
      $IC �������Ǥ���� 0 �򡢤Ǥ��ʤ���� -1 ���֤���
      ���δؿ��� $IC->info �����ꤷ�ơ�¾�Υɥ饤�дؿ����黲�Ȥ���������ݻ����뤳�Ȥ��Ǥ��롣  */


  int (*create_ic) (MInputContext *ic);

  /***en
      @brief Destroy an input context.

      This function is called from the function minput_destroy_ic ()
      and destroys the input context $IC.  It frees all memory
      allocated for $IC->info (if any) after finishing all the tasks
      of destroying the input method.  But, the other members of $IC
      should not be touched.  */

  /***ja
      @brief ���ϥ���ƥ����Ȥ��˲�����.

      �ؿ� minput_destroy_ic () ����ƤФ졢���ϥ���ƥ����� $IC 
      ���˲����롣���ϥ���ƥ����Ȥ��˲������٤ƽ�λ���������ǡ�$IC->info 
      �˳�����Ƥ��Ƥ�������(�����)���٤Ƴ������롣��������$IC 
      ��¾�Υ��Ф˱ƶ���Ϳ���ƤϤʤ�ʤ���  */

  void (*destroy_ic) (MInputContext *ic);

  /***en
      @brief Filter an input key.

      This function is called from the function minput_filter () and
      filters an input key.  $KEY and $ARG are the same as what given
      to minput_filter ().

      The task of the function is to handle $KEY, update the internal
      state of $IC.  If $KEY is absorbed by the input method and no
      text is produced, it returns 1.  Otherwise, it returns 0.

      It may update $IC->status, $IC->preedit, $IC->cursor_pos,
      $IC->ncandidates, $IC->candidates, and $IC->produced if that is
      necessary for the member \<callback\>.

      The meaning of $ARG depends on the input method river.  See the
      documentation of @c minput_default_driver and @c
      minput_gui_driver for instance.  */

  /***ja
      @brief ���ϥ�����ե��륿����.

      �ؿ� minput_filter () ����ƤФ졢���ϥ�����ե��륿���롣���� 
      $KEY, $ARG �ϴؿ� minput_filter () �Τ�Τ�Ʊ����

      ���δؿ��� $KEY ���������$IC ���������֤򹹿����롣 $KEY 
      �����ϥ᥽�åɤ˵ۼ�����ƥƥ����Ȥ���������ʤ��ä����ˤϡ� 
      1 ���֤��������Ǥʤ���� 0 ���֤���

      ���� \<callback\> ��ɬ�פǤ���С�$IC->status, $IC->preedit,
      $IC->cursor_pos, $IC->ncandidates, $IC->candidates,
      $IC->produced �򹹿��Ǥ��롣

      $ARG �ΰ�̣�����ϥ᥽�åɥɥ饤�Ф˰�¸���롣��� @c
      minput_default_driver �ޤ��� @c minput_gui_driver 
      �������򻲾ȤΤ��ȡ� */

  int (*filter) (MInputContext *ic, MSymbol key, void *arg);

  /***en
      @brief  Lookup a produced text in an input context.

      It is called from the function minput_lookup () and looks up a
      produced text in the input context $IC.  This function
      concatenate a text produced by the input key $KEY (if any) to
      M-text $MT.  If $KEY was correctly handled by the input method
      of $IC, it returns 0.  Otherwise, it returns 1.

      The meaning of $ARG depends on the input method driver.  See the
      documentation of @c minput_default_driver and @c
      minput_gui_driver for instance.  */

  /***ja
      @brief ���ϥ���ƥ����Ȥ����������ƥ����Ȥγ���.

      �ؿ� minput_lookup () ����ƤФ졢���ϥ���ƥ����� $IC 
      �����������ƥ����Ȥ򸡺����롣���ϥ��� $KEY 
      �ˤ�ä����������ƥ����Ȥ�����С�M-text $MT ���ɲä��롣 $KEY
      �����ϥ᥽�å� $IC �ˤ�ä����������������� 0 ���֤��������Ǥʤ���� 1 ���֤���

      $ARG �ΰ�̣�����ϥ᥽�åɥɥ饤�Ф˰�¸���롣��� @c
      minput_default_driver �ޤ��� @c minput_gui_driver �������򻲾Ȥ�
      ���ȡ� */

  int (*lookup) (MInputContext *ic, MSymbol key, void *arg, MText *mt);

  /***en
      @brief List of callback functions.

      List of callback functions.  Keys are one of
      @b Minput_preedit_start, @b Minput_preedit_draw,
      @b Minput_preedit_done, @b Minput_status_start, @b Minput_status_draw,
      @b Minput_status_done, @b Minput_candidates_start,
      @b Minput_candidates_draw, @b Minput_candidates_done,
      @b Minput_set_spot, @b Minput_toggle, @b Minput_reset,
      @b Minput_get_surrounding_text, @b Minput_delete_surrounding_text.
      Values are functions of type #MInputCallbackFunc.  */
  /***ja
      @brief ������Хå��ؿ��Υꥹ��.

      ������Хå��ؿ��Υꥹ�ȡ������ϼ��Τ����줫��
      @b Minput_preedit_start, @b Minput_preedit_draw,
      @b Minput_preedit_done, @b Minput_status_start, @b Minput_status_draw,
      @b Minput_status_done, @b Minput_candidates_start,
      @b Minput_candidates_draw, @b Minput_candidates_done,
      @b Minput_set_spot, @b Minput_toggle, @b Minput_reset,
      @b Minput_get_surrounding_text, @b Minput_delete_surrounding_text��
      �ͤ�#MInputCallbackFunc ���δؿ���  */
  MPlist *callback_list;

} MInputDriver;
/*=*/
/*** @} */
/*=*/

extern MInputDriver minput_default_driver;

extern MSymbol Minput_method;
extern MSymbol Minput_driver;

extern MInputDriver *minput_driver;

/** Symbols for callback commands.  */
extern MSymbol Minput_preedit_start;
extern MSymbol Minput_preedit_draw;
extern MSymbol Minput_preedit_done;
extern MSymbol Minput_status_start;
extern MSymbol Minput_status_draw;
extern MSymbol Minput_status_done;
extern MSymbol Minput_candidates_start;
extern MSymbol Minput_candidates_draw;
extern MSymbol Minput_candidates_done;
extern MSymbol Minput_set_spot;
extern MSymbol Minput_toggle;
extern MSymbol Minput_reset;
extern MSymbol Minput_get_surrounding_text;
extern MSymbol Minput_delete_surrounding_text;

/** Symbols for special input key event.  */
extern MSymbol Minput_focus_move;
extern MSymbol Minput_focus_in;
extern MSymbol Minput_focus_out;

/** Symbols describing input method command/variable.  */
extern MSymbol Minherited;
extern MSymbol Mcustomized;
extern MSymbol Mconfigured;

/*** @addtogroup m17nInputMethod 
     @{ */
/*=*/
/***en
    @brief Structure of input method.

    The type @c MInputMethod is the structure of input method
    objects.  */
/***ja
    @brief ���ϥ᥽�åɤι�¤��.

    @c MInputMethod �ϡ����ϥ᥽�åɥ��֥��������Ѥι�¤�Τη��Ǥ��롣  */

struct MInputMethod
{
  /***en Which language this input method is for.  The value is @c
      Mnil if the input method is foreign.  */
  /***ja �ɤθ����Ѥ����ϥ᥽�åɤ���
      ���ϥ᥽�åɤ������Τ�ΤǤ�������ͤ� @c Mnil ��  */
  MSymbol language;

  /***en Name of the input method.  If the input method is foreign, it
      must has a property of key @c Minput_driver and the value must be a
      pointer to a proper input method driver.  */
  /***ja ���ϥ᥽�åɤ�̾���������᥽�åɤǤ�����ˤϡ�@c
      Minput_driver �򥭡��Ȥ���ץ�ѥƥ�������������ͤ�Ŭ�ڤ����ϥ᥽�åɥɥ饤�ФؤΥݥ��󥿤Ǥʤ��ƤϤʤ�ʤ���*/
  MSymbol name;

  /***en Input method driver of the input method.  */
  /***ja �������ϥ᥽�å��Ѥ����ϥ᥽�åɥɥ饤�С�  */
  MInputDriver driver;

  /***en The argument given to minput_open_im (). */
  /***ja minput_open_im () ���Ϥ���������  */
  void *arg;

  /***en Pointer to extra information that \<driver\>.open_im ()
      setups. */
  /***ja \<driver\>.open_im () �����ꤹ���ɲþ���ؤΥݥ��󥿡� */
  void *info;
};

/*=*/

/***en
    @brief Bit-masks to specify how candidates of input method is changed.  */

/***ja
    @brief ���ϥ᥽�åɤ����ϸ��䤬�ɤ��ѹ����줿���򼨤��ӥåȥޥ���.  */

enum MInputCandidatesChanged
  {
    MINPUT_CANDIDATES_LIST_CHANGED = 1,
    MINPUT_CANDIDATES_INDEX_CHANGED = 2,
    MINPUT_CANDIDATES_SHOW_CHANGED = 4,
    MINPUT_CANDIDATES_CHANGED_MAX
  };


/*=*/

/***en
    @brief Structure of input context.

    The type @c MInputContext is the structure of input context
    objects.  */

/***ja
    @brief ���ϥ���ƥ������ѹ�¤��.

    @c MInputContext �ϡ����ϥ���ƥ����ȥ��֥��������Ѥι�¤�Τη��Ǥ��롣  */

struct MInputContext
{
  /***en Backward pointer to the input method.  It is set up by the
      function minput_create_ic ().  */
  /***ja ���ϥ᥽�åɤؤεեݥ��󥿡��ؿ� minput_create_ic () 
      �ˤ�ä����ꤵ��롣  */ 
  MInputMethod *im;

  /***en M-text produced by the input method.  It is set up by the
      function minput_filter () .  */
  /***ja ���ϥ᥽�åɤˤ�ä���������� M-text���ؿ� minput_filter () 
      �ˤ�ä����ꤵ��롣  */
  MText *produced;

  /***en Argument given to the function minput_create_ic (). */
  /***ja �ؿ� minput_create_ic () ���Ϥ��������� */
  void *arg;

  /***en Flag telling whether the input context is currently active or
      inactive.  The value is set to 1 (active) when the input context
      is created.  It is toggled by the function minput_toggle ().  */
  /***ja ���ϥ���ƥ����Ȥ������ƥ��֤��ɤ����򼨤��ե饰��
      ���ϥ���ƥ����Ȥ��������줿�����Ǥ��ͤ� 1 �ʥ����ƥ��֡ˤǤ��ꡢ�ؿ� 
      minput_toggle () �ˤ�äƥȥ��뤵��롣  */
  int active;

  /***en Spot location and size of the input context.  */
  /***ja ���ϥ���ƥ����ȤΥ��ݥåȤΰ��֤��礭��.  */
  struct {
    /***en X and Y coordinate of the spot.  */
    /***ja ���ݥåȤ� X, Y ��ɸ.  */
    int x, y;

    /***en Ascent and descent pixels of the line of the spot.  */
    /***ja ���ݥåȤΥ�����Ȥȥǥ�����ȤΥԥ������.  */
    int ascent, descent;

    /***en Font size for preedit text in 1/10 point.  */
    /***ja preedit �ƥ������ѤΥե���ȥ����� (1/10 �ݥ����ñ��).  */
    int fontsize;

    /***en M-text at the spot, or NULL.  */
    /***ja ���ݥåȾ�� M-text���ޤ��� NULL.  */
    MText *mt;

    /***en Character position in \<mt\> at the spot.  */
    /***ja \<mt\> �ˤ����륹�ݥåȤ�ʸ������.  */
    int pos;
  } spot;

  /***en The usage of the following members depends on the input
      method driver.  The descriptions below are for the driver of an
      internal input method.  They are set by the function
      \<im\>->driver.filter ().  */
  /***ja �ʲ��Υ��Фλ���ˡ�����ϥ᥽�åɥɥ饤�Фˤ�äưۤʤ롣
      �ʲ��������ϡ��������ϥ᥽�å��Ѥ����ϥɥ饤�Ф��Ф����ΤǤ��롣
      �����ϴؿ� \<im\>->driver.filter () �ˤ�ä����ꤵ��롣  */

  /***en Pointer to extra information that \<im\>->driver.create_ic ()
      setups.  It is used to record the internal state of the input
      context.  */
  /***ja \<im\>->driver.create_ic () �����ꤹ���ɲþ���ؤΥݥ��󥿡�
      ���ϥ���ƥ����Ȥ��������֤�Ͽ���뤿����Ѥ����롣 */
  void *info;

  /***en M-text describing the current status of the input
      context.  */
  /***ja ���ϥ���ƥ����Ȥθ��ߤξ��֤�ɽ�� M-text  */
  MText *status;

  /***en The function \<im\>->driver.filter () sets the value to 1 when
      it changes \<status\>.  */
  /***ja �ؿ� \<im\>->driver.filter () �ϡ�\<status\> ���Ѥ����ݤˤ����ͤ� 1 
      �����ꤹ�롣  */
  int status_changed;

  /***en M-text containing the current preedit text.  The function
      \<im\>->driver.filter () sets the value.  */
  /***ja ���ߤ� preedit �ƥ����Ȥ�ޤ� M-text���ؿ� 
      \<im\>->driver.filter () �ˤ�ä����ꤵ��롣  */ 
  MText *preedit;

  /***en The function \<im\>->driver.filter () sets the value to 1 when
      it changes \<preedit\>.  */
  /***ja �ؿ� \<im\>->driver.filter () �ϡ�\<preedit\> ���Ѥ����ݤˤ����ͤ� 
      1 �����ꤹ�롣  */
  int preedit_changed;

  /***en Cursor position of \<preedit\>.  */
  /***ja \<preedit\>�Υ����������  */
  int cursor_pos;

  /***en The function \<im\>->driver.filter () sets the value to 1 when
      it changes \<cursor_pos\>.  */
  /***ja �ؿ� \<im\>->driver.filter () �ϡ�\<cursor_pos\> ���Ѥ����ݤˤ����ͤ� 
      1 �����ꤹ�롣  */
  int cursor_pos_changed;

  /***en Plist of the current candidate groups.  Each element is an
      M-text or a plist.  If an element is an M-text (i.e. the key is Mtext),
      candidates in that group are characters in the M-text.  If it is
      a plist (i.e. the key is Mplist), each element is an M-text, and
      candidates in that group are those M-texts.  */
  /***ja ���ߤθ��䥰�롼�פ� Plist �������Ǥ� M-text �� plist �Ǥ��롣
      ���Ǥ� M-text �ξ��ʥ����� Mtext �Ǥ�����ˤˤϡ����Υ��롼�פθ���Ϥ��� 
      M-text ��γ�ʸ���Ǥ��롣 ���Ǥ� plist �ξ��ʥ����� Mplist
      �Ǥ�����ˤˤϡ����Υꥹ�Ȥγ����Ǥ� M-text �Ǥ��ꡢ����餬���Υ��롼�פθ���Ȥʤ롣  */
  MPlist *candidate_list;

  /***en Index number of the currently selected candidate in all the
      candidates. The index of the first candidate is 0.  If the
      number is 8, and the first candidate group contains 7
      candidates, the currently selected candidate is the second element of the
      second candidate group.  */
  /***ja �������򤵤�Ƥ�����䤬��������ǲ����ܤ���򼨤�����ǥå�����
      �ǽ�θ���Υ���ǥå����� 0���ǽ�θ��䥰�롼�פ˼��Ĥθ��䤬�ޤޤ�Ƥ��ꡢ�����ͤ�
      8 �ʤ�С����ߤθ���������ܤθ��䥰�롼�פ������ܤ����ǤȤ������Ȥˤʤ롣
      */
  int candidate_index;

  /* @{ */
  /***en Start and the end positions of the preedit text where
       \<candidate_list\> corresponds to.  */
  /***ja preedit �ƥ�������ǡ�\<candidate_list\>���б�����ǽ�ȺǸ�ΰ��֡�
       */
  int candidate_from, candidate_to;
  /* @} */

  /***en Flag telling whether the current candidate group must be
      shown or not.  The function \<im\>->driver.filter () sets the
      value to 1 when an input method required to show candidates, and
      sets the value to 0 otherwise.  */
  /***ja ���ߤθ��䥰�롼�פ�ɽ�����뤫�ɤ����򼨤��ե饰��
      �ؿ� \<im\>->driver.filter () �ϡ����ϥ᥽�åɤ������ɽ�����׵ᤷ���������ͤ�
      1 �ˡ�����ʳ��λ� 0 �����ꤹ�롣  */
  int candidate_show;

  /***en The function \<im\>->driver.filter () sets the value to bitwise
      OR of @c enum @c MInputCandidatesChanged when it changed any of
      the above members (\<candidate_XXX\>), and sets the value to 0
      otherwise.  */
  /***ja �ؿ� \<im\>->driver.filter () �ϡ��嵭�Υ��� \<candidate_XXX\>
      �Σ��ĤǤ��ѹ������ݤˤϡ������ͤ� @c enum @c
      MInputCandidatesChanged �Υӥå�ñ�̤Ǥ����� OR �����ꤹ�롣����
      �Ǥʤ���� 0 �����ꤹ�롣 */
  int candidates_changed;

  /***en Plist that can be freely used by \<im\>->driver functions.
      The driver of internal input method uses it to exchange extra
      arguments and result for callback functions.  The function
      \<im\>->driver.create_ic () sets this to an empty plist, and the
      function \<im\>->driver.destroy_ic () frees it by using
      m17n_object_unref ().  */
  /***ja \<im\>->driver �δؿ����ˤ�äƼ�ͳ�˻��ѤǤ��� plist��
      �������ϥ᥽�å��ѥɥ饤�ФϤ���򥳡���Хå��ؿ��Ȥΰ���������
      �μ��Ϥ��˻��Ѥ��롣�ؿ� \<im\>->driver.create_ic () �Ϥ��� plist
      ��������ꤹ�롣�ؿ�\<im\>->driver.destroy_ic () ��
      m17n_object_unref () ���Ѥ��Ƥ��� plist ��������롣  */
  MPlist *plist;
};

/*=*/
/*** @} */
/*=*/

extern MInputMethod *minput_open_im (MSymbol language, MSymbol name,
				     void *arg);

/*=*/

extern void minput_close_im (MInputMethod *im);

extern MInputContext *minput_create_ic (MInputMethod *im, void *arg);

extern void minput_destroy_ic (MInputContext *ic);

extern int minput_filter (MInputContext *ic, MSymbol key, void *arg);

extern int minput_lookup (MInputContext *ic, MSymbol key, void *arg,
			  MText *mt);
extern void minput_set_spot (MInputContext *ic, int x, int y, int ascent,
			     int descent, int fontsize, MText *mt, int pos);
extern void minput_toggle (MInputContext *ic);

extern void minput_reset_ic (MInputContext *ic);

extern MText *minput_get_description (MSymbol language, MSymbol name);

extern MPlist *minput_get_title_icon (MSymbol language, MSymbol name);

extern MPlist *minput_get_command (MSymbol language, MSymbol name,
				   MSymbol command);
extern int minput_config_command (MSymbol language, MSymbol name,
				  MSymbol command, MPlist *keyseq);
extern MPlist *minput_get_variable (MSymbol language, MSymbol name,
				    MSymbol variable);
extern int minput_config_variable (MSymbol language, MSymbol name,
				   MSymbol variable, MPlist *value);
extern char *minput_config_file (void);

extern int minput_save_config (void);

extern int minput_callback (MInputContext *ic, MSymbol command);

/* obsolete functions */
extern MPlist *minput_get_commands (MSymbol language, MSymbol name);

extern int minput_assign_command_keys (MSymbol language, MSymbol name,
				       MSymbol command, MPlist *keys);
extern MPlist *minput_get_variables (MSymbol language, MSymbol name);

extern int minput_set_variable (MSymbol language, MSymbol name,
				MSymbol variable, void *value);

extern MPlist *minput_parse_im_names (MText *mt);

extern MPlist *minput_list (MSymbol lang);

extern MInputMethod *mdebug_dump_im (MInputMethod *im, int indent);

M17N_END_HEADER

#endif /* _M17N_H_ */

/*
  Local Variables:
  coding: euc-japan
  End:
*/
