/* symbol.c -- symbol module.
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
    @addtogroup m17nSymbol

    @brief Symbol objects and API for them.

    The m17n library uses objects called @e symbols as unambiguous
    identifiers.  Symbols are similar to atoms in the X library, but a
    symbol can have zero or more @e symbol @e properties.  A symbol
    property consists of a @e key and a @e value, where key is also a
    symbol and value is anything that can be cast to <tt>(void *)</tt>.  
    "The symbol property that belongs to the symbol S and
    whose key is K" may be shortened to "K property of S".

    Symbols are used mainly in the following three ways.

    @li As keys of symbol properties and other properties.

    @li To represent various objects, e.g. charsets, coding systems,
    fontsets.

    @li As arguments of the m17n library functions to control
    their behavior.

    There is a special kind of symbol, a @e managing @e key.  The
    value of a property whose key is a managing key must be a @e
    managed @e object.  See @ref m17nObject for the detail.
*/
/***ja
    @addtogroup m17nSymbol ����ܥ�

    @brief ����ܥ륪�֥������ȤȤ���˴ؤ��� API.

    m17n �饤�֥��ϰ�դ˷�ޤ뼱�̻ҤȤ��� @e ����ܥ� 
    �ȸƤ֥��֥������Ȥ��Ѥ��롣����ܥ�� X �饤�֥��Υ��ȥ�Ȼ��Ƥ��뤬��
    0 �İʾ�� @e ����ܥ�ץ�ѥƥ� ����Ĥ��Ȥ��Ǥ��롣����ܥ�ץ�ѥƥ���
    @e ���� �� @e �� ����ʤ롣�����Ϥ��켫�Υ���ܥ�Ǥ��ꡢ�ͤ� 
    <tt>(void *)</tt> ���˥��㥹�ȤǤ����Τʤ鲿�Ǥ�褤���֥���ܥ� 
    S �����ĥ���ܥ�ץ�ѥƥ��Τ��������� K �Τ�Ρפ��ñ�ˡ�S �� K 
    �ץ�ѥƥ��פȸƤ֤��Ȥ����롣

    ����ܥ�����Ӥϼ�˰ʲ���3�̤�Ǥ��롣

    @li ����ܥ�ץ�ѥƥ������¾�Υץ�ѥƥ��Υ�����ɽ����

    @li ʸ�����åȡ������ɷϡ��ե���ȥ��åȤʤɤγƼ索�֥������Ȥ�ɽ����

    @li m17n �饤�֥��ؿ��ΰ����Ȥʤꡢ�ؿ��ε�ư�����椹�롣

    @e �������� �ȸƤФ�����̤ʥ���ܥ뤬���ꡢ���������򥭡��Ȥ��ƻ��ĥץ�ѥƥ����ͤ�
    @e ���������֥������� �Ǥʤ��ƤϤʤ�ʤ����ܺ٤� @ref m17nObject ���ȡ�
*/

/*=*/

#if !defined (FOR_DOXYGEN) || defined (DOXYGEN_INTERNAL_MODULE)
/*** @addtogroup m17nInternal
     @{ */

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "m17n.h"
#include "m17n-misc.h"
#include "internal.h"
#include "symbol.h"
#include "character.h"
#include "mtext.h"
#include "plist.h"

static int num_symbols;

#define SYMBOL_TABLE_SIZE 1024

static MSymbol symbol_table[SYMBOL_TABLE_SIZE];

static unsigned
hash_string (const char *str, int len)
{
  unsigned hash = 0;
  const char *end = str + len;
  unsigned c;

  while (str < end)
    {
      c = *((unsigned char *) str++);
      if (c >= 0140)
	c -= 40;
      hash = ((hash << 3) + (hash >> 28) + c);
    }
  return hash & (SYMBOL_TABLE_SIZE - 1);
}


static MPlist *
serialize_symbol (void *val)
{
  MPlist *plist = mplist ();

  mplist_add (plist, Msymbol, val);
  return plist;
}

static void *
deserialize_symbol (MPlist *plist)
{
  return (MPLIST_SYMBOL_P (plist) ? MPLIST_SYMBOL (plist) : Mnil);
}


/* Internal API */

int
msymbol__init ()
{
  num_symbols = 0;
  Mnil = (MSymbol) 0;
  Mt = msymbol ("t");
  Msymbol = msymbol ("symbol");
  Mstring = msymbol ("string");
  return 0;
}

void
msymbol__fini ()
{
  int i;
  MSymbol sym;

  for (i = 0; i < SYMBOL_TABLE_SIZE; i++)
    for (sym = symbol_table[i]; sym; sym = sym->next)
      if (! MPLIST_TAIL_P (&sym->plist))
	{
	  if (sym->plist.key->managing_key)
	    M17N_OBJECT_UNREF (MPLIST_VAL (&sym->plist));
	  M17N_OBJECT_UNREF (sym->plist.next);
	  sym->plist.key = Mnil;
	}
}

void
msymbol__free_table ()
{
  int i;
  MSymbol sym, next;
  int freed_symbols = 0;

  for (i = 0; i < SYMBOL_TABLE_SIZE; i++)
    {
      for (sym = symbol_table[i]; sym; sym = next)
	{
	  next = sym->next;
	  free (sym->name);
	  free (sym);
	  freed_symbols++;
	}
      symbol_table[i] = NULL;
    }
  if (mdebug__flags[MDEBUG_FINI])
    fprintf (mdebug__output, "%16s %7d %7d %7d\n", "Symbol",
	     num_symbols, freed_symbols, num_symbols - freed_symbols);
  num_symbols = 0;
}


MSymbol
msymbol__with_len (const char *name, int len)
{
  char *p = alloca (len + 1);

  memcpy (p, name, len);
  p[len] = '\0';
  return msymbol (p);
}

/** Return a plist of symbols that has non-NULL property PROP.  If
    PROP is Mnil, return a plist of all symbols.  Values of the plist
    is NULL.  */

MPlist *
msymbol__list (MSymbol prop)
{
  MPlist *plist = mplist ();
  int i;
  MSymbol sym;

  for (i = 0; i < SYMBOL_TABLE_SIZE; i++)
    for (sym = symbol_table[i]; sym; sym = sym->next)
      if (prop == Mnil || msymbol_get (sym, prop))
	mplist_push (plist, sym, NULL);
  return plist;
}


/** Canonicalize the name of SYM, and return a symbol of the
    canonicalized name.  Canonicalization is done by this rule:
	o convert all uppercase characters to lowercase.
	o remove all non alpha-numeric characters.
	o change the leading "ibm" to "cp".
	o change the leading "windows-" to "cp".
	o change the leading "cp" to "ibm".
	o remove the leading "iso".
    For instance:
	"ISO-8859-2" -> "88592"
	"euc-JP" -> "eucjp"
	"IBM851" -> "cp851"
	"windows-1250" -> "cp250"

    This function is used to canonicalize charset and coding system
    names.  */

MSymbol
msymbol__canonicalize (MSymbol sym)
{
  char *name = sym->name;
  /* Extra 2 bytes are for changing "cpXXX" to "ibmXXX" and
     terminating '\0'.  */
  char *canon = (char *) alloca (strlen (name) + 2);
  char *p = canon;

  for (; *name; name++)
    if (ISALNUM (*name))
      *p++ = TOLOWER (*name);
  *p = '\0';
  if (p - canon > 3 && canon[0] == 'i')
    {
      if (canon[1] == 'b' && canon[2] == 'm' && isdigit (canon[3]))
	{
	  /* Change "ibmXXX" to "cpXXX".  */
	  canon++;
	  canon[0] = 'c';
	  canon[1] = 'p';
	}
      else if (canon[1] == 's' && canon[2] == 'o')
	{
	  /* Change "isoXXX" to "XXX".  */
	  canon += 3;
	}
    }
  else if (p - canon > 2
	   && canon[0] == 'c' && canon[1] == 'p' && isdigit (canon[2]))
    {
      /* Change "cpXXX" to "ibmXXX".  */
      for (; p >= canon + 2; p--)
	p[1] = p[0];
      canon[0] = 'i';
      canon[1] = 'b';
      canon[2] = 'm';
    }
  else if (canon[0] == 'w' && p - canon > 7
	   && memcmp (canon + 1, "indows", 6) == 0
	   && isdigit (canon[7]))
    {
      /* Change "windowsXXX" to "cpXXX" */
      canon += 5;
      canon[0] = 'c';
      canon[1] = 'p';
    }
  return msymbol (canon);
}

MTextPropSerializeFunc msymbol__serializer = serialize_symbol;
MTextPropDeserializeFunc msymbol__deserializer = deserialize_symbol;

/*** @} */
#endif /* !FOR_DOXYGEN || DOXYGEN_INTERNAL_MODULE */


/* External API */

/*** @addtogroup m17nSymbol */
/*** @{ */

/*=*/
/***en
    @brief Symbol whose name is "nil".

    The symbol #Mnil has the name <tt>"nil"</tt> and, in general,
    represents @e false or @e no.  When coerced to "int", its value is
    zero.  #Mnil can't have any symbol property.  */
/***ja
    @brief "nil" ��̾���Ȥ��ƻ��ĥ���ܥ�.

    ����ܥ� #Mnil �� <tt>"nil"</tt> 
    �Ȥ���̾������������̤ˡֵ��פޤ��ϡ�����פ��̣���롣
    "int" ���Ѵ����줿��硢�ͤ� 0 �Ǥ��롣
    #Mnil ���ȤϤ����ʤ륷��ܥ�ץ�ѥƥ�������ʤ���  */

MSymbol Mnil;

/*=*/

/***en
    @brief Symbol whose name is "t".

    The symbol #Mt has the name <tt>"t"</tt> and, in general,
    represents @e true or @e yes.  */
/***ja
    @brief "t" ��̾���Ȥ��ƻ��ĥ���ܥ�.

    ����ܥ� #Mt �� <tt>"t"</tt> �Ȥ���̾������������̤ˡֿ��פޤ��ϡֹ���פ��̣���롣  */

MSymbol Mt;

/*=*/

/***en
    @brief Symbol whose name is "string".

    The symbol #Mstring has the name <tt>"string"</tt> and is used
    as an argument of the functions mchar_define_property (),
    etc.  */
/***ja
    @brief "string" ��̾���Ȥ��ƻ��ĥ���ܥ�.

    ����ܥ� #Mstring �� <tt>"string"</tt> �Ȥ���̾����������ؿ� 
    mchar_define_property () �ʤɤΰ����Ȥ����Ѥ����롣  */

MSymbol Mstring;

/*=*/

/***en
    @brief Symbol whose name is "symbol".

    The symbol #Msymbol has the name <tt>"symbol"</tt> and is used
    as an argument of the functions mchar_define_property (),
    etc.  */
/***ja
    @brief "symbol" ��̾���Ȥ��ƻ��ĥ���ܥ�.

    ����Ѥߥ���ܥ� #Msymbol �� <tt>"symbol"</tt> �Ȥ���̾����������ؿ�
    mchar_define_property () �ʤɤΰ����Ȥ��ƻȤ��롣  */

MSymbol Msymbol;

/*=*/

/***en
    @brief Get a symbol.

    The msymbol () function returns the canonical symbol whose name is
    $NAME.  If there is none, one is created.  The created one is not
    a managing key.

    Symbols whose name starts by two spaces are reserved by the m17n
    library, and are used by the library only internally.

    @return
    This function returns the found or created symbol.

    @errors
    This function never fails.  */
/***ja
    @brief ����ܥ������.

    �ؿ� msymbol () �� $NAME 
    �Ȥ���̾����������������줿����ܥ���֤������Τ褦�ʥ���ܥ뤬¸�ߤ��ʤ����ˤϡ��������롣�������줿����ܥ�ϴ��������ǤϤʤ���

    ����ʸ����ĤǻϤޤ륷��ܥ�� m17n �饤�֥���ѤǤ��ꡢ����Ū�ˤΤ��Ѥ����롣

    @return
    ���δؿ��ϸ��Ĥ�����������������������ܥ���֤���

    @errors
    ���δؿ��Ϸ褷�Ƽ��Ԥ��ʤ���

    @latexonly \IPAlabel{msymbol} @endlatexonly  */

/***
    @seealso
    msymbol_as_managing_key (), msymbol_name (), msymbol_exist ()  */

MSymbol
msymbol (const char *name)
{
  MSymbol sym;
  int len;
  unsigned hash;

  len = strlen (name);
  if (len == 3 && name[0] == 'n' && name[1] == 'i' && name[2] == 'l')
    return Mnil;
  hash = hash_string (name, len);
  len++;
  for (sym = symbol_table[hash]; sym; sym = sym->next)
    if (len == sym->length
	&& *name == *(sym->name)
	&& ! memcmp (name, sym->name, len))
      return sym;

  num_symbols++;
  MTABLE_CALLOC (sym, 1, MERROR_SYMBOL);
  MTABLE_MALLOC (sym->name, len, MERROR_SYMBOL);
  memcpy (sym->name, name, len);
  sym->length = len;
  sym->next = symbol_table[hash];
  symbol_table[hash] = sym;
  return sym;
}

/***en
    @brief Create a managing key.

    The msymbol_as_managing_key () function returns a newly created
    managing key whose name is $NAME.  It there already exists a
    symbol of name $NAME, it returns #Mnil.

    Symbols whose name starts by two spaces are reserved by the m17n
    library, and are used by the library only internally.

    @return
    If the operation was successful, this function returns the created
    symbol.  Otherwise, it returns #Mnil.  */
/***ja
    @brief ������������.

    �ؿ� msymbol_as_managing_key () ��̾�� $NAME 
    ����Ŀ��������줿�����������֤������Ǥ�̾�� $NAME ����ĥ���ܥ뤬����С�
    #Mnil ���֤���

    ����ʸ����ĤǻϤޤ륷��ܥ�� m17n �饤�֥���ѤǤ��ꡢ����Ū�ˤΤ��Ѥ����롣

    @return
    ��������������С����δؿ���������������ܥ���֤��������Ǥʤ����
    #Mnil ���֤���   */

/***
    @errors
    MERROR_SYMBOL

    @seealso
    msymbol (), msymbol_exist ()  */

MSymbol
msymbol_as_managing_key (const char *name)
{
  MSymbol sym;
  int len;
  unsigned hash;

  len = strlen (name);
  if (len == 3 && name[0] == 'n' && name[1] == 'i' && name[2] == 'l')
    MERROR (MERROR_SYMBOL, Mnil);
  hash = hash_string (name, len);
  len++;
  for (sym = symbol_table[hash]; sym; sym = sym->next)
    if (len == sym->length
	&& *name == *(sym->name)
	&& ! memcmp (name, sym->name, len))
      MERROR (MERROR_SYMBOL, Mnil);

  num_symbols++;
  MTABLE_CALLOC (sym, 1, MERROR_SYMBOL);
  sym->managing_key = 1;
  MTABLE_MALLOC (sym->name, len, MERROR_SYMBOL);
  memcpy (sym->name, name, len);
  sym->length = len;
  sym->next = symbol_table[hash];
  symbol_table[hash] = sym;
  return sym;
}

/*=*/

/***en
    @brief Check if a symbol is a managing key.

    The msymbol_is_managing_key () function checks if the symbol
    $SYMBOL is a managing key or not.

    @return 
    Return 1 if the symbol is a managing key.  Otherwise,
    return 0.  */

int
msymbol_is_managing_key (MSymbol symbol)
{
  return (symbol->managing_key == 1);
}

/*=*/

/***en
    @brief Search for a symbol that has a specified name.

    The msymbol_exist () function searches for the symbol whose name
    is $NAME.

    @return
    If such a symbol exists, msymbol_exist () returns that symbol.
    Otherwise it returns the predefined symbol #Mnil.

    @errors
    This function never fails.  */
/***ja
    @brief ���ꤵ�줿̾������ĥ���ܥ��õ��.

    �ؿ� msymbol_exist () �� $NAME �Ȥ���̾������ĥ���ܥ��õ����

    @return
    �⤷���Τ褦�ʥ���ܥ뤬¸�ߤ���ʤ�Ф��Υ���ܥ���֤��������Ǥʤ���С�����Ѥߥ���ܥ�
    #Mnil ���֤���  

    @errors
    ���δؿ��Ϸ褷�Ƽ��Ԥ��ʤ���     */

/***@seealso
    msymbol_name (), msymbol ()  */

MSymbol
msymbol_exist (const char *name)
{
  MSymbol sym;
  int len;
  unsigned hash;

  len = strlen (name);
  if (len == 3 && name[0] == 'n' && name[1] == 'i' && name[2] == 'l')
    return Mnil;
  hash = hash_string (name, len);
  len++;
  for (sym = symbol_table[hash]; sym; sym = sym->next)
    if (len == sym->length
	&& *name == *(sym->name)
	&& ! memcmp (name, sym->name, len))
      return sym;
  return Mnil;
}

/*=*/

/***en
    @brief Get symbol name.

    The msymbol_name () function returns a pointer to a string
    containing the name of $SYMBOL.

    @errors
    This function never fails.  */
/***ja
    @brief ����ܥ��̾��������.

    �ؿ� msymbol_name () �ϻ��ꤵ�줿����ܥ� $SYMBOL 
    ��̾����ޤ�ʸ����ؤΥݥ��󥿤��֤���

    @errors
    ���δؿ��Ϸ褷�Ƽ��Ԥ��ʤ���     */

/***@seealso
    msymbol (), msymbol_exist ()  */

char *
msymbol_name (MSymbol symbol)
{
  return (symbol == Mnil ? "nil" : symbol->name);
}

/*=*/
/***en
    @brief Set the value of a symbol property.

    The msymbol_put () function assigns $VAL to the value of the
    symbol property that belongs to $SYMBOL and whose key is $KEY.  If
    the symbol property already has a value, $VAL overwrites the old
    one.  Both $SYMBOL and $KEY must not be #Mnil.

    If $KEY is a managing key, $VAL must be a managed object.  In this
    case, the reference count of the old value, if not @c NULL, is
    decremented by one, and that of $VAL is incremented by one.

    @return
    If the operation was successful, msymbol_put () returns 0.
    Otherwise it returns -1 and assigns an error code to the external
    variable #merror_code.  */
/***ja
    @brief ����ܥ�ץ�ѥƥ����ͤ����ꤹ��.

    �ؿ� msymbol_put () �ϡ�����ܥ� $SYMBOL ��ǥ����� $KEY �Ǥ��륷��ܥ�ץ�ѥƥ����ͤ�
    $VAL �����ꤹ�롣���Υ���ܥ�ץ�ѥƥ��ˤ��Ǥ��ͤ�����о�񤭤��롣
    $SYMBOL, $KEY �Ȥ� #Mnil �Ǥ��äƤϤʤ�ʤ���

    $KEY �����������ʤ�С�$VAL �ϴ��������֥������ȤǤʤ��ƤϤʤ�ʤ������ξ�硢�Ť��ͤλ��ȿ���
    @c NULL �Ǥʤ���� 1 ���餵�졢$VAL �λ��ȿ��� 1 ���䤵��롣

    @return
    ��������������С�msymbol_put () �� 0 ���֤��������Ǥʤ���� -1 
    ���֤��������ѿ� #merror_code �˥��顼�����ɤ����ꤹ�롣  */

/***
    @errors
    @c MERROR_SYMBOL

    @seealso
    msymbol_get () */

int
msymbol_put (MSymbol symbol, MSymbol key, void *val)
{
  if (symbol == Mnil || key == Mnil)
    MERROR (MERROR_SYMBOL, -1);
  mplist_put (&symbol->plist, key, val);
  return 0;
}

/*=*/

/***en
    @brief Get the value of a symbol property.

    The msymbol_get () function searches for the value of the symbol
    property that belongs to $SYMBOL and whose key is $KEY.  If
    $SYMBOL has such a symbol property, its value is returned.
    Otherwise @c NULL is returned.

    @return
    If an error is detected, msymbol_get () returns @c NULL and
    assigns an error code to the external variable #merror_code.  */
/***ja
    @brief ����ܥ�ץ�ѥƥ����ͤ�����.

    �ؿ� msymbol_get () �ϡ�����ܥ� $SYMBOL 
    �����ĥ���ܥ�ץ�ѥƥ��Τ����������� $KEY 
    �Ǥ����Τ�õ�����⤷�������륷��ܥ�ץ�ѥƥ���¸�ߤ���С�������ͤ��֤��������Ǥʤ����
    @c NULL ���֤���

    @return 
    ���顼�����Ф��줿��硢msymbol_get () �� @c NULL 
    ���֤��������ѿ� #merror_code �˥��顼�����ɤ����ꤹ�롣  */

/***
    @errors
    @c MERROR_SYMBOL

    @seealso
    msymbol_put () */

void *
msymbol_get (MSymbol symbol, MSymbol key)
{
  MPlist *plist;

  if (symbol == Mnil || key == Mnil)
    return NULL;
  plist = &symbol->plist;
  MPLIST_FIND (plist, key);
  return (MPLIST_TAIL_P (plist) ? NULL : MPLIST_VAL (plist));
}

/*=*/
/***en
    @brief Set the value (function pointer) of a symbol property.

    The msymbol_put_func () function is similar to msymbol_put () but for
    setting function pointer $FUNC as the property value of $SYMBOL for
    key $KEY.  */

/***ja
    @brief ����ܥ�ץ�ѥƥ�����(�ؿ��ݥ���)�����ꤹ��.

    �ؿ� msymbol_put_func () �ϡ��ؿ� msymbol_put () ��Ʊ�ͤˡ�����ܥ�
    $SYMBOL �Υ����� $KEY �Ǥ��륷��ܥ�ץ�ѥƥ����ͤ����ꤹ�롣â��
    �����ͤϴؿ��ݥ��� $FUNC �Ǥ��롣 */

/***
    @seealso
     msymbol_put (), M17N_FUNC ()  */
int
msymbol_put_func (MSymbol symbol, MSymbol key, M17NFunc func)
{
  if (symbol == Mnil || key == Mnil)
    MERROR (MERROR_SYMBOL, -1);
  mplist_put_func (&symbol->plist, key, func);
  return 0;
}

/*=*/

/***en
    @brief Get the value (function pointer) of a symbol property.

    The msymbol_get_func () function is similar to msymbol_get () but for
    getting a function pointer form the property of symbol $SYMBOL.  */

/***ja
    @brief ����ܥ�ץ�ѥƥ����� (�ؿ��ݥ���) ������.

    �ؿ� msymbol_get_func () �ϡ��ؿ� msymbol_get () ��Ʊ�ͤˡ�����ܥ�
    $SYMBOL �����ĥ���ܥ�ץ�ѥƥ��Τ����������� $KEY �Ǥ����Τ����롣â��
    �����ͤϴؿ��ݥ��󥿤�Ǥ��롣    */

/***
    @seealso
    msymbol_get ()  */

M17NFunc
msymbol_get_func (MSymbol symbol, MSymbol key)
{
  if (symbol == Mnil || key == Mnil)
    return NULL;
  return mplist_get_func (&symbol->plist, key);
}

/*** @} */

#include <stdio.h>

/*** @addtogroup m17nDebug */
/*=*/
/*** @{ */

/***en
    @brief Dump a symbol.

    The mdebug_dump_symbol () function prints symbol $SYMBOL in a
    human readable way to the stderr or to what specified by the
    environment variable MDEBUG_OUTPUT_FILE.  $INDENT specifies how
    many columns to indent the lines but the first one.

    @return
    This function returns $SYMBOL.

    @errors
    MERROR_DEBUG  */
/***ja
    @brief ����ܥ�����פ���.

    �ؿ� mdebug_dump_symbol () �ϥ���ܥ� $symbol ��ɸ�२�顼���Ϥ⤷
    ���ϴĶ��ѿ� MDEBUG_DUMP_FONT �ǻ��ꤵ�줿�ե�����˿ʹ֤˲��ɤʷ�
    �ǰ������롣 $INDENT �ϣ����ܰʹߤΥ���ǥ�Ȥ���ꤹ�롣

    @return
    ���δؿ��� $SYMBOL ���֤��� 

    @errors
    MERROR_DEBUG  */

MSymbol
mdebug_dump_symbol (MSymbol symbol, int indent)
{
  char *prefix;
  MPlist *plist;
  char *name;

  if (indent < 0)
    MERROR (MERROR_DEBUG, Mnil);
  prefix = (char *) alloca (indent + 1);
  memset (prefix, 32, indent);
  prefix[indent] = 0;

  if (symbol == Mnil)
    plist = NULL, name = "nil";
  else
    plist = &symbol->plist, name = symbol->name;

  fprintf (mdebug__output, "%s%s", prefix, name);
  while (plist && MPLIST_KEY (plist) != Mnil)
    {
      fprintf (mdebug__output, ":%s", MPLIST_KEY (plist)->name);
      plist = MPLIST_NEXT (plist);
    }
  return symbol;
}

/***en
    @brief Dump all symbol names.

    The mdebug_dump_all_symbols () function prints names of all
    symbols to the stderr or to what specified by the environment
    variable MDEBUG_OUTPUT_FILE.  $INDENT specifies how many columns
    to indent the lines but the first one.

    @return
    This function returns #Mnil.

    @errors
    MERROR_DEBUG  */
/***ja
    @brief ���٤ƤΥ���ܥ�̾�����פ���.

    �ؿ� mdebug_dump_all_symbols () �ϡ����٤ƤΥ���ܥ��̾����ɸ�२
    �顼���Ϥ⤷���ϴĶ��ѿ� MDEBUG_DUMP_FONT �ǻ��ꤵ�줿�ե�����˰�
    �����롣 $INDENT �ϣ����ܰʹߤΥ���ǥ�Ȥ���ꤹ�롣

    @return
    ���δؿ��� #Mnil ���֤��� 

    @errors
    MERROR_DEBUG  */


MSymbol
mdebug_dump_all_symbols (int indent)
{
  char *prefix;
  int i, n;
  MSymbol sym;

  if (indent < 0)
    MERROR (MERROR_DEBUG, Mnil);
  prefix = (char *) alloca (indent + 1);
  memset (prefix, 32, indent);
  prefix[indent] = 0;

  fprintf (mdebug__output, "(symbol-list");
  for (i = n = 0; i < SYMBOL_TABLE_SIZE; i++)
    if ((sym = symbol_table[i]))
      {
	fprintf (mdebug__output, "\n%s  (%4d", prefix, i);
	for (; sym; sym = sym->next, n++)
	  fprintf (mdebug__output, " '%s'", sym->name);
	fprintf (mdebug__output, ")");
      }
  fprintf (mdebug__output, "\n%s  (total %d)", prefix, n);
  fprintf (mdebug__output, ")");
  return Mnil;
}

/*** @} */ 

/*
  Local Variables:
  coding: euc-japan
  End:
*/
