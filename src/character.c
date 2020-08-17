/* character.c -- character module.
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
    @addtogroup m17nCharacter
    @brief Character objects and API for them.

    The m17n library represents a @e character by a character code (an
    integer).  The minimum character code is @c 0.  The maximum
    character code is defined by the macro #MCHAR_MAX.  It is
    assured that #MCHAR_MAX is not smaller than @c 0x3FFFFF (22
    bits).

    Characters @c 0 to @c 0x10FFFF are equivalent to the Unicode
    characters of the same code values.

    A character can have zero or more properties called @e character
    @e properties.  A character property consists of a @e key and a
    @e value, where key is a symbol and value is anything that can be
    cast to <tt>(void *)</tt>.  "The character property that belongs
    to character C and whose key is K" may be shortened to "the K
    property of C".  */

/***ja
    @addtogroup m17nCharacter
    @brief ʸ�����֥������ȤȤ���˴ؤ��� API.

    m17n �饤�֥��� @e ʸ�� ��ʸ�������ɡ������ˤ�ɽ�����롣
    �Ǿ���ʸ�������ɤ� @c 0 �Ǥ��ꡢ�����ʸ�������ɤϥޥ��� #MCHAR_MAX 
    �ˤ�ä��������Ƥ��롣#MCHAR_MAX �� @c 0x3FFFFF��22�ӥåȡ�
    �ʾ�Ǥ��뤳�Ȥ��ݾڤ���Ƥ��롣

    @c 0 ���� @c 0x10FFFF �ޤǤ�ʸ���ϡ������Ʊ���ͤ���� Unicode 
    ��ʸ���˳�����Ƥ��Ƥ��롣

    ��ʸ���� @e ʸ���ץ�ѥƥ� �ȸƤ֥ץ�ѥƥ��� 0 �İʾ���Ĥ��Ȥ��Ǥ��롣
    ʸ���ץ�ѥƥ��� @e ���� �� @e �� ����ʤ롣
    �����ϥ���ܥ�Ǥ��ꡢ�ͤ� <tt>(void *)</tt> ���˥��㥹�ȤǤ����Τʤ鲿�Ǥ�褤�� 
    ��ʸ�� C ��ʸ���ץ�ѥƥ��Τ��������� K �Ǥ����Ρפ��ñ�ˡ�ʸ�� C 
    �� K �ץ�ѥƥ��פȸƤ֤��Ȥ����롣  */
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

#include "m17n-core.h"
#include "m17n-misc.h"
#include "internal.h"

typedef struct
{
  MSymbol type;
  void *mdb;
  MCharTable *table;
} MCharPropRecord;

static MPlist *char_prop_list;

static void
free_string (int from, int to, void *str, void *arg)
{
  free (str);
}


/* Internal API */

int
mchar__init ()
{
  Mname = msymbol ("name");
  Mcategory = msymbol ("category");
  Mcombining_class = msymbol ("combining-class");
  Mbidi_category = msymbol ("bidirectional-category");
  Msimple_case_folding = msymbol ("simple-case-folding");
  Mcomplicated_case_folding = msymbol ("complicated-case-folding");
  Mcased = msymbol ("cased");
  Msoft_dotted = msymbol ("soft-dotted");
  Mcase_mapping = msymbol ("case-mapping");
  Mblock = msymbol ("block");
  Mscript = msymbol ("script");

  return 0;
}

void
mchar__fini (void)
{
  MPlist *p;

  if (char_prop_list)
    {
      for (p = char_prop_list; mplist_key (p) != Mnil; p = mplist_next (p))
	{
	  MCharPropRecord *record = mplist_value (p);

	  if (record->table)
	    {
	      if (record->type == Mstring)
		mchartable_map (record->table, NULL, free_string, NULL);
	      M17N_OBJECT_UNREF (record->table);
	    }
	  free (record);
	}
      M17N_OBJECT_UNREF (char_prop_list);
    }
}

void
mchar__define_prop (MSymbol key, MSymbol type, void *mdb)
{
  MCharPropRecord *record;

  if (char_prop_list)
    record = mplist_get (char_prop_list, key);
  else
    char_prop_list = mplist (), record = NULL;
  if (record)
    {
      if (record->table)
	M17N_OBJECT_UNREF (record->table);
    }
  else
    {
      MSTRUCT_CALLOC (record, MERROR_CHAR);
      mplist_put (char_prop_list, key, record);
    }

  record->type = type;
  record->mdb = mdb;
  if (mdb)
    {
      record->table = NULL;
    }
  else
    {
      void *default_value = NULL;

      if (type == Minteger)
	default_value = (void *) -1;
      record->table = mchartable (type, default_value);
    }
}


/*** @} */
#endif /* !FOR_DOXYGEN || DOXYGEN_INTERNAL_MODULE */

/* External API */

/*** @addtogroup m17nCharacter
     @{ */
/*=*/

#ifdef FOR_DOXYGEN
/***en
    @brief Maximum character code.

    The macro #MCHAR_MAX gives the maximum character code.  */

/***ja
    @brief ʸ�������ɤκ�����.

    �ޥ��� #MCHAR_MAX ��ʸ�������ɤκ����ͤ�ɽ����  */

#define MCHAR_MAX
/*=*/
#endif /* FOR_DOXYGEN */

/***en
    @name Variables: Keys of character properties

    These symbols are used as keys of character properties.  */

/***ja
    @ingroup m17nCharacter
    @name �ѿ�: ʸ���ץ�ѥƥ��Υ���

    �����Υ���ܥ��ʸ���ץ�ѥƥ��Υ����Ȥ��ƻȤ��롣*/

/*=*/
/*** @{ */

/***en
    @brief Key for script.

    The symbol #Mscript has the name <tt>"script"</tt> and is used as the key
    of a character property.  The value of such a property is a symbol
    representing the script to which the character belongs.

    Each symbol that represents a script has one of the names listed in
    the <em>Unicode Technical Report #24</em>.  */

/***ja
    @brief ������ץȤ�ɽ�魯����.

    ����ܥ� #Mscript �� <tt>"script"</tt> 
    �Ȥ���̾���������ʸ���ץ�ѥƥ��Υ����Ȥ��ƻȤ��롣
    ���Υץ�ѥƥ����ͤϡ�����ʸ����°���륹����ץȤ�ɽ�魯����ܥ�Ǥ��롣

    ������ץȤ�ɽ�魯����ܥ��̾���ϡ�<em>Unicode Technical Report
    #24</em> �˥ꥹ�Ȥ���Ƥ����ΤΤ����줫�Ǥ��롣  */

MSymbol Mscript;

/*=*/

/***en
    @brief Key for character name.

    The symbol #Mname has the name <tt>"name"</tt> and is used as
    the key of a character property.  The value of such a property is a
    C-string representing the name of the character.  */

/***ja
    @brief ̾����ɽ�魯����.

    ����ܥ� #Mname �� <tt>"name"</tt> 
    �Ȥ���̾���������ʸ���ץ�ѥƥ��Υ����Ȥ��ƻȤ��롣
    ���Υץ�ѥƥ����ͤϤ���ʸ����̾����ɽ�魯 C ��ʸ����Ǥ��롣  */

MSymbol Mname;

/*=*/

/***en
    @brief Key for general category.

    The symbol #Mcategory has the name <tt>"category"</tt> and is
    used as the key of a character property.  The value of such a
    property is a symbol representing the <em>general category</em> of
    the character.

    Each symbol that represents a general category has one of the
    names listed as abbreviations for <em>General Category</em> in
    Unicode.  */

/***ja
    @brief ���̥��ƥ����ɽ�魯����.

    ����ܥ� #Mcategory �� <tt>"category"</tt> 
    �Ȥ���̾���������ʸ���ץ�ѥƥ��Υ����Ȥ��ƻȤ��롣
    ���Υץ�ѥƥ����ͤϡ��б����� <em>���̥��ƥ���</em> ��ɽ�魯����ܥ�Ǥ��롣

    ���̥��ƥ����ɽ�魯����ܥ��̾���ϡ�<em>General Category</em> 
    �ξ�ά���Ȥ��� Unicode ���������Ƥ����ΤǤ��롣  */

MSymbol Mcategory;

/*=*/

/***en
    @brief Key for canonical combining class.

    The symbol #Mcombining_class has the name
    <tt>"combining-class"</tt> and is used as the key of a character
    property.  The value of such a property is an integer that
    represents the <em>canonical combining class</em> of the character.

    The meaning of each integer that represents a canonical combining
    class is identical to the one defined in Unicode.  */

/***ja
    @brief ɸ���祯�饹��ɽ�魯����.

    ����ܥ� #Mcombining_class �� <tt>"combining-class"</tt> 
    �Ȥ���̾���������ʸ���ץ�ѥƥ��Υ����Ȥ��ƻȤ��롣
    ���Υץ�ѥƥ����ͤϡ��б����� @e ɸ���祯�饹 ��ɽ�魯�����Ǥ��롣

    ɸ���祯�饹��ɽ�魯�����ΰ�̣�ϡ�Unicode 
    ���������Ƥ����Τ�Ʊ���Ǥ��롣  */

MSymbol Mcombining_class;
/*=*/

/***en
    @brief Key for bidi category.

    The symbol #Mbidi_category has the name <tt>"bidi-category"</tt>
    and is used as the key of a character property.  The value of such
    a property is a symbol that represents the <em>bidirectional
    category</em> of the character.

    Each symbol that represents a bidirectional category has one of
    the names listed as types of <em>Bidirectional Category</em> in
    Unicode.  */

/***ja
    @brief ���������ƥ����ɽ�魯����.

    ����ܥ� #Mbidi_category �� <tt>"bidi-category"</tt> 
    �Ȥ���̾���������ʸ���ץ�ѥƥ��Υ����Ȥ��ƻȤ��롣
    ���Υץ�ѥƥ����ͤϡ��б����� @e ���������ƥ��� ��ɽ�魯����ܥ�Ǥ��롣

    ���������ƥ����ɽ�魯����ܥ��̾���ϡ�<em>Bidirectional
    Category</em> �η��Ȥ��� Unicode ���������Ƥ����ΤǤ��롣  */

MSymbol Mbidi_category;
/*=*/

/***en
    @brief Key for corresponding single lowercase character.

    The symbol #Msimple_case_folding has the name
    <tt>"simple-case-folding"</tt> and is used as the key of a
    character property.  The value of such a property is the
    corresponding single lowercase character that is used when
    comparing M-texts ignoring cases.

    If a character requires a complicated comparison (i.e. cannot be
    compared by simply mapping to another single character), the value
    of such a property is @c 0xFFFF.  In this case, the character has
    another property whose key is #Mcomplicated_case_folding.  */

/***ja
    @brief �б����뾮ʸ����ʸ����ɽ�魯����.

    ����ܥ� #Msimple_case_folding �� <tt>"simple-case-folding"</tt> 
    �Ȥ���̾���������ʸ���ץ�ѥƥ��Υ����Ȥ��ƻȤ��롣
    ���Υץ�ѥƥ����ͤϡ��б����뾮ʸ����ʸ���Ǥ��ꡢ��ʸ������ʸ���ζ��̤�̵�뤷��ʸ������Ӥκݤ˻Ȥ��롣

    ʣ���������ˡ��ɬ�פȤ���ʸ���Ǥ��ä����
    ���̤ΰ�ʸ�����б��դ��뤳�Ȥˤ�ä���ӤǤ��ʤ����ˡ����Υץ�ѥƥ����ͤ� 
    @c 0xFFFF �ˤʤ롣���ξ�礽��ʸ���ϡ�#Mcomplicated_case_folding
    �Ȥ���������ʸ���ץ�ѥƥ�����ġ�  */

MSymbol Msimple_case_folding;
/*=*/

/***en
    @brief Key for corresponding multiple lowercase characters.

    The symbol #Mcomplicated_case_folding has the name
    <tt>"complicated-case-folding"</tt> and is used as the key of a
    character property.  The value of such a property is the
    corresponding M-text that contains a sequence of lowercase
    characters to be used for comparing M-texts ignoring case.  */

/***ja
    @brief �б����뾮ʸ�������ɽ�魯����.

    ����ܥ� #Mcomplicated_case_folding �� 
    <tt>"complicated-case-folding"</tt> 
    �Ȥ���̾���������ʸ���ץ�ѥƥ��Υ����Ȥ��ƻȤ��롣
    ���Υץ�ѥƥ����ͤϡ��б����뾮ʸ���󤫤�ʤ� M-text �Ǥ��ꡢ��ʸ������ʸ���ζ��̤�̵�뤷��ʸ������Ӥκݤ˻�
    ���롣
      */

MSymbol Mcomplicated_case_folding;
/*=*/

/***en
    @brief Key for values used in case operation.

    The symbol #Mcased has the name <tt>"cased"</tt> and is used as
    the key of charater property.  The value of such a property is an
    integer value 1, 2, or 3 representing "cased", "case-ignorable",
    and both of them respective.  See the Unicode Standard 5.0
    (Section 3.13 Default Case Algorithm) for the detail.
 */

/***ja
    @brief Case �������Ѥ������ͤΥ���.

    ����ܥ� #Mcased �ϡ�<tt>"cased"</tt> �Ȥ���̾���������ʸ���ץ��
    �ƥ��Υ����Ȥ��ƻȤ��롣���Υץ�ѥƥ����ͤ������� 1, 2, 3 �Τ���
    �줫�Ǥ��ꡢ���줾�� "cased", "case-ignorable", ����ξ�����̣���롣
    �ܺ٤ˤĤ��Ƥϡ�the Unicode Standard 5.0 (Section 3.13 Default
    Case Algorithm) ���ȡ�
 */
MSymbol Mcased;

/*=*/
/***en
    @brief Key for values used in case operation.

    The symbol #Msoft_dotted has the name <tt>"soft-dotted"</tt> and
    is used as the key of charater property.  The value of such a
    property is #Mt if a character has "Soft_Dotted" property, and
    #Mnil otherwise.  See the Unicode Standard 5.0 (Section 3.13
    Default Case Algorithm) for the detail.  */

/***ja
    @brief Case �������Ѥ������ͤΥ���.

    ����ܥ� #Msoft_dotted �ϡ�<tt>"soft-dotted"</tt> �Ȥ���̾���������
    ʸ���ץ�ѥƥ��Υ����Ȥ��ƻȤ��롣���Υץ�ѥƥ����ͤϡ�ʸ����
    "Soft_Dotted"�ץ�ѥƥ�����ľ��ˤ� #Mt, �����Ǥʤ���� #Mnil ��
    ���롣 �ܺ٤ˤĤ��Ƥϡ�the Unicode Standard 5.0 (Section 3.13
    Default Case Algorithm) ���ȡ�
 */
MSymbol Msoft_dotted;

/*=*/
/***en 
    @brief Key for values used in case operation.

    The symbol #Mcase_mapping has the name <tt>"case-mapping"</tt> and
    is used as the key of charater property.  The value of such a
    property is a plist of three M-Texts; lower, title, and upper of
    the corresponding character.  See the Unicode Standard 5.0
    (Section 5.18 Case Mappings) for the detail.  */

/***ja
    @brief Case �������Ѥ������ͤΥ���.

    ����ܥ� #Mcase_mapping �ϡ�<tt>"case-mapping"</tt> �Ȥ���̾�����
    ����ʸ���ץ�ѥƥ��Υ����Ȥ��ƻȤ��롣���Υץ�ѥƥ����ͤϡ�����
    �� M-text�����ʤ������ʸ���� lower, title, �� upper����ʤ� plist
    �Ǥ��롣 �ܺ٤ˤĤ��Ƥϡ�the Unicode Standard 5.0 (Section 3.13
    Default Case Algorithm) ���ȡ�
*/
MSymbol Mcase_mapping;

/*=*/
/***en 
    @brief Key for script block name.

    The symbol #Mblock the name <tt>"block"</tt> and is used as the
    key of charater property.  The value of such a property is a
    symbol representing a script block of the corresponding
    character.  */
/***ja
    @brief ������ץȥ֥�å�̾��ɽ������.

    ����ܥ� #Mblock �ϡ�<tt>"block"</tt> �Ȥ���̾��������ʸ���ץ��
    �ƥ��Υ����Ȥ��ƻȤ��롣���Υץ�ѥƥ����ͤϡ�����ʸ���Υ������
    �ȥ֥�å�̾��ɽ������ܥ�Ǥ��롣*/
MSymbol Mblock;

/*** @} */
/*=*/

/***en
    @brief Define a character property.

    The mchar_define_property () function searches the m17n database
    for a data whose tags are \<#Mchar_table, $TYPE, $SYM \>.
    Here, $SYM is a symbol whose name is $NAME.  $TYPE must be
    #Mstring, #Mtext, #Msymbol, #Minteger, or #Mplist.

    @return
    If the operation was successful, mchar_define_property () returns
    $SYM.  Otherwise it returns #Mnil.  */

/***ja
    @brief ʸ���ץ�ѥƥ����������.

    �ؿ� mchar_define_property () �ϡ� \<#Mchar_table, $TYPE, $SYM \>
    �Ȥ�����������ä��ǡ����١����� m17n �������١�������õ����  
    ������ $SYM �� $NAME �Ȥ���̾���Υ���ܥ�Ǥ��롣$TYPE ��#Mstring,
    #Mtext, #Msymbol, #Minteger, #Mplist �Τ����줫�Ǥʤ���Фʤ�ʤ���

    @return
    ��������������� mchar_define_property () ��$SYM ���֤���
    ���Ԥ������� #Mnil ���֤���  */

/***
    @errors
    @c MERROR_DB

    @seealso
    mchar_get_prop (), mchar_put_prop ()  */

MSymbol
mchar_define_property (const char *name, MSymbol type)
{
  MSymbol key = msymbol (name);
  void *mdb;

  mdb = mdatabase_find (Mchar_table, type, key, Mnil);
  if (! mdb)
    return Mnil;
  mchar__define_prop (key, type, mdb);
  return key;
}

/*=*/

/***en
    @brief Get the value of a character property.

    The mchar_get_prop () function searches character $C for the
    character property whose key is $KEY.

    @return
    If the operation was successful, mchar_get_prop () returns the
    value of the character property.  Otherwise it returns @c
    NULL.  */

/***ja
    @brief ʸ���ץ�ѥƥ����ͤ�����.

    �ؿ� mchar_get_prop () �ϡ�ʸ�� $C ��ʸ���ץ�ѥƥ��Τ��������� 
    $KEY �Ǥ����Τ�õ����

    @return
    ��������������� mchar_get_prop () �ϸ��Ĥ��ä��ץ�ѥƥ����ͤ��֤���
    ���Ԥ������� @c NULL ���֤���

    @latexonly \IPAlabel{mchar_get_prop} @endlatexonly
*/
/***
    @errors
    @c MERROR_SYMBOL, @c MERROR_DB

    @seealso
    mchar_define_property (), mchar_put_prop ()  */

void *
mchar_get_prop (int c, MSymbol key)
{
  MCharPropRecord *record;

  if (! char_prop_list)
    return NULL;
  record = mplist_get (char_prop_list, key);
  if (! record)
    return NULL;
  if (record->mdb)
    {
      record->table = mdatabase_load (record->mdb);
      if (! record->table)
	MERROR (MERROR_DB, NULL);
      record->mdb = NULL;
    }
  return mchartable_lookup (record->table, c);
}

/*=*/

/***en
    @brief Set the value of a character property.

    The mchar_put_prop () function searches character $C for the
    character property whose key is $KEY and assigns $VAL to the value
    of the found property.

    @return
    If the operation was successful, mchar_put_prop () returns 0.
    Otherwise, it returns -1.  */
/***ja
    @brief ʸ���ץ�ѥƥ����ͤ����ꤹ��.

    �ؿ� mchar_put_prop () �ϡ�ʸ�� $C ��ʸ���ץ�ѥƥ��Τ��������� $KEY 
    �Ǥ����Τ�õ���������ͤȤ��� $VAL �����ꤹ�롣

    @return
    ��������������� mchar_put_prop () ��0���֤������Ԥ�������-1���֤���  */
/***
    @errors
    @c MERROR_SYMBOL, @c MERROR_DB

    @seealso
    mchar_define_property (), mchar_get_prop ()   */

int
mchar_put_prop (int c, MSymbol key, void *val)
{
  MCharPropRecord *record;

  if (! char_prop_list)
    MERROR (MERROR_CHAR, -1);
  record = mplist_get (char_prop_list, key);
  if (! record)
    return -1;
  if (record->mdb)
    {
      record->table = mdatabase_load (record->mdb);
      if (! record->table)
	MERROR (MERROR_DB, -1);
      record->mdb = NULL;
    }
  return mchartable_set (record->table, c, val);
}

/*=*/

/***en
    @brief Get the char-table for a character property.

    The mchar_get_prop_table () function returns a char-table that
    contains the character property whose key is $KEY.  If $TYPE is
    not NULL, this function stores the type of the property in the
    place pointed by $TYPE.  See mchar_define_property () for types of
    character property.

    @return
    If $KEY is a valid character property key, this function returns a
    char-table.  Otherwise NULL is retuned.  */

/***ja
    @brief ʸ���ץ�ѥƥ���ʸ���ơ��֥������.

    �ؿ� mchar_get_prop_table () �ϡ������� $KEY �Ǥ���ʸ���ץ�ѥƥ�
    ��ޤ�ʸ���ơ��֥���֤����⤷ $TYPE �� NULL �Ǥʤ���С� $TYPE ��
    �ؤ������ˤ���ʸ���Υץ�ѥƥ����Ǽ���롣ʸ���ץ�ѥƥ��μ���
    �˴ؤ��Ƥ� mchar_define_property () �򸫤衣

    @return
    �⤷ $KEY ��������ʸ���ץ�ѥƥ��Υ����Ǥ���С�ʸ���ơ��֥뤬�֤�
    ��롣�����Ǥʤ����� NULL ���֤���롣  */

MCharTable *
mchar_get_prop_table (MSymbol key, MSymbol *type)
{
  MCharPropRecord *record;

  if (! char_prop_list)
    return NULL;
  record = mplist_get (char_prop_list, key);
  if (! record)
    return NULL;
  if (record->mdb)
    {
      record->table = mdatabase_load (record->mdb);
      if (! record->table)
	MERROR (MERROR_DB, NULL);
      record->mdb = NULL;
    }
  if (type)
    *type = record->type;
  return record->table;
}

/*** @} */

/*
  Local Variables:
  coding: euc-japan
  End:
*/
