/* mtext.c -- M-text module.
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
    @addtogroup m17nMtext
    @brief M-text objects and API for them.

    In the m17n library, text is represented as an object called @e
    M-text rather than as a C-string (<tt>char *</tt> or <tt>unsigned
    char *</tt>).  An M-text is a sequence of characters whose length
    is equals to or more than 0, and can be coined from various
    character sources, e.g. C-strings, files, character codes, etc.

    M-texts are more useful than C-strings in the following points.

    @li M-texts can handle mixture of characters of various scripts,
    including all Unicode characters and more.  This is an
    indispensable facility when handling multilingual text.

    @li Each character in an M-text can have properties called @e text
    @e properties. Text properties store various kinds of information
    attached to parts of an M-text to provide application programs
    with a unified view of those information.  As rich information can
    be stored in M-texts in the form of text properties, functions in
    application programs can be simple.

    In addition, the library provides many functions to manipulate an
    M-text just the same way as a C-string.  */

/***ja
    @addtogroup m17nMtext

    @brief M-text ���֥������ȤȤ���˴ؤ��� API.

    m17n �饤�֥��ϡ� C-string��<tt>char *</tt> �� <tt>unsigned
    char *</tt>�ˤǤϤʤ� @e M-text �ȸƤ֥��֥������Ȥǥƥ����Ȥ�ɽ�����롣
    M-text ��Ĺ�� 0 �ʾ��ʸ����Ǥ��ꡢ���ʸ���������ʤ��Ȥ��� 
    C-string���ե����롢ʸ�����������ˤ�������Ǥ��롣

    M-text �ˤϡ�C-string �ˤʤ��ʲ�����ħ�����롣     

    @li M-text ������¿���μ����ʸ����Ʊ���ˡ����ߤ����ơ�Ʊ���˰������Ȥ��Ǥ��롣
    Unicode �����Ƥ�ʸ���Ϥ����󡢤��¿����ʸ���ޤǤⰷ�����Ȥ��Ǥ��롣
    �����¿����ƥ����Ȥ򰷤���Ǥ�ɬ�ܤε�ǽ�Ǥ��롣

    @li M-text ��γ�ʸ���ϡ�@e �ƥ����ȥץ�ѥƥ� 
    �ȸƤФ��ץ�ѥƥ��������
    �ƥ����ȥץ�ѥƥ��ˤ�äơ��ƥ����Ȥγ����̤˴ؤ����͡��ʾ����
    M-text ����ݻ����뤳�Ȥ���ǽ�ˤʤ롣
    ���Τ��ᡢ�����ξ���򥢥ץꥱ�������ץ�����������Ū�˰������Ȥ���ǽ�ˤʤ롣
    �ޤ���M-text 
    ���Τ�˭�٤ʾ������Ĥ��ᡢ���ץꥱ�������ץ������γƴؿ�����ǲ����뤳�Ȥ��Ǥ��롣

    �����m17n �饤�֥��ϡ� C-string 
    �����뤿����󶡤�����δؿ���Ʊ���Τ�Τ� M-text 
    �����뤿��˥��ݡ��Ȥ��Ƥ��롣  */

/*=*/

#if !defined (FOR_DOXYGEN) || defined (DOXYGEN_INTERNAL_MODULE)
/*** @addtogroup m17nInternal
     @{ */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "m17n.h"
#include "m17n-misc.h"
#include "internal.h"
#include "textprop.h"
#include "character.h"
#include "mtext.h"
#include "plist.h"

static M17NObjectArray mtext_table;

static MSymbol M_charbag;

/** Increment character position CHAR_POS and unit position UNIT_POS
    so that they point to the next character in M-text MT.  No range
    check for CHAR_POS and UNIT_POS.  */

#define INC_POSITION(mt, char_pos, unit_pos)			\
  do {								\
    int c;							\
								\
    if ((mt)->format <= MTEXT_FORMAT_UTF_8)			\
      {								\
	c = (mt)->data[(unit_pos)];				\
	(unit_pos) += CHAR_UNITS_BY_HEAD_UTF8 (c);		\
      }								\
    else if ((mt)->format <= MTEXT_FORMAT_UTF_16BE)		\
      {								\
	c = ((unsigned short *) ((mt)->data))[(unit_pos)];	\
								\
	if ((mt)->format != MTEXT_FORMAT_UTF_16)		\
	  c = SWAP_16 (c);					\
	(unit_pos) += CHAR_UNITS_BY_HEAD_UTF16 (c);		\
      }								\
    else							\
      (unit_pos)++;						\
    (char_pos)++;						\
  } while (0)


/** Decrement character position CHAR_POS and unit position UNIT_POS
    so that they point to the previous character in M-text MT.  No
    range check for CHAR_POS and UNIT_POS.  */

#define DEC_POSITION(mt, char_pos, unit_pos)				\
  do {									\
    if ((mt)->format <= MTEXT_FORMAT_UTF_8)				\
      {									\
	unsigned char *p1 = (mt)->data + (unit_pos);			\
	unsigned char *p0 = p1 - 1;					\
									\
	while (! CHAR_HEAD_P (p0)) p0--;				\
	(unit_pos) -= (p1 - p0);					\
      }									\
    else if ((mt)->format <= MTEXT_FORMAT_UTF_16BE)			\
      {									\
	int c = ((unsigned short *) ((mt)->data))[(unit_pos) - 1];	\
									\
	if ((mt)->format != MTEXT_FORMAT_UTF_16)			\
	  c = SWAP_16 (c);						\
	(unit_pos) -= 2 - (c < 0xD800 || c >= 0xE000);			\
      }									\
    else								\
      (unit_pos)--;							\
    (char_pos)--;							\
  } while (0)

#define FORMAT_COVERAGE(fmt)					\
  (fmt == MTEXT_FORMAT_UTF_8 ? MTEXT_COVERAGE_FULL		\
   : fmt == MTEXT_FORMAT_US_ASCII ? MTEXT_COVERAGE_ASCII	\
   : fmt >= MTEXT_FORMAT_UTF_32LE ? MTEXT_COVERAGE_FULL		\
   : MTEXT_COVERAGE_UNICODE)

/* Compoare sub-texts in MT1 (range FROM1 and TO1) and MT2 (range
   FROM2 to TO2). */

static int
compare (MText *mt1, int from1, int to1, MText *mt2, int from2, int to2)
{
  if (mt1->format == mt2->format
      && (mt1->format <= MTEXT_FORMAT_UTF_8))
    {
      unsigned char *p1, *pend1, *p2, *pend2;
      int unit_bytes = UNIT_BYTES (mt1->format);
      int nbytes;
      int result;

      p1 = mt1->data + mtext__char_to_byte (mt1, from1) * unit_bytes;
      pend1 = mt1->data + mtext__char_to_byte (mt1, to1) * unit_bytes;

      p2 = mt2->data + mtext__char_to_byte (mt2, from2) * unit_bytes;
      pend2 = mt2->data + mtext__char_to_byte (mt2, to2) * unit_bytes;

      if (pend1 - p1 < pend2 - p2)
	nbytes = pend1 - p1;
      else
	nbytes = pend2 - p2;
      result = memcmp (p1, p2, nbytes);
      if (result)
	return result;
      return ((pend1 - p1) - (pend2 - p2));
    }
  for (; from1 < to1 && from2 < to2; from1++, from2++)
    {
      int c1 = mtext_ref_char (mt1, from1);
      int c2 = mtext_ref_char (mt2, from2);

      if (c1 != c2)
	return (c1 > c2 ? 1 : -1);
    }
  return (from2 == to2 ? (from1 < to1) : -1);
}


/* Return how many units are required in UTF-8 to represent characters
   between FROM and TO of MT.  */

static int
count_by_utf_8 (MText *mt, int from, int to)
{
  int n, c;

  for (n = 0; from < to; from++)
    {
      c = mtext_ref_char (mt, from);
      n += CHAR_UNITS_UTF8 (c);
    }
  return n;
}


/* Return how many units are required in UTF-16 to represent
   characters between FROM and TO of MT.  */

static int
count_by_utf_16 (MText *mt, int from, int to)
{
  int n, c;

  for (n = 0; from < to; from++)
    {
      c = mtext_ref_char (mt, from);
      n += CHAR_UNITS_UTF16 (c);
    }
  return n;
}


/* Insert text between FROM and TO of MT2 at POS of MT1.  */

static MText *
insert (MText *mt1, int pos, MText *mt2, int from, int to)
{
  int pos_unit = POS_CHAR_TO_BYTE (mt1, pos);
  int from_unit = POS_CHAR_TO_BYTE (mt2, from);
  int new_units = POS_CHAR_TO_BYTE (mt2, to) - from_unit;
  int unit_bytes;

  if (mt1->nchars == 0)
    mt1->format = mt2->format, mt1->coverage = mt2->coverage;
  else if (mt1->format != mt2->format)
    {
      /* Be sure to make mt1->format sufficient to contain all
	 characters in mt2.  */
      if (mt1->format == MTEXT_FORMAT_UTF_8
	  || mt1->format == MTEXT_FORMAT_UTF_32
	  || (mt1->format == MTEXT_FORMAT_UTF_16
	      && mt2->format <= MTEXT_FORMAT_UTF_16BE
	      && mt2->format != MTEXT_FORMAT_UTF_8))
	;
      else if (mt1->format == MTEXT_FORMAT_US_ASCII)
	{
	  if (mt2->format == MTEXT_FORMAT_UTF_8)
	    mt1->format = MTEXT_FORMAT_UTF_8, mt1->coverage = mt2->coverage;
	  else if (mt2->format == MTEXT_FORMAT_UTF_16
		   || mt2->format == MTEXT_FORMAT_UTF_32)
	    mtext__adjust_format (mt1, mt2->format);
	  else
	    mtext__adjust_format (mt1, MTEXT_FORMAT_UTF_8);
	}
      else
	{
	  mtext__adjust_format (mt1, MTEXT_FORMAT_UTF_8);
	  pos_unit = POS_CHAR_TO_BYTE (mt1, pos);
	}
    }

  unit_bytes = UNIT_BYTES (mt1->format);

  if (mt1->format == mt2->format)
    {
      int pos_byte = pos_unit * unit_bytes;
      int total_bytes = (mt1->nbytes + new_units) * unit_bytes;
      int new_bytes = new_units * unit_bytes;

      if (total_bytes + unit_bytes > mt1->allocated)
	{
	  mt1->allocated = total_bytes + unit_bytes;
	  if (mt1->data)
	    MTABLE_REALLOC (mt1->data, mt1->allocated, MERROR_MTEXT);
	  else
	    MTABLE_CALLOC (mt1->data, mt1->allocated, MERROR_MTEXT);
	}
      memmove (mt1->data + pos_byte + new_bytes, mt1->data + pos_byte,
	       (mt1->nbytes - pos_unit + 1) * unit_bytes);
      memcpy (mt1->data + pos_byte, mt2->data + from_unit * unit_bytes,
	      new_bytes);
    }
  else if (mt1->format == MTEXT_FORMAT_UTF_8)
    {
      unsigned char *p;
      int total_bytes, i, c;

      new_units = count_by_utf_8 (mt2, from, to);
      total_bytes = mt1->nbytes + new_units;

      if (total_bytes + 1 > mt1->allocated)
	{
	  mt1->allocated = total_bytes + 1;
	  MTABLE_REALLOC (mt1->data, mt1->allocated, MERROR_MTEXT);
	}
      p = mt1->data + pos_unit;
      memmove (p + new_units, p, mt1->nbytes - pos_unit + 1);
      for (i = from; i < to; i++)
	{
	  c = mtext_ref_char (mt2, i);
	  p += CHAR_STRING_UTF8 (c, p);
	}
    }
  else if (mt1->format == MTEXT_FORMAT_UTF_16)
    {
      unsigned short *p;
      int total_bytes, i, c;

      new_units = count_by_utf_16 (mt2, from, to);
      total_bytes = (mt1->nbytes + new_units) * USHORT_SIZE;

      if (total_bytes + USHORT_SIZE > mt1->allocated)
	{
	  mt1->allocated = total_bytes + USHORT_SIZE;
	  MTABLE_REALLOC (mt1->data, mt1->allocated, MERROR_MTEXT);
	}
      p = (unsigned short *) mt1->data + pos_unit;
      memmove (p + new_units, p,
	       (mt1->nbytes - pos_unit + 1) * USHORT_SIZE);
      for (i = from; i < to; i++)
	{
	  c = mtext_ref_char (mt2, i);
	  p += CHAR_STRING_UTF16 (c, p);
	}
    }
  else				/* MTEXT_FORMAT_UTF_32 */
    {
      unsigned int *p;
      int total_bytes, i;

      new_units = to - from;
      total_bytes = (mt1->nbytes + new_units) * UINT_SIZE;

      if (total_bytes + UINT_SIZE > mt1->allocated)
	{
	  mt1->allocated = total_bytes + UINT_SIZE;
	  MTABLE_REALLOC (mt1->data, mt1->allocated, MERROR_MTEXT);
	}
      p = (unsigned *) mt1->data + pos_unit;
      memmove (p + new_units, p,
	       (mt1->nbytes - pos_unit + 1) * UINT_SIZE);
      for (i = from; i < to; i++)
	*p++ = mtext_ref_char (mt2, i);
    }

  mtext__adjust_plist_for_insert
    (mt1, pos, to - from,
     mtext__copy_plist (mt2->plist, from, to, mt1, pos));
  mt1->nchars += to - from;
  mt1->nbytes += new_units;
  if (mt1->cache_char_pos > pos)
    {
      mt1->cache_char_pos += to - from;
      mt1->cache_byte_pos += new_units;
    }

  return mt1;
}


static MCharTable *
get_charbag (MText *mt)
{
  MTextProperty *prop = mtext_get_property (mt, 0, M_charbag);
  MCharTable *table;
  int i;

  if (prop)
    {
      if (prop->end == mt->nchars)
	return ((MCharTable *) prop->val);
      mtext_detach_property (prop);
    }

  table = mchartable (Msymbol, (void *) 0);
  for (i = mt->nchars - 1; i >= 0; i--)
    mchartable_set (table, mtext_ref_char (mt, i), Mt);
  prop = mtext_property (M_charbag, table, MTEXTPROP_VOLATILE_WEAK);
  mtext_attach_property (mt, 0, mtext_nchars (mt), prop);
  M17N_OBJECT_UNREF (prop);
  return table;
}


/* span () : Number of consecutive chars starting at POS in MT1 that
   are included (if NOT is Mnil) or not included (if NOT is Mt) in
   MT2.  */

static int
span (MText *mt1, MText *mt2, int pos, MSymbol not)
{
  int nchars = mtext_nchars (mt1);
  MCharTable *table = get_charbag (mt2);
  int i;

  for (i = pos; i < nchars; i++)
    if ((MSymbol) mchartable_lookup (table, mtext_ref_char (mt1, i)) == not)
      break;
  return (i - pos);
}


static int
count_utf_8_chars (const void *data, int nitems)
{
  unsigned char *p = (unsigned char *) data;
  unsigned char *pend = p + nitems;
  int nchars = 0;

  while (p < pend)
    {
      int i, n;

      for (; p < pend && *p < 128; nchars++, p++);
      if (p == pend)
	return nchars;
      if (! CHAR_HEAD_P_UTF8 (p))
	return -1;
      n = CHAR_UNITS_BY_HEAD_UTF8 (*p);
      if (p + n > pend)
	return -1;
      for (i = 1; i < n; i++)
	if (CHAR_HEAD_P_UTF8 (p + i))
	  return -1;
      p += n;
      nchars++;
    }
  return nchars;
}

static int
count_utf_16_chars (const void *data, int nitems, int swap)
{
  unsigned short *p = (unsigned short *) data;
  unsigned short *pend = p + nitems;
  int nchars = 0;
  int prev_surrogate = 0;

  for (; p < pend; p++)
    {
      int c = *p;

      if (swap)
	c = SWAP_16 (c);
      if (prev_surrogate)
	{
	  if (c < 0xDC00 || c >= 0xE000)
	    /* Invalid surrogate */
	    nchars++;
	}
      else
	{
	  if (c >= 0xD800 && c < 0xDC00)
	    prev_surrogate = 1;
	  nchars++;
	}
    }
  if (prev_surrogate)
    nchars++;
  return nchars;
}


static int
find_char_forward (MText *mt, int from, int to, int c)
{
  int from_byte = POS_CHAR_TO_BYTE (mt, from);

  if (mt->format <= MTEXT_FORMAT_UTF_8)
    {
      unsigned char *p = mt->data + from_byte;

      while (from < to && STRING_CHAR_ADVANCE_UTF8 (p) != c) from++;
    }
  else if (mt->format <= MTEXT_FORMAT_UTF_16BE)
    {
      unsigned short *p = (unsigned short *) (mt->data) + from_byte;

      if (mt->format == MTEXT_FORMAT_UTF_16)
	while (from < to && STRING_CHAR_ADVANCE_UTF16 (p) != c) from++;
      else if (c < 0x10000)
	{
	  c = SWAP_16 (c);
	  while (from < to && *p != c)
	    {
	      from++;
	      p += ((*p & 0xFF) < 0xD8 || (*p & 0xFF) >= 0xE0) ? 1 : 2;
	    }
	}
      else if (c < 0x110000)
	{
	  int c1 = (c >> 10) + 0xD800;
	  int c2 = (c & 0x3FF) + 0xDC00;

	  c1 = SWAP_16 (c1);
	  c2 = SWAP_16 (c2);
	  while (from < to && (*p != c1 || p[1] != c2))
	    {
	      from++;
	      p += ((*p & 0xFF) < 0xD8 || (*p & 0xFF) >= 0xE0) ? 1 : 2;
	    }
	}
      else
	from = to;
    }
  else
    {
      unsigned *p = (unsigned *) (mt->data) + from_byte;
      unsigned c1 = c;

      if (mt->format != MTEXT_FORMAT_UTF_32)
	c1 = SWAP_32 (c1);
      while (from < to && *p++ != c1) from++;
    }

  return (from < to ? from : -1);
}


static int
find_char_backward (MText *mt, int from, int to, int c)
{
  int to_byte = POS_CHAR_TO_BYTE (mt, to);

  if (mt->format <= MTEXT_FORMAT_UTF_8)
    {
      unsigned char *p = mt->data + to_byte;

      while (from < to)
	{
	  for (p--; ! CHAR_HEAD_P (p); p--);
	  if (c == STRING_CHAR (p))
	    break;
	  to--;
	}
    }
  else if (mt->format <= MTEXT_FORMAT_UTF_16LE)
    {
      unsigned short *p = (unsigned short *) (mt->data) + to_byte;

      if (mt->format == MTEXT_FORMAT_UTF_16)
	{
	  while (from < to)
	    {
	      p--;
	      if (*p >= 0xDC00 && *p < 0xE000)
		p--;
	      if (c == STRING_CHAR_UTF16 (p))
		break;
	      to--;
	    }
	}
      else if (c < 0x10000)
	{
	  c = SWAP_16 (c);
	  while (from < to && p[-1] != c)
	    {
	      to--;
	      p -= ((p[-1] & 0xFF) < 0xD8 || (p[-1] & 0xFF) >= 0xE0) ? 1 : 2;
	    }
	}
      else if (c < 0x110000)
	{
	  int c1 = (c >> 10) + 0xD800;
	  int c2 = (c & 0x3FF) + 0xDC00;

	  c1 = SWAP_16 (c1);
	  c2 = SWAP_16 (c2);
	  while (from < to && (p[-1] != c2 || p[-2] != c1))
	    {
	      to--;
	      p -= ((p[-1] & 0xFF) < 0xD8 || (p[-1] & 0xFF) >= 0xE0) ? 1 : 2;
	    }
	}
    }
  else
    {
      unsigned *p = (unsigned *) (mt->data) + to_byte;
      unsigned c1 = c;

      if (mt->format != MTEXT_FORMAT_UTF_32)
	c1 = SWAP_32 (c1);
      while (from < to && p[-1] != c1) to--, p--;
    }

  return (from < to ? to - 1 : -1);
}


static void
free_mtext (void *object)
{
  MText *mt = (MText *) object;

  if (mt->plist)
    mtext__free_plist (mt);
  if (mt->data && mt->allocated >= 0)
    free (mt->data);
  M17N_OBJECT_UNREGISTER (mtext_table, mt);
  free (object);
}

/** Case handler (case-folding comparison and case conversion) */

/** Structure for an iterator used in case-fold comparison.  */

struct casecmp_iterator {
  MText *mt;
  int pos;
  MText *folded;
  unsigned char *foldedp;
  int folded_len;
};

static int
next_char_from_it (struct casecmp_iterator *it)
{
  int c, c1;

  if (it->folded)
    {
      c = STRING_CHAR_AND_BYTES (it->foldedp, it->folded_len);
      return c;
    }

  c = mtext_ref_char (it->mt, it->pos);
  c1 = (int) mchar_get_prop (c, Msimple_case_folding);
  if (c1 == 0xFFFF)
    {
      it->folded
	= (MText *) mchar_get_prop (c, Mcomplicated_case_folding);
      it->foldedp = it->folded->data;
      c = STRING_CHAR_AND_BYTES (it->foldedp, it->folded_len);
      return c;
    }

  if (c1 >= 0)
    c = c1;
  return c;
}

static void
advance_it (struct casecmp_iterator *it)
{
  if (it->folded)
    {
      it->foldedp += it->folded_len;
      if (it->foldedp == it->folded->data + it->folded->nbytes)
	it->folded = NULL;
    }
  if (! it->folded)
    {
      it->pos++;
    }
}

static int
case_compare (MText *mt1, int from1, int to1, MText *mt2, int from2, int to2)
{
  struct casecmp_iterator it1, it2;

  it1.mt = mt1, it1.pos = from1, it1.folded = NULL;
  it2.mt = mt2, it2.pos = from2, it2.folded = NULL;

  while (it1.pos < to1 && it2.pos < to2)
    {
      int c1 = next_char_from_it (&it1);
      int c2 = next_char_from_it (&it2);

      if (c1 != c2)
	return (c1 > c2 ? 1 : -1);
      advance_it (&it1);
      advance_it (&it2);
    }
  return (it2.pos == to2 ? (it1.pos < to1) : -1);
}

static MCharTable *tricky_chars, *cased, *soft_dotted, *case_mapping;
static MCharTable *combining_class;

/* Languages that require special handling in case-conversion.  */
static MSymbol Mlt, Mtr, Maz;

static MText *gr03A3;
static MText *lt0049, *lt004A, *lt012E, *lt00CC, *lt00CD, *lt0128;
static MText *tr0130, *tr0049, *tr0069;

static int
init_case_conversion ()
{
  Mlt = msymbol ("lt");
  Mtr = msymbol ("tr");
  Maz = msymbol ("az");

  gr03A3 = mtext ();
  mtext_cat_char (gr03A3, 0x03C2);

  lt0049 = mtext ();
  mtext_cat_char (lt0049, 0x0069);
  mtext_cat_char (lt0049, 0x0307);

  lt004A = mtext ();
  mtext_cat_char (lt004A, 0x006A);
  mtext_cat_char (lt004A, 0x0307);

  lt012E = mtext ();
  mtext_cat_char (lt012E, 0x012F);
  mtext_cat_char (lt012E, 0x0307);

  lt00CC = mtext ();
  mtext_cat_char (lt00CC, 0x0069);
  mtext_cat_char (lt00CC, 0x0307);
  mtext_cat_char (lt00CC, 0x0300);

  lt00CD = mtext ();
  mtext_cat_char (lt00CD, 0x0069);
  mtext_cat_char (lt00CD, 0x0307);
  mtext_cat_char (lt00CD, 0x0301);

  lt0128 = mtext ();
  mtext_cat_char (lt0128, 0x0069);
  mtext_cat_char (lt0128, 0x0307);
  mtext_cat_char (lt0128, 0x0303);

  tr0130 = mtext ();
  mtext_cat_char (tr0130, 0x0069);

  tr0049 = mtext ();
  mtext_cat_char (tr0049, 0x0131);

  tr0069 = mtext ();
  mtext_cat_char (tr0069, 0x0130);

  if (! (cased = mchar_get_prop_table (msymbol ("cased"), NULL)))
    return -1;
  if (! (soft_dotted = mchar_get_prop_table (msymbol ("soft-dotted"), NULL)))
    return -1;
  if (! (case_mapping = mchar_get_prop_table (msymbol ("case-mapping"), NULL)))
    return -1;
  if (! (combining_class = mchar_get_prop_table (Mcombining_class, NULL)))
    return -1;

  tricky_chars = mchartable (Mnil, 0);
  mchartable_set (tricky_chars, 0x0049, (void *) 1);
  mchartable_set (tricky_chars, 0x004A, (void *) 1);
  mchartable_set (tricky_chars, 0x00CC, (void *) 1);
  mchartable_set (tricky_chars, 0x00CD, (void *) 1);
  mchartable_set (tricky_chars, 0x0128, (void *) 1);
  mchartable_set (tricky_chars, 0x012E, (void *) 1);
  mchartable_set (tricky_chars, 0x0130, (void *) 1);
  mchartable_set (tricky_chars, 0x0307, (void *) 1);
  mchartable_set (tricky_chars, 0x03A3, (void *) 1);
  return 0;
}

#define CASE_CONV_INIT(ret)		\
  do {					\
    if (! tricky_chars			\
	&& init_case_conversion () < 0)	\
      MERROR (MERROR_MTEXT, ret);	\
  } while (0)

/* Replace the character at POS of MT with VAR and increment I and LEN.  */

#define REPLACE(var)					\
  do {							\
    int varlen = var->nchars;				\
							\
    mtext_replace (mt, pos, pos + 1, var, 0, varlen);	\
    pos += varlen;					\
    end += varlen - 1;					\
  } while (0)

/* Delete the character at POS of MT and decrement LEN.  */

#define DELETE				\
  do {					\
    mtext_del (mt, pos, pos + 1);	\
    end--;				\
  } while (0)

#define LOOKUP								\
  do {									\
    MPlist *pl = (MPlist *) mchartable_lookup (case_mapping, c);	\
									\
    if (pl)								\
      {									\
	/* Lowercase is the 1st element. */				\
	MText *lower = MPLIST_VAL ((MPlist *) MPLIST_VAL (pl));		\
	int llen = mtext_nchars (lower);				\
									\
	if (mtext_ref_char (lower, 0) != c || llen > 1)			\
	  {								\
	    mtext_replace (mt, pos, pos + 1, lower, 0, llen);		\
	    pos += llen;						\
	    end += llen - 1;						\
	  }								\
	else								\
	  pos++;							\
      }									\
    else								\
      pos++;								\
  } while (0)


int
uppercase_precheck (MText *mt, int pos, int end)
{
  for (; pos < end; pos++)
    if (mtext_ref_char (mt, pos) == 0x0307 &&
	(MSymbol) mtext_get_prop (mt, pos, Mlanguage) == Mlt)
      return 1;
  return 0;
}

int
lowercase_precheck (MText *mt, int pos, int end)
{
  int c;
  MSymbol lang;

  for (; pos < end; pos++)
    {
      c = mtext_ref_char (mt, pos);

      if ((int) mchartable_lookup (tricky_chars, c) == 1)
      {
	if (c == 0x03A3)
	  return 1;

	lang = mtext_get_prop (mt, pos, Mlanguage);

	if (lang == Mlt &&
	    (c == 0x0049 || c == 0x004A || c == 0x012E))
	  return 1;

	if ((lang == Mtr || lang == Maz) &&
	    (c == 0x0307 || c == 0x0049))
	  return 1;
      }
    }
  return 0;
}

#define CASED 1
#define CASE_IGNORABLE 2

int
final_sigma (MText *mt, int pos)
{
  int i, len = mtext_len (mt);
  int c;

  for (i = pos - 1; i >= 0; i--)
    {
      c = (int) mchartable_lookup (cased, mtext_ref_char (mt, i));
      if (c == -1)
	c = 0;
      if (c & CASED)
	break;
      if (! (c & CASE_IGNORABLE))
	return 0;
    }

  if (i == -1)
    return 0;

  for (i = pos + 1; i < len; i++)
    {
      c = (int) mchartable_lookup (cased, mtext_ref_char (mt, i));
      if (c == -1)
	c = 0;
      if (c & CASED)
	return 0;
      if (! (c & CASE_IGNORABLE))
	return 1;
    }

  return 1;
}

int
after_soft_dotted (MText *mt, int i)
{
  int c, class;

  for (i--; i >= 0; i--)
    {
      c = mtext_ref_char (mt, i);
      if ((MSymbol) mchartable_lookup (soft_dotted, c) == Mt)
	return 1;
      class = (int) mchartable_lookup (combining_class, c);
      if (class == 0 || class == 230)
	return 0;
    }

  return 0;
}

int
more_above (MText *mt, int i)
{
  int class, len = mtext_len (mt);

  for (i++; i < len; i++)
    {
      class = (int) mchartable_lookup (combining_class,
				       mtext_ref_char (mt, i));
      if (class == 230)
	return 1;
      if (class == 0)
	return 0;
    }

  return 0;
}

int
before_dot (MText *mt, int i)
{
  int c, class, len = mtext_len (mt);

  for (i++; i < len; i++)
    {
      c = mtext_ref_char (mt, i);
      if (c == 0x0307)
	return 1;
      class = (int) mchartable_lookup (combining_class, c);
      if (class == 230 || class == 0)
	return 0;
    }

  return 0;
}

int
after_i (MText *mt, int i)
{
  int c, class;

  for (i--; i >= 0; i--)
    {
      c = mtext_ref_char (mt, i);
      if (c == (int) 'I')
	return 1;
      class = (int) mchartable_lookup (combining_class, c);
      if (class == 230 || class == 0)
	return 0;
    }

  return 0;
}


/* Internal API */

int
mtext__init ()
{
  M17N_OBJECT_ADD_ARRAY (mtext_table, "M-text");
  M_charbag = msymbol_as_managing_key ("  charbag");
  mtext_table.count = 0;
  Mlanguage = msymbol ("language");
  return 0;
}


void
mtext__fini (void)
{
  mtext__wseg_fini ();
}


int
mtext__char_to_byte (MText *mt, int pos)
{
  int char_pos, byte_pos;
  int forward;

  if (pos < mt->cache_char_pos)
    {
      if (mt->cache_char_pos == mt->cache_byte_pos)
	return pos;
      if (pos < mt->cache_char_pos - pos)
	{
	  char_pos = byte_pos = 0;
	  forward = 1;
	}
      else
	{
	  char_pos = mt->cache_char_pos;
	  byte_pos = mt->cache_byte_pos;
	  forward = 0;
	}
    }
  else
    {
      if (mt->nchars - mt->cache_char_pos == mt->nbytes - mt->cache_byte_pos)
	return (mt->cache_byte_pos + (pos - mt->cache_char_pos));
      if (pos - mt->cache_char_pos < mt->nchars - pos)
	{
	  char_pos = mt->cache_char_pos;
	  byte_pos = mt->cache_byte_pos;
	  forward = 1;
	}
      else
	{
	  char_pos = mt->nchars;
	  byte_pos = mt->nbytes;
	  forward = 0;
	}
    }
  if (forward)
    while (char_pos < pos)
      INC_POSITION (mt, char_pos, byte_pos);
  else
    while (char_pos > pos)
      DEC_POSITION (mt, char_pos, byte_pos);
  mt->cache_char_pos = char_pos;
  mt->cache_byte_pos = byte_pos;
  return byte_pos;
}

/* mtext__byte_to_char () */

int
mtext__byte_to_char (MText *mt, int pos_byte)
{
  int char_pos, byte_pos;
  int forward;

  if (pos_byte < mt->cache_byte_pos)
    {
      if (mt->cache_char_pos == mt->cache_byte_pos)
	return pos_byte;
      if (pos_byte < mt->cache_byte_pos - pos_byte)
	{
	  char_pos = byte_pos = 0;
	  forward = 1;
	}
      else
	{
	  char_pos = mt->cache_char_pos;
	  byte_pos = mt->cache_byte_pos;
	  forward = 0;
	}
    }
  else
    {
      if (mt->nchars - mt->cache_char_pos == mt->nbytes - mt->cache_byte_pos)
	return (mt->cache_char_pos + (pos_byte - mt->cache_byte_pos));
      if (pos_byte - mt->cache_byte_pos < mt->nbytes - pos_byte)
	{
	  char_pos = mt->cache_char_pos;
	  byte_pos = mt->cache_byte_pos;
	  forward = 1;
	}
      else
	{
	  char_pos = mt->nchars;
	  byte_pos = mt->nbytes;
	  forward = 0;
	}
    }
  if (forward)
    while (byte_pos < pos_byte)
      INC_POSITION (mt, char_pos, byte_pos);
  else
    while (byte_pos > pos_byte)
      DEC_POSITION (mt, char_pos, byte_pos);
  mt->cache_char_pos = char_pos;
  mt->cache_byte_pos = byte_pos;
  return char_pos;
}

/* Estimated extra bytes that malloc will use for its own purpose on
   each memory allocation.  */
#define MALLOC_OVERHEAD 4
#define MALLOC_MININUM_BYTES 12

void
mtext__enlarge (MText *mt, int nbytes)
{
  nbytes += MAX_UTF8_CHAR_BYTES;
  if (mt->allocated >= nbytes)
    return;
  if (nbytes < MALLOC_MININUM_BYTES)
    nbytes = MALLOC_MININUM_BYTES;
  while (mt->allocated < nbytes)
    mt->allocated = mt->allocated * 2 + MALLOC_OVERHEAD;
  MTABLE_REALLOC (mt->data, mt->allocated, MERROR_MTEXT);
}

int
mtext__takein (MText *mt, int nchars, int nbytes)
{
  if (mt->plist)
    mtext__adjust_plist_for_insert (mt, mt->nchars, nchars, NULL);
  mt->nchars += nchars;
  mt->nbytes += nbytes;
  mt->data[mt->nbytes] = 0;
  return 0;
}


int
mtext__cat_data (MText *mt, unsigned char *p, int nbytes,
		 enum MTextFormat format)
{
  int nchars = -1;

  if (mt->format > MTEXT_FORMAT_UTF_8)
    MERROR (MERROR_MTEXT, -1);
  if (format == MTEXT_FORMAT_US_ASCII)
    nchars = nbytes;
  else if (format == MTEXT_FORMAT_UTF_8)
    nchars = count_utf_8_chars (p, nbytes);
  if (nchars < 0)
    MERROR (MERROR_MTEXT, -1);
  mtext__enlarge (mt, mtext_nbytes (mt) + nbytes + 1);
  memcpy (MTEXT_DATA (mt) + mtext_nbytes (mt), p, nbytes);
  mtext__takein (mt, nchars, nbytes);
  return nchars;
}

MText *
mtext__from_data (const void *data, int nitems, enum MTextFormat format,
		  int need_copy)
{
  MText *mt;
  int nchars, nbytes, unit_bytes;

  if (format == MTEXT_FORMAT_US_ASCII)
    {
      const char *p = (char *) data, *pend = p + nitems;

      while (p < pend)
	if (*p++ < 0)
	  MERROR (MERROR_MTEXT, NULL);
      nchars = nbytes = nitems;
      unit_bytes = 1;
    }
  else if (format == MTEXT_FORMAT_UTF_8)
    {
      if ((nchars = count_utf_8_chars (data, nitems)) < 0)
	MERROR (MERROR_MTEXT, NULL);
      nbytes = nitems;
      unit_bytes = 1;
    }
  else if (format <= MTEXT_FORMAT_UTF_16BE)
    {
      if ((nchars = count_utf_16_chars (data, nitems,
					format != MTEXT_FORMAT_UTF_16)) < 0)
	MERROR (MERROR_MTEXT, NULL);
      nbytes = USHORT_SIZE * nitems;
      unit_bytes = USHORT_SIZE;
    }
  else				/* MTEXT_FORMAT_UTF_32XX */
    {
      nchars = nitems;
      nbytes = UINT_SIZE * nitems;
      unit_bytes = UINT_SIZE;
    }

  mt = mtext ();
  mt->format = format;
  mt->coverage = FORMAT_COVERAGE (format);
  mt->allocated = need_copy ? nbytes + unit_bytes : -1;
  mt->nchars = nchars;
  mt->nbytes = nitems;
  if (need_copy)
    {
      MTABLE_MALLOC (mt->data, mt->allocated, MERROR_MTEXT);
      memcpy (mt->data, data, nbytes);
      mt->data[nbytes] = 0;
    }
  else
    mt->data = (unsigned char *) data;
  return mt;
}


void
mtext__adjust_format (MText *mt, enum MTextFormat format)
{
  int i, c;

  if (mt->nchars > 0)
    switch (format)
      {
      case MTEXT_FORMAT_US_ASCII:
	{
	  unsigned char *p = mt->data;

	  for (i = 0; i < mt->nchars; i++)
	    *p++ = mtext_ref_char (mt, i);
	  mt->nbytes = mt->nchars;
	  mt->cache_byte_pos = mt->cache_char_pos;
	  break;
	}

      case MTEXT_FORMAT_UTF_8:
	{
	  unsigned char *p0, *p1;

	  i = count_by_utf_8 (mt, 0, mt->nchars) + 1;
	  MTABLE_MALLOC (p0, i, MERROR_MTEXT);
	  mt->allocated = i;
	  for (i = 0, p1 = p0; i < mt->nchars; i++)
	    {
	      c = mtext_ref_char (mt, i);
	      p1 += CHAR_STRING_UTF8 (c, p1);
	    }
	  *p1 = '\0';
	  free (mt->data);
	  mt->data = p0;
	  mt->nbytes = p1 - p0;
	  mt->cache_char_pos = mt->cache_byte_pos = 0;
	  break;
	}

      default:
	if (format == MTEXT_FORMAT_UTF_16)
	  {
	    unsigned short *p0, *p1;

	    i = (count_by_utf_16 (mt, 0, mt->nchars) + 1) * USHORT_SIZE;
	    MTABLE_MALLOC (p0, i, MERROR_MTEXT);
	    mt->allocated = i;
	    for (i = 0, p1 = p0; i < mt->nchars; i++)
	      {
		c = mtext_ref_char (mt, i);
		p1 += CHAR_STRING_UTF16 (c, p1);
	      }
	    *p1 = 0;
	    free (mt->data);
	    mt->data = (unsigned char *) p0;
	    mt->nbytes = p1 - p0;
	    mt->cache_char_pos = mt->cache_byte_pos = 0;
	    break;
	  }
	else
	  {
	    unsigned int *p;

	    mt->allocated = (mt->nchars + 1) * UINT_SIZE;
	    MTABLE_MALLOC (p, mt->allocated, MERROR_MTEXT);
	    for (i = 0; i < mt->nchars; i++)
	      p[i] = mtext_ref_char (mt, i);
	    p[i] = 0;
	    free (mt->data);
	    mt->data = (unsigned char *) p;
	    mt->nbytes = mt->nchars;
	    mt->cache_byte_pos = mt->cache_char_pos;
	  }
      }
  mt->format = format;
  mt->coverage = FORMAT_COVERAGE (format);
}


/* Find the position of a character at the beginning of a line of
   M-Text MT searching backward from POS.  */

int
mtext__bol (MText *mt, int pos)
{
  int byte_pos;

  if (pos == 0)
    return pos;
  byte_pos = POS_CHAR_TO_BYTE (mt, pos);
  if (mt->format <= MTEXT_FORMAT_UTF_8)
    {
      unsigned char *p = mt->data + byte_pos;

      if (p[-1] == '\n')
	return pos;
      p--;
      while (p > mt->data && p[-1] != '\n')
	p--;
      if (p == mt->data)
	return 0;
      byte_pos = p - mt->data;
      return POS_BYTE_TO_CHAR (mt, byte_pos);
    }
  else if (mt->format <= MTEXT_FORMAT_UTF_16BE)
    {
      unsigned short *p = ((unsigned short *) (mt->data)) + byte_pos;
      unsigned short newline = (mt->format == MTEXT_FORMAT_UTF_16
				? 0x0A00 : 0x000A);

      if (p[-1] == newline)
	return pos;
      p--;
      while (p > (unsigned short *) (mt->data) && p[-1] != newline)
	p--;
      if (p == (unsigned short *) (mt->data))
	return 0;
      byte_pos = p - (unsigned short *) (mt->data);
      return POS_BYTE_TO_CHAR (mt, byte_pos);;
    }
  else
    {
      unsigned *p = ((unsigned *) (mt->data)) + byte_pos;
      unsigned newline = (mt->format == MTEXT_FORMAT_UTF_32
			  ? 0x0A000000 : 0x0000000A);

      if (p[-1] == newline)
	return pos;
      p--, pos--;
      while (p > (unsigned *) (mt->data) && p[-1] != newline)
	p--, pos--;
      return pos;
    }
}


/* Find the position of a character at the end of a line of M-Text MT
   searching forward from POS.  */

int
mtext__eol (MText *mt, int pos)
{
  int byte_pos;

  if (pos == mt->nchars)
    return pos;
  byte_pos = POS_CHAR_TO_BYTE (mt, pos);
  if (mt->format <= MTEXT_FORMAT_UTF_8)
    {
      unsigned char *p = mt->data + byte_pos;
      unsigned char *endp;

      if (*p == '\n')
	return pos + 1;
      p++;
      endp = mt->data + mt->nbytes;
      while (p < endp && *p != '\n')
	p++;
      if (p == endp)
	return mt->nchars;
      byte_pos = p + 1 - mt->data;
      return POS_BYTE_TO_CHAR (mt, byte_pos);
    }
  else if (mt->format <= MTEXT_FORMAT_UTF_16BE)
    {
      unsigned short *p = ((unsigned short *) (mt->data)) + byte_pos;
      unsigned short *endp;
      unsigned short newline = (mt->format == MTEXT_FORMAT_UTF_16
				? 0x0A00 : 0x000A);

      if (*p == newline)
	return pos + 1;
      p++;
      endp = (unsigned short *) (mt->data) + mt->nbytes;
      while (p < endp && *p != newline)
	p++;
      if (p == endp)
	return mt->nchars;
      byte_pos = p + 1 - (unsigned short *) (mt->data);
      return POS_BYTE_TO_CHAR (mt, byte_pos);
    }
  else
    {
      unsigned *p = ((unsigned *) (mt->data)) + byte_pos;
      unsigned *endp;
      unsigned newline = (mt->format == MTEXT_FORMAT_UTF_32
			  ? 0x0A000000 : 0x0000000A);

      if (*p == newline)
	return pos + 1;
      p++, pos++;
      endp = (unsigned *) (mt->data) + mt->nbytes;
      while (p < endp && *p != newline)
	p++, pos++;
      return pos;
    }
}

int
mtext__lowercase (MText *mt, int pos, int end)
{
  int opos = pos;
  int c;
  MText *orig = NULL;
  MSymbol lang;

  if (lowercase_precheck (mt, pos, end))
    orig = mtext_dup (mt);

  for (; pos < end; opos++)
    {
      c = mtext_ref_char (mt, pos);
      lang = (MSymbol) mtext_get_prop (mt, pos, Mlanguage);

      if (c == 0x03A3 && final_sigma (orig, opos))
	REPLACE (gr03A3);

      else if (lang == Mlt)
	{
	  if (c == 0x00CC)
    	    REPLACE (lt00CC);
	  else if (c == 0x00CD)
	    REPLACE (lt00CD);
	  else if (c == 0x0128)
	    REPLACE (lt0128);
	  else if (orig && more_above (orig, opos))
	    {
	      if (c == 0x0049)
		REPLACE (lt0049);
	      else if (c == 0x004A)
		REPLACE (lt004A);
	      else if (c == 0x012E)
		REPLACE (lt012E);
	      else
		LOOKUP;
	    }
	  else
	    LOOKUP;
	}

      else if (lang == Mtr || lang == Maz)
	{
	  if (c == 0x0130)
	    REPLACE (tr0130);
	  else if (c == 0x0307 && after_i (orig, opos))
	    DELETE;
	  else if (c == 0x0049 && ! before_dot (orig, opos))
	    REPLACE (tr0049);
	  else
	    LOOKUP;
	}

      else
	LOOKUP;
    }

  if (orig)
    m17n_object_unref (orig);

  return end;
}

int
mtext__titlecase (MText *mt, int pos, int end)
{
  int opos = pos;
  int c;
  MText *orig = NULL;
  MSymbol lang;
  MPlist *pl;

  /* Precheck for titlecase is identical to that for uppercase. */
  if (uppercase_precheck (mt, pos, end))
    orig = mtext_dup (mt);

  for (; pos < end; opos++)
    {
      c = mtext_ref_char (mt, pos);
      lang = (MSymbol) mtext_get_prop (mt, pos, Mlanguage);

      if ((lang == Mtr || lang == Maz) && c == 0x0069)
	REPLACE (tr0069);

      else if (lang == Mlt && c == 0x0307 && after_soft_dotted (orig, opos))
	DELETE;

      else if ((pl = (MPlist *) mchartable_lookup (case_mapping, c)))
	{
	  /* Titlecase is the 2nd element. */
	  MText *title
	    = (MText *) mplist_value (mplist_next (mplist_value (pl)));
	  int tlen = mtext_len (title);

	  if (mtext_ref_char (title, 0) != c || tlen > 1)
	    {
	      mtext_replace (mt, pos, pos + 1, title, 0, tlen);
	      pos += tlen;
	      end += tlen - 1;
	    }

	  else
	    pos++;
	}

      else
	pos++;
    }

  if (orig)
    m17n_object_unref (orig);

  return end;
}

int
mtext__uppercase (MText *mt, int pos, int end)
{
  int opos = pos;
  int c;
  MText *orig = NULL;
  MSymbol lang;
  MPlist *pl;

  CASE_CONV_INIT (-1);

  if (uppercase_precheck (mt, 0, end))
    orig = mtext_dup (mt);

  for (; pos < end; opos++)
    {
      c = mtext_ref_char (mt, pos);
      lang = (MSymbol) mtext_get_prop (mt, pos, Mlanguage);

      if (lang == Mlt && c == 0x0307 && after_soft_dotted (orig, opos))
	DELETE;

      else if ((lang == Mtr || lang == Maz) && c == 0x0069)
	REPLACE (tr0069);
	       
      else
	{
	  if ((pl = (MPlist *) mchartable_lookup (case_mapping, c)) != NULL)
	    {
	      MText *upper;
	      int ulen;

	      /* Uppercase is the 3rd element. */
	      upper = (MText *) mplist_value (mplist_next (mplist_next (mplist_value (pl))));
	      ulen = mtext_len (upper);

	      if (mtext_ref_char (upper, 0) != c || ulen > 1)
		{
		  mtext_replace (mt, pos, pos + 1, upper, 0, ulen);
		  pos += ulen;
		  end += ulen - 1;
		}

	      else
		pos++;
	    }

	  else						 /* pl == NULL */
	    pos++;
	}
    }

  if (orig)
    m17n_object_unref (orig);

  return end;
}

/*** @} */
#endif /* !FOR_DOXYGEN || DOXYGEN_INTERNAL_MODULE */


/* External API */

#ifdef WORDS_BIGENDIAN
const enum MTextFormat MTEXT_FORMAT_UTF_16 = MTEXT_FORMAT_UTF_16BE;
#else
const enum MTextFormat MTEXT_FORMAT_UTF_16 = MTEXT_FORMAT_UTF_16LE;
#endif

#ifdef WORDS_BIGENDIAN
const int MTEXT_FORMAT_UTF_32 = MTEXT_FORMAT_UTF_32BE;
#else
const int MTEXT_FORMAT_UTF_32 = MTEXT_FORMAT_UTF_32LE;
#endif

/*** @addtogroup m17nMtext */
/*** @{ */
/*=*/

/***en The symbol whose name is "language".  */
/***ja "language" �Ȥ���̾������ĥ���ܥ�.  */
MSymbol Mlanguage;

/*=*/

/***en
    @brief Allocate a new M-text.

    The mtext () function allocates a new M-text of length 0 and
    returns a pointer to it.  The allocated M-text will not be freed
    unless the user explicitly does so with the m17n_object_unref ()
    function.  */

/***ja
    @brief ������M-text�������Ƥ�.

    �ؿ� mtext () �ϡ�Ĺ�� 0 �ο����� M-text 
    �������ơ�����ؤΥݥ��󥿤��֤���������Ƥ�줿 M-text �ϡ��ؿ�
    m17n_object_unref () �ˤ�äƥ桼��������Ū�˹Ԥʤ�ʤ��¤ꡢ��������ʤ���

    @latexonly \IPAlabel{mtext} @endlatexonly  */

/***
    @seealso
    m17n_object_unref ()  */

MText *
mtext ()
{
  MText *mt;

  M17N_OBJECT (mt, free_mtext, MERROR_MTEXT);
  mt->format = MTEXT_FORMAT_US_ASCII;
  mt->coverage = MTEXT_COVERAGE_ASCII;
  M17N_OBJECT_REGISTER (mtext_table, mt);
  return mt;
}

/***en
    @brief Allocate a new M-text with specified data.

    The mtext_from_data () function allocates a new M-text whose
    character sequence is specified by array $DATA of $NITEMS
    elements.  $FORMAT specifies the format of $DATA.

    When $FORMAT is either #MTEXT_FORMAT_US_ASCII or
    #MTEXT_FORMAT_UTF_8, the contents of $DATA must be of the type @c
    unsigned @c char, and $NITEMS counts by byte.

    When $FORMAT is either #MTEXT_FORMAT_UTF_16LE or
    #MTEXT_FORMAT_UTF_16BE, the contents of $DATA must be of the type
    @c unsigned @c short, and $NITEMS counts by unsigned short.

    When $FORMAT is either #MTEXT_FORMAT_UTF_32LE or
    #MTEXT_FORMAT_UTF_32BE, the contents of $DATA must be of the type
    @c unsigned, and $NITEMS counts by unsigned.

    The character sequence of the M-text is not modifiable.  
    The contents of $DATA must not be modified while the M-text is alive.

    The allocated M-text will not be freed unless the user explicitly
    does so with the m17n_object_unref () function.  Even in that case,
    $DATA is not freed.

    @return
    If the operation was successful, mtext_from_data () returns a
    pointer to the allocated M-text.  Otherwise it returns @c NULL and
    assigns an error code to the external variable #merror_code.  */
/***ja
    @brief ����Υǡ����򸵤˿����� M-text �������Ƥ�.

    �ؿ� mtext_from_data () �ϡ����ǿ� $NITEMS ������ $DATA 
    �ǻ��ꤵ�줿ʸ�������Ŀ����� M-text �������Ƥ롣$FORMAT �� $DATA
    �Υե����ޥåȤ򼨤���

    $FORMAT �� #MTEXT_FORMAT_US_ASCII �� #MTEXT_FORMAT_UTF_8 �ʤ�С�
    $DATA �����Ƥ� @c unsigned @c char ���Ǥ��ꡢ$NITEMS 
    �ϥХ���ñ�̤�ɽ����Ƥ��롣

    $FORMAT �� #MTEXT_FORMAT_UTF_16LE �� #MTEXT_FORMAT_UTF_16BE �ʤ�С�
    $DATA �����Ƥ� @c unsigned @c short ���Ǥ��ꡢ$NITEMS �� unsigned
    short ñ�̤Ǥ��롣

    $FORMAT �� #MTEXT_FORMAT_UTF_32LE �� #MTEXT_FORMAT_UTF_32BE �ʤ�С�
    $DATA �����Ƥ� @c unsigned ���Ǥ��ꡢ$NITEMS �� unsigned ñ�̤Ǥ��롣

    ������Ƥ�줿 M-text ��ʸ������ѹ��Ǥ��ʤ���$DATA �����Ƥ� 
    M-text ��ͭ���ʴ֤��ѹ����ƤϤʤ�ʤ���

    ������Ƥ�줿 M-text �ϡ��ؿ� m17n_object_unref () 
    �ˤ�äƥ桼��������Ū�˹Ԥʤ�ʤ��¤ꡢ��������ʤ������ξ��Ǥ� $DATA �ϲ�������ʤ���

    @return 
    ��������������С�mtext_from_data () �ϳ�����Ƥ�줿M-text 
    �ؤΥݥ��󥿤��֤��������Ǥʤ���� @c NULL ���֤������ѿ� #merror_code 
    �˥��顼�����ɤ����ꤹ�롣  */

/***
    @errors
    @c MERROR_MTEXT  */

MText *
mtext_from_data (const void *data, int nitems, enum MTextFormat format)
{
  if (nitems < 0
      || format < MTEXT_FORMAT_US_ASCII || format >= MTEXT_FORMAT_MAX)
    MERROR (MERROR_MTEXT, NULL);
  return mtext__from_data (data, nitems, format, 0);
}

/*=*/

/***en
    @brief Get information about the text data in M-text.

    The mtext_data () function returns a pointer to the text data of
    M-text $MT.  If $FMT is not NULL, the format of the text data is
    stored in it.  If $NUNITS is not NULL, the number of units of the
    text data is stored in it.

    If $POS_IDX is not NULL and it points to a non-negative number,
    what it points to is a character position.  In this case, the
    return value is a pointer to the text data of a character at that
    position.

    Otherwise, if $UNIT_IDX is not NULL, it points to a unit position.
    In this case, the return value is a pointer to the text data of a
    character containing that unit.

    The character position and unit position of the return value are
    stored in $POS_IDX and $UNIT_DIX respectively if they are not
    NULL.

    <ul>

    <li> If the format of the text data is MTEXT_FORMAT_US_ASCII or
    MTEXT_FORMAT_UTF_8, one unit is unsigned char.

    <li> If the format is MTEXT_FORMAT_UTF_16LE or
    MTEXT_FORMAT_UTF_16BE, one unit is unsigned short.

    <li> If the format is MTEXT_FORMAT_UTF_32LE or
    MTEXT_FORMAT_UTF_32BE, one unit is unsigned int.

    </ul> */

void *
mtext_data (MText *mt, enum MTextFormat *fmt, int *nunits,
	    int *pos_idx, int *unit_idx)
{
  void *data;
  int pos = 0, unit_pos = 0;

  if (fmt)
    *fmt = mt->format;
  data = MTEXT_DATA (mt);
  if (pos_idx && *pos_idx >= 0)
    {
      pos = *pos_idx;
      if (pos > mtext_nchars (mt))
	MERROR (MERROR_MTEXT, NULL);
      unit_pos = POS_CHAR_TO_BYTE (mt, pos);
    }
  else if (unit_idx)
    {
      unit_pos = *unit_idx;

      if (unit_pos < 0 || unit_pos > mtext_nbytes (mt))
	MERROR (MERROR_MTEXT, NULL);
      pos = POS_BYTE_TO_CHAR (mt, unit_pos);
      unit_pos = POS_CHAR_TO_BYTE (mt, pos);
    }
  if (nunits)
    *nunits = mtext_nbytes (mt) - unit_pos;
  if (pos_idx)
    *pos_idx = pos;
  if (unit_idx)
    *unit_idx = unit_pos;
  if (unit_pos > 0)
    {
      if (mt->format <= MTEXT_FORMAT_UTF_8)
	data = (unsigned char *) data + unit_pos;
      else if (mt->format <= MTEXT_FORMAT_UTF_16BE)
	data = (unsigned short *) data + unit_pos;
      else
	data = (unsigned int *) data + unit_pos;
    }
  return data;
}

/*=*/

/***en
    @brief Number of characters in M-text.

    The mtext_len () function returns the number of characters in
    M-text $MT.  */

/***ja
    @brief M-text ���ʸ���ο�.

    �ؿ� mtext_len () �� M-text $MT ���ʸ���ο����֤���

    @latexonly \IPAlabel{mtext_len} @endlatexonly  */

int
mtext_len (MText *mt)
{
  return (mt->nchars);
}

/*=*/

/***en
    @brief Return the character at the specified position in an M-text.

    The mtext_ref_char () function returns the character at $POS in
    M-text $MT.  If an error is detected, it returns -1 and assigns an
    error code to the external variable #merror_code.  */

/***ja
    @brief M-text ��λ��ꤵ�줿���֤�ʸ�����֤�.

    �ؿ� mtext_ref_char () �ϡ�M-text $MT �ΰ��� $POS 
    ��ʸ�����֤������顼�����Ф��줿���� -1 ���֤��������ѿ� #merror_code
    �˥��顼�����ɤ����ꤹ�롣

    @latexonly \IPAlabel{mtext_ref_char} @endlatexonly  */

/***
    @errors
    @c MERROR_RANGE  */

int
mtext_ref_char (MText *mt, int pos)
{
  int c;

  M_CHECK_POS (mt, pos, -1);
  if (mt->format <= MTEXT_FORMAT_UTF_8)
    {
      unsigned char *p = mt->data + POS_CHAR_TO_BYTE (mt, pos);

      c = STRING_CHAR_UTF8 (p);
    }
  else if (mt->format <= MTEXT_FORMAT_UTF_16BE)
    {
      unsigned short *p
	= (unsigned short *) (mt->data) + POS_CHAR_TO_BYTE (mt, pos);
      unsigned short p1[2];

      if (mt->format != MTEXT_FORMAT_UTF_16)
	{
	  p1[0] = SWAP_16 (*p);
	  if (p1[0] >= 0xD800 && p1[0] < 0xDC00)
	    p1[1] = SWAP_16 (p[1]);
	  p = p1;
	}
      c = STRING_CHAR_UTF16 (p);
    }
  else
    {
      c = ((unsigned *) (mt->data))[pos];
      if (mt->format != MTEXT_FORMAT_UTF_32)
	c = SWAP_32 (c);
    }
  return c;
}

/*=*/

/***en
    @brief Store a character into an M-text.

    The mtext_set_char () function sets character $C, which has no
    text properties, at $POS in M-text $MT.

    @return
    If the operation was successful, mtext_set_char () returns 0.
    Otherwise it returns -1 and assigns an error code to the external
    variable #merror_code.  */

/***ja
    @brief M-text �˰�ʸ�������ꤹ��.

    �ؿ� mtext_set_char () �ϡ��ƥ����ȥץ�ѥƥ�̵����ʸ�� $C �� 
    M-text $MT �ΰ��� $POS �����ꤹ�롣

    @return
    ��������������� mtext_set_char () �� 0 ���֤������Ԥ���� -1 
    ���֤��������ѿ� #merror_code �˥��顼�����ɤ����ꤹ�롣

    @latexonly \IPAlabel{mtext_set_char} @endlatexonly  */

/***
    @errors
    @c MERROR_RANGE */

int
mtext_set_char (MText *mt, int pos, int c)
{
  int pos_unit;
  int old_units, new_units;
  int delta;
  unsigned char *p;
  int unit_bytes;

  M_CHECK_POS (mt, pos, -1);
  M_CHECK_READONLY (mt, -1);

  mtext__adjust_plist_for_change (mt, pos, 1, 1);

  if (mt->format <= MTEXT_FORMAT_UTF_8)
    {
      if (c >= 0x80)
	mt->format = MTEXT_FORMAT_UTF_8, mt->coverage = MTEXT_COVERAGE_FULL;
    }
  else if (mt->format <= MTEXT_FORMAT_UTF_16BE)
    {
      if (c >= 0x110000)
	mtext__adjust_format (mt, MTEXT_FORMAT_UTF_8);
      else if (mt->format != MTEXT_FORMAT_UTF_16)
	mtext__adjust_format (mt, MTEXT_FORMAT_UTF_16);
    }
  else if (mt->format != MTEXT_FORMAT_UTF_32)
    mtext__adjust_format (mt, MTEXT_FORMAT_UTF_32);

  unit_bytes = UNIT_BYTES (mt->format);
  pos_unit = POS_CHAR_TO_BYTE (mt, pos);
  p = mt->data + pos_unit * unit_bytes;
  old_units = CHAR_UNITS_AT (mt, p);
  new_units = CHAR_UNITS (c, mt->format);
  delta = new_units - old_units;

  if (delta)
    {
      if (mt->cache_char_pos > pos)
	mt->cache_byte_pos += delta;

      if ((mt->nbytes + delta + 1) * unit_bytes > mt->allocated)
	{
	  mt->allocated = (mt->nbytes + delta + 1) * unit_bytes;
	  MTABLE_REALLOC (mt->data, mt->allocated, MERROR_MTEXT);
	}

      memmove (mt->data + (pos_unit + new_units) * unit_bytes, 
	       mt->data + (pos_unit + old_units) * unit_bytes,
	       (mt->nbytes - pos_unit - old_units + 1) * unit_bytes);
      mt->nbytes += delta;
      mt->data[mt->nbytes * unit_bytes] = 0;
    }
  switch (mt->format)
    {
    case MTEXT_FORMAT_US_ASCII:
      mt->data[pos_unit] = c;
      break;
    case MTEXT_FORMAT_UTF_8:
      {
	unsigned char *p = mt->data + pos_unit;
	CHAR_STRING_UTF8 (c, p);
	break;
      }
    default:
      if (mt->format == MTEXT_FORMAT_UTF_16)
	{
	  unsigned short *p = (unsigned short *) mt->data + pos_unit;

	  CHAR_STRING_UTF16 (c, p);
	}
      else
	((unsigned *) mt->data)[pos_unit] = c;
    }
  return 0;
}

/*=*/

/***en
    @brief  Append a character to an M-text.

    The mtext_cat_char () function appends character $C, which has no
    text properties, to the end of M-text $MT.

    @return
    This function returns a pointer to the resulting M-text $MT.  If
    $C is an invalid character, it returns @c NULL.  */

/***ja
    @brief M-text �˰�ʸ���ɲä���.

    �ؿ� mtext_cat_char () �ϡ��ƥ����ȥץ�ѥƥ�̵����ʸ�� $C �� 
    M-text $MT ���������ɲä��롣

    @return
    ���δؿ����ѹ����줿 M-text $MT �ؤΥݥ��󥿤��֤���$C 
    ��������ʸ���Ǥʤ����ˤ� @c NULL ���֤���  */

/***
    @seealso
    mtext_cat (), mtext_ncat ()  */

MText *
mtext_cat_char (MText *mt, int c)
{
  int nunits;
  int unit_bytes = UNIT_BYTES (mt->format);

  M_CHECK_READONLY (mt, NULL);
  if (c < 0 || c > MCHAR_MAX)
    return NULL;
  mtext__adjust_plist_for_insert (mt, mt->nchars, 1, NULL);

  if (c >= 0x80
      && (mt->format == MTEXT_FORMAT_US_ASCII
	  || (c >= 0x10000
	      && (mt->format == MTEXT_FORMAT_UTF_16LE
		  || mt->format == MTEXT_FORMAT_UTF_16BE))))

    {
      mtext__adjust_format (mt, MTEXT_FORMAT_UTF_8);
      unit_bytes = 1;
    }
  else if (mt->format >= MTEXT_FORMAT_UTF_32LE)
    {
      if (mt->format != MTEXT_FORMAT_UTF_32)
	mtext__adjust_format (mt, MTEXT_FORMAT_UTF_32);
    }
  else if (mt->format >= MTEXT_FORMAT_UTF_16LE)
    {
      if (mt->format != MTEXT_FORMAT_UTF_16)
	mtext__adjust_format (mt, MTEXT_FORMAT_UTF_16);
    }

  nunits = CHAR_UNITS (c, mt->format);
  if ((mt->nbytes + nunits + 1) * unit_bytes > mt->allocated)
    {
      mt->allocated = (mt->nbytes + nunits * 16 + 1) * unit_bytes;
      MTABLE_REALLOC (mt->data, mt->allocated, MERROR_MTEXT);
    }
  
  if (mt->format <= MTEXT_FORMAT_UTF_8)
    {
      unsigned char *p = mt->data + mt->nbytes;
      p += CHAR_STRING_UTF8 (c, p);
      *p = 0;
    }
  else if (mt->format == MTEXT_FORMAT_UTF_16)
    {
      unsigned short *p = (unsigned short *) mt->data + mt->nbytes;
      p += CHAR_STRING_UTF16 (c, p);
      *p = 0;
    }
  else
    {
      unsigned *p = (unsigned *) mt->data + mt->nbytes;
      *p++ = c;
      *p = 0;
    }

  mt->nchars++;
  mt->nbytes += nunits;
  return mt;
}

/*=*/

/***en
    @brief  Create a copy of an M-text.

    The mtext_dup () function creates a copy of M-text $MT while
    inheriting all the text properties of $MT.

    @return
    This function returns a pointer to the created copy.  */

/***ja
    @brief M-text �Υ��ԡ�����.

    �ؿ� mtext_dup () �ϡ�M-text $MT �Υ��ԡ����롣$MT 
    �Υƥ����ȥץ�ѥƥ��Ϥ��٤ƷѾ�����롣

    @return
    ���δؿ��Ϻ��줿���ԡ��ؤΥݥ��󥿤��֤���

     @latexonly \IPAlabel{mtext_dup} @endlatexonly  */

/***
    @seealso
    mtext_duplicate ()  */

MText *
mtext_dup (MText *mt)
{
  return mtext_duplicate (mt, 0, mtext_nchars (mt));
}

/*=*/

/***en
    @brief  Append an M-text to another.

    The mtext_cat () function appends M-text $MT2 to the end of M-text
    $MT1 while inheriting all the text properties.  $MT2 itself is not
    modified.

    @return
    This function returns a pointer to the resulting M-text $MT1.  */

/***ja
    @brief 2�Ĥ� M-text��Ϣ�뤹��.

    �ؿ� mtext_cat () �ϡ� M-text $MT2 �� M-text $MT1 
    ���������դ��ä��롣$MT2 �Υƥ����ȥץ�ѥƥ��Ϥ��٤ƷѾ�����롣$MT2 ���ѹ�����ʤ���

    @return
    ���δؿ����ѹ����줿 M-text $MT1 �ؤΥݥ��󥿤��֤���

    @latexonly \IPAlabel{mtext_cat} @endlatexonly  */

/***
    @seealso
    mtext_ncat (), mtext_cat_char ()  */

MText *
mtext_cat (MText *mt1, MText *mt2)
{
  M_CHECK_READONLY (mt1, NULL);

  if (mt2->nchars > 0)
    insert (mt1, mt1->nchars, mt2, 0, mt2->nchars);
  return mt1;
}


/*=*/

/***en
    @brief Append a part of an M-text to another.

    The mtext_ncat () function appends the first $N characters of
    M-text $MT2 to the end of M-text $MT1 while inheriting all the
    text properties.  If the length of $MT2 is less than $N, all
    characters are copied.  $MT2 is not modified.  

    @return 
    If the operation was successful, mtext_ncat () returns a
    pointer to the resulting M-text $MT1.  If an error is detected, it
    returns @c NULL and assigns an error code to the global variable
    #merror_code.  */

/***ja
    @brief M-text �ΰ������̤� M-text ���ղä���.

    �ؿ� mtext_ncat () �ϡ�M-text $MT2 �ΤϤ���� $N ʸ���� M-text
    $MT1 ���������դ��ä��롣$MT2 �Υƥ����ȥץ�ѥƥ��Ϥ��٤ƷѾ�����롣$MT2
    ��Ĺ���� $N �ʲ��ʤ�С�$MT2 �Τ��٤Ƥ�ʸ�����ղä���롣 $MT2 ���ѹ�����ʤ���

    @return
    ����������������硢mtext_ncat () ���ѹ����줿 M-text $MT1
    �ؤΥݥ��󥿤��֤������顼�����Ф��줿���� @c NULL ���֤��������ѿ�
    #merror_code �˥��顼�����ɤ����ꤹ�롣

    @latexonly \IPAlabel{mtext_ncat} @endlatexonly  */

/***
    @errors
    @c MERROR_RANGE

    @seealso
    mtext_cat (), mtext_cat_char ()  */

MText *
mtext_ncat (MText *mt1, MText *mt2, int n)
{
  M_CHECK_READONLY (mt1, NULL);
  if (n < 0)
    MERROR (MERROR_RANGE, NULL);
  if (mt2->nchars > 0)
    insert (mt1, mt1->nchars, mt2, 0, mt2->nchars < n ? mt2->nchars : n);
  return mt1;
}


/*=*/

/***en
    @brief Copy an M-text to another.

    The mtext_cpy () function copies M-text $MT2 to M-text $MT1 while
    inheriting all the text properties.  The old text in $MT1 is
    overwritten and the length of $MT1 is extended if necessary.  $MT2
    is not modified.

    @return
    This function returns a pointer to the resulting M-text $MT1.  */

/***ja
    @brief M-text ���̤� M-text �˥��ԡ�����.

    �ؿ� mtext_cpy () �� M-text $MT2 �� M-text $MT1 �˾�񤭥��ԡ����롣
    $MT2 �Υƥ����ȥץ�ѥƥ��Ϥ��٤ƷѾ�����롣$MT1 
    ��Ĺ����ɬ�פ˱����ƿ��Ф���롣$MT2 ���ѹ�����ʤ���

    @return
    ���δؿ����ѹ����줿 M-text $MT1 �ؤΥݥ��󥿤��֤���

    @latexonly \IPAlabel{mtext_cpy} @endlatexonly  */

/***
    @seealso
    mtext_ncpy (), mtext_copy ()  */

MText *
mtext_cpy (MText *mt1, MText *mt2)
{
  M_CHECK_READONLY (mt1, NULL);
  mtext_del (mt1, 0, mt1->nchars);
  if (mt2->nchars > 0)
    insert (mt1, 0, mt2, 0, mt2->nchars);
  return mt1;
}

/*=*/

/***en
    @brief Copy the first some characters in an M-text to another.

    The mtext_ncpy () function copies the first $N characters of
    M-text $MT2 to M-text $MT1 while inheriting all the text
    properties.  If the length of $MT2 is less than $N, all characters
    of $MT2 are copied.  The old text in $MT1 is overwritten and the
    length of $MT1 is extended if necessary.  $MT2 is not modified.

    @return
    If the operation was successful, mtext_ncpy () returns a pointer
    to the resulting M-text $MT1.  If an error is detected, it returns
    @c NULL and assigns an error code to the global variable 
    #merror_code.  */

/***ja
    @brief M-text �˴ޤޤ��ǽ�β�ʸ�����򥳥ԡ�����.

    �ؿ� mtext_ncpy () �ϡ�M-text $MT2 �κǽ�� $N ʸ���� M-text $MT1 
    �˾�񤭥��ԡ����롣$MT2 �Υƥ����ȥץ�ѥƥ��Ϥ��٤ƷѾ�����롣�⤷ $MT2
    ��Ĺ���� $N ���⾮������� $MT2 �Τ��٤Ƥ�ʸ���򥳥ԡ����롣$MT1 
    ��Ĺ����ɬ�פ˱����ƿ��Ф���롣$MT2 ���ѹ�����ʤ���

    @return 
    ����������������硢mtext_ncpy () ���ѹ����줿 M-text $MT1 
    �ؤΥݥ��󥿤��֤������顼�����Ф��줿���� @c NULL ���֤��������ѿ� 
    #merror_code �˥��顼�����ɤ����ꤹ�롣

    @latexonly \IPAlabel{mtext_ncpy} @endlatexonly  */

/***
    @errors
    @c MERROR_RANGE

    @seealso
    mtext_cpy (), mtext_copy ()  */

MText *
mtext_ncpy (MText *mt1, MText *mt2, int n)
{
  M_CHECK_READONLY (mt1, NULL);
  if (n < 0)
    MERROR (MERROR_RANGE, NULL);
  mtext_del (mt1, 0, mt1->nchars);
  if (mt2->nchars > 0)
    insert (mt1, 0, mt2, 0, mt2->nchars < n ? mt2->nchars : n);
  return mt1;
}

/*=*/

/***en
    @brief Create a new M-text from a part of an existing M-text.

    The mtext_duplicate () function creates a copy of sub-text of
    M-text $MT, starting at $FROM (inclusive) and ending at $TO
    (exclusive) while inheriting all the text properties of $MT.  $MT
    itself is not modified.

    @return 
    If the operation was successful, mtext_duplicate ()
    returns a pointer to the created M-text.  If an error is detected,
    it returns NULL and assigns an error code to the external variable
    #merror_code.  */

/***ja
    @brief ��¸�� M-text �ΰ������鿷���� M-text ��Ĥ���.

    �ؿ� mtext_duplicate () �ϡ�M-text $MT �� $FROM ��$FROM ���Τ�ޤ�ˤ���
    $TO ��$TO ���Τϴޤޤʤ��ˤޤǤ���ʬ�Υ��ԡ����롣���ΤȤ� $MT 
    �Υƥ����ȥץ�ѥƥ��Ϥ��٤ƷѾ�����롣$MT ���Τ�Τ��ѹ�����ʤ���

    @return
    ��������������С�mtext_duplicate () �Ϻ��줿 M-text 
    �ؤΥݥ��󥿤��֤������顼�����Ф��줿���� @c NULL ���֤��������ѿ� 
    #merror_code �˥��顼�����ɤ����ꤹ�롣

    @latexonly \IPAlabel{mtext_duplicate} @endlatexonly  */

/***
    @errors
    @c MERROR_RANGE

    @seealso
    mtext_dup ()  */

MText *
mtext_duplicate (MText *mt, int from, int to)
{
  MText *new = mtext ();

  M_CHECK_RANGE (mt, from, to, NULL, new);
  new->format = mt->format;
  new->coverage = mt->coverage;
  insert (new, 0, mt, from, to);
  return new;
}

/*=*/

/***en
    @brief Copy characters in the specified range into an M-text.

    The mtext_copy () function copies the text between $FROM
    (inclusive) and $TO (exclusive) in M-text $MT2 to the region
    starting at $POS in M-text $MT1 while inheriting the text
    properties.  The old text in $MT1 is overwritten and the length of
    $MT1 is extended if necessary.  $MT2 is not modified.

    @return
    If the operation was successful, mtext_copy () returns a pointer
    to the modified $MT1.  Otherwise, it returns @c NULL and assigns
    an error code to the external variable #merror_code.  */

/***ja
    @brief M-text �˻����ϰϤ�ʸ���򥳥ԡ�����.

    �ؿ� mtext_copy () �ϡ� M-text $MT2 �� $FROM ��$FROM ���Τ�ޤ�ˤ��� 
    $TO ��$TO ���Τϴޤޤʤ��ˤޤǤ��ϰϤΥƥ����Ȥ� M-text $MT1 �ΰ��� $POS
    �����񤭥��ԡ����롣$MT2 �Υƥ����ȥץ�ѥƥ��Ϥ��٤ƷѾ�����롣$MT1 
    ��Ĺ����ɬ�פ˱����ƿ��Ф���롣$MT2 ���ѹ�����ʤ���

    @latexonly \IPAlabel{mtext_copy} @endlatexonly

    @return
    ����������������硢mtext_copy () ���ѹ����줿 $MT1 
    �ؤΥݥ��󥿤��֤��������Ǥʤ���� @c NULL ���֤��������ѿ� #merror_code 
    �˥��顼�����ɤ����ꤹ�롣  */

/***
    @errors
    @c MERROR_RANGE

    @seealso
    mtext_cpy (), mtext_ncpy ()  */

MText *
mtext_copy (MText *mt1, int pos, MText *mt2, int from, int to)
{
  M_CHECK_POS_X (mt1, pos, NULL);
  M_CHECK_READONLY (mt1, NULL);
  M_CHECK_RANGE_X (mt2, from, to, NULL);
  mtext_del (mt1, pos, mt1->nchars);
  return insert (mt1, pos, mt2, from, to);
}

/*=*/


/***en
    @brief Delete characters in the specified range destructively.

    The mtext_del () function deletes the characters in the range
    $FROM (inclusive) and $TO (exclusive) from M-text $MT
    destructively.  As a result, the length of $MT shrinks by ($TO -
    $FROM) characters.

    @return
    If the operation was successful, mtext_del () returns 0.
    Otherwise, it returns -1 and assigns an error code to the external
    variable #merror_code.  */

/***ja
    @brief �����ϰϤ�ʸ�����˲�Ū�˼�����.

    �ؿ� mtext_del () �ϡ�M-text $MT �� $FROM ��$FROM ���Τ�ޤ�ˤ���
    $TO ��$TO ���Τϴޤޤʤ��ˤޤǤ�ʸ�����˲�Ū�˼����������Ū��
    $MT ��Ĺ���� ($TO @c - $FROM) �����̤ळ�Ȥˤʤ롣

    @return
    ��������������� mtext_del () �� 0 ���֤��������Ǥʤ���� -1 
    ���֤��������ѿ� #merror_code �˥��顼�����ɤ����ꤹ�롣  */

/***
    @errors
    @c MERROR_RANGE

    @seealso
    mtext_ins ()  */

int
mtext_del (MText *mt, int from, int to)
{
  int from_byte, to_byte;
  int unit_bytes = UNIT_BYTES (mt->format);

  M_CHECK_READONLY (mt, -1);
  M_CHECK_RANGE (mt, from, to, -1, 0);

  from_byte = POS_CHAR_TO_BYTE (mt, from);
  to_byte = POS_CHAR_TO_BYTE (mt, to);

  if (mt->cache_char_pos >= to)
    {
      mt->cache_char_pos -= to - from;
      mt->cache_byte_pos -= to_byte - from_byte;
    }
  else if (mt->cache_char_pos > from)
    {
      mt->cache_char_pos -= from;
      mt->cache_byte_pos -= from_byte;
    }

  mtext__adjust_plist_for_delete (mt, from, to - from);
  memmove (mt->data + from_byte * unit_bytes, 
	   mt->data + to_byte * unit_bytes,
	   (mt->nbytes - to_byte + 1) * unit_bytes);
  mt->nchars -= (to - from);
  mt->nbytes -= (to_byte - from_byte);
  mt->cache_char_pos = from;
  mt->cache_byte_pos = from_byte;
  return 0;
}


/*=*/

/***en
    @brief Insert an M-text into another M-text.

    The mtext_ins () function inserts M-text $MT2 into M-text $MT1, at
    position $POS.  As a result, $MT1 is lengthen by the length of
    $MT2.  On insertion, all the text properties of $MT2 are
    inherited.  The original $MT2 is not modified.

    @return
    If the operation was successful, mtext_ins () returns 0.
    Otherwise, it returns -1 and assigns an error code to the external
    variable #merror_code.  */

/***ja
    @brief M-text ���̤� M-text ����������.

    �ؿ� mtext_ins () �� M-text $MT1 �� $POS �ΰ��֤��̤� M-text $MT2 
    ���������롣���η�� $MT1 ��Ĺ���� $MT2 ��Ĺ��ʬ���������롣�����κݡ�$MT2
    �Υƥ����ȥץ�ѥƥ��Ϥ��٤ƷѾ�����롣$MT2 ���Τ�Τ��ѹ�����ʤ���

    @return
    ��������������� mtext_ins () �� 0 ���֤��������Ǥʤ���� -1 
    ���֤��������ѿ� #merror_code �˥��顼�����ɤ����ꤹ�롣  */

/***
    @errors
    @c MERROR_RANGE , @c MERROR_MTEXT

    @seealso
    mtext_del () , mtext_insert ()  */

int
mtext_ins (MText *mt1, int pos, MText *mt2)
{
  M_CHECK_READONLY (mt1, -1);
  M_CHECK_POS_X (mt1, pos, -1);

  if (mt2->nchars == 0)
    return 0;
  insert (mt1, pos, mt2, 0, mt2->nchars);
  return 0;
}

/*=*/

/***en
    @brief Insert sub-text of an M-text into another M-text.

    The mtext_insert () function inserts sub-text of M-text $MT2
    between $FROM (inclusive) and $TO (exclusive) into M-text $MT1, at
    position $POS.  As a result, $MT1 is lengthen by ($TO - $FROM).
    On insertion, all the text properties of the sub-text of $MT2 are
    inherited.

    @return 
    If the operation was successful, mtext_insert () returns
    0.  Otherwise, it returns -1 and assigns an error code to the
    external variable #merror_code.  */

/***ja
    @brief M-text �ΰ������̤� M-text ����������.

    �ؿ� mtext_insert () �� M-text $MT1 ��� $POS �ΰ��֤ˡ��̤� 
    M-text $MT2 �� $FROM ��$FROM ���Τ�ޤ�ˤ��� $TO ��$TO ���Τϴޤ�
    �ʤ��ˤޤǤ�ʸ�����������롣���Ū�� $MT1 ��Ĺ���� ($TO - $FROM) 
    �������Ӥ롣�����κݡ� $MT2 ��Υƥ����ȥץ�ѥƥ��Ϥ��٤ƷѾ�����
    �롣

    @return
    ��������������С�mtext_insert () �� 0 ���֤��������Ǥʤ���� -1 
    ���֤��������ѿ� #merror_code �˥��顼�����ɤ����ꤹ�롣  */

/***
    @errors
    @c MERROR_MTEXT , @c MERROR_RANGE

    @seealso
    mtext_ins ()  */

int
mtext_insert (MText *mt1, int pos, MText *mt2, int from, int to)
{
  M_CHECK_READONLY (mt1, -1);
  M_CHECK_POS_X (mt1, pos, -1);
  M_CHECK_RANGE (mt2, from, to, -1, 0);

  insert (mt1, pos, mt2, from, to);
  return 0;
}

/*=*/

/***en
    @brief Insert a character into an M-text.

    The mtext_ins_char () function inserts $N copies of character $C
    into M-text $MT at position $POS.  As a result, $MT is lengthen by
    $N.

    @return
    If the operation was successful, mtext_ins () returns 0.
    Otherwise, it returns -1 and assigns an error code to the external
    variable #merror_code.  */

/***ja
    @brief M-text ��ʸ������������.

    �ؿ� mtext_ins_char () �� M-text $MT �� $POS �ΰ��֤�ʸ�� $C �Υ��ԡ��� $N
    ���������롣���η�� $MT1 ��Ĺ���� $N ���������롣

    @return
    ��������������� mtext_ins_char () �� 0 ���֤��������Ǥʤ���� -1
    ���֤��������ѿ� #merror_code �˥��顼�����ɤ����ꤹ�롣  */

/***
    @errors
    @c MERROR_RANGE

    @seealso
    mtext_ins, mtext_del ()  */

int
mtext_ins_char (MText *mt, int pos, int c, int n)
{
  int nunits;
  int unit_bytes = UNIT_BYTES (mt->format);
  int pos_unit;
  int i;

  M_CHECK_READONLY (mt, -1);
  M_CHECK_POS_X (mt, pos, -1);
  if (c < 0 || c > MCHAR_MAX)
    MERROR (MERROR_MTEXT, -1);
  if (n <= 0)
    return 0;
  mtext__adjust_plist_for_insert (mt, pos, n, NULL);

  if (c >= 0x80
      && (mt->format == MTEXT_FORMAT_US_ASCII
	  || (c >= 0x10000 && (mt->format == MTEXT_FORMAT_UTF_16LE
			       || mt->format == MTEXT_FORMAT_UTF_16BE))))
    {
      mtext__adjust_format (mt, MTEXT_FORMAT_UTF_8);
      unit_bytes = 1;
    }
  else if (mt->format >= MTEXT_FORMAT_UTF_32LE)
    {
      if (mt->format != MTEXT_FORMAT_UTF_32)
	mtext__adjust_format (mt, MTEXT_FORMAT_UTF_32);
    }
  else if (mt->format >= MTEXT_FORMAT_UTF_16LE)
    {
      if (mt->format != MTEXT_FORMAT_UTF_16)
	mtext__adjust_format (mt, MTEXT_FORMAT_UTF_16);
    }

  nunits = CHAR_UNITS (c, mt->format);
  if ((mt->nbytes + nunits * n + 1) * unit_bytes > mt->allocated)
    {
      mt->allocated = (mt->nbytes + nunits * n + 1) * unit_bytes;
      MTABLE_REALLOC (mt->data, mt->allocated, MERROR_MTEXT);
    }
  pos_unit = POS_CHAR_TO_BYTE (mt, pos);
  if (mt->cache_char_pos > pos)
    {
      mt->cache_char_pos += n;
      mt->cache_byte_pos += nunits * n;
    }
  memmove (mt->data + (pos_unit + nunits * n) * unit_bytes,
	   mt->data + pos_unit * unit_bytes,
	   (mt->nbytes - pos_unit + 1) * unit_bytes);
  if (mt->format <= MTEXT_FORMAT_UTF_8)
    {
      unsigned char *p = mt->data + pos_unit;

      for (i = 0; i < n; i++)
	p += CHAR_STRING_UTF8 (c, p);
    }
  else if (mt->format == MTEXT_FORMAT_UTF_16)
    {
      unsigned short *p = (unsigned short *) mt->data + pos_unit;

      for (i = 0; i < n; i++)
	p += CHAR_STRING_UTF16 (c, p);
    }
  else
    {
      unsigned *p = (unsigned *) mt->data + pos_unit;

      for (i = 0; i < n; i++)
	*p++ = c;
    }
  mt->nchars += n;
  mt->nbytes += nunits * n;
  return 0;
}

/*=*/

/***en
    @brief Replace sub-text of M-text with another.

    The mtext_replace () function replaces sub-text of M-text $MT1
    between $FROM1 (inclusive) and $TO1 (exclusive) with the sub-text
    of M-text $MT2 between $FROM2 (inclusive) and $TO2 (exclusive).
    The new sub-text inherits text properties of the old sub-text.

    @return 
    If the operation was successful, mtext_replace () returns
    0.  Otherwise, it returns -1 and assigns an error code to the
    external variable #merror_code.  */

/***ja
    @brief M-text �ΰ������̤� M-text �ΰ������ִ�����.

    �ؿ� mtext_replace () �ϡ� M-text $MT1 �� $FROM1 ��$FROM1 ���Τ��
    ��ˤ��� $TO1 ��$TO1 ���Τϴޤޤʤ��ˤޤǤ� M-text $MT2 �� 
    $FROM2 ��$FROM2 ���Τ�ޤ�ˤ��� $TO2 ��$TO2 ���Τϴޤޤʤ��ˤ���
    �������롣�������������줿��ʬ�ϡ��֤����������Υƥ����ȥץ�ѥƥ�
    ���٤Ƥ�Ѿ����롣

    @return 
    ��������������С� mtext_replace () �� 0 ���֤��������Ǥ�
    ����� -1 ���֤��������ѿ� #merror_code �˥��顼�����ɤ����ꤹ�롣  */

/***
    @errors
    @c MERROR_MTEXT , @c MERROR_RANGE

    @seealso
    mtext_insert ()  */

int
mtext_replace (MText *mt1, int from1, int to1,
	       MText *mt2, int from2, int to2)
{
  int len1, len2;
  int from1_byte, from2_byte, old_bytes, new_bytes;
  int unit_bytes, total_bytes;
  unsigned char *p;
  int free_mt2 = 0;

  M_CHECK_READONLY (mt1, -1);
  M_CHECK_RANGE_X (mt1, from1, to1, -1);
  M_CHECK_RANGE_X (mt2, from2, to2, -1);

  if (from1 == to1)
    {
      struct MTextPlist *saved = mt2->plist;

      mt2->plist = NULL;
      insert (mt1, from1, mt2, from2, to2);
      mt2->plist = saved;
      return 0;
    }

  if (from2 == to2)
    {
      return mtext_del (mt1, from1, to1);
    }

  if (mt1 == mt2)
    {
      mt2 = mtext_duplicate (mt2, from2, to2);
      to2 -= from2;
      from2 = 0;
      free_mt2 = 1;
    }

  if (mt1->format != mt2->format
      && mt1->format == MTEXT_FORMAT_US_ASCII)
    mt1->format = MTEXT_FORMAT_UTF_8;
  if (mt1->format != mt2->format
      && mt1->coverage < mt2->coverage)
    mtext__adjust_format (mt1, mt2->format);
  if (mt1->format != mt2->format)
    {
      mt2 = mtext_duplicate (mt2, from2, to2);
      mtext__adjust_format (mt2, mt1->format);
      to2 -= from2;
      from2 = 0;
      free_mt2 = 1;
    }

  len1 = to1 - from1;
  len2 = to2 - from2;
  mtext__adjust_plist_for_change (mt1, from1, len1, len2);

  unit_bytes = UNIT_BYTES (mt1->format);
  from1_byte = POS_CHAR_TO_BYTE (mt1, from1) * unit_bytes;
  from2_byte = POS_CHAR_TO_BYTE (mt2, from2) * unit_bytes;
  old_bytes = POS_CHAR_TO_BYTE (mt1, to1) * unit_bytes - from1_byte;
  new_bytes = POS_CHAR_TO_BYTE (mt2, to2) * unit_bytes - from2_byte;
  total_bytes = mt1->nbytes * unit_bytes + (new_bytes - old_bytes);
  if (total_bytes + unit_bytes > mt1->allocated)
    {
      mt1->allocated = total_bytes + unit_bytes;
      MTABLE_REALLOC (mt1->data, mt1->allocated, MERROR_MTEXT);
    }
  p = mt1->data + from1_byte;
  if (to1 < mt1->nchars
      && old_bytes != new_bytes)
    memmove (p + new_bytes, p + old_bytes,
	     (mt1->nbytes + 1) * unit_bytes - (from1_byte + old_bytes));
  memcpy (p, mt2->data + from2_byte, new_bytes);
  mt1->nchars += len2 - len1;
  mt1->nbytes += (new_bytes - old_bytes) / unit_bytes;
  if (mt1->cache_char_pos >= to1)
    {
      mt1->cache_char_pos += len2 - len1;
      mt1->cache_byte_pos += new_bytes - old_bytes;
    }
  else if (mt1->cache_char_pos > from1)
    {
      mt1->cache_char_pos = from1;
      mt1->cache_byte_pos = from1_byte;
    }

  if (free_mt2)
    M17N_OBJECT_UNREF (mt2);
  return 0;
}

/*=*/

/***en
    @brief Search a character in an M-text.

    The mtext_character () function searches M-text $MT for character
    $C.  If $FROM is less than $TO, the search begins at position $FROM
    and goes forward but does not exceed ($TO - 1).  Otherwise, the search
    begins at position ($FROM - 1) and goes backward but does not
    exceed $TO.  An invalid position specification is regarded as both
    $FROM and $TO being 0.

    @return
    If $C is found, mtext_character () returns the position of its
    first occurrence.  Otherwise it returns -1 without changing the
    external variable #merror_code.  If an error is detected, it returns -1 and
    assigns an error code to the external variable #merror_code.  */

/***ja
    @brief M-text ���ʸ����õ��.

    �ؿ� mtext_character () �� M-text $MT ���ʸ�� $C ��õ�����⤷ 
    $FROM �� $TO ��꾮������С�õ���ϰ��� $FROM �������������ء����� 
    ($TO - 1) �ޤǿʤࡣ�����Ǥʤ���а��� ($FROM - 1) ������Ƭ�����ء�����
    $TO �ޤǿʤࡣ���֤λ���˸�꤬������ϡ�$FROM �� $TO 
    ��ξ���� 0 �����ꤵ�줿��ΤȤߤʤ���

    @return
    �⤷ $C �����Ĥ���С�mtext_character () 
    �Ϥ��κǽ�νи����֤��֤������Ĥ���ʤ��ä����ϳ����ѿ� #merror_code
    ���ѹ������� -1 ���֤������顼�����Ф��줿���� -1 ���֤��������ѿ�
    #merror_code �˥��顼�����ɤ����ꤹ�롣  */

/***
    @seealso
    mtext_chr(), mtext_rchr ()  */

int
mtext_character (MText *mt, int from, int to, int c)
{
  if (from < to)
    {
      /* We do not use M_CHECK_RANGE () because this function should
	 not set merror_code.  */
      if (from < 0 || to > mt->nchars)
	return -1;
      return find_char_forward (mt, from, to, c);
    }
  else
    {
      /* ditto */
      if (to < 0 || from > mt->nchars)
	return -1;
      return find_char_backward (mt, to, from, c);
    }
}


/*=*/

/***en
    @brief Return the position of the first occurrence of a character in an M-text.

    The mtext_chr () function searches M-text $MT for character $C.
    The search starts from the beginning of $MT and goes toward the end.

    @return
    If $C is found, mtext_chr () returns its position; otherwise it
    returns -1.  */

/***ja
    @brief M-text ��ǻ��ꤵ�줿ʸ�����ǽ�˸������֤��֤�.

    �ؿ� mtext_chr () �� M-text $MT ���ʸ�� $C ��õ����õ���� $MT 
    ����Ƭ�������������˿ʤࡣ

    @return
    �⤷ $C �����Ĥ���С�mtext_chr () 
    �Ϥ��νи����֤��֤������Ĥ���ʤ��ä����� -1 ���֤���

    @latexonly \IPAlabel{mtext_chr} @endlatexonly  */

/***
    @errors
    @c MERROR_RANGE

    @seealso
    mtext_rchr (), mtext_character ()  */

int
mtext_chr (MText *mt, int c)
{
  return find_char_forward (mt, 0, mt->nchars, c);
}

/*=*/

/***en
    @brief Return the position of the last occurrence of a character in an M-text.

    The mtext_rchr () function searches M-text $MT for character $C.
    The search starts from the end of $MT and goes backwardly toward the
    beginning.

    @return
    If $C is found, mtext_rchr () returns its position; otherwise it
    returns -1.  */

/***ja
    @brief M-text ��ǻ��ꤵ�줿ʸ�����Ǹ�˸������֤��֤�.

    �ؿ� mtext_rchr () �� M-text $MT ���ʸ�� $C ��õ����õ���� $MT 
    �κǸ夫����Ƭ�����ؤȸ�����˿ʤࡣ

    @return
    �⤷ $C �����Ĥ���С�mtext_rchr () 
    �Ϥ��νи����֤��֤������Ĥ���ʤ��ä����� -1 ���֤���

    @latexonly \IPAlabel{mtext_rchr} @endlatexonly  */

/***
    @errors
    @c MERROR_RANGE

    @seealso
    mtext_chr (), mtext_character ()  */

int
mtext_rchr (MText *mt, int c)
{
  return find_char_backward (mt, mt->nchars, 0, c);
}


/*=*/

/***en
    @brief Compare two M-texts character-by-character.

    The mtext_cmp () function compares M-texts $MT1 and $MT2 character
    by character.

    @return
    This function returns 1, 0, or -1 if $MT1 is found greater than,
    equal to, or less than $MT2, respectively.  Comparison is based on
    character codes.  */

/***ja
    @brief ��Ĥ� M-text ��ʸ��ñ�̤���Ӥ���.

    �ؿ� mtext_cmp () �ϡ� M-text $MT1 �� $MT2 ��ʸ��ñ�̤���Ӥ��롣

    @return
    ���δؿ��ϡ�$MT1 �� $MT2 ����������� 0��$MT1 �� $MT2 ����礭�����
    1��$MT1 �� $MT2 ��꾮������� -1 ���֤�����Ӥ�ʸ�������ɤ˴�Ť���

    @latexonly \IPAlabel{mtext_cmp} @endlatexonly  */

/***
    @seealso
    mtext_ncmp (), mtext_casecmp (), mtext_ncasecmp (),
    mtext_compare (), mtext_case_compare ()  */

int
mtext_cmp (MText *mt1, MText *mt2)
{
  return compare (mt1, 0, mt1->nchars, mt2, 0, mt2->nchars);
}


/*=*/

/***en
    @brief Compare initial parts of two M-texts character-by-character.

    The mtext_ncmp () function is similar to mtext_cmp (), but
    compares at most $N characters from the beginning.

    @return
    This function returns 1, 0, or -1 if $MT1 is found greater than,
    equal to, or less than $MT2, respectively.  */

/***ja
    @brief ��Ĥ� M-text ����Ƭ��ʬ��ʸ��ñ�̤���Ӥ���.

    �ؿ� mtext_ncmp () �ϡ��ؿ� mtext_cmp () Ʊ�ͤ� M-text 
    Ʊ�Τ���Ӥ���Ƭ������� $N ʸ���ޤǤ˴ؤ��ƹԤʤ���

    @return
    ���δؿ��ϡ�$MT1 �� $MT2 ����������� 0��$MT1 �� $MT2 ����礭����� 
    1��$MT1 �� $MT2 ��꾮������� -1 ���֤���

    @latexonly \IPAlabel{mtext_ncmp} @endlatexonly  */

/***
    @seealso
    mtext_cmp (), mtext_casecmp (), mtext_ncasecmp ()
    mtext_compare (), mtext_case_compare ()  */

int
mtext_ncmp (MText *mt1, MText *mt2, int n)
{
  if (n < 0)
    return 0;
  return compare (mt1, 0, (mt1->nchars < n ? mt1->nchars : n),
		  mt2, 0, (mt2->nchars < n ? mt2->nchars : n));
}

/*=*/

/***en
    @brief Compare specified regions of two M-texts.

    The mtext_compare () function compares two M-texts $MT1 and $MT2,
    character-by-character.  The compared regions are between $FROM1
    and $TO1 in $MT1 and $FROM2 to $TO2 in MT2.  $FROM1 and $FROM2 are
    inclusive, $TO1 and $TO2 are exclusive.  $FROM1 being equal to
    $TO1 (or $FROM2 being equal to $TO2) means an M-text of length
    zero.  An invalid region specification is regarded as both $FROM1
    and $TO1 (or $FROM2 and $TO2) being 0.

    @return
    This function returns 1, 0, or -1 if $MT1 is found greater than,
    equal to, or less than $MT2, respectively.  Comparison is based on
    character codes.  */

/***ja
    @brief ��Ĥ� M-text �λ��ꤷ���ΰ�Ʊ�Τ���Ӥ���.

    �ؿ� mtext_compare () ����Ĥ� M-text $MT1 �� $MT2 
    ��ʸ��ñ�̤���Ӥ��롣��Ӥ��оݤ� $MT1 �Τ��� $FROM1 ���� $TO1 �ޤǤȡ�$MT2 
    �Τ��� $FROM2 ���� $TO2 �ޤǤǤ��롣$FROM1 �� $FROM2 �ϴޤޤ졢$TO1 
    �� $TO2 �ϴޤޤ�ʤ���$FROM1 �� $TO1 �ʤ��뤤�� $FROM2 �� $TO2 
    �ˤ�����������Ĺ������� M-text ���̣���롣�ϰϻ���˸�꤬������ϡ�
    $FROM1 �� $TO1 �ʤ��뤤�� $FROM2 �� $TO2 �� ξ���� 0 �����ꤵ�줿��ΤȤߤʤ���

    @return
    ���δؿ��ϡ�$MT1 �� $MT2 ����������� 0��$MT1 �� $MT2 ����礭�����
    1 ��$MT1 �� $MT2 ��꾮������� -1 ���֤�����Ӥ�ʸ�������ɤ˴�Ť���  */

/***
    @seealso
    mtext_cmp (), mtext_ncmp (), mtext_casecmp (), mtext_ncasecmp (),
    mtext_case_compare ()  */

int
mtext_compare (MText *mt1, int from1, int to1, MText *mt2, int from2, int to2)
{
  if (from1 < 0 || from1 > to1 || to1 > mt1->nchars)
    from1 = to1 = 0;

  if (from2 < 0 || from2 > to2 || to2 > mt2->nchars)
    from2 = to2 = 0;

  return compare (mt1, from1, to1, mt2, from2, to2);
}

/*=*/

/***en
    @brief Search an M-text for a set of characters.

    The mtext_spn () function returns the length of the initial
    segment of M-text $MT1 that consists entirely of characters in
    M-text $MT2.  */

/***ja
    @brief ���뽸���ʸ���� M-text �����õ��.

    �ؿ� mtext_spn () �ϡ�M-text $MT1 ����Ƭ���� M-text $MT2 
    �˴ޤޤ��ʸ�������ǤǤ��Ƥ�����ʬ��Ĺ�����֤���

    @latexonly \IPAlabel{mtext_spn} @endlatexonly  */

/***
    @seealso
    mtext_cspn ()  */

int
mtext_spn (MText *mt, MText *accept)
{
  return span (mt, accept, 0, Mnil);
}

/*=*/

/***en
    @brief Search an M-text for the complement of a set of characters.

    The mtext_cspn () returns the length of the initial segment of
    M-text $MT1 that consists entirely of characters not in M-text $MT2.  */

/***ja
    @brief ���뽸���°���ʤ�ʸ���� M-text �����õ��.

    �ؿ� mtext_cspn () �ϡ�M-text $MT1 ����Ƭ��ʬ�� M-text $MT2
    �˴ޤޤ�ʤ�ʸ�������ǤǤ��Ƥ�����ʬ��Ĺ�����֤���

    @latexonly \IPAlabel{mtext_cspn} @endlatexonly  */

/***
    @seealso
    mtext_spn ()  */

int
mtext_cspn (MText *mt, MText *reject)
{
  return span (mt, reject, 0, Mt);
}

/*=*/

/***en
    @brief Search an M-text for any of a set of characters.

    The mtext_pbrk () function locates the first occurrence in M-text
    $MT1 of any of the characters in M-text $MT2.

    @return
    This function returns the position in $MT1 of the found character.
    If no such character is found, it returns -1. */

/***ja
    @brief ���뽸���°��ʸ���� M-text ���椫��õ��.

    �ؿ� mtext_pbrk () �ϡ�M-text $MT1 ��� M-text $MT2 
    ��ʸ���Τɤ줫���ǽ�˸������֤�Ĵ�٤롣

    @return 
    ���Ĥ��ä�ʸ���Ρ�$MT1 
    ��ˤ�����и����֤��֤����⤷���Τ褦��ʸ�����ʤ���� -1 ���֤���

    @latexonly \IPAlabel{mtext_pbrk} @endlatexonly  */

int
mtext_pbrk (MText *mt, MText *accept)
{
  int nchars = mtext_nchars (mt);
  int len = span (mt, accept, 0, Mt);

  return (len == nchars ? -1 : len);
}

/*=*/

/***en
    @brief Look for a token in an M-text.

    The mtext_tok () function searches a token that firstly occurs
    after position $POS in M-text $MT.  Here, a token means a
    substring each of which does not appear in M-text $DELIM.  Note
    that the type of $POS is not @c int but pointer to @c int.

    @return
    If a token is found, mtext_tok () copies the corresponding part of
    $MT and returns a pointer to the copy.  In this case, $POS is set
    to the end of the found token.  If no token is found, it returns
    @c NULL without changing the external variable #merror_code.  If an
    error is detected, it returns @c NULL and assigns an error code
    to the external variable #merror_code. */

/***ja
    @brief M-text ��Υȡ������õ��.

    �ؿ� mtext_tok () �ϡ�M-text $MT ����ǰ��� $POS 
    �ʹߺǽ�˸����ȡ������õ���������ǥȡ�����Ȥ� M-text $DELIM
    ����˸����ʤ�ʸ����������ʤ���ʬʸ����Ǥ��롣$POS �η��� @c int �ǤϤʤ��� @c
    int �ؤΥݥ��󥿤Ǥ��뤳�Ȥ���ա�

    @return
    �⤷�ȡ����󤬸��Ĥ���� mtext_tok ()�Ϥ��Υȡ����������������ʬ�� 
    $MT �򥳥ԡ��������Υ��ԡ��ؤΥݥ��󥿤��֤������ξ�硢$POS 
    �ϸ��Ĥ��ä��ȡ�����ν�ü�˥��åȤ���롣�ȡ����󤬸��Ĥ���ʤ��ä����ϳ����ѿ�
    #merror_code ���Ѥ����� @c NULL ���֤������顼�����Ф��줿���� 
    @c NULL ���֤��������ѿ� #merror_code �˥��顼�����ɤ����ꤹ�롣

    @latexonly \IPAlabel{mtext_tok} @endlatexonly  */

/***
    @errors
    @c MERROR_RANGE  */

MText *
mtext_tok (MText *mt, MText *delim, int *pos)
{
  int nchars = mtext_nchars (mt);
  int pos2;

  M_CHECK_POS (mt, *pos, NULL);

  /*
    Skip delimiters starting at POS in MT.
    Never do *pos += span(...), or you will change *pos
    even though no token is found.
   */
  pos2 = *pos + span (mt, delim, *pos, Mnil);

  if (pos2 == nchars)
    return NULL;

  *pos = pos2 + span (mt, delim, pos2, Mt);
  return (insert (mtext (), 0, mt, pos2, *pos));
}

/*=*/

/***en
    @brief Locate an M-text in another.

    The mtext_text () function finds the first occurrence of M-text
    $MT2 in M-text $MT1 after the position $POS while ignoring
    difference of the text properties.

    @return
    If $MT2 is found in $MT1, mtext_text () returns the position of it
    first occurrence.  Otherwise it returns -1.  If $MT2 is empty, it
    returns 0.  */

/***ja
    @brief M-text ����̤� M-text ��õ��.

    �ؿ� mtext_text () �ϡ�M-text $MT1 ��ǰ��� $POS �ʹߤ˸����� 
    M-text $MT2 �κǽ�ΰ��֤�Ĵ�٤롣�ƥ����ȥץ�ѥƥ��ΰ㤤��̵�뤵��롣

    @return
    $MT1 ��� $MT2 �����Ĥ���С�mtext_text() 
    �Ϥ��κǽ�νи����֤��֤������Ĥ���ʤ����� -1 ���֤����⤷ $MT2 �����ʤ�� 0 ���֤���

    @latexonly \IPAlabel{mtext_text} @endlatexonly  */

int
mtext_text (MText *mt1, int pos, MText *mt2)
{
  int from = pos;
  int c = mtext_ref_char (mt2, 0);
  int nbytes2 = mtext_nbytes (mt2);
  int limit;
  int use_memcmp = (mt1->format == mt2->format
		    || (mt1->format < MTEXT_FORMAT_UTF_8
			&& mt2->format == MTEXT_FORMAT_UTF_8));
  int unit_bytes = UNIT_BYTES (mt1->format);

  if (from + mtext_nchars (mt2) > mtext_nchars (mt1))
    return -1;
  limit = mtext_nchars (mt1) - mtext_nchars (mt2) + 1;

  while (1)
    {
      int pos_byte;

      if ((pos = mtext_character (mt1, from, limit, c)) < 0)
	return -1;
      pos_byte = POS_CHAR_TO_BYTE (mt1, pos);
      if (use_memcmp
	  ? ! memcmp (mt1->data + pos_byte * unit_bytes, 
		      mt2->data, nbytes2 * unit_bytes)
	  : ! compare (mt1, pos, mt2->nchars, mt2, 0, mt2->nchars))
	break;
      from = pos + 1;
    }
  return pos;
}

/***en
    @brief Locate an M-text in a specific range of another.

    The mtext_search () function searches for the first occurrence of
    M-text $MT2 in M-text $MT1 in the region $FROM and $TO while
    ignoring difference of the text properties.  If $FROM is less than
    $TO, the forward search starts from $FROM, otherwise the backward
    search starts from $TO.

    @return
    If $MT2 is found in $MT1, mtext_search () returns the position of the
    first occurrence.  Otherwise it returns -1.  If $MT2 is empty, it
    returns 0.  */

/***ja
    @brief M-text ���������ΰ���̤� M-text ��õ��.

    �ؿ� mtext_search () �ϡ�M-text $MT1 ��� $FROM ���� $TO 
    �ޤǤδ֤��ΰ��M-text $MT2 
    ���ǽ�˸�������֤�Ĵ�٤롣�ƥ����ȥץ�ѥƥ��ΰ㤤��̵�뤵��롣�⤷
    $FROM �� $TO ��꾮�������õ���ϰ��� $FROM �������������ء������Ǥʤ����
    $TO ������Ƭ�����ؿʤࡣ

    @return
    $MT1 ��� $MT2 �����Ĥ���С�mtext_search() 
    �Ϥ��κǽ�νи����֤��֤������Ĥ���ʤ����� -1 ���֤����⤷ $MT2 �����ʤ�� 0 ���֤���
    */

int
mtext_search (MText *mt1, int from, int to, MText *mt2)
{
  int c = mtext_ref_char (mt2, 0);
  int from_byte;
  int nbytes2 = mtext_nbytes (mt2);

  if (mt1->format > MTEXT_FORMAT_UTF_8
      || mt2->format > MTEXT_FORMAT_UTF_8)
    MERROR (MERROR_MTEXT, -1);

  if (from < to)
    {
      to -= mtext_nchars (mt2);
      if (from > to)
	return -1;
      while (1)
	{
	  if ((from = find_char_forward (mt1, from, to, c)) < 0)
	    return -1;
	  from_byte = POS_CHAR_TO_BYTE (mt1, from);
	  if (! memcmp (mt1->data + from_byte, mt2->data, nbytes2))
	    break;
	  from++;
	}
    }
  else if (from > to)
    {
      from -= mtext_nchars (mt2);
      if (from < to)
	return -1;
      while (1)
	{
	  if ((from = find_char_backward (mt1, to, from + 1, c)) < 0)
	    return -1;
	  from_byte = POS_CHAR_TO_BYTE (mt1, from);
	  if (! memcmp (mt1->data + from_byte, mt2->data, nbytes2))
	    break;
	  from--;
	}
    }

  return from;
}

/*=*/

/***en
    @brief Compare two M-texts ignoring cases.

    The mtext_casecmp () function is similar to mtext_cmp (), but
    ignores cases on comparison.

    @return
    This function returns 1, 0, or -1 if $MT1 is found greater than,
    equal to, or less than $MT2, respectively.  */

/***ja
    @brief ��Ĥ� M-text ����ʸ������ʸ���ζ��̤�̵�뤷����Ӥ���.

    �ؿ� mtext_casecmp () �ϡ��ؿ� mtext_cmp () Ʊ�ͤ� M-text 
    Ʊ�Τ���Ӥ���ʸ������ʸ���ζ��̤�̵�뤷�ƹԤʤ���

    @return
    ���δؿ��ϡ�$MT1 �� $MT2 ����������� 0��$MT1 �� $MT2 
    ����礭����� 1��$MT1 �� $MT2 ��꾮������� -1 ���֤���

    @latexonly \IPAlabel{mtext_casecmp} @endlatexonly  */

/***
    @seealso
    mtext_cmp (), mtext_ncmp (), mtext_ncasecmp ()
    mtext_compare (), mtext_case_compare ()  */

int
mtext_casecmp (MText *mt1, MText *mt2)
{
  return case_compare (mt1, 0, mt1->nchars, mt2, 0, mt2->nchars);
}

/*=*/

/***en
    @brief Compare initial parts of two M-texts ignoring cases.

    The mtext_ncasecmp () function is similar to mtext_casecmp (), but
    compares at most $N characters from the beginning.

    @return
    This function returns 1, 0, or -1 if $MT1 is found greater than,
    equal to, or less than $MT2, respectively.  */

/***ja
    @brief ��Ĥ� M-text ����Ƭ��ʬ����ʸ������ʸ���ζ��̤�̵�뤷����Ӥ���.

    �ؿ� mtext_ncasecmp () �ϡ��ؿ� mtext_casecmp () Ʊ�ͤ� M-text 
    Ʊ�Τ���Ӥ���Ƭ������� $N ʸ���ޤǤ˴ؤ��ƹԤʤ���

    @return
    ���δؿ��ϡ�$MT1 �� $MT2 ����������� 0��$MT1 �� $MT2 
    ����礭����� 1��$MT1 �� $MT2 ��꾮������� -1 ���֤���

    @latexonly \IPAlabel{mtext_ncasecmp} @endlatexonly  */

/***
    @seealso
    mtext_cmp (), mtext_casecmp (), mtext_casecmp ()
    mtext_compare (), mtext_case_compare ()  */

int
mtext_ncasecmp (MText *mt1, MText *mt2, int n)
{
  if (n < 0)
    return 0;
  return case_compare (mt1, 0, (mt1->nchars < n ? mt1->nchars : n),
		       mt2, 0, (mt2->nchars < n ? mt2->nchars : n));
}

/*=*/

/***en
    @brief Compare specified regions of two M-texts ignoring cases. 

    The mtext_case_compare () function compares two M-texts $MT1 and
    $MT2, character-by-character, ignoring cases.  The compared
    regions are between $FROM1 and $TO1 in $MT1 and $FROM2 to $TO2 in
    MT2.  $FROM1 and $FROM2 are inclusive, $TO1 and $TO2 are
    exclusive.  $FROM1 being equal to $TO1 (or $FROM2 being equal to
    $TO2) means an M-text of length zero.  An invalid region
    specification is regarded as both $FROM1 and $TO1 (or $FROM2 and
    $TO2) being 0.

    @return
    This function returns 1, 0, or -1 if $MT1 is found greater than,
    equal to, or less than $MT2, respectively.  Comparison is based on
    character codes.  */

/***ja 
    @brief ��Ĥ� M-text �λ��ꤷ���ΰ����ʸ������ʸ���ζ��̤�̵�뤷����Ӥ���.

    �ؿ� mtext_compare () ����Ĥ� M-text $MT1 �� $MT2 
    ����ʸ������ʸ���ζ��̤�̵�뤷��ʸ��ñ�̤���Ӥ��롣��Ӥ��оݤ� $MT1 
    �� $FROM1 ���� $TO1 �ޤǡ�$MT2 �� $FROM2 ���� $TO2 �ޤǤǤ��롣
    $FROM1 �� $FROM2 �ϴޤޤ졢$TO1 �� $TO2 �ϴޤޤ�ʤ���$FROM1 �� $TO1
    �ʤ��뤤�� $FROM2 �� $TO2 �ˤ�����������Ĺ������� M-text 
    ���̣���롣�ϰϻ���˸�꤬������ϡ�$FROM1 �� $TO1 �ʤ��뤤�� 
    $FROM2 �� $TO2 ��ξ���� 0 �����ꤵ�줿��Τȸ��ʤ���

    @return
    ���δؿ��ϡ�$MT1 �� $MT2 ����������� 0��$MT1 �� $MT2 ����礭�����
    1��$MT1 �� $MT2 ��꾮������� -1���֤�����Ӥ�ʸ�������ɤ˴�Ť���

  @latexonly \IPAlabel{mtext_case_compare} @endlatexonly
*/

/***
    @seealso
    mtext_cmp (), mtext_ncmp (), mtext_casecmp (), mtext_ncasecmp (),
    mtext_compare ()  */

int
mtext_case_compare (MText *mt1, int from1, int to1,
		    MText *mt2, int from2, int to2)
{
  if (from1 < 0 || from1 > to1 || to1 > mt1->nchars)
    from1 = to1 = 0;

  if (from2 < 0 || from2 > to2 || to2 > mt2->nchars)
    from2 = to2 = 0;

  return case_compare (mt1, from1, to1, mt2, from2, to2);
}

/*=*/

/***en
    @brief Lowercase an M-text.

    The mtext_lowercase () function destructively converts each
    character in M-text $MT to lowercase.  Adjacent characters in $MT
    may affect the case conversion.  If the Mlanguage text property is
    attached to $MT, it may also affect the conversion.  The length of
    $MT may change.  Characters that cannot be converted to lowercase
    is left unchanged.  All the text properties are inherited.

    @return
    This function returns the length of the updated $MT.
*/

/***ja
    @brief M-text ��ʸ���ˤ���.

    �ؿ� mtext_lowercase () �� M-text $MT ��γ�ʸ�����˲�Ū�˾�ʸ������
    �����롣�Ѵ��˺ݤ������ܤ���ʸ���αƶ�������뤳�Ȥ����롣$MT �˥�
    �����ȥץ�ѥƥ� Mlanguage ���դ��Ƥ�����ϡ�������Ѵ��˱ƶ���
    Ϳ�����롣$MT ��Ĺ�����Ѥ�뤳�Ȥ����롣��ʸ�����Ѵ��Ǥ��ʤ��ä�ʸ
    ���Ϥ��Τޤ޻Ĥ롣�ƥ����ȥץ�ѥƥ��Ϥ��٤ƷѾ�����롣

    @return
    ���δؿ��Ϲ������ $MT ��Ĺ�����֤���
*/

/***
    @seealso
     mtext_titlecase (), mtext_uppercase ()
*/

int
mtext_lowercase (MText *mt)

{
  CASE_CONV_INIT (-1);

  return mtext__lowercase (mt, 0, mtext_len (mt));
}

/*=*/

/***en
    @brief Titlecase an M-text.

    The mtext_titlecase () function destructively converts the first
    character with the cased property in M-text $MT to titlecase and
    the others to lowercase.  The length of $MT may change.  If the
    character cannot be converted to titlecase, it is left unchanged.
    All the text properties are inherited.

    @return
    This function returns the length of the updated $MT.
*/

/***ja
    @brief M-text �򥿥��ȥ륱�����ˤ���.

    �ؿ� mtext_titlecase () �� M-text $MT ��� cased �ץ�ѥƥ������
    �ǽ��ʸ���򥿥��ȥ륱�����ˡ������Ƥ���ʹߤ�ʸ����ʸ�����˲�Ū
    ���Ѵ����롣$MT ��Ĺ�����Ѥ�뤳�Ȥ����롣�����ȥ륱�����ˤ��Ѵ���
    ���ʤ��ä����Ϥ��Τޤޤ��Ѥ��ʤ����ƥ����ȥץ�ѥƥ��Ϥ��٤Ʒ�
    ������롣

    @return
    ���δؿ��Ϲ������ $MT ��Ĺ�����֤���
*/

/***
    @seealso
     mtext_lowercase (), mtext_uppercase ()
*/

int
mtext_titlecase (MText *mt)
{
  int len = mtext_len (mt), from, to;

  CASE_CONV_INIT (-1);

  /* Find 1st cased character. */
  for (from = 0; from < len; from++)
    {
      int csd = (int) mchartable_lookup (cased, mtext_ref_char (mt, from));

      if (csd > 0 && csd & CASED)
	break;
    }

  if (from == len)
    return len;

  if (from == len - 1)
    return (mtext__titlecase (mt, from, len));

  /* Go through following combining characters. */
  for (to = from + 1;
       (to < len
	&& ((int) mchartable_lookup (combining_class, mtext_ref_char (mt, to))
	    > 0));
       to++);

  /* Titlecase the region and prepare for next lowercase operation.
     MT may be shortened or lengthened. */
  from = mtext__titlecase (mt, from, to);

  return (mtext__lowercase (mt, from, mtext_len (mt)));
}

/*=*/

/***en
    @brief Uppercase an M-text.


    The mtext_uppercase () function destructively converts each
    character in M-text $MT to uppercase.  Adjacent characters in $MT
    may affect the case conversion.  If the Mlanguage text property is
    attached to $MT, it may also affect the conversion.  The length of
    $MT may change.  Characters that cannot be converted to uppercase
    is left unchanged.  All the text properties are inherited.

    @return
    This function returns the length of the updated $MT.
*/

/***ja
    @brief M-text ����ʸ���ˤ���.

    �ؿ� mtext_uppercase () �� M-text $MT ��γ�ʸ�����˲�Ū����ʸ������
    �����롣�Ѵ��˺ݤ������ܤ���ʸ���αƶ�������뤳�Ȥ����롣$MT �˥�
    �����ȥץ�ѥƥ� Mlanguage ���դ��Ƥ�����ϡ�������Ѵ��˱ƶ���
    Ϳ�����롣$MT ��Ĺ�����Ѥ�뤳�Ȥ����롣��ʸ�����Ѵ��Ǥ��ʤ��ä�ʸ
    ���Ϥ��Τޤ޻Ĥ롣�ƥ����ȥץ�ѥƥ��Ϥ��٤ƷѾ�����롣

    @return
    ���δؿ��Ϲ������ $MT ��Ĺ�����֤���
*/

/***
    @seealso
     mtext_lowercase (), mtext_titlecase ()
*/

int
mtext_uppercase (MText *mt)
{
  CASE_CONV_INIT (-1);

  return (mtext__uppercase (mt, 0, mtext_len (mt)));
}

/*** @} */

#include <stdio.h>

/*** @addtogroup m17nDebug */
/*=*/
/*** @{  */

/***en
    @brief Dump an M-text.

    The mdebug_dump_mtext () function prints the M-text $MT in a human
    readable way to the stderr or to what specified by the environment
    variable MDEBUG_OUTPUT_FILE.  $INDENT specifies how many columns
    to indent the lines but the first one.  If $FULLP is zero, this
    function prints only a character code sequence.  Otherwise, it
    prints the internal byte sequence and text properties as well.

    @return
    This function returns $MT.  */
/***ja
    @brief M-text �����פ���.

    �ؿ� mdebug_dump_mtext () �� M-text $MT ��ɸ�२�顼���Ϥ⤷���ϴ�
    ���ѿ� MDEBUG_DUMP_FONT �ǻ��ꤵ�줿�ե�����˿ʹ֤˲��ɤʷ��ǰ���
    ���롣 $INDENT �ϣ����ܰʹߤΥ���ǥ�Ȥ���ꤹ�롣$FULLP �� 0 �ʤ�
    �С�ʸ���������������������롣�����Ǥʤ���С������Х�����ȥƥ�
    ���ȥץ�ѥƥ���������롣

    @return
    ���δؿ��� $MT ���֤���  */

MText *
mdebug_dump_mtext (MText *mt, int indent, int fullp)
{
  int i;

  if (! fullp)
    {
      fprintf (mdebug__output, "\"");
      for (i = 0; i < mt->nchars; i++)
	{
	  int c = mtext_ref_char (mt, i);

	  if (c == '"' || c == '\\')
	    fprintf (mdebug__output, "\\%c", c);
	  else if ((c >= ' ' && c < 127) || c == '\n')
	    fprintf (mdebug__output, "%c", c);
	  else
	    fprintf (mdebug__output, "\\x%02X", c);
	}
      fprintf (mdebug__output, "\"");
      return mt;
    }

  fprintf (mdebug__output,
	   "(mtext (size %d %d %d) (cache %d %d)",
	   mt->nchars, mt->nbytes, mt->allocated,
	   mt->cache_char_pos, mt->cache_byte_pos);

  if (mt->nchars > 0)
    {
      char *prefix = (char *) alloca (indent + 1);
      unsigned char *p;

      memset (prefix, 32, indent);
      prefix[indent] = 0;

      fprintf (mdebug__output, "\n%s (bytes \"", prefix);
      for (i = 0; i < mt->nbytes; i++)
	fprintf (mdebug__output, "\\x%02x", mt->data[i]);
      fprintf (mdebug__output, "\")\n");
      fprintf (mdebug__output, "%s (chars \"", prefix);
      p = mt->data;
      for (i = 0; i < mt->nchars; i++)
	{
	  int len;
	  int c = STRING_CHAR_AND_BYTES (p, len);

	  if (c == '"' || c == '\\')
	    fprintf (mdebug__output, "\\%c", c);
	  else if (c >= ' ' && c < 127)
	    fputc (c, mdebug__output);
	  else
	    fprintf (mdebug__output, "\\x%X", c);
	  p += len;
	}
      fprintf (mdebug__output, "\")");
      if (mt->plist)
	{
	  fprintf (mdebug__output, "\n%s ", prefix);
	  dump_textplist (mt->plist, indent + 1);
	}
    }
  fprintf (mdebug__output, ")");
  return mt;
}

/*** @} */ 

/*
  Local Variables:
  coding: euc-japan
  End:
*/
