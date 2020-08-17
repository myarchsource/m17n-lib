/* chartab.h -- character table module.
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
    @addtogroup m17nChartable
    @brief Chartable objects and API for them.

    The m17n library supports enormous number of characters.  Thus, if
    attributes of each character are to be stored in a simple array,
    such an array would be impractically big.  The attributes usually
    used, however, are often assigned only to a range of characters.
    Even when all characters have attributes, characters of
    consecutive character code tend to have the same attribute values.

    The m17n library utilizes this tendency to store characters and
    their attribute values efficiently in an object called @e
    Chartable.  Although a chartable object is not a simple array,
    application programs can handle a chartable as if it is an array.
    Attribute values of a character can be obtained by accessing a
    Chartable for the attribute with the character code of the
    specified character.

    A chartable is a managed object.  */

/***ja
    @addtogroup m17nChartable ʸ���ơ��֥�

    @brief ʸ���ơ��֥�Ȥ���˴ؤ��� API.

    m17n �饤�֥�꤬����ʸ���ζ��֤Ϲ���Ǥ��뤿�ᡢʸ����ξ����ñ�������˳�Ǽ���褦�Ȥ���ȡ���������ϵ���ˤʤꤹ���������Ū�Ǥ��롣
    �������̾�ɬ�פȤʤ�ʸ���ˤĤ��Ƥξ���ϡ�����������ϰϤ�ʸ���ˤΤ��դ��Ƥ��뤳�Ȥ�¿����
    ��ʸ���˴ؤ��ƾ��󤬤�����ˤ⡢Ϣ³����ʸ�������ɤ����ʸ����Ʊ���������Ĥ��Ȥ�¿����

    ���Τ褦�ʷ��������Ѥ���ʸ���Ȥ����ղþ�����ΨŪ�˳�Ǽ���뤿��ˡ�
    m17n �饤�֥��� @e ʸ���ơ��֥� (chartable) �ȸƤ֥��֥������Ȥ��Ѥ��롣
    ʸ���ơ��֥������ǤϤʤ��������ץꥱ�������ץ�����ʸ���ơ��֥������ΰ��Ȥ��ư������Ȥ��Ǥ��롣
    ����ʸ���ˤĤ��Ƥ�����ξ���ϡ����ξ�������ʸ���ơ��֥�򤽤�ʸ���Υ����ɤǰ�������
    �������롣  

    ʸ���ơ��֥�ϴ��������֥������ȤǤ��롣 */

/*=*/
#if !defined (FOR_DOXYGEN) || defined (DOXYGEN_INTERNAL_MODULE)
/*** @addtogroup m17nInternal
     @{ */

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <stdio.h>

#include "m17n.h"
#include "m17n-misc.h"
#include "internal.h"
#include "symbol.h"

static M17NObjectArray chartable_table;

/*** Maximum depth of char-table.  */
#define CHAR_TAB_MAX_DEPTH 3

/** @name Define: Number of characters covered by char-table of each level.
    @{ */

/** BITs for number of characters covered by char-table of each
    level.  */
#if MCHAR_MAX < 0x400000

#define SUB_BITS_0	22	/* i.e. 0x400000 chars */
#define SUB_BITS_1	16	/* i.e. 0x10000 chars */
#define SUB_BITS_2	12	/* i.e. 0x1000 chars */
#define SUB_BITS_3	7	/* i.e. 0x80 chars */

#else  /* MCHAR_MAX >= 0x400000 */

#define SUB_BITS_0	31
#define SUB_BITS_1	24
#define SUB_BITS_2	16
#define SUB_BITS_3	8
#endif

/** @} */

/** How many characters a char-table covers at each level.  */
static const int chartab_chars[] =
  { (1 << SUB_BITS_0),
    (1 << SUB_BITS_1),
    (1 << SUB_BITS_2),
    (1 << SUB_BITS_3) };

/** How many slots a char-table has at each level.  */
static const int chartab_slots[] =
  { (1 << (SUB_BITS_0 - SUB_BITS_1)),
    (1 << (SUB_BITS_1 - SUB_BITS_2)),
    (1 << (SUB_BITS_2 - SUB_BITS_3)),
    (1 << SUB_BITS_3) };

/** Mask bits to obtain the valid bits from a character code for looking
    up a char-table of each level.  */
static const int chartab_mask[] =
  { (int) ((((unsigned) 1) << SUB_BITS_0) - 1),
    (1 << SUB_BITS_1) - 1,
    (1 << SUB_BITS_2) - 1,
    (1 << SUB_BITS_3) - 1 };

/** Bit-shifting counts to obtain a valid index from a character code
    for looking up a char-table of each level.  */
static const int chartab_shift[] =
  { SUB_BITS_1, SUB_BITS_2, SUB_BITS_3, 0 };


/** Index for looking up character C in a char-table at DEPTH.  */
#define SUB_IDX(depth, c)	\
  (((c) & chartab_mask[depth]) >> chartab_shift[depth])


/** Structure of sub char-table.  */
typedef struct MSubCharTable MSubCharTable;

struct MSubCharTable
{
#if SUB_BITS_0 > 24

  /* The depth of the table; 0, 1, 2, or 3. */
  int depth;

  /* The minimum character covered by the table.  */
  int min_char;

#else  /* SUB_BITS_0 <= 24 */

  /* The value is ((<depth> << 24) | <min_char>).  */
  int depth_min_char;

#endif	/* SUB_BITS_0 <= 24 */

  /** The default value of characters covered by the table.  */
  void *default_value;

  /** For a table of bottom level, array of values.  For a non-bottom
     table, array of sub char-tables.  It may be NULL if all
     characters covered by the table has <default_value>.  */
  union {
    void **values;
    MSubCharTable *tables;
  } contents;
};

#if SUB_BITS_0 > 24
#define TABLE_DEPTH(table) ((table)->depth)
#define TABLE_MIN_CHAR(table) ((table)->min_char)
#define SET_DEPTH_MIN_CHAR(table, DEPTH, MIN_CHAR) \
  ((table)->depth = (DEPTH), (table)->min_char = (MIN_CHAR))
#else  /* SUB_BITS_0 <= 24 */
#define TABLE_DEPTH(table) ((table)->depth_min_char >> 24)
#define TABLE_MIN_CHAR(table) ((table)->depth_min_char & 0xFFFFFF)
#define SET_DEPTH_MIN_CHAR(table, DEPTH, MIN_CHAR) \
  ((table)->depth_min_char = ((DEPTH) << 24) | (MIN_CHAR))
#endif  /* SUB_BITS_0 <= 24 */

/** Structure of char-table.  */

struct MCharTable
{
  /** Common header for a managed object.  */
  M17NObject control;

  /** Key of the table.  */
  MSymbol key;

  /** The minimum and maximum characters covered by the table.  */
  int min_char, max_char;

  MSubCharTable subtable;
};




/* Local functions.  */

/** Allocate and initialize an array of sub-tables for sub char-table
    TABLE.  It is assumed that TABLE_DEPTH (TABLE) <
    CHAR_TAB_MAX_DEPTH.*/

static void
make_sub_tables (MSubCharTable *table, int managedp)
{
  int depth = TABLE_DEPTH (table);
  int min_char = TABLE_MIN_CHAR (table);
  int slots = chartab_slots[depth];
  int chars = chartab_chars[depth + 1];
  MSubCharTable *tables;
  int i;

  MTABLE_MALLOC (tables, slots, MERROR_CHARTABLE);

  for (i = 0; i < slots; i++, min_char += chars)
    {
      SET_DEPTH_MIN_CHAR (tables + i, depth + 1, min_char);
      tables[i].default_value = table->default_value;
      tables[i].contents.tables = NULL;
    }
  if (managedp && table->default_value)
    M17N_OBJECT_REF_NTIMES (tables->default_value, slots);
  table->contents.tables = tables;
}


/** Allocate and initialize an array of values for sub char-table
    TABLE.  It is assumed that TABLE_DEPTH (TABLE) ==
    CHAR_TAB_MAX_DEPTH.  */

static void
make_sub_values (MSubCharTable *table, int managedp)
{
  int slots = chartab_slots[CHAR_TAB_MAX_DEPTH];
  void **values;
  int i;

  MTABLE_MALLOC (values, slots, MERROR_CHARTABLE);

  for (i = 0; i < slots; i++)
    values[i] = table->default_value;
  if (managedp && table->default_value)
    M17N_OBJECT_REF_NTIMES (table->default_value, slots);
  table->contents.values = values;
}


/** Free contents of sub char-table TABLE and the default value of
    TABLE.  Free also the sub-tables recursively.  */

static void
free_sub_tables (MSubCharTable *table, int managedp)
{
  int depth = TABLE_DEPTH (table);
  int slots = chartab_slots[depth];

  if (table->contents.tables)
    {
      if (depth < CHAR_TAB_MAX_DEPTH)
	{
	  while (slots--)
	    free_sub_tables (table->contents.tables + slots, managedp);
	  free (table->contents.tables);
	}
      else
	{
	  if (managedp)
	    while (slots--)
	      {
		if (table->contents.values[slots])
		  M17N_OBJECT_UNREF (table->contents.values[slots]);
	      }
	  free (table->contents.values);
	}
      table->contents.tables = NULL;
    }
  if (managedp && table->default_value)
    M17N_OBJECT_UNREF (table->default_value);
}


/** In sub char-table TABLE, set value VAL for characters of the range
    FROM and TO. */

static void
set_chartable_range (MSubCharTable *table, int from, int to, void *val,
		     int managedp)
{
  int depth = TABLE_DEPTH (table);
  int min_char = TABLE_MIN_CHAR (table);
  int max_char = min_char + (chartab_chars[depth] - 1);
  int i;

  if (max_char < 0 || max_char > MCHAR_MAX)
    max_char = MCHAR_MAX;

  if (from < min_char)
    from = min_char;
  if (to > max_char)
    to = max_char;

  if (from == min_char && to == max_char)
    {
      free_sub_tables (table, managedp);
      if (managedp && val)
	M17N_OBJECT_REF (val);
      table->default_value = val;
      return;
    }

  if (depth < CHAR_TAB_MAX_DEPTH)
    {
      if (! table->contents.tables)
	make_sub_tables (table, managedp);
      i = SUB_IDX (depth, from);
      table = table->contents.tables + i;
      while (i < chartab_slots[depth] && TABLE_MIN_CHAR (table) <= to)
	{
	  set_chartable_range (table, from, to, val, managedp);
	  table++, i++;
	}
    }
  else
    {
      int idx_from = SUB_IDX (depth, from);
      int idx_to = SUB_IDX (depth, to);

      if (! table->contents.values)
	make_sub_values (table, managedp);
      for (i = idx_from; i <= idx_to; i++)
	{
	  if (managedp && table->contents.values[i])
	    M17N_OBJECT_UNREF (table->contents.values[i]);
	  table->contents.values[i] = val;
	}
      if (managedp && val)
	M17N_OBJECT_REF_NTIMES (val, (idx_to - idx_from + 1));
    }
}


/** Lookup the sub char-table TABLE for the character C.  If NEXT_C is
    not NULL, set *NEXT_C to the next interesting character to lookup
    for.  If DEFAULT_P is zero, the next interesting character is what
    possibly has the different value than C.  Otherwise, the next
    interesting character is what possibly has the default value (if C
    has a value deferent from the default value) or has a value
    different from the default value (if C has the default value).  */

static void *
lookup_chartable (MSubCharTable *table, int c, int *next_c, int default_p)
{
  int depth = TABLE_DEPTH (table);
  void *val;
  void *default_value = table->default_value;
  int idx;

  while (1)
    {
      if (! table->contents.tables)
	{
	  if (next_c)
	    *next_c = TABLE_MIN_CHAR (table) + chartab_chars[depth];
	  return table->default_value;
	}
      if (depth == CHAR_TAB_MAX_DEPTH)
	break;
      table = table->contents.tables + SUB_IDX (depth, c);
      depth++;
    }

  idx = SUB_IDX (depth, c);
  val = table->contents.values[idx];

  if (next_c)
    {
      int max_char = TABLE_MIN_CHAR (table) + (chartab_chars[depth] - 1);

      if (max_char < 0 || max_char > MCHAR_MAX)
	max_char = MCHAR_MAX;
      if (default_p && val != default_value)
	{
	  do { c++, idx++; }
	  while (c >= 0 && c <= max_char
		 && table->contents.values[idx] != default_value);
	}
      else
	{
	  do { c++, idx++; }
	  while (c >= 0 && c <= max_char
		 && table->contents.values[idx] == val);
	}
      *next_c = c;
    }
  return val;
}

/** Call FUNC for characters in sub char-table TABLE.  Ignore such
    characters that has a value IGNORE.  FUNC is called with four
    arguments; FROM, TO, VAL, and ARG (same as FUNC_ARG).  If
    DEFAULT_P is zero, FROM and TO are range of characters that has
    the same value VAL.  Otherwise, FROM and TO are range of
    characters that has the different value than the default value of
    TABLE.  */

static void
map_chartable (MSubCharTable *table, void *ignore, int default_p,
		void (*func) (int, int, void *, void *),
		void *func_arg)
{
  void *current;
  int from = 0;
  int c, next_c;

  current = lookup_chartable (table, 0, &next_c, default_p);
  c = next_c;
  while (c >= 0 && c <= MCHAR_MAX)
    {
      void *next = lookup_chartable (table, c, &next_c, default_p);

      if (current != next)
	{
	  if (current != ignore)
	    (*func) (from, c - 1, current, func_arg);
	  current = next;
	  from = c;
	}
      c = next_c;
    }
  if (from <= MCHAR_MAX && current != ignore)
    (*func) (from, MCHAR_MAX, current, func_arg);
}


/* Return the smallest character whose value is not DEFAULT_VALUE in
   TABLE.  If all characters in TABLE have DEFAULT_VALUE, return
   -1.  */

static int
chartab_min_non_default_char (MSubCharTable *table, void *default_value)
{
  int depth = TABLE_DEPTH (table);
  int slots;
  int i, c;

  if (!table->contents.tables)
    return (default_value == table->default_value
	    ? -1 : TABLE_MIN_CHAR (table));

  slots = chartab_slots[depth];

  if (depth == CHAR_TAB_MAX_DEPTH)
    {
      for (i = 0; i < slots; i++)
	if (table->contents.values[i] != default_value)
	  return (TABLE_MIN_CHAR (table) + i);
    }
  else
    {
      for (i = 0; i < slots; i++)
	if ((c = chartab_min_non_default_char (table->contents.tables + i,
					       default_value))
	    >= 0)
	  return c;
    }
  return -1;
}


/* Return the largest character whose value is not DEFAULT_VALUE in
   TABLE.  If all characters in TABLE have DEFAULT_VALUE, return
   -1.  */

static int
chartab_max_non_default_char (MSubCharTable *table, void *default_value)
{
  int depth = TABLE_DEPTH (table);
  int slots;
  int i, c;

  if (!table->contents.tables)
    return (default_value == table->default_value
	    ? -1 : TABLE_MIN_CHAR (table) + chartab_chars[depth] - 1);

  slots = chartab_slots[depth];

  if (depth == CHAR_TAB_MAX_DEPTH)
    {
      for (i = slots - 1; i >= 0; i--)
	if (table->contents.values[i] != default_value)
	  return (TABLE_MIN_CHAR (table) + i);
    }
  else
    {
      for (i = slots - 1; i >= 0; i--)
	if ((c = chartab_max_non_default_char (table->contents.tables + i,
					       default_value))
	    >= 0)
	  return c;
    }
  return -1;
}

static void
free_chartable (void *object)
{
  MCharTable *table = (MCharTable *) object;
  int managedp = table->key != Mnil && table->key->managing_key;

  if (table->subtable.contents.tables)
    {
      int i;

      for (i = 0; i < chartab_slots[0]; i++)
	free_sub_tables (table->subtable.contents.tables + i, managedp);
      free (table->subtable.contents.tables);
      if (managedp && table->subtable.default_value)
	M17N_OBJECT_UNREF (table->subtable.default_value);
    }
  M17N_OBJECT_UNREGISTER (chartable_table, table);
  free (object);
}

#include <stdio.h>

/* Support function of mdebug_dump_chartab.  */

static void
dump_sub_chartab (MSubCharTable *table, void *default_value,
		  MSymbol key, int indent)
{
  int depth = TABLE_DEPTH (table);
  int min_char = TABLE_MIN_CHAR (table);
  int max_char = min_char + (chartab_chars[depth] - 1);
  char *prefix = (char *) alloca (indent + 1);
  int i;

  if (max_char < 0 || max_char > MCHAR_MAX)
    max_char = MCHAR_MAX;

  memset (prefix, 32, indent);
  prefix[indent] = 0;

  if (! table->contents.tables && table->default_value == default_value)
    return;
  fprintf (mdebug__output, "\n%s(sub%d (U+%04X U+%04X) ",
	   prefix, depth, min_char, max_char);
  if (key == Msymbol)
    {
      if (table->default_value)
	fprintf (mdebug__output, "(default %s)",
		 ((MSymbol) table->default_value)->name);
      else
	fprintf (mdebug__output, "(default nil)");
    }
  else
    fprintf (mdebug__output, "(default #x%X)", (unsigned) table->default_value);

  default_value = table->default_value;
  if (table->contents.tables)
    {
      if (depth < CHAR_TAB_MAX_DEPTH)
	for (i = 0; i < chartab_slots[depth]; i++)
	  dump_sub_chartab (table->contents.tables + i, default_value,
			    key, indent  + 2);
      else
	for (i = 0; i < chartab_slots[depth]; i++, min_char++)
	  {
	    void **val = table->contents.values + i;

	    if (val == default_value)
	      continue;
	    default_value = *val;
	    fprintf (mdebug__output, "\n%s  (U+%04X", prefix, min_char);
	    while (i + 1 < chartab_slots[depth]
		   && val[1] == default_value)
	      i++, val++, min_char++;
	    fprintf (mdebug__output, "-U+%04X ", min_char);
	    if (key == Msymbol)
	      {
		if (default_value)
		  fprintf (mdebug__output, "%s)",
			   ((MSymbol) default_value)->name);
		else
		  fprintf (mdebug__output, "nil)");
	      }
	    else
	      fprintf (mdebug__output, " #xx%X)", (unsigned) default_value);
	  }
    }
  fprintf (mdebug__output, ")");
}


/* Internal API */

int
mchartable__init ()
{
  M17N_OBJECT_ADD_ARRAY (chartable_table, "Chartable");
  return 0;
}

void
mchartable__fini ()
{
}

void *
mchartable__lookup (MCharTable *table, int c, int *next_c, int default_p)
{
  return lookup_chartable (&table->subtable, c, next_c, default_p);
}

/*** @} */
#endif /* !FOR_DOXYGEN || DOXYGEN_INTERNAL_MODULE */


/* External API */

/*** @addtogroup m17nChartable */
/*** @{ */
/*=*/

/***en
    @brief Symbol whose name is "char-table".

    The symbol @c Mchar_table has the name <tt>"char-table"</tt>.  */

/***ja
    @brief "char-table" �Ȥ���̾������ĥ���ܥ�.

    ����ܥ� @c Mchar_table ��̾�� <tt>"char-table"</tt> ����ġ�
    */

MSymbol Mchar_table;

/*=*/

/***en
    @brief Create a new chartable.

    The mchartable () function creates a new chartable object with
    symbol $KEY and the default value $DEFAULT_VALUE.  If $KEY is a
    managing key, the elements of the table (including the default
    value) are managed objects or NULL.

    @return
    If the operation was successful, mchartable () returns a pointer
    to the created chartable.  Otherwise it returns @c NULL and
    assigns an error code to the external variable #merror_code.  */

/***ja
    @brief ������ʸ���ơ��֥����.

    �ؿ� mchartable () �ϥ����� $KEY �����ǤΥǥե�����ͤ� 
    $DEFAULT_VALUE �Ǥ��뿷����ʸ���ơ��֥���롣�⤷ $KEY 
    �����������Ǥ���С����Υơ��֥�����Ǥϡʥǥե�����ͤ�ޤ�ơ˴��������֥������Ȥ� 
    NULL �Τ����줫�Ǥ��롣

    @return
    ��������������� mchartable () �Ϻ������줿ʸ���ơ��֥�ؤΥݥ��󥿤��֤���
    ���Ԥ������� @c NULL ���֤��������ѿ� #merror_code �˥��顼�����ɤ����ꤹ�롣  */

MCharTable *
mchartable (MSymbol key, void *default_value)
{
  MCharTable *table;

  M17N_OBJECT (table, free_chartable, MERROR_CHARTABLE);
  M17N_OBJECT_REGISTER (chartable_table, table);
  table->key = key;
  table->min_char = -1;
  table->max_char = -1;
  SET_DEPTH_MIN_CHAR (&table->subtable, 0, 0);
  table->subtable.default_value = default_value;
  if (key != Mnil && key->managing_key && default_value)
    M17N_OBJECT_REF (default_value);
  table->subtable.contents.tables = NULL;
  return table;
}

/*=*/

/***en
    @brief Return the minimum character whose value is set in a chartabe.

    The mchartable_min_char () function return the minimum character
    whose value is set in chartable $TABLE.  No character is set its
    value, the function returns -1.
 */

int
mchartable_min_char (MCharTable *table)
{
  return table->min_char;
}

/*=*/

/***en
    @brief Return the maximum character whose value is set in a chartabe.

    The mchartable_max_char () function return the maximum character
    whose value is set in chartable $TABLE.  No character is set its
    value, the function returns -1.
 */

int
mchartable_max_char (MCharTable *table)
{
  return table->max_char;
}

/*=*/

/***en
    @brief Return the assigned value of a character in a chartable.

    The mchartable_lookup () function returns the value assigned to
    character $C in chartable $TABLE.  If no value has been set for $C
    explicitly, the default value of $TABLE is returned.  If $C is not
    a valid character, mchartable_lookup () returns @c NULL and
    assigns an error code to the external variable #merror_code.  */

/***ja
    @brief ʸ���ơ��֥����ʸ���˳�����Ƥ�줿�ͤ��֤�.

    �ؿ� mchartable_lookup () ��ʸ���ơ��֥� $TABLE ���ʸ�� $C 
    �˳�����Ƥ�줿�ͤ��֤���$C ���Ф�������Ū���ͤ��ʤ���С�$TABLE 
    �Υǥե�����ͤ��֤���$C ��������ʸ���Ǥʤ���С�mchartable_lookup () �� 
    @c NULL ���֤��������ѿ� #merror_code �˥��顼�����ɤ����ꤹ�롣  */

/***
    @errors
    @c MERROR_CHAR

    @seealso
    mchartable_set ()  */

void *
mchartable_lookup (MCharTable *table, int c)
{
  M_CHECK_CHAR (c, NULL);

  if (c < table->min_char || c > table->max_char)
    return table->subtable.default_value;
  return lookup_chartable (&table->subtable, c, NULL, 0);
}

/*=*/

/***en
    @brief Assign a value to a character in a chartable.

    The mchartable_set () function sets the value of character $C in
    chartable $TABLE to $VAL.

    @return
    If the operation was successful, mchartable_set () returns 0.
    Otherwise it returns -1 and assigns an error code to the external
    variable #merror_code.  */

/***ja
    @brief ʸ���ơ��֥���Ǥ�ʸ�����ͤ����ꤹ��.

    �ؿ� mchartable_set () �ϡ�ʸ���ơ��֥� $TABLE ���ʸ�� $C 
    ���� $VAL �������Ƥ롣

    @return
    ��������������С�mchartable_set () �� 0 ���֤��������Ǥʤ���� -1 
    ���֤��������ѿ� #merror_code �˥��顼�����ɤ����ꤹ�롣  */

/***
    @errors
    @c MERROR_CHAR

    @seealso
    mchartable_lookup (), mchartable_set_range ()  */


int
mchartable_set (MCharTable *table, int c, void *val)
{
  int managedp = table->key != Mnil && table->key->managing_key;
  MSubCharTable *sub = &table->subtable;
  int i;

  M_CHECK_CHAR (c, -1);

  if (table->max_char < 0)
    table->min_char = table->max_char = c;
  else
    {
      if (c < table->min_char)
	table->min_char = c;
      else if (c > table->max_char)
	table->max_char = c;
    }

  for (i = 0; i < CHAR_TAB_MAX_DEPTH; i++)
    {
      if (! sub->contents.tables)
	{
	  if (sub->default_value == val)
	    return 0;
	  make_sub_tables (sub, managedp);
	}
      sub = sub->contents.tables + SUB_IDX (i, c);
    }
  if (! sub->contents.values)
    {
      if (sub->default_value == val)
	return 0;
      make_sub_values (sub, managedp);
    }
  sub->contents.values[SUB_IDX (3, c)] = val;
  if (managedp && val)
    M17N_OBJECT_REF (val);
  return 0;
}

/*=*/

/***en
    @brief Assign a value to the characters in the specified range.

    The mchartable_set_range () function assigns value $VAL to the
    characters from $FROM to $TO (both inclusive) in chartable $TABLE.

    @return
    If the operation was successful, mchartable_set_range () returns
    0.  Otherwise it returns -1 and assigns an error code to the
    external variable #merror_code.  If $FROM is greater than $TO,
    mchartable_set_range () returns immediately without an error.  */

/***ja
    @brief �����ϰϤ�ʸ�����ͤ����ꤹ��.

    �ؿ� mchartable_set_range () �ϡ�ʸ���ơ��֥� $TABLE ��� $FROM 
    ���� $TO �ޤǡ�ξü��ޤ�ˤ�ʸ���ˡ��ͤȤ��� $VAL �����ꤹ�롣

    @return
    ��������������� mchartable_set_range () �� 0 ���֤��������Ǥʤ���� 
    -1 ���֤��������ѿ� #merror_code �˥��顼�����ɤ����ꤹ�롣$FROM �� 
    $TO ����礭���Ȥ��ˤϡ� mchartable_set_range () 
    �ϲ��⤻�������顼�ⵯ�����ʤ���  */

/***
    @errors
    @c MERROR_CHAR

    @seealso
    mchartable_set ()  */

int
mchartable_set_range (MCharTable *table, int from, int to, void *val)
{
  int managedp = table->key != Mnil && table->key->managing_key;

  M_CHECK_CHAR (from, -1);
  M_CHECK_CHAR (to, -1);

  if (from > to)
    return 0;

  if (table->max_char < 0)
    table->min_char = from, table->max_char = to;
  else{
    if (from < table->min_char)
      table->min_char = from;
    if (to > table->max_char)
      table->max_char = to;
  }
  set_chartable_range (&table->subtable, from, to, val, managedp);
  return 0;
}

/*=*/

/***en
    @brief Search for characters that have non-default value.

    The mchartable_range () function searches chartable $TABLE for the
    first and the last character codes that do not have the default
    value of $TABLE, and set $FROM and $TO to them, respectively.  If
    all characters have the default value, both $FROM and $TO are set
    to -1.  */

/***ja
    @brief �ͤ��ǥե���ȤȰۤʤ�ʸ����õ��.

    �ؿ� mchartable_range () ��ʸ���ơ��֥� $TABLE ��ǡ�$TABLE 
    �Υǥե�����Ͱʳ����ͤ���ĺǽ�ȺǸ��ʸ����õ�������줾��� $FROM 
    �� $TO �����ꤹ�롣���٤Ƥ�ʸ�����ͤȤ��ƥǥե�����ͤ�ȤäƤ�����ˤ�
    $FROM �� $TO �� -1�����ꤹ�롣  */

void
mchartable_range (MCharTable *table, int *from, int *to)
{
  *from = chartab_min_non_default_char (&table->subtable,
					table->subtable.default_value);
  if (*from == -1)
    *to = -1;
  else
    *to = chartab_max_non_default_char (&table->subtable,
					table->subtable.default_value);
}

/*=*/

/***en
    @brief Call a function for characters in a chartable.

    The mchartable_map () function calls function $FUNC for characters
    in chartable $TABLE.  No function call occurs for characters that
    have value $IGNORE in $TABLE.  Comparison of $IGNORE and character
    value is done with the operator @c ==.  Be careful when you use
    string literals or pointers.

    Instead of calling $FUNC for each character, mchartable_map ()
    tries to optimize the number of function calls, i.e. it makes a
    single function call for a chunk of characters when those
    consecutive characters have the same value.

    No matter how long the character chunk is, $FUNC is called with
    four arguments; $FROM, $TO, $VAL, and $ARG.  $FROM and $TO (both
    inclusive) defines the range of characters that have value $VAL.
    $ARG is the same as $FUNC_ARG.

    @return
    This function always returns 0.  */

/***ja
    @brief ʸ���ơ��֥����ʸ�����Ф��ƻ���δؿ���Ƥ�.

    �ؿ� mchartable_map () �ϡ�ʸ���ơ��֥� $TABLE ���ʸ�����Ф��ƴؿ�
    $FUNC ��Ƥ֡�������$TABLE ��Ǥ��ͤ� $IGNORE 
    �Ǥ���ʸ���ˤĤ��Ƥϴؿ��ƤӽФ���Ԥʤ�ʤ���$IGNORE ��ʸ�����ͤ���Ӥ� 
    @c == �ǹԤʤ��Τǡ�ʸ�����ƥ���ݥ��󥿤�Ȥ��ݤˤ���դ��פ��롣

    mchartable_map () �ϡ���ʸ�����Ȥ� $FUNC 
    ��Ƥ֤ΤǤϤʤ����ؿ��ƤӽФ��β�����Ŭ�����褦�Ȥ��롣
    ���ʤ����Ϣ³����ʸ����Ʊ���ͤ���äƤ������ˤϡ�����ʸ���ΤޤȤޤ����ΤˤĤ��ư��٤δؿ��Ƥӽ�
    �������Ԥʤ�ʤ���

    ʸ���ΤޤȤޤ���礭���ˤ�����餺��$FUNC �� $FROM, $TO, $VAL, $ARG 
    �Σ������ǸƤФ�롣$FROM �� $TO ��ξü��ޤ�ˤ� $VAL 
    ���ͤȤ��ƻ���ʸ�����ϰϤ򼨤���$ARG �� $FUNC_ARG ���Τ�ΤǤ��롣

    @return
    ���δؿ��Ͼ��0���֤���  */

int
mchartable_map (MCharTable *table, void *ignore,
		void (*func) (int, int, void *, void *),
		void *func_arg)
{
  map_chartable (&table->subtable, ignore, 0, func, func_arg);
  return 0;
}

/*=*/

/*** @} */

/*** @addtogroup m17nDebug */
/*=*/
/*** @{  */

/***en
    @brief Dump a chartable.

    The mdebug_dump_chartab () function prints a chartable $TABLE in a
    human readable way to the stderr or to what specified by the
    environment variable MDEBUG_OUTPUT_FILE.  $INDENT specifies how
    many columns to indent the lines but the first one.

    @return
    This function returns $TABLE.  */

/***ja
    @brief ʸ���ơ��֥�����פ���.

    �ؿ� mdebug_dump_chartab () ��ʸ���ơ��֥� $TABLE ��ɸ�२�顼����
    �⤷���ϴĶ��ѿ� MDEBUG_DUMP_FONT �ǻ��ꤵ�줿�ե�����˿ʹ֤˲���
    �ʷ��ǰ������롣$INDENT �ϣ����ܰʹߤΥ���ǥ�Ȥ���ꤹ�롣

    @return
    ���δؿ��� $TABLE ���֤���  */

MCharTable *
mdebug_dump_chartab (MCharTable *table, int indent)
{
  fprintf (mdebug__output, "(chartab (U+%04X U+%04X)",
	   table->min_char, table->max_char);
  dump_sub_chartab (&table->subtable, table->subtable.default_value,
		    table->key, indent + 2);
  fprintf (mdebug__output, ")");
  return table;
}

/*** @} */

/*
  Local Variables:
  coding: euc-japan
  End:
*/
