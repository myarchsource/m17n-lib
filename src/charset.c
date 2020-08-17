/* charset.c -- charset module.
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
    @addtogroup m17nCharset
    @brief Charset objects and API for them.

    The m17n library uses @e charset objects to represent a coded
    character sets (CCS).  The m17n library supports many predefined
    coded character sets.  Moreover, application programs can add
    other charsets.  A character can belong to multiple charsets.

    The m17n library distinguishes the following three concepts:

    @li A @e code-point is a number assigned by the CCS to each
    character.  Code-points may or may not be continuous.  The type
    @c unsigned is used to represent a code-point.  An invalid
    code-point is represented by the macro @c MCHAR_INVALID_CODE.

    @li A @e character @e index is the canonical index of a character
    in a CCS.  The character that has the character index N occupies
    the Nth position when all the characters in the current CCS are
    sorted by their code-points.  Character indices in a CCS are
    continuous and start with 0.

    @li A @e character @e code is the internal representation in the
    m17n library of a character.  A character code is a signed integer
    of 21 bits or longer.

    Each charset object defines how characters are converted between
    code-points and character codes.  To @e encode means converting
    code-points to character codes and to @e decode means converting
    character codes to code-points.  */

/***ja
    @addtogroup m17nCharset
    @brief ʸ�����åȥ��֥������ȤȤ���˴ؤ��� API.

    m17n �饤�֥��ϡ���沽ʸ������ (CCS) �� @e ʸ�����å� 
    �ȸƤ֥��֥������Ȥ�ɽ�����롣
    m17n �饤�֥���¿������沽ʸ������򤢤餫���᥵�ݡ��Ȥ��Ƥ��뤷�����ץꥱ�������ץ���ब�ȼ���ʸ�����åȤ��ɲä��뤳�Ȥ��ǽ�Ǥ��롣
    ��Ĥ�ʸ����ʣ����ʸ�����åȤ�°���Ƥ�褤��

    m17n �饤�֥��ϡ��ʲ��γ�ǰ����̤��Ƥ���:

    @li @e �����ɥݥ���� �Ȥϡ�CCS ��������θġ���ʸ�����Ф������������ͤǤ��롣
    �����ɥݥ���Ȥ�Ϣ³���Ƥ���Ȥϸ¤�ʤ��������ɥݥ���Ȥ�
    @c unsigned ���ˤ�ä�ɽ����롣̵���ʥ����ɥݥ���Ȥϥޥ���
    @c MCHAR_INVALID_CODE ��ɽ����롣

    @li @e ʸ������ǥå��� �Ȥϡ�CCS ��ǳ�ʸ���˳�����Ƥ������������줿����ǥå����Ǥ��롣
    ʸ������ǥå����� N ��ʸ���ϡ�CCS �����ʸ���򥳡��ɥݥ���Ƚ���¤٤��Ȥ��� N ���ܤ˸����롣
    CCS ���ʸ������ǥå�����Ϣ³���Ƥ��ꡢ0 ����Ϥޤ롣

    @li @e ʸ�������� �Ȥϡ�m17n �饤�֥����ˤ�����ʸ��������ɽ���Ǥ��ꡢ21 �ӥåȰʾ��Ĺ�����������դ������Ǥ��롣

    ��ʸ�����åȥ��֥������Ȥϡ�����ʸ�����åȤ�°����ʸ���Υ����ɥݥ���Ȥ�ʸ�������ɤȤδ֤��Ѵ����ꤹ�롣
    �����ɥݥ���Ȥ���ʸ�������ɤؤ��Ѵ��� @e �ǥ�����
    �ȸƤӡ�ʸ�������ɤ��饳���ɥݥ���Ȥؤ��Ѵ��� @e ���󥳡��� �ȸƤ֡�  */


/*=*/
#if !defined (FOR_DOXYGEN) || defined (DOXYGEN_INTERNAL_MODULE)
/*** @addtogroup m17nInternal
     @{ */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "m17n.h"
#include "m17n-misc.h"
#include "internal.h"
#include "symbol.h"
#include "database.h"
#include "chartab.h"
#include "plist.h"
#include "charset.h"
#include "coding.h"

static int unified_max;

/** List of all charsets ever defined.  */

struct MCharsetList
{
  int size, inc, used;
  MCharset **charsets;
};

static struct MCharsetList charset_list;

static MPlist *charset_definition_list;

/** Make a charset object from the template of MCharset structure
    CHARSET, and return a pointer to the new charset object.
    CHARSET->code_range[4N + 2] and CHARSET->code_range[4N + 3] are
    not yet set.  */

static MCharset *
make_charset (MCharset *charset)
{
  unsigned min_code, max_code;
  int i, n;
  int *range = charset->code_range;

  if (charset->dimension < 1 || charset->dimension > 4)
    MERROR (MERROR_CHARSET, NULL);
  if ((charset->final_byte > 0 && charset->final_byte < '0')
      || charset->final_byte > 127)
    MERROR (MERROR_CHARSET, NULL);

  for (i = 0, n = 1; i < 4; i++)
    {
      if (range[i * 4] > range[i * 4 + 1])
	MERROR (MERROR_CHARSET, NULL);
      range[i * 4 + 2] = range[i * 4 + 1] - range[i * 4] + 1;
      n *= range[i * 4 + 2];
      range[i * 4 + 3] = n;
    }

  min_code = range[0] | (range[4] << 8) | (range[8] << 16) | (range[12] << 24);
  if (charset->min_code == 0)
    charset->min_code = min_code;
  else if (charset->min_code < min_code)
    MERROR (MERROR_CHARSET, NULL);
  max_code = range[1] | (range[5] << 8) | (range[9] << 16) | (range[13] << 24);
  if (charset->max_code == 0)  
    charset->max_code = max_code;
  else if (charset->max_code > max_code)
    MERROR (MERROR_CHARSET, NULL);

  charset->code_range_min_code = min_code;
  charset->fully_loaded = 0;
  charset->simple = 0;

  if (charset->method == Msubset)
    {
      MCharset *parent;

      if (charset->nparents != 1)
	MERROR (MERROR_CHARSET, NULL);
      parent = charset->parents[0];
      if (parent->method == Msuperset
	  || charset->min_code - charset->subset_offset < parent->min_code
	  || charset->max_code - charset->subset_offset > parent->max_code)
	MERROR (MERROR_CHARSET, NULL);
    }
  else if (charset->method == Msuperset)
    {
      if (charset->nparents < 2)
	MERROR (MERROR_CHARSET, NULL);
      for (i = 0; i < charset->nparents; i++)
	if (charset->min_code > charset->parents[i]->min_code
	    || charset->max_code < charset->parents[i]->max_code)
	  MERROR (MERROR_CHARSET, NULL);
    }
  else
    {
      charset->no_code_gap
	= (charset->dimension == 1
	   || (range[2] == 256
	       && (charset->dimension == 2
		   || (range[6] == 256
		       && (charset->dimension == 3
			   || range[10] == 256)))));

      if (! charset->no_code_gap)
	{
	  int j;

	  memset (charset->code_range_mask, 0,
		  sizeof charset->code_range_mask);
	  for (i = 0; i < 4; i++)
	    for (j = range[i * 4]; j <= range[i * 4 + 1]; j++)
	      charset->code_range_mask[j] |= (1 << i);
	}

      if (charset->method == Moffset)
	{
	  charset->max_char = charset->min_char + range[15] - 1;
	  if (charset->min_char < 0
	      || charset->max_char < 0 || charset->max_char > unified_max)
	    MERROR (MERROR_CHARSET, NULL);
	  charset->simple = charset->no_code_gap;
	  charset->fully_loaded = 1;
	}
      else if (charset->method == Munify)
	{
	  /* The magic number 12 below is to align to the SUB_BITS_2
	     (defined in chartab.c) boundary in a char-table.  */
	  unified_max -= ((range[15] >> 12) + 1) << 12;
	  charset->unified_max = unified_max;
	}
      else if (charset->method != Mmap)
	MERROR (MERROR_CHARSET, NULL);
    }

  MLIST_APPEND1 (&charset_list, charsets, charset, MERROR_CHARSET);

  if (charset->final_byte > 0)
    {
      MLIST_APPEND1 (&mcharset__iso_2022_table, charsets, charset,
		     MERROR_CHARSET);
      if (charset->revision <= 0)
	{
	  int chars = range[2];

	  if (chars == 128)	/* ASCII case */
	    chars = 94;
	  else if (chars == 256) /* ISO-8859-X case */
	    chars = 96;
	  MCHARSET_ISO_2022 (charset->dimension, chars, charset->final_byte)
	    = charset;
	}
    }

  return charset;
}

static int
load_charset_fully (MCharset *charset)
{
  if (charset->method == Msubset)
    {
      MCharset *parent = charset->parents[0];

      if (! parent->fully_loaded
	  && load_charset_fully (parent) < 0)
	MERROR (MERROR_CHARSET, -1);
      if (parent->method == Moffset)
	{
	  unsigned code;

	  code = charset->min_code - charset->subset_offset;
	  charset->min_char = DECODE_CHAR (parent, code);
	  code = charset->max_code - charset->subset_offset;
	  charset->max_char = DECODE_CHAR (parent, code);
	}
      else
	{
	  unsigned min_code = charset->min_code - charset->subset_offset;
	  unsigned max_code = charset->max_code - charset->subset_offset;
	  int min_char = DECODE_CHAR (parent, min_code);
	  int max_char = min_char;
	  
	  for (++min_code; min_code <= max_code; min_code++)
	    {
	      int c = DECODE_CHAR (parent, min_code);

	      if (c >= 0)
		{
		  if (c < min_char)
		    min_char = c;
		  else if (c > max_char)
		    max_char = c;
		}
	    }
	  charset->min_char = min_char;
	  charset->max_char = max_char;
	}
    }
  else if (charset->method == Msuperset)
    {
      int min_char = 0, max_char = 0;
      int i;

      for (i = 0; i < charset->nparents; i++)
	{
	  MCharset *parent = charset->parents[i];

	  if (! parent->fully_loaded
	      && load_charset_fully (parent) < 0)
	    MERROR (MERROR_CHARSET, -1);
	  if (i == 0)
	    min_char = parent->min_char, max_char = parent->max_char;
	  else if (parent->min_char < min_char)
	    min_char = parent->min_char;
	  else if (parent->max_char > max_char)
	    max_char = parent->max_char;
	}
      charset->min_char = min_char;
      charset->max_char = max_char;
    }
  else 				/* charset->method is Mmap or Munify */
    {
      MDatabase *mdb = mdatabase_find (Mcharset, charset->name, Mnil, Mnil);
      MPlist *plist;

      if (! mdb || ! (plist = mdatabase_load (mdb)))
	MERROR (MERROR_CHARSET, -1);
      charset->decoder = mplist_value (plist);
      charset->encoder = mplist_value (mplist_next (plist));
      M17N_OBJECT_UNREF (plist);
      mchartable_range (charset->encoder,
			&charset->min_char, &charset->max_char);
      if (charset->method == Mmap)
	charset->simple = charset->no_code_gap;
      else
	charset->max_char = charset->unified_max + 1 + charset->code_range[15];
    }

  charset->fully_loaded = 1;
  return 0;
}

/** Load a data of type @c charset from the file FD.  */

static void *
load_charset (FILE *fp, MSymbol charset_name)
{
  MCharset *charset = MCHARSET (charset_name);
  int *decoder;
  MCharTable *encoder;
  int size;
  int i, c;
  int found = 0;
  MPlist *plist;

  if (! charset)
    MERROR (MERROR_DB, NULL);
  size = (charset->code_range[15]
	  - (charset->min_code - charset->code_range_min_code));
  MTABLE_MALLOC (decoder, size, MERROR_DB);
  for (i = 0; i < size; i++)
    decoder[i] = -1;
  encoder = mchartable (Minteger, (void *) MCHAR_INVALID_CODE);

  while ((c = getc (fp)) != EOF)
    {
      unsigned code1, code2, c1, c2;
      int idx1, idx2;
      char buf[256];

      ungetc (c, fp);
      if (! fgets (buf, 256, fp))
	break;
      if (c != '#')
	{
	  if (sscanf (buf, "0x%x-0x%x 0x%x", &code1, &code2, &c1) == 3)
	    {
	      idx1 = CODE_POINT_TO_INDEX (charset, code1);
	      if (idx1 >= size)
		continue;
	      idx2 = CODE_POINT_TO_INDEX (charset, code2);
	      if (idx2 >= size)
		idx2 = size - 1;
	      c2 = c1 + (idx2 - idx1);
	    }
	  else if (sscanf (buf, "0x%x 0x%x", &code1, &c1) == 2)
	    {
	      idx1 = idx2 = CODE_POINT_TO_INDEX (charset, code1);
	      if (idx1 >= size)
		continue;
	      c2 = c1;
	    }
	  else
	    continue;
	  if (idx1 >= 0 && idx2 >= 0)
	    {
	      decoder[idx1] = c1;
	      mchartable_set (encoder, c1, (void *) code1);
	      for (idx1++, c1++; idx1 <= idx2; idx1++, c1++)
		{
		  code1 = INDEX_TO_CODE_POINT (charset, idx1);
		  decoder[idx1] = c1;
		  mchartable_set (encoder, c1, (void *) code1);
		}
	      found++;
	    }
	}
    }

  if (! found)
    {
      free (decoder);
      M17N_OBJECT_UNREF (encoder);
      return NULL;
    }
  plist = mplist ();
  mplist_add (plist, Mt, decoder);
  mplist_add (plist, Mt, encoder);
  return plist;
}


/* Internal API */

MPlist *mcharset__cache;

/* Predefined charsets.  */
MCharset *mcharset__ascii;
MCharset *mcharset__binary;
MCharset *mcharset__m17n;
MCharset *mcharset__unicode;

MCharsetISO2022Table mcharset__iso_2022_table;

/** Initialize charset handler.  */

int
mcharset__init ()
{
  MPlist *param, *pl;

  unified_max = MCHAR_MAX;

  mdatabase__load_charset_func = load_charset;
  mcharset__cache = mplist ();
  mplist_set (mcharset__cache, Mt, NULL);

  MLIST_INIT1 (&charset_list, charsets, 128);
  MLIST_INIT1 (&mcharset__iso_2022_table, charsets, 128);
  charset_definition_list = mplist ();

  memset (mcharset__iso_2022_table.classified, 0,
	  sizeof (mcharset__iso_2022_table.classified));

  Mmethod = msymbol ("method");
  Moffset = msymbol ("offset");
  Mmap = msymbol ("map");
  Munify = msymbol ("unify");
  Msubset = msymbol ("subset");
  Msuperset = msymbol ("superset");

  Mdimension = msymbol ("dimension");
  Mmin_range = msymbol ("min-range");
  Mmax_range = msymbol ("max-range");
  Mmin_code = msymbol ("min-code");
  Mmax_code = msymbol ("max-code");
  Mascii_compatible = msymbol ("ascii-compatible");
  Mfinal_byte = msymbol ("final-byte");
  Mrevision = msymbol ("revision");
  Mmin_char = msymbol ("min-char");
  Mmapfile = msymbol_as_managing_key ("mapfile");
  Mparents = msymbol_as_managing_key ("parents");
  Msubset_offset = msymbol ("subset-offset");
  Mdefine_coding = msymbol ("define-coding");
  Maliases = msymbol_as_managing_key ("aliases");

  param = mplist ();
  pl = param;
  /* Setup predefined charsets.  */
  pl = mplist_add (pl, Mmethod, Moffset);
  pl = mplist_add (pl, Mmin_range, (void *) 0);
  pl = mplist_add (pl, Mmax_range, (void *) 0x7F);
  pl = mplist_add (pl, Mascii_compatible, Mt);
  pl = mplist_add (pl, Mfinal_byte, (void *) 'B');
  pl = mplist_add (pl, Mmin_char, (void *) 0);
  Mcharset_ascii = mchar_define_charset ("ascii", param);

  mplist_put (param, Mmax_range, (void *) 0xFF);
  mplist_put (param, Mfinal_byte, NULL);
  Mcharset_iso_8859_1 = mchar_define_charset ("iso-8859-1", param);

  mplist_put (param, Mmax_range, (void *) 0x10FFFF);
  Mcharset_unicode = mchar_define_charset ("unicode", param);

  mplist_put (param, Mmax_range, (void *) MCHAR_MAX);
  Mcharset_m17n = mchar_define_charset ("m17n", param);

  mplist_put (param, Mmax_range, (void *) 0xFF);
  Mcharset_binary = mchar_define_charset ("binary", param);

  M17N_OBJECT_UNREF (param);

  mcharset__ascii = MCHARSET (Mcharset_ascii);
  mcharset__binary = MCHARSET (Mcharset_binary);
  mcharset__m17n = MCHARSET (Mcharset_m17n);
  mcharset__unicode = MCHARSET (Mcharset_unicode);

  return 0;
}

void
mcharset__fini (void)
{
  int i;
  MPlist *plist;

  for (i = 0; i < charset_list.used; i++)
    {
      MCharset *charset = charset_list.charsets[i];

      if (charset->decoder)
	free (charset->decoder);
      if (charset->encoder)
	M17N_OBJECT_UNREF (charset->encoder);
      free (charset);
    }
  M17N_OBJECT_UNREF (mcharset__cache);
  MLIST_FREE1 (&charset_list, charsets);
  MLIST_FREE1 (&mcharset__iso_2022_table, charsets);
  MPLIST_DO (plist, charset_definition_list)
    M17N_OBJECT_UNREF (MPLIST_VAL (plist));
  M17N_OBJECT_UNREF (charset_definition_list);
}


MCharset *
mcharset__find (MSymbol name)
{
  MCharset *charset;

  charset = msymbol_get (name, Mcharset);
  if (! charset)
    {
      MPlist *param = mplist_get (charset_definition_list, name);

      MPLIST_KEY (mcharset__cache) = Mt;
      if (! param)
	return NULL;
      param = mplist__from_plist (param);
      mchar_define_charset (MSYMBOL_NAME (name), param);
      charset = msymbol_get (name, Mcharset);
      M17N_OBJECT_UNREF (param);
    }
  MPLIST_KEY (mcharset__cache) = name;
  MPLIST_VAL (mcharset__cache) = charset;
  return charset;
}


/** Return the character corresponding to code-point CODE in CHARSET.
    If CODE is invalid for CHARSET, return -1.  */

int
mcharset__decode_char (MCharset *charset, unsigned code)
{
  int idx;

  if (code < 128 && charset->ascii_compatible)
    return (int) code;
  if (code < charset->min_code || code > charset->max_code)
    return -1;

  if (! charset->fully_loaded
      && load_charset_fully (charset) < 0)
    MERROR (MERROR_CHARSET, -1);

  if (charset->method == Msubset)
    {
      MCharset *parent = charset->parents[0];

      code -= charset->subset_offset;
      return DECODE_CHAR (parent, code);
    }

  if (charset->method == Msuperset)
    {
      int i;

      for (i = 0; i < charset->nparents; i++)
	{
	  MCharset *parent = charset->parents[i];
	  int c = DECODE_CHAR (parent, code);

	  if (c >= 0)
	    return c;
	}
      return -1;
    }

  idx = CODE_POINT_TO_INDEX (charset, code);
  if (idx < 0)
    return -1;

  if (charset->method == Mmap)
    return charset->decoder[idx];

  if (charset->method == Munify)
    {
      int c = charset->decoder[idx];

      if (c < 0)
	c = charset->unified_max + 1 + idx;
      return c;
    }

  /* Now charset->method should be Moffset.  */
  return (charset->min_char + idx);
}


/** Return the code point of character C in CHARSET.  If CHARSET does not
    contain C, return MCHAR_INVALID_CODE.  */

unsigned
mcharset__encode_char (MCharset *charset, int c)
{
  if (! charset->fully_loaded
      && load_charset_fully (charset) < 0)
    MERROR (MERROR_CHARSET, MCHAR_INVALID_CODE);

  if (charset->method == Msubset)
    {
      MCharset *parent = charset->parents[0];
      unsigned code = ENCODE_CHAR (parent, c);

      if (code == MCHAR_INVALID_CODE)
	return code;
      code += charset->subset_offset;
      if (code >= charset->min_code && code <= charset->max_code)
	return code;
      return MCHAR_INVALID_CODE;
    }

  if (charset->method == Msuperset)
    {
      int i;

      for (i = 0; i < charset->nparents; i++)
	{
	  MCharset *parent = charset->parents[i];
	  unsigned code = ENCODE_CHAR (parent, c);

	  if (code != MCHAR_INVALID_CODE)
	    return code;
	}
      return MCHAR_INVALID_CODE;
    }

  if (c < charset->min_char || c > charset->max_char)
    return MCHAR_INVALID_CODE;

  if (charset->method == Mmap)
    return (unsigned) mchartable_lookup (charset->encoder, c);

  if (charset->method == Munify)
    {
      if (c > charset->unified_max)
	{
	  c -= charset->unified_max - 1;
	  return INDEX_TO_CODE_POINT (charset, c);
	}
      return (unsigned) mchartable_lookup (charset->encoder, c);
    }

  /* Now charset->method should be Moffset */
  c -= charset->min_char;
  return INDEX_TO_CODE_POINT (charset, c);
}

int
mcharset__load_from_database ()
{
  MDatabase *mdb = mdatabase_find (msymbol ("charset-list"), Mnil, Mnil, Mnil);
  MPlist *def_list, *plist;
  MPlist *definitions = charset_definition_list;
  int mdebug_flag = MDEBUG_CHARSET;

  if (! mdb)
    return 0;
  MDEBUG_PUSH_TIME ();
  def_list = (MPlist *) mdatabase_load (mdb);
  MDEBUG_PRINT_TIME ("CHARSET", (mdebug__output, " to load data."));
  MDEBUG_POP_TIME ();
  if (! def_list)
    return -1;

  MDEBUG_PUSH_TIME ();
  MPLIST_DO (plist, def_list)
    {
      MPlist *pl, *p;
      MSymbol name;

      if (! MPLIST_PLIST_P (plist))
	MERROR (MERROR_CHARSET, -1);
      pl = MPLIST_PLIST (plist);
      if (! MPLIST_SYMBOL_P (pl))
	MERROR (MERROR_CHARSET, -1);
      name = MPLIST_SYMBOL (pl);
      pl = MPLIST_NEXT (pl);
      definitions = mplist_add (definitions, name, pl);
      M17N_OBJECT_REF (pl);
      p = mplist__from_plist (pl);
      mchar_define_charset (MSYMBOL_NAME (name), p);
      M17N_OBJECT_UNREF (p);
    }

  M17N_OBJECT_UNREF (def_list);
  MDEBUG_PRINT_TIME ("CHARSET", (mdebug__output, " to parse the loaded data."));
  MDEBUG_POP_TIME ();
  return 0;
}

/*** @} */
#endif /* !FOR_DOXYGEN || DOXYGEN_INTERNAL_MODULE */


/* External API */

/*** @addtogroup m17nCharset */
/*** @{ */
/*=*/

#ifdef FOR_DOXYGEN
/***en
    @brief Invalid code-point.

    The macro #MCHAR_INVALID_CODE gives the invalid code-point.  */

/***ja
    @brief ̵���ʥ����ɥݥ����.

    �ޥ��� #MCHAR_INVALID_CODE ��̵���ʥ����ɥݥ���Ȥ򼨤���  */

#define MCHAR_INVALID_CODE
#endif

/*=*/

/***en
    @name Variables: Symbols representing a charset.

    Each of the following symbols represents a predefined charset.  */

/***ja
    @name �ѿ�: ʸ�����åȤ�ɽ����������Ѥߥ���ܥ�.

    �ʲ��γƥ���ܥ�ϡ�����Ѥ�ʸ�����åȤ�ɽ�����롣  */
/*=*/
/*** @{ */
/*=*/
/***en
    @brief Symbol representing the charset ASCII.

    The symbol #Mcharset_ascii has name <tt>"ascii"</tt> and represents
    the charset ISO 646, USA Version X3.4-1968 (ISO-IR-6).  */
/***ja
    @brief ASCII ʸ�����åȤ�ɽ�����륷��ܥ�.

    ����ܥ� #Mcharset_ascii �� <tt>"ascii"</tt> �Ȥ���̾��������� 
    ISO 646, USA Version X3.4-1968 (ISO-IR-6) ʸ�����åȤ�ɽ�����롣
     */

MSymbol Mcharset_ascii;

/*=*/
/***en
    @brief Symbol representing the charset ISO/IEC 8859/1.

    The symbol #Mcharset_iso_8859_1 has name <tt>"iso-8859-1"</tt>
    and represents the charset ISO/IEC 8859-1:1998.  */
/***ja
    @brief ISO/IEC 8859-1:1998 ʸ�����åȤ�ɽ�����륷��ܥ�.

    ����ܥ� #Mcharset_iso_8859_1 �� <tt>"iso-8859-1"</tt> 
    �Ȥ���̾���������ISO/IEC 8859-1:1998 ʸ�����åȤ�ɽ�����롣
    */

MSymbol Mcharset_iso_8859_1;

/***en
    @brief Symbol representing the charset Unicode.

    The symbol #Mcharset_unicode has name <tt>"unicode"</tt> and
    represents the charset Unicode.  */
/***ja
    @brief Unicode ʸ�����åȤ�ɽ�����륷��ܥ�.

    ����ܥ� #Mcharset_unicode �� <tt>"unicode"</tt> 
    �Ȥ���̾���������Unicode ʸ�����åȤ�ɽ�����롣 */

MSymbol Mcharset_unicode;

/*=*/
/***en
    @brief Symbol representing the largest charset.

    The symbol #Mcharset_m17n has name <tt>"m17n"</tt> and
    represents the charset that contains all characters supported by
    the m17n library.  */ 
/***ja
    @brief ��ʸ����ޤ�ʸ�����åȤ�ɽ�����륷��ܥ�.

    ����ܥ� #Mcharset_m17n �� <tt>"m17n"</tt> �Ȥ���̾���������
    m17n �饤�֥�꤬�������Ƥ�ʸ����ޤ�ʸ�����åȤ�ɽ�����롣 */

MSymbol Mcharset_m17n;

/*=*/
/***en
    @brief Symbol representing the charset for ill-decoded characters.

    The symbol #Mcharset_binary has name <tt>"binary"</tt> and
    represents the fake charset which the decoding functions put to an
    M-text as a text property when they encounter an invalid byte
    (sequence).  

    See @ref m17nConv for more details.  */

/***ja
    @brief �������ǥ����ɤǤ��ʤ�ʸ����ʸ�����åȤ�ɽ�����륷��ܥ�.

    ����ܥ� #Mcharset_binary �� <tt>"binary"</tt> 
    �Ȥ���̾������������� (fake) ʸ�����åȤ�ɽ�����롣
    �ǥ����ɴؿ��ϡ�M-text �Υƥ����ȥץ�ѥƥ��Ȥ��ơ�̵���ʥХ��ȡʥ��������󥹡ˤ������������֤��ղä��롣

     �ܺ٤� @ref m17nConv ���ȤΤ��ȡ� */

MSymbol Mcharset_binary;

/*=*/
/*** @} */
/*=*/

/***en
    @name Variables: Parameter keys for mchar_define_charset ().

    These are the predefined symbols to use as parameter keys for the
    function mchar_define_charset () (which see).  */

/***ja
    @name �ѿ�: mchar_define_charset �ѤΥѥ�᡼��������

    �����ϡ��ؿ� mchar_define_charset () �ѤΥѥ�᡼���������Ȥ��ƻȤ��륷��ܥ�Ǥ��롣 
    �ܤ����Ϥ��δؿ��β���򻲾ȤΤ��ȡ�*/
/*** @{ */
/*=*/

MSymbol Mmethod;
MSymbol Mdimension;
MSymbol Mmin_range;
MSymbol Mmax_range;
MSymbol Mmin_code;
MSymbol Mmax_code;
MSymbol Mascii_compatible;
MSymbol Mfinal_byte;
MSymbol Mrevision;
MSymbol Mmin_char;
MSymbol Mmapfile;
MSymbol Mparents;
MSymbol Msubset_offset;
MSymbol Mdefine_coding;
MSymbol Maliases;
/*=*/
/*** @} */
/*=*/

/***en
    @name Variables: Symbols representing charset methods.

    These are the predefined symbols that can be a value of the
    @b Mmethod parameter of a charset used in an argument to the
    mchar_define_charset () function.

    A method specifies how code-points and character codes are
    converted.  See the documentation of the mchar_define_charset ()
    function for the details.  */

/***ja
    @name �ѿ�: ʸ�����åȤΥ᥽�åɻ���˻Ȥ��륷��ܥ�

    �����ϡ�ʸ�����åȤ� @e �᥽�å� ����ꤹ�뤿�������Ѥߥ���ܥ�Ǥ��ꡢʸ�����åȤ�
    @b Mmethod �ѥ�᡼�����ͤȤʤ뤳�Ȥ��Ǥ��롣
    �����ͤϴؿ� mchar_define_charset () �ΰ����Ȥ��ƻȤ��롣

    �᥽�åɤȤϡ������ɥݥ���Ȥ�ʸ�������ɤ�����Ѵ�����ݤ������Τ��ȤǤ��롣
    �ܤ����ϴؿ� mchar_define_charset () �β���򻲾ȤΤ��ȡ�  */
/*** @{ */
/*=*/
/***en
    @brief Symbol for the offset type method of charset.

    The symbol #Moffset has the name <tt>"offset"</tt> and, when used
    as a value of @b Mmethod parameter of a charset, it means that the
    conversion of code-points and character codes of the charset is
    done by this calculation:

@verbatim
CHARACTER-CODE = CODE-POINT - MIN-CODE + MIN-CHAR
@endverbatim

    where, MIN-CODE is a value of @b Mmin_code parameter of the charset,
    and MIN-CHAR is a value of @b Mmin_char parameter.  */

/***ja
    @brief ���ե��åȷ��Υ᥽�åɤ򼨤�����ܥ�.

    ����ܥ� #Moffset �� <tt>"offset"</tt> �Ȥ���̾���������ʸ�����åȤ�
    @b Mmethod �ѥ�᡼�����ͤȤ����Ѥ���줿���ˤϡ������ɥݥ���Ȥ�ʸ�����åȤ�ʸ�������ɤδ֤��Ѵ����ʲ��μ��˽��äƹԤ��뤳�Ȥ��̣���롣

@verbatim
ʸ�������� = �����ɥݥ���� - MIN-CODE + MIN-CHAR
@endverbatim

    �����ǡ�MIN-CODE ��ʸ�����åȤ� @b Mmin_code �ѥ�᡼�����ͤǤ��ꡢMIN-CHAR ��
    @b Mmin_char �ѥ�᡼�����ͤǤ��롣 */

MSymbol Moffset;
/*=*/

/***en @brief Symbol for the map type method of charset.

    The symbol #Mmap has the name <tt>"map"</tt> and, when used as a
    value of @b Mmethod parameter of a charset, it means that the
    conversion of code-points and character codes of the charset is
    done by map looking up.  The map must be given by @b Mmapfile
    parameter.  */

/***ja @brief �ޥå׷��Υ᥽�åɤ򼨤�����ܥ�.

    ����ܥ� #Mmap �� <tt>"map"</tt> �Ȥ���̾���������ʸ�����åȤ� 
    @b Mmethod �ѥ�᡼�����ͤȤ����Ѥ���줿���ˤϡ������ɥݥ���Ȥ�ʸ�����åȤ�ʸ�������ɤδ֤��Ѵ����ޥåפ򻲾Ȥ��뤳�Ȥˤ�äƹԤ��뤳�Ȥ��̣���롣
    �ޥåפ� @b Mmapfile �ѥ�᡼���Ȥ���Ϳ���ʤ���Фʤ�ʤ��� */

MSymbol Mmap;
/*=*/

/***en @brief Symbol for the unify type method of charset.

    The symbol #Munify has the name <tt>"unify"</tt> and, when used as
    a value of @b Mmethod parameter of a charset, it means that the
    conversion of code-points and character codes of the charset is
    done by map looking up and offsetting.  The map must be given by
    @b Mmapfile parameter.  For this kind of charset, a unique
    continuous character code space for all characters is assigned.

    If the map has an entry for a code-point, the conversion is done
    by looking up the map.  Otherwise, the conversion is done by this
    calculation:

@verbatim
CHARACTER-CODE = CODE-POINT - MIN-CODE + LOWEST-CHAR-CODE
@endverbatim

    where, MIN-CODE is a value of @b Mmin_code parameter of the charset,
    and LOWEST-CHAR-CODE is the lowest character code of the assigned
    code space.  */

/***ja @brief ��˥ե������Υ᥽�åɤ򼨤�����ܥ�.

    ����ܥ� #Munify �� <tt>"unify"</tt> �Ȥ���̾���������ʸ�����åȤ� 
    @b Mmethod �ѥ�᡼�����ͤȤ����Ѥ���줿���ˤϡ������ɥݥ���Ȥ�ʸ�����åȤ�ʸ�������ɤδ֤��Ѵ������ޥåפλ��Ȥȥ��ե��åȤ��Ȥ߹�碌�ˤ�äƹԤ��뤳�Ȥ��̣���롣
    �ޥåפ� @b Mmapfile �ѥ�᡼���Ȥ���Ϳ���ʤ���Фʤ�ʤ���
    ���μ�γ�ʸ�����åȤˤϡ���ʸ�����Ф���Ϣ³���륳���ɥ��ڡ��������줾�������Ƥ��롣

    �����ɥݥ���Ȥ��ޥåפ˴ޤޤ�Ƥ���С��Ѵ��ϥޥå׻��Ȥˤ�äƹԤ��롣
    �����Ǥʤ���С��ʲ��μ��˽�����

@verbatim
CHARACTER-CODE = CODE-POINT - MIN-CODE + LOWEST-CHAR-CODE
@endverbatim
    
    �����ǡ�MIN-CODE ��ʸ�����åȤ� @b Mmin_code �ѥ�᡼�����ͤǤ��ꡢ
    LOWEST-CHAR-CODE �ϳ�����Ƥ�줿�����ɥ��ڡ����κǤ⾮����ʸ�������ɤǤ��롣
    */

MSymbol Munify;
/*=*/

/***en
    @brief Symbol for the subset type method of charset.

    The symbol #Msubset has the name <tt>"subset"</tt> and, when used
    as a value of @b Mmethod parameter of a charset, it means that the
    charset is a subset of a parent charset.  The parent charset must
    be given by @b Mparents parameter.  The conversion of code-points
    and character codes of the charset is done conceptually by this
    calculation:

@verbatim
CHARACTER-CODE = PARENT-CODE (CODE-POINT) + SUBSET-OFFSET
@endverbatim

    where, PARENT-CODE is a pseudo function that returns a character
    code of CODE-POINT in the parent charset, and SUBSET-OFFSET is a
    value given by @b Msubset_offset parameter.  */

/***ja @brief ���֥��åȷ��Υ᥽�åɤ򼨤�����ܥ�.

    ����ܥ� #Msubset �� <tt>"subset"</tt> �Ȥ���̾���������ʸ�����åȤ�
    @b Mmethod �ѥ�᡼�����ͤȤ����Ѥ���줿���ˤϡ�����ʸ�����åȤ��̤�ʸ�����åȡʿ�ʸ�����åȡˤ���ʬ����Ǥ��뤳�Ȥ��̣���롣
    ��ʸ�����åȤ� @b Mparents �ѥ�᡼���ˤ�ä�Ϳ�����ʤ��ƤϤʤ�ʤ���
    �����ɥݥ���Ȥ�ʸ�����åȤ�ʸ�������ɤδ֤��Ѵ��ϡ���ǰŪ�ˤϰʲ��μ��˽�����

@verbatim
CHARACTER-CODE = PARENT-CODE (CODE-POINT) + SUBSET-OFFSET
@endverbatim

    ������ PARENT-CODE �� CODE-POINT 
    �ο�ʸ�����å���Ǥ�ʸ�������ɤ��֤����ؿ��Ǥ��ꡢSUBSET-OFFSET �� 
    @b Msubset_offset �ѥ�᡼����Ϳ�������ͤǤ��롣
    */

MSymbol Msubset;
/*=*/

/***en
    @brief Symbol for the superset type method of charset.

    The symbol #Msuperset has the name <tt>"superset"</tt> and, when
    used as a value of @b Mmethod parameter of a charset, it means that
    the charset is a superset of parent charsets.  The parent charsets
    must be given by @b Mparents parameter.  */

/***ja
    @brief �����ѡ����åȷ��Υ᥽�åɤ򼨤�����ܥ�.

    ����ܥ� #Msuperset �� <tt>"superset"</tt> �Ȥ���̾���������ʸ�����åȤ�
    @b Mmethod �ѥ�᡼�����ͤȤ����Ѥ���줿���ˤϡ�����ʸ�����åȤ��̤�ʸ�����åȡʿ�ʸ�����åȡˤξ�̽���Ǥ��뤳�Ȥ��̣���롣
    ��ʸ�����åȤ� @b Mparents �ѥ�᡼���ˤ�ä�Ϳ�����ʤ��ƤϤʤ�ʤ���
    */

MSymbol Msuperset;
/*=*/
/*** @}  */ 

/***en
    @brief Define a charset.

    The mchar_define_charset () function defines a new charset and
    makes it accessible via a symbol whose name is $NAME.  $PLIST
    specifies parameters of the charset as below:

    <ul>

    <li> Key is @b Mmethod, value is a symbol.

    The value specifies the method for decoding/encoding code-points
    in the charset.  It must be #Moffset, #Mmap (default), #Munify,
    #Msubset, or #Msuperset.

    <li> Key is @b Mdimension, value is an integer

    The value specifies the dimension of code-points of the charset.
    It must be 1 (default), 2, 3, or 4.

    <li> Key is @b Mmin_range, value is an unsigned integer

    The value specifies the minimum range of a code-point, which means
    that the Nth byte of the value is the minimum Nth byte of
    code-points of the charset.   The default value is 0.

    <li> Key is @b Mmax_range, value is an unsigned integer

    The value specifies the maximum range of a code-point, which means
    that the Nth byte of the value is the maximum Nth byte of
    code-points of the charset.  The default value is 0xFF, 0xFFFF,
    0xFFFFFF, or 0xFFFFFFFF if the dimension is 1, 2, 3, or 4
    respectively.

    <li> Key is @b Mmin_code, value is an unsigned integer

    The value specifies the minimum code-point of
    the charset.  The default value is the minimum range.

    <li> Key is @b Mmax_code, value is an unsigned integer

    The value specifies the maximum code-point of
    the charset.  The default value is the maximum range.

    <li> Key is @b Mascii_compatible, value is a symbol

    The value specifies whether the charset is ASCII compatible or
    not.  If the value is #Mnil (default), it is not ASCII
    compatible, else compatible.

    <li> Key is @b Mfinal_byte, value is an integer

    The value specifies the @e final @e byte of the charset registered
    in The International Registry.  It must be 0 (default) or 32..127.
    The value 0 means that the charset is not in the registry.

    <li> Key is @b Mrevision, value is an integer

    The value specifies the @e revision @e number of the charset
    registered in The International Registry.  It must be 0..127.  If
    the charset is not in The International Registry, the value is
    ignored.  The value 0 means that the charset has no revision
    number.

    <li> Key is @b Mmin_char, value is an integer

    The value specifies the minimum character code of the charset.
    The default value is 0.

    <li> Key is @b Mmapfile, value is an M-text

    If the method is #Mmap or #Munify, a data that contains
    mapping information is added to the m17n database by calling
    the function mdatabase_define () with the value as an argument $EXTRA_INFO,
    i.e. the value is used as a file name of the data.

    Otherwise, this parameter is ignored.

    <li> Key is @b Mparents, value is a plist

    If the method is #Msubset, the value must is a plist of length
    1, and the value of the plist must be a symbol representing a
    parent charset.

    If the method is #Msuperset, the value must be a plist of length
    less than 9, and the values of the plist must be symbols
    representing subset charsets.

    Otherwise, this parameter is ignored.

    <li> Key is @b Mdefine_coding, value is a symbol

    If the dimension of the charset is 1, the value specifies whether
    or not to define a coding system of the same name whose type is
    #Mcharset.  A coding system is defined if the value is not #Mnil.

    Otherwise, this parameter is ignored.

    </ul>

    @return
    If the operation was successful, mchar_define_charset () returns a
    symbol whose name is $NAME.  Otherwise it returns #Mnil and
    assigns an error code to the external variable #merror_code.  */

/***ja
    @brief ʸ�����åȤ��������.

    �ؿ� mchar_define_charset () �Ͽ�����ʸ�����åȤ������������� 
    $NAME �Ȥ���̾������ĥ���ܥ��ͳ�ǥ��������Ǥ���褦�ˤ��롣
    $PLIST ����������ʸ�����åȤΥѥ�᡼����ʲ��Τ褦�˻��ꤹ�롣

    <ul>

    <li> ������ @b Mmethod ���ͤ�����ܥ�λ�

    �ͤϡ�#Moffset, #Mmap (�ǥե������), #Munify, #Msubset,
    #Msuperset �Τ����줫�Ǥ��ꡢʸ�����åȤΥ����ɥݥ���Ȥ�ǥ����ɡ����󥳡��ɤ���ݤΥ᥽�åɤ���ꤹ�롣

    <li> ������ @b Mdimension ���ͤ������ͤλ�

    �ͤϡ�1 (�ǥե������), 2, 3, 4 
    �Τ����줫�Ǥ��ꡢʸ�����åȤΥ����ɥݥ���Ȥμ����Ǥ��롣

    <li> ������ @b Mmin_range ���ͤ����������ͤλ�

    �ͤϥ����ɥݥ���ȤκǾ����ͤǤ��롣���ʤ���������ͤ� N 
    ���ܤΥХ��ȤϤ���ʸ�����åȤΥ����ɥݥ���Ȥ� N ���ܤΥХ��ȤκǾ��Τ�ΤȤʤ롣
    �ǥե�����ͤ� 0 ��

    <li> ������ @b Mmax_range ���ͤ����������ͤλ�

    �ͤϥ����ɥݥ���Ȥκ�����ͤǤ��롣���ʤ���������ͤ� N 
    ���ܤΥХ��ȤϤ���ʸ�����åȤΥ����ɥݥ���Ȥ� N ���ܤΥХ��Ȥκ���Τ�ΤȤʤ롣
    �ǥե�����ͤϡ������ɥݥ���Ȥμ����� 1, 2, 3, 4 �λ������줾��
    0xFF, 0xFFFF, 0xFFFFFF, 0xFFFFFFFF ��

    <li> ������ @b Mmin_code ���ͤ����������ͤλ�

    �ͤϤ���ʸ�����åȤκǾ��Υ����ɥݥ���ȤǤ��롣�ǥե�����ͤ� 
    @b Mmin_range ���͡�

    <li> ������ @b Mmax_code ���ͤ����������ͤλ�

    �ͤϤ���ʸ�����åȤκ���Υ����ɥݥ���ȤǤ��롣�ǥե�����ͤ� 
    @b Mmax_range ���͡�

    <li> ������  @b Mascii_compatible ���ͤ�����ܥ�λ�

    �ͤϤ���ʸ�����åȤ� ASCII �ߴ��Ǥ��뤫�ɤ����򼨤����ǥե�����ͤ�
    #Mnil �Ǥ���иߴ��ǤϤʤ�������ʳ��ξ��ϸߴ��Ǥ��롣

    <li> ������  @b Mfinal_byte ���ͤ������ͤλ�

    �ͤϤ���ʸ�����åȤ� The International Registry ����Ͽ����Ƥ��� 
    @e ��ü�Х��� �Ǥ��ꡢ0 (�ǥե������) �Ǥ��뤫 32..127 �Ǥ��롣0 
    ����Ͽ����Ƥ��ʤ����Ȥ��̣���롣

    <li> ������  @b Mrevision ���ͤ������ͤλ�

    �ͤ� The International Registry ����Ͽ����Ƥ��� @e revision @e
    number �Ǥ��ꡢ0..127 �Ǥ��롣
    ʸ�����åȤ���Ͽ����Ƥ��ʤ����ˤϤ����ͤ�̵�뤵��롣
    0 �� revision number ��¸�ߤ��ʤ����Ȥ��̣���롣

    <li> ������  @b Mmin_char ���ͤ������ͤλ�

    �ͤϤ���ʸ�����åȤκǾ���ʸ�������ɤǤ��롣�ǥե�����ͤ� 0 ��

    <li> ������ @b Mmapfile ���ͤ� M-text �λ�

    �᥽�åɤ� #Mmap �� #Munify �λ����ؿ� mdatabase_define () 
    �򤳤��ͤ���� $EXTRA_INFO �Ȥ��ƸƤ֤��Ȥˤ�äơ��ޥåԥ󥰤˴ؤ���ǡ�����
    m17n �ǡ����١������ɲä���롣
    ���ʤ���������ͤϥǡ����ե������̾���Ǥ��롣

    �����Ǥʤ���С����Υѥ�᡼����̵�뤵��롣

    <li> ������ @b Mparents ���ͤ� plist �λ�

    �᥽�åɤ� #Msubset �ʤ�С��ͤ�Ĺ�� 1 �� plist 
    �Ǥ��ꡢ�����ͤϤ���ʸ�����åȤξ�̽���Ȥʤ�ʸ�����åȤ򼨤�����ܥ�Ǥ��롣

    �᥽�åɤ� #Msuperset �ʤ�С��ͤ�Ĺ�� 8 �ʲ��� plist 
    �Ǥ��ꡢ�������ͤϤ���ʸ�����åȤβ��̽���Ǥ���ʸ�����åȤ򼨤�����ܥ�Ǥ��롣

    �����Ǥʤ���С����Υѥ�᡼����̵�뤵��롣

    <li> ������  @b Mdefine_coding ���ͤ�����ܥ�λ�

    ʸ�����åȤμ����� 1 �ʤ�С��ͤ� #Mnil �ʳ��ξ��� #Mcharset ��
    ��Ʊ��̾������ĥ����ɷϤ�������롣

    �����Ǥʤ���С����Υѥ�᡼����̵�뤵��롣

    </ul>

    @return 
    ��������������С�mchar_define_charset() �� $NAME 
    �Ȥ���̾���Υ���ܥ���֤��������Ǥʤ���� #Mnil ���֤��������ѿ� 
    #merror_code �˥��顼�����ɤ����ꤹ�롣*/

/***
    @errors
    @c MERROR_CHARSET  */

MSymbol
mchar_define_charset (const char *name, MPlist *plist)
{
  MSymbol sym = msymbol (name);
  MCharset *charset;
  int i;
  unsigned min_range, max_range;
  MPlist *pl;
  MText *mapfile = (MText *) mplist_get (plist, Mmapfile);

  MSTRUCT_CALLOC (charset, MERROR_CHARSET);
  charset->name = sym;
  charset->method = (MSymbol) mplist_get (plist, Mmethod);
  if (! charset->method)
    {
      if (mapfile)
	charset->method = Mmap;
      else
	charset->method = Moffset;
    }
  if (charset->method == Mmap || charset->method == Munify)
    {
      if (! mapfile)
	MERROR (MERROR_CHARSET, Mnil);
      mdatabase_define (Mcharset, sym, Mnil, Mnil, NULL, mapfile->data);
    }
  if (! (charset->dimension = (int) mplist_get (plist, Mdimension)))
    charset->dimension = 1;

  min_range = (unsigned) mplist_get (plist, Mmin_range);
  if ((pl = mplist_find_by_key (plist, Mmax_range)))
    {
      max_range = (unsigned) MPLIST_VAL (pl);
      if (max_range >= 0x1000000)
	charset->dimension = 4;
      else if (max_range >= 0x10000 && charset->dimension < 3)
	charset->dimension = 3;
      else if (max_range >= 0x100 && charset->dimension < 2)
	charset->dimension = 2;
    }
  else if (charset->dimension == 1)
    max_range = 0xFF;
  else if (charset->dimension == 2)
    max_range = 0xFFFF;
  else if (charset->dimension == 3)
    max_range = 0xFFFFFF;
  else
    max_range = 0xFFFFFFFF;

  memset (charset->code_range, 0, sizeof charset->code_range);
  for (i = 0; i < charset->dimension; i++, min_range >>= 8, max_range >>= 8)
    {
      charset->code_range[i * 4] = min_range & 0xFF;
      charset->code_range[i * 4 + 1] = max_range & 0xFF;
    }
  if ((charset->min_code = (int) mplist_get (plist, Mmin_code)) < min_range)
    charset->min_code = min_range;
  if ((charset->max_code = (int) mplist_get (plist, Mmax_code)) > max_range)
    charset->max_code = max_range;
  charset->ascii_compatible
    = (MSymbol) mplist_get (plist, Mascii_compatible) != Mnil;
  charset->final_byte = (int) mplist_get (plist, Mfinal_byte);
  charset->revision = (int) mplist_get (plist, Mrevision);
  charset->min_char = (int) mplist_get (plist, Mmin_char);
  pl = (MPlist *) mplist_get (plist, Mparents);
  charset->nparents = pl ? mplist_length (pl) : 0;
  if (charset->nparents > 8)
    charset->nparents = 8;
  for (i = 0; i < charset->nparents; i++, pl = MPLIST_NEXT (pl))
    {
      MSymbol parent_name;

      if (MPLIST_KEY (pl) != Msymbol)
	MERROR (MERROR_CHARSET, Mnil);
      parent_name = MPLIST_SYMBOL (pl);
      if (! (charset->parents[i] = MCHARSET (parent_name)))
	MERROR (MERROR_CHARSET, Mnil);
    }

  charset->subset_offset = (int) mplist_get (plist, Msubset_offset);

  msymbol_put (sym, Mcharset, charset);
  charset = make_charset (charset);
  if (! charset)
    return Mnil;
  msymbol_put (msymbol__canonicalize (sym), Mcharset, charset);

  for (pl = (MPlist *) mplist_get (plist, Maliases);
       pl && MPLIST_KEY (pl) == Msymbol;
       pl = MPLIST_NEXT (pl))
    {
      MSymbol alias = MPLIST_SYMBOL (pl);

      msymbol_put (alias, Mcharset, charset);
      msymbol_put (msymbol__canonicalize (alias), Mcharset, charset);
    }

  if (mplist_get (plist, Mdefine_coding)
      && charset->dimension == 1
      && charset->code_range[0] == 0 && charset->code_range[1] == 255)
    mconv__register_charset_coding (sym);
  return (sym);
}

/*=*/

/***en
    @brief Resolve charset name.

    The mchar_resolve_charset () function returns $SYMBOL if it
    represents a charset.  Otherwise, canonicalize $SYMBOL as to a
    charset name, and if the canonicalized name represents a charset,
    return it.  Otherwise, return #Mnil.  */

/***ja
    @brief ʸ�����å�̾���褹��.

    �ؿ� mchar_resolve_charset () �� $SYMBOL 
    ��ʸ�����åȤ򼨤��Ƥ���Ф�����֤���

    �����Ǥʤ���С�$SYMBOL ��ʸ�����å�̾�Ȥ����������������줬ʸ�����åȤ򼨤��Ƥ��Ƥ����������������Τ��֤���
    �����Ǥʤ���С�#Mnil ���֤��� */

MSymbol
mchar_resolve_charset (MSymbol symbol)
{
  MCharset *charset = (MCharset *) msymbol_get (symbol, Mcharset);

  if (! charset)
    {
      symbol = msymbol__canonicalize (symbol);
      charset = (MCharset *) msymbol_get (symbol, Mcharset);
    }

  return (charset ? charset->name : Mnil);
}

/*=*/

/***en
    @brief List symbols representing charsets.

    The mchar_list_charsets () function makes an array of symbols
    representing a charset, stores the pointer to the array in a place
    pointed to by $SYMBOLS, and returns the length of the array.  */

/***ja
    @brief ʸ�����åȤ�ɽ�魯����ܥ����󤹤�.

    �ؿ� mchar_list_charsets () 
    �ϡ�ʸ�����åȤ򼨤�����ܥ���¤٤�������ꡢ$SYMBOLS 
    �ǥݥ���Ȥ��줿���ˤ�������ؤΥݥ��󥿤��֤��������Ĺ�����֤��� */

int
mchar_list_charset (MSymbol **symbols)
{
  int i;

  MTABLE_MALLOC ((*symbols), charset_list.used, MERROR_CHARSET);
  for (i = 0; i < charset_list.used; i++)
    (*symbols)[i] = charset_list.charsets[i]->name;
  return i;
}

/*=*/

/***en
    @brief Decode a code-point.

    The mchar_decode () function decodes code-point $CODE in the
    charset represented by the symbol $CHARSET_NAME to get a character
    code.

    @return
    If decoding was successful, mchar_decode () returns the decoded
    character code.  Otherwise it returns -1.  */

/***ja
    @brief �����ɥݥ���Ȥ�ǥ����ɤ���.

    �ؿ� mchar_decode () �ϡ�����ܥ� $CHARSET_NAME �Ǽ������ʸ�����å����
    $CODE �Ȥ��������ɥݥ���Ȥ�ǥ����ɤ���ʸ�������ɤ����롣

    @return
    �ǥ����ɤ���������С�mchar_decode () �ϥǥ����ɤ��줿ʸ�������ɤ��֤���
    �����Ǥʤ���� -1 ���֤���  */

/***
    @seealso
    mchar_encode ()  */

int
mchar_decode (MSymbol charset_name, unsigned code)
{
  MCharset *charset = MCHARSET (charset_name);

  if (! charset)
    return MCHAR_INVALID_CODE;
  return DECODE_CHAR (charset, code);
}

/*=*/

/***en
    @brief Encode a character code.

    The mchar_encode () function encodes character code $C to get a
    code-point in the charset represented by the symbol $CHARSET_NAME.

    @return
    If encoding was successful, mchar_encode () returns the encoded
    code-point.  Otherwise it returns #MCHAR_INVALID_CODE.  */

/***ja
    @brief ʸ�������ɤ򥨥󥳡��ɤ���.

    �ؿ� mchar_encode () �ϡ�ʸ�������� $C �򥨥󥳡��ɤ��ƥ���ܥ�
    $CHARSET_NAME �Ǽ������ʸ�����å���ˤ����륳���ɥݥ���Ȥ����롣

    @return
    ���󥳡��ɤ���������С�mchar_encode () �ϥ��󡼥ɤ��줿�����ɥݥ���Ȥ��֤���
    �����Ǥʤ���� #MCHAR_INVALID_CODE ���֤���  */

/***
    @seealso
    mchar_decode ()  */

unsigned
mchar_encode (MSymbol charset_name, int c)
{
  MCharset *charset = MCHARSET (charset_name);

  if (! charset)
    return MCHAR_INVALID_CODE;
  return ENCODE_CHAR (charset, c);
}

/*=*/

/***en
    @brief Call a function for all the characters in a specified charset.

    The mcharset_map_chars () function calls $FUNC for all the
    characters in the charset named $CHARSET_NAME.  A call is done for
    a chunk of consecutive characters rather than character by
    character.

    $FUNC receives three arguments: $FROM, $TO, and $ARG.  $FROM and
    $TO specify the range of character codes in $CHARSET.  $ARG is the
    same as $FUNC_ARG.

    @return
    If the operation was successful, mcharset_map_chars () returns 0.
    Otherwise, it returns -1 and assigns an error code to the external
    variable #merror_code.  */

/***ja
    @brief ���ꤷ��ʸ�����åȤΤ��٤Ƥ�ʸ�����Ф��ƴؿ���Ƥ�.

    �ؿ� mcharset_map_chars () �� $CHARSET_NAME 
    �Ȥ���̾�������ʸ�����å���Τ��٤Ƥ�ʸ�����Ф��� $FUNC ��Ƥ֡�
    �ƤӽФ��ϰ�ʸ����ǤϤʤ���Ϣ³����ʸ���ΤޤȤޤ�ñ�̤ǹԤʤ��롣

    �ؿ� $FUNC �ˤ�$FROM, $TO, $ARG �Σ��������Ϥ���롣$FROM �� $TO 
    �� $CHARSET ���ʸ�������ɤ��ϰϤ���ꤹ�롣$ARG �� $FUNC_ARG 
    ��Ʊ���Ǥ��롣

    @return
    ��������������� mcharset_map_chars () �� 0 ���֤���
    �����Ǥʤ���� -1 ���֤��������ѿ� #merror_code �˥��顼�����ɤ����ꤹ�롣  */

/*** 
    @errors
    @c MERROR_CHARSET */

int
mchar_map_charset (MSymbol charset_name,
		   void (*func) (int from, int to, void *arg),
		   void *func_arg)
{
  MCharset *charset;

  charset = MCHARSET (charset_name);
  if (! charset)
    MERROR (MERROR_CHARSET, -1);

  if (charset->encoder)
    {
      int c = charset->min_char;
      int next_c;

      if ((int) mchartable__lookup (charset->encoder, c, &next_c, 1) < 0)
	c = next_c;
      while (c <= charset->max_char)
	{
	  if ((int) mchartable__lookup (charset->encoder, c, &next_c, 1) >= 0)
	    (*func) (c, next_c - 1, func_arg);	  
	  c = next_c;
	}
    }
  else
    (*func) (charset->min_char, charset->max_char, func_arg);
  return 0;
}

/*=*/

/*** @} */

/*
  Local Variables:
  coding: euc-japan
  End:
*/
