/* input.c -- input method module.
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
    @addtogroup m17nInputMethod
    @brief API for Input method.

    An input method is an object to enable inputting various
    characters.  An input method is identified by a pair of symbols,
    LANGUAGE and NAME.  This pair decides an input method driver of the
    input method.  An input method driver is a set of functions for
    handling the input method.  There are two kinds of input methods;
    internal one and foreign one.

    <ul>
    <li> Internal Input Method

    An internal input method has non @c Mnil LANGUAGE, and its body is
    defined in the m17n database by the tag <Minput_method, LANGUAGE,
    NAME>.  For this kind of input methods, the m17n library uses two
    predefined input method drivers, one for CUI use and the other for
    GUI use.  Those drivers utilize the input processing engine
    provided by the m17n library itself.  The m17n database may
    provide input methods that are not limited to a specific language.
    The database uses @c Mt as LANGUAGE of those input methods.

    An internal input method accepts an input key which is a symbol
    associated with an input event.  As there is no way for the @c
    m17n @c library to know how input events are represented in an
    application program, an application programmer has to convert an
    input event to an input key by himself.  See the documentation of
    the function minput_event_to_key () for the detail.

    <li> Foreign Input Method @anchor foreign-input-method

    A foreign input method has @c Mnil LANGUAGE, and its body is
    defined in an external resource (e.g. XIM of X Window System).
    For this kind of input methods, the symbol NAME must have a
    property of key #Minput_driver, and the value must be a pointer
    to an input method driver.  Therefore, by preparing a proper
    driver, any kind of input method can be treated in the framework
    of the @c m17n @c library.

    For convenience, the m17n-X library provides an input method
    driver that enables the input style of OverTheSpot for XIM, and
    stores #Minput_driver property of the symbol @c Mxim with a
    pointer to the driver.  See the documentation of m17n GUI API for
    the detail.

    </ul>

    PROCESSING FLOW

    The typical processing flow of handling an input method is: 

     @li open an input method
     @li create an input context for the input method
     @li filter an input key
     @li look up a produced text in the input context  */

/*=*/
/***ja
    @addtogroup m17nInputMethod
    @brief ���ϥ᥽�å���API.

    ���ϥ᥽�åɤ�¿�ͤ�ʸ�������Ϥ��뤿��Υ��֥������ȤǤ��롣
    ���ϥ᥽�åɤϥ���ܥ� LANGUAGE �� NAME ���Ȥˤ�äƼ��̤��졢
    �����ȹ礻�ˤ�ä����ϥ᥽�åɥɥ饤�Ф����ꤹ�롣
    ���ϥ᥽�åɥɥ饤�ФȤϡ��������ϥ᥽�åɤ򰷤�����δؿ��ν��ޤ�Ǥ��롣
    ���ϥ᥽�åɤˤ������᥽�åɤȳ����᥽�åɤ�����ब���롣

    <ul> 
    <li> �������ϥ᥽�å�

    �������ϥ᥽�åɤȤ� LANGUAGE �� @c Mnil �ʳ��Τ�ΤǤ��ꡢ��������
    ��m17n �ǡ����١�����<Minput_method, LANGUAGE, NAME> �Ȥ�����������
    �����������Ƥ��롣���μ�����ϥ᥽�åɤ��Ф��ơ�m17n �饤�֥���
    ��CUI �Ѥ� GUI �Ѥ��줾������ϥ᥽�åɥɥ饤�Ф򤢤餫�����������
    ���롣�����Υɥ饤�Ф� m17n �饤�֥�꼫�Τ����Ͻ������󥸥����
    �Ѥ��롣m17n �ǡ����١����ˤϡ�����θ������ѤǤʤ����ϥ᥽�åɤ���
    �����뤳�Ȥ�Ǥ������Τ褦�����ϥ᥽�åɤ� LANGUAGE �� @c Mt �Ǥ��롣

    �������ϥ᥽�åɤϡ��桼�������ϥ��٥�Ȥ��б���������ܥ�Ǥ�����
    �ϥ����������롣@c m17n @c �饤�֥�� �����ϥ��٥�Ȥ����ץꥱ��
    �����ץ����Ǥɤ�ɽ������Ƥ��뤫���Τ뤳�Ȥ��Ǥ��ʤ��Τǡ���
    �ϥ��٥�Ȥ������ϥ����ؤ��Ѵ��ϥ��ץꥱ�������ץ���ޤ���Ǥ��
    �Ԥ�ʤ��ƤϤʤ�ʤ����ܺ٤ˤĤ��Ƥϴؿ� minput_event_to_key () ��
    �����򻲾ȡ�

    <li> �������ϥ᥽�å� @anchor foreign-input-method

    �������ϥ᥽�åɤȤ� LANGUAGE �� @c Mnil �Τ�ΤǤ��ꡢ�������Τϳ�
    ���Υ꥽�����Ȥ����������롣�ʤ��Ȥ���X Window System ��XIM ��
    �ɡ�) ���μ�����ϥ᥽�åɤǤϡ�����ܥ� NAME �� #Minput_driver ��
    �����Ȥ���ץ�ѥƥ�������������ͤ����ϥ᥽�åɥɥ饤�ФؤΥݥ���
    ���Ǥ��롣���Τ��Ȥˤ�ꡢŬ�ڤʥɥ饤�Ф�������뤳�Ȥˤ�äơ���
    ���ʤ��������ϥ᥽�åɤ�@c m17n @c �饤�֥�� �����Ȥ���ǰ�����
    ���Ǥ��롣

    �������δ������顢m17n X �饤�֥��� XIM �� OverTheSpot �����ϥ���
    �����¸��������ϥ᥽�åɥɥ饤�Ф��󶡤����ޤ�����ܥ� @c Mxim ��
    #Minput_driver �ץ�ѥƥ����ͤȤ��Ƥ��Υɥ饤�ФؤΥݥ��󥿤��ݻ�
    ���Ƥ��롣�ܺ٤ˤĤ��Ƥ� m17n GUI API �Υɥ�����Ȥ򻲾ȤΤ��ȡ�

    </ul> 

    ������ή��

    ���ϥ᥽�åɽ�����ŵ��Ū�ʽ����ϰʲ��Τ褦�ˤʤ롣
    
    @li ���ϥ᥽�åɤΥ����ץ�
    @li �������ϥ᥽�åɤ����ϥ���ƥ����Ȥ�����
    @li ���ϥ��٥�ȤΥե��륿
    @li ���ϥ���ƥ����ȤǤ������ƥ����Ȥθ���     */

/*=*/

#if !defined (FOR_DOXYGEN) || defined (DOXYGEN_INTERNAL_MODULE)
/*** @addtogroup m17nInternal
     @{ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "config.h"

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#include "m17n.h"
#include "m17n-misc.h"
#include "internal.h"
#include "mtext.h"
#include "input.h"
#include "symbol.h"
#include "plist.h"
#include "database.h"
#include "charset.h"

static int mdebug_flag = MDEBUG_INPUT;

static int fully_initialized;

/** Symbols to load an input method data.  */
static MSymbol Mtitle, Mmacro, Mmodule, Mstate, Minclude;

/** Symbols for actions.  */
static MSymbol Minsert, Mdelete, Mmark, Mmove, Mpushback, Mundo, Mcall, Mshift;
static MSymbol Mselect, Mshow, Mhide, Mcommit, Munhandle, Mpop;
static MSymbol Mset, Madd, Msub, Mmul, Mdiv, Mequal, Mless, Mgreater;
static MSymbol Mless_equal, Mgreater_equal;
static MSymbol Mcond;
static MSymbol Mplus, Mminus, Mstar, Mslash, Mand, Mor, Mnot;
static MSymbol Mswitch_im, Mpush_im, Mpop_im;

/** Special action symbol.  */
static MSymbol Mat_reload;

static MSymbol M_candidates;

static MSymbol Mcandidate_list, Mcandidate_index;

static MSymbol Minit, Mfini;

/** Symbols for variables.  */
static MSymbol Mcandidates_group_size, Mcandidates_charset;
static MSymbol Mfallback_input_method;

/** Symbols for key events.  */
static MSymbol one_char_symbol[256];

static MSymbol M_key_alias;

static MSymbol Mdescription, Mcommand, Mvariable, Mglobal, Mconfig;

static MSymbol M_gettext;

/** Structure to hold a map.  */

struct MIMMap
{
  /** List of actions to take when we reach the map.  In a root map,
      the actions are executed only when there is no more key.  */
  MPlist *map_actions;

  /** List of deeper maps.  If NULL, this is a terminal map.  */
  MPlist *submaps;

  /** List of actions to take when we leave the map successfully.  In
      a root map, the actions are executed only when none of submaps
      handle the current key.  */
  MPlist *branch_actions;
};

typedef MPlist *(*MIMExternalFunc) (MPlist *plist);

typedef struct
{
  MSymbol name;
  void *handle;
  MPlist *func_list;		/* function name vs (MIMExternalFunc *) */
} MIMExternalModule;

struct MIMState
{
  M17NObject control;

  /** Name of the state.  */
  MSymbol name;

  /** Title of the state, or NULL.  */
  MText *title;

  /** Key translation map of the state.  Built by merging all maps of
      branches.  */
  MIMMap *map;
};

#define CUSTOM_FILE "config.mic"

static MPlist *load_im_info_keys;

/* List of input method information.  The format is:
     (LANGUAGE NAME t:IM_INFO ... ... ...)  */
static MPlist *im_info_list;

/* Database for user's customization file.  */
static MDatabase *im_custom_mdb;

/* List of input method information loaded from im_custom_mdb.  The
   format is the same as im_info_list.  */
static MPlist *im_custom_list;

/* List of input method information configured by
   minput_config_command and minput_config_variable.  The format is
   the same as im_info_list.  */
static MPlist *im_config_list;

/* Global input method information.  It points into the element of
   im_info_list corresponding to LANGUAGE == `nil' and NAME ==
   `global'.  */
static MInputMethodInfo *global_info;

/* List of fallback input methods: well-formed plist of this form:
     ((im-lang1 im-name1) (im-lang2 im-name2) ...)
   The elements are in reverse preference order.  */

static MPlist *fallback_input_methods;

static int update_global_info (void);
static int update_custom_info (void);
static MInputMethodInfo *get_im_info (MSymbol, MSymbol, MSymbol, MSymbol);


/* Initialize fallback_input_methods.  Called by fully_initialize ()
   after fully_initialized is set to 1.  */

static void
prepare_fallback_input_methods ()
{
  MPlist *plist, *pl, *p;

  fallback_input_methods = mplist ();
  if ((plist = minput_get_variable (Mt, Mnil, Mfallback_input_method)) == NULL)
    return;
  plist = MPLIST_PLIST (plist);
  plist = MPLIST_NEXT (plist);	/* skip name */
  plist = MPLIST_NEXT (plist);	/* skip description */
  plist = MPLIST_NEXT (plist);	/* skip status */
  if (! plist)
    return;
  /* Now PLIST must be "LANGUAGE-NAME, ..." */
  if (MPLIST_MTEXT_P (plist))
    {
      plist = minput_parse_im_names (MPLIST_MTEXT (plist));
      MPLIST_DO (p, plist) 
	if (MPLIST_KEY (p) == Mplist)
	  mplist_push (fallback_input_methods, Mplist, MPLIST_VAL (p));
      M17N_OBJECT_UNREF (plist);
    }
}

static void
fully_initialize ()
{
  char *key_names[32]
    = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	"BackSpace", "Tab", "Linefeed", "Clear", NULL, "Return", NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, "Escape", NULL, NULL, NULL, NULL };
  char buf[6], buf2[32], buf3[2];
  int i, j;
  /* Maximum case: '\215', C-M-m, C-M-M, M-Return, C-A-m, C-A-M, A-Return
     plus one for cyclic alias.  */
  MSymbol alias[8];

  M_key_alias = msymbol ("  key-alias");

  buf3[1] = '\0';

  /* Aliases for 0x00-0x1F */
  buf[0] = 'C';
  buf[1] = '-';
  buf[3] = '\0';
  for (i = 0, buf[2] = '@'; i < ' '; i++, buf[2]++)
    {
      j = 0;
      buf3[0] = i;
      alias[j++] = msymbol (buf3);
      alias[j++] = one_char_symbol[i] = msymbol (buf);
      if (key_names[i] || (buf[2] >= 'A' && buf[2] <= 'Z'))
	{
	  if (key_names[i])
	    {
	      /* Ex: `Escape' == `C-['  */
	      alias[j++] = msymbol (key_names[i]);
	    }
	  if (buf[2] >= 'A' && buf[2] <= 'Z')
	    {
	      /* Ex: `C-a' == `C-A'  */
	      buf[2] += 32;
	      alias[j++] = msymbol (buf);
	      buf[2] -= 32;
	    }
	}
      /* Establish cyclic alias chain.  */
      alias[j] = alias[0];
      while (--j >= 0)
	msymbol_put (alias[j], M_key_alias, alias[j + 1]);
    }

  /* Aliases for 0x20-0x7E */
  buf[0] = 'S';
  for (i = buf[2] = ' '; i < 127; i++, buf[2]++)
    {
      one_char_symbol[i] = msymbol (buf + 2);
      if (i >= 'A' && i <= 'Z')
	{
	  /* Ex: `A' == `S-A' == `S-a'.  */
	  alias[0] = alias[3] = one_char_symbol[i];
	  alias[1] = msymbol (buf);
	  buf[2] += 32;
	  alias[2] = msymbol (buf);
	  buf[2] -= 32;
	  for (j = 0; j < 3; j++)
	    msymbol_put (alias[j], M_key_alias, alias[j + 1]);
	}
    }

  /* Aliases for 0x7F */
  buf3[0] = 0x7F;
  alias[0] = alias[3] = msymbol (buf3);
  alias[1] = one_char_symbol[127] = msymbol ("Delete");
  alias[2] = msymbol ("C-?");
  for (j = 0; j < 3; j++)
    msymbol_put (alias[j], M_key_alias, alias[j + 1]);

  /* Aliases for 0x80-0x9F */
  buf[0] = 'C';
  /* buf[1] = '-'; -- already done */
  buf[3] = '-';
  buf[5] = '\0';
  buf2[1] = '-';
  for (i = 128, buf[4] = '@'; i < 160; i++, buf[4]++)
    {
      j = 0;
      buf3[0] = i;
      alias[j++] = msymbol (buf3);
      /* `C-M-a' == `C-A-a' */
      buf[2] = 'M';
      alias[j++] = one_char_symbol[i] = msymbol (buf);
      buf[2] = 'A';
      alias[j++] = msymbol (buf);
      if (key_names[i - 128])
	{
	  /* Ex: `M-Escape' == `A-Escape' == `C-M-['.  */
	  buf2[0] = 'M';
	  strcpy (buf2 + 2, key_names[i - 128]);
	  alias[j++] = msymbol (buf2);
	  buf2[0] = 'A';
	  alias[j++] = msymbol (buf2);
	}
      if (buf[4] >= 'A' && buf[4] <= 'Z')
	{
	  /* Ex: `C-M-a' == `C-M-A'.  */
	  buf[4] += 32;
	  buf[2] = 'M';
	  alias[j++] = msymbol (buf);
	  buf[2] = 'A';
	  alias[j++] = msymbol (buf);
	  buf[4] -= 32;
	}

      /* Establish cyclic alias chain.  */
      alias[j] = alias[0];
      while (--j >= 0)
	msymbol_put (alias[j], M_key_alias, alias[j + 1]);
    }

  /* Aliases for 0xA0-0xFF */
  for (i = 160, buf[4] = ' '; i < 255; i++, buf[4]++)
    {
      j = 0;
      buf3[0] = i;
      alias[j++] = msymbol (buf3);
      buf[2] = 'M';
      alias[j++] = one_char_symbol[i] = msymbol (buf + 2);
      buf[2] = 'A';
      alias[j++] = msymbol (buf + 2);
      alias[j]= alias[0];
      while (--j >= 0)
	msymbol_put (alias[j], M_key_alias, alias[j + 1]);
    }

  buf3[0] = (char) 255;
  alias[0] = alias[3] = msymbol (buf3);
  alias[1] = one_char_symbol[255] = msymbol ("M-Delete");
  alias[2] = msymbol ("A-Delete");
  for (j = 0; j < 3; j++)
    msymbol_put (alias[j], M_key_alias, alias[j + 1]);

  /* Aliases for keys that can't be mapped to one-char-symbol
     (e.g. C-A-1) */
  /* buf is already set to "C-?-".  */
  for (i = ' '; i <= '~'; i++)
    {
      if (i == '@')
	{
	  i = '_';
	  continue;
	}
      if (i == 'a')
	{
	  i = 'z';
	  continue;
	}
      buf[2] = 'M';
      buf[4] = i;
      alias[0] = alias[2] = msymbol (buf);
      buf[2] = 'A';
      alias[1] = msymbol (buf);
      for (j = 0; j < 2; j++)
	msymbol_put (alias[j], M_key_alias, alias[j + 1]);
    }

  Minput_method = msymbol ("input-method");
  Mtitle = msymbol ("title");
  Mmacro = msymbol ("macro");
  Mmodule = msymbol ("module");
  Mmap = msymbol ("map");
  Mstate = msymbol ("state");
  Minclude = msymbol ("include");
  Minsert = msymbol ("insert");
  M_candidates = msymbol ("  candidates");
  Mdelete = msymbol ("delete");
  Mmove = msymbol ("move");
  Mmark = msymbol ("mark");
  Mpushback = msymbol ("pushback");
  Mpop = msymbol ("pop");
  Mundo = msymbol ("undo");
  Mcall = msymbol ("call");
  Mshift = msymbol ("shift");
  Mselect = msymbol ("select");
  Mshow = msymbol ("show");
  Mhide = msymbol ("hide");
  Mcommit = msymbol ("commit");
  Munhandle = msymbol ("unhandle");
  Mset = msymbol ("set");
  Madd = msymbol ("add");
  Msub = msymbol ("sub");
  Mmul = msymbol ("mul");
  Mdiv = msymbol ("div");
  Mequal = msymbol ("=");
  Mless = msymbol ("<");
  Mgreater = msymbol (">");
  Mless_equal = msymbol ("<=");
  Mgreater_equal = msymbol (">=");
  Mcond = msymbol ("cond");
  Mplus = msymbol ("+");
  Mminus = msymbol ("-");
  Mstar = msymbol ("*");
  Mslash = msymbol ("/");
  Mand = msymbol ("&");
  Mor = msymbol ("|");
  Mnot = msymbol ("!");
  Mswitch_im = msymbol ("switch-im");
  Mpush_im = msymbol ("push-im");
  Mpop_im = msymbol ("pop-im");

  Mat_reload = msymbol ("-reload");

  Mcandidates_group_size = msymbol ("candidates-group-size");
  Mcandidates_charset = msymbol ("candidates-charset");
  Mfallback_input_method = msymbol ("fallback-input-method");

  Mcandidate_list = msymbol_as_managing_key ("  candidate-list");
  Mcandidate_index = msymbol ("  candidate-index");

  Minit = msymbol ("init");
  Mfini = msymbol ("fini");

  Mdescription = msymbol ("description");
  Mcommand = msymbol ("command");
  Mvariable = msymbol ("variable");
  Mglobal = msymbol ("global");
  Mconfig = msymbol ("config");
  M_gettext = msymbol ("_");

  load_im_info_keys = mplist ();
  mplist_add (load_im_info_keys, Mstate, Mnil);
  mplist_push (load_im_info_keys, Mmap, Mnil);

  im_info_list = mplist ();
  im_config_list = im_custom_list = NULL;
  im_custom_mdb = NULL;
  update_custom_info ();
  global_info = NULL;
  update_global_info ();

  fully_initialized = 1;
  prepare_fallback_input_methods ();
}

#define MINPUT__INIT()		\
  do {				\
    if (! fully_initialized)	\
      fully_initialize ();	\
  } while (0)


static int
marker_code (MSymbol sym, int surrounding)
{
  char *name;

  if (sym == Mnil)
    return -1;
  name = MSYMBOL_NAME (sym);
  return (name[0] != '@' ? -1
	  : (((name[1] >= '0' && name[1] <= '9')
	      || name[1] == '<' || name[1] == '>' || name[1] == '='
	      || name[1] == '[' || name[1] == ']'
	      || name[1] == '@')
	     && name[2] == '\0') ? name[1]
	  : (name[1] != '+' && name[1] != '-') ? -1
	  : (name[2] == '\0' || surrounding) ? name[1]
	  : -1);
}


/* Return a plist containing an integer value of VAR.  The plist must
   not be UNREFed. */

static MPlist *
resolve_variable (MInputContextInfo *ic_info, MSymbol var)
{
  MPlist *plist = mplist__assq (ic_info->vars, var);

  if (plist)
    {
      plist = MPLIST_PLIST (plist);
      return MPLIST_NEXT (plist);
    }

  plist = mplist ();
  mplist_push (ic_info->vars, Mplist, plist);
  M17N_OBJECT_UNREF (plist);
  plist = mplist_add (plist, Msymbol, var);
  plist = mplist_add (plist, Minteger, (void *) 0);
  return plist;
}

static MText *
get_surrounding_text (MInputContext *ic, int len)
{
  MText *mt = NULL;

  mplist_push (ic->plist, Minteger, (void *) len);
  if (minput_callback (ic, Minput_get_surrounding_text) >= 0
      && MPLIST_MTEXT_P (ic->plist))
    mt = MPLIST_MTEXT (ic->plist);
  mplist_pop (ic->plist);
  return mt;
}

static void
delete_surrounding_text (MInputContext *ic, int pos)
{
  MInputContextInfo *ic_info = (MInputContextInfo *) ic->info;

  mplist_push (ic->plist, Minteger, (void *) pos);
  minput_callback (ic, Minput_delete_surrounding_text);
  mplist_pop (ic->plist);
  if (pos < 0)
    {
      M17N_OBJECT_UNREF (ic_info->preceding_text);
      ic_info->preceding_text = NULL;
    }
  else if (pos > 0)
    {
      M17N_OBJECT_UNREF (ic_info->following_text);
      ic_info->following_text = NULL;
    }
}

static int
get_preceding_char (MInputContext *ic, int pos)
{
  MInputContextInfo *ic_info = (MInputContextInfo *) ic->info;
  MText *mt;
  int len;

  if (pos && ic_info->preceding_text)
    {
      len = mtext_nchars (ic_info->preceding_text);
      if (pos <= len)
	return mtext_ref_char (ic_info->preceding_text, len - pos);
    }
  mt = get_surrounding_text (ic, - pos);
  if (! mt)
    return -2;
  len = mtext_nchars (mt);
  if (ic_info->preceding_text)
    {
      if (mtext_nchars (ic_info->preceding_text) < len)
	{
	  M17N_OBJECT_UNREF (ic_info->preceding_text);
	  ic_info->preceding_text = mt;
	}
      else
	M17N_OBJECT_UNREF (mt);
    }
  else
    ic_info->preceding_text = mt;
  if (pos > len)
    return -1;
  return mtext_ref_char (ic_info->preceding_text, len - pos);
}

static int
get_following_char (MInputContext *ic, int pos)
{
  MInputContextInfo *ic_info = (MInputContextInfo *) ic->info;
  MText *mt;
  int len;

  if (ic_info->following_text)
    {
      len = mtext_nchars (ic_info->following_text);
      if (pos < len)
	return mtext_ref_char (ic_info->following_text, pos);
    }
  mt = get_surrounding_text (ic, pos + 1);
  if (! mt)
    return -2;
  len = mtext_nchars (mt);
  if (ic_info->following_text)
    {
      if (mtext_nchars (ic_info->following_text) < len)
	{
	  M17N_OBJECT_UNREF (ic_info->following_text);
	  ic_info->following_text = mt;
	}
      else
	M17N_OBJECT_UNREF (mt);
    }
  else
    ic_info->following_text = mt;
  if (pos >= len)
    return -1;
  return mtext_ref_char (ic_info->following_text, pos);
}

static int
surrounding_pos (MSymbol sym, int *pos)
{
  char *name;

  if (sym == Mnil)
    return 0;
  name = MSYMBOL_NAME (sym);
  if (name[0] == '@'
      && (name[1] == '-' ? (name[2] >= '1' && name[2] <= '9')
	  : name[1] == '+' ? (name[2] >= '0' && name[2] <= '9')
	  : 0))
    {
      *pos = name[1] == '-' ? - atoi (name + 2) : atoi (name + 2);
      return 1;
    }
  return 0;
}

static int
integer_value (MInputContext *ic, MPlist *arg, int surrounding)
{
  MInputContextInfo *ic_info = (MInputContextInfo *) ic->info;
  int code, pos;
  MText *preedit = ic->preedit;
  int len = mtext_nchars (preedit);

  if (MPLIST_INTEGER_P (arg))
    return MPLIST_INTEGER (arg);

  code = marker_code (MPLIST_SYMBOL (arg), surrounding);
  if (code < 0)
    {
      MPlist *val = resolve_variable (ic_info, MPLIST_SYMBOL (arg));

      return (MPLIST_INTEGER_P (val) ? MPLIST_INTEGER (val) : 0);
    }
  if (code == '@')
    return ic_info->key_head;
  if ((code == '-' || code == '+'))
    {
      char *name = MSYMBOL_NAME (MPLIST_SYMBOL (arg));

      if (name[2])
	{
	  pos = atoi (name + 1);
	  if (pos == 0 && code == '-')
	    return get_preceding_char (ic, 0);
	  pos = ic->cursor_pos + pos;
	  if (pos < 0)
	    {
	      if (ic->produced && mtext_len (ic->produced) + pos >= 0)
		return mtext_ref_char (ic->produced,
				       mtext_len (ic->produced) + pos);
	      return get_preceding_char (ic, - pos);
	    }
	  else if (pos >= len)
	    return get_following_char (ic, pos - len);
	}
      else
	pos = ic->cursor_pos + (code == '+' ? 1 : -1);
    }
  else if (code >= '0' && code <= '9')
    pos = code - '0';
  else if (code == '=')
    pos = ic->cursor_pos;
  else if (code == '[')
    pos = ic->cursor_pos - 1;
  else if (code == ']')
    pos = ic->cursor_pos + 1;
  else if (code == '<')
    pos = 0;
  else if (code == '>')
    pos = len - 1;
  return (pos >= 0 && pos < len ? mtext_ref_char (preedit, pos) : -1);
}

static int
parse_expression (MPlist *plist)
{
  MSymbol op;

  if (MPLIST_INTEGER_P (plist) || MPLIST_SYMBOL_P (plist))
    return 0;
  if (! MPLIST_PLIST_P (plist))
    return -1;
  plist = MPLIST_PLIST (plist);
  op = MPLIST_SYMBOL (plist);
  if (op != Mplus && op != Mminus && op != Mstar && op != Mslash
      && op != Mand && op != Mor && op != Mnot
      && op != Mless && op != Mgreater && op != Mequal
      && op != Mless_equal && op != Mgreater_equal)
    MERROR (MERROR_IM, -1);
  MPLIST_DO (plist, MPLIST_NEXT (plist))
    if (parse_expression (plist) < 0)
      return -1;
  return 0;
}

static int
resolve_expression (MInputContext *ic, MPlist *plist)
{
  int val;
  MSymbol op;
  
  if (MPLIST_INTEGER_P (plist))
    return MPLIST_INTEGER (plist);
  if (MPLIST_SYMBOL_P (plist))
    return integer_value (ic, plist, 1);
  if (! MPLIST_PLIST_P (plist))
    return 0;
  plist = MPLIST_PLIST (plist);
  if (! MPLIST_SYMBOL_P (plist))
    return 0;
  op = MPLIST_SYMBOL (plist);
  plist = MPLIST_NEXT (plist);
  val = resolve_expression (ic, plist);
  if (op == Mplus)
    MPLIST_DO (plist, MPLIST_NEXT (plist))
      val += resolve_expression (ic, plist);
  else if (op == Mminus)
    MPLIST_DO (plist, MPLIST_NEXT (plist))
      val -= resolve_expression (ic, plist);
  else if (op == Mstar)
    MPLIST_DO (plist, MPLIST_NEXT (plist))
      val *= resolve_expression (ic, plist);
  else if (op == Mslash)
    MPLIST_DO (plist, MPLIST_NEXT (plist))
      val /= resolve_expression (ic, plist);
  else if (op == Mand)
    MPLIST_DO (plist, MPLIST_NEXT (plist))
      val &= resolve_expression (ic, plist);
  else if (op == Mor)
    MPLIST_DO (plist, MPLIST_NEXT (plist))
      val |= resolve_expression (ic, plist);
  else if (op == Mnot)
    val = ! val;
  else if (op == Mless)
    val = val < resolve_expression (ic, MPLIST_NEXT (plist));
  else if (op == Mequal)
    val = val == resolve_expression (ic, MPLIST_NEXT (plist));
  else if (op == Mgreater)
    val = val > resolve_expression (ic, MPLIST_NEXT (plist));
  else if (op == Mless_equal)
    val = val <= resolve_expression (ic, MPLIST_NEXT (plist));
  else if (op == Mgreater_equal)
    val = val >= resolve_expression (ic, MPLIST_NEXT (plist));
  return val;
}

/* Parse PLIST as an action list.  PLIST should have this form:
      PLIST ::= ( (ACTION-NAME ACTION-ARG *) *).
   Return 0 if successfully parsed, otherwise return -1.  */

static int
parse_action_list (MPlist *plist, MPlist *macros)
{
  MPLIST_DO (plist, plist)
    {
      if (MPLIST_MTEXT_P (plist))
	{
	  /* This is a short form of (insert MTEXT).  */
	  /* if (mtext_nchars (MPLIST_MTEXT (plist)) == 0)
	     MERROR (MERROR_IM, -1); */
	}
      else if (MPLIST_PLIST_P (plist)
	       && (MPLIST_MTEXT_P (MPLIST_PLIST (plist))
		   || MPLIST_PLIST_P (MPLIST_PLIST (plist))))
	{
	  MPlist *pl;

	  /* This is a short form of (insert (GROUPS *)).  */
	  MPLIST_DO (pl, MPLIST_PLIST (plist))
	    {
	      if (MPLIST_PLIST_P (pl))
		{
		  MPlist *elt;

		  MPLIST_DO (elt, MPLIST_PLIST (pl))
		    if (! MPLIST_MTEXT_P (elt)
			|| mtext_nchars (MPLIST_MTEXT (elt)) == 0)
		      MERROR (MERROR_IM, -1);
		}
	      else
		{
		  if (! MPLIST_MTEXT_P (pl)
		      || mtext_nchars (MPLIST_MTEXT (pl)) == 0)
		    MERROR (MERROR_IM, -1);
		}
	    }
	}
      else if (MPLIST_INTEGER_P (plist))
	{
	  int c = MPLIST_INTEGER (plist);

	  if (c < 0 || c > MCHAR_MAX)
	    MERROR (MERROR_IM, -1);
	}
      else if (MPLIST_PLIST_P (plist)
	       && MPLIST_SYMBOL_P (MPLIST_PLIST (plist)))
	{
	  MPlist *pl = MPLIST_PLIST (plist);
	  MSymbol action_name = MPLIST_SYMBOL (pl);

	  pl = MPLIST_NEXT (pl);

	  if (action_name == M_candidates)
	    {
	      /* This is an already regularised action.  */
	      continue;
	    }
	  if (action_name == Minsert)
	    {
	      if (MPLIST_MTEXT_P (pl))
		{
		  if (mtext_nchars (MPLIST_MTEXT (pl)) == 0)
		    MERROR (MERROR_IM, -1);
		}
	      else if (MPLIST_INTEGER_P (pl))
		{
		  int c = MPLIST_INTEGER (pl);

		  if (c < 0 || c > MCHAR_MAX)
		    MERROR (MERROR_IM, -1);
		}
	      else if (MPLIST_PLIST_P (pl))
		{
		  MPLIST_DO (pl, MPLIST_PLIST (pl))
		    {
		      if (MPLIST_PLIST_P (pl))
			{
			  MPlist *elt;

			  MPLIST_DO (elt, MPLIST_PLIST (pl))
			    if (! MPLIST_MTEXT_P (elt)
				|| mtext_nchars (MPLIST_MTEXT (elt)) == 0)
			      MERROR (MERROR_IM, -1);
			}
		      else
			{
			  if (! MPLIST_MTEXT_P (pl)
			      || mtext_nchars (MPLIST_MTEXT (pl)) == 0)
			    MERROR (MERROR_IM, -1);
			}
		    }
		}
	      else if (! MPLIST_SYMBOL_P (pl))
		MERROR (MERROR_IM, -1); 
	    }
	  else if (action_name == Mselect
		   || action_name == Mdelete
		   || action_name == Mmove)
	    {
	      if (parse_expression (pl) < 0)
		return -1;
	    }
	  else if (action_name == Mmark
		   || action_name == Mcall
		   || action_name == Mshift)
	    {
	      if (! MPLIST_SYMBOL_P (pl))
		MERROR (MERROR_IM, -1);
	    }
	  else if (action_name == Mundo)
	    {
	      if (! MPLIST_TAIL_P (pl))
		{
		  if (! MPLIST_SYMBOL_P (pl)
		      && ! MPLIST_INTEGER_P (pl))
		    MERROR (MERROR_IM, -1);		    
		}
	    }
	  else if (action_name == Mpushback)
	    {
	      if (MPLIST_MTEXT_P (pl))
		{
		  MText *mt = MPLIST_MTEXT (pl);

		  if (mtext_nchars (mt) != mtext_nbytes (mt))
		    MERROR (MERROR_IM, -1);		    
		}
	      else if (MPLIST_PLIST_P (pl))
		{
		  MPlist *p;

		  MPLIST_DO (p, MPLIST_PLIST (pl))
		    if (! MPLIST_SYMBOL_P (p))
		      MERROR (MERROR_IM, -1);
		}
	      else if (! MPLIST_INTEGER_P (pl))
		MERROR (MERROR_IM, -1);
	    }
	  else if (action_name == Mset || action_name == Madd
		   || action_name == Msub || action_name == Mmul
		   || action_name == Mdiv)
	    {
	      if (! MPLIST_SYMBOL_P (pl))
		MERROR (MERROR_IM, -1);
	      if (parse_expression (MPLIST_NEXT (pl)) < 0)
		return -1;
	    }
	  else if (action_name == Mequal || action_name == Mless
		   || action_name == Mgreater || action_name == Mless_equal
		   || action_name == Mgreater_equal)
	    {
	      if (parse_expression (pl) < 0
		  || parse_expression (MPLIST_NEXT (pl)) < 0)
		return -1;
	      pl = MPLIST_NEXT (MPLIST_NEXT (pl));
	      if (! MPLIST_PLIST_P (pl))
		MERROR (MERROR_IM, -1);
	      if (parse_action_list (MPLIST_PLIST (pl), macros) < 0)
		MERROR (MERROR_IM, -1);
	      pl = MPLIST_NEXT (pl);
	      if (MPLIST_PLIST_P (pl)
		  && parse_action_list (MPLIST_PLIST (pl), macros) < 0)
		MERROR (MERROR_IM, -1);
	    }
	  else if (action_name == Mshow || action_name == Mhide
		   || action_name == Mcommit || action_name == Munhandle
		   || action_name == Mpop
		   || action_name == Mpush_im || action_name == Mpop_im
		   || action_name == Mswitch_im)
	    ;
	  else if (action_name == Mcond)
	    {
	      MPLIST_DO (pl, pl)
		if (! MPLIST_PLIST_P (pl))
		  MERROR (MERROR_IM, -1);
	    }
	  else if (! macros || ! mplist_get (macros, action_name))
	    MERROR (MERROR_IM, -1);
	}
      else if (! MPLIST_SYMBOL_P (plist))
	MERROR (MERROR_IM, -1);
    }

  return 0;
}

static MPlist *
resolve_command (MPlist *cmds, MSymbol command)
{
  MPlist *plist;

  if (! cmds || ! (plist = mplist__assq (cmds, command)))
    return NULL;
  plist = MPLIST_PLIST (plist);	/* (NAME DESC STATUS [KEYSEQ ...]) */
  plist = MPLIST_NEXT (plist);
  plist = MPLIST_NEXT (plist);
  plist = MPLIST_NEXT (plist);
  return plist;
}

/* Load a translation into MAP from PLIST.
   PLIST has this form:
      PLIST ::= ( KEYSEQ MAP-ACTION * )  */

static int
load_translation (MIMMap *map, MPlist *keylist, MPlist *map_actions,
		  MPlist *branch_actions, MPlist *macros)
{
  MSymbol *keyseq;
  int len, i;

  if (MPLIST_MTEXT_P (keylist))
    {
      MText *mt = MPLIST_MTEXT (keylist);

      len = mtext_nchars (mt);
      if (MFAILP (len > 0 && len == mtext_nbytes (mt)))
	return -1;
      keyseq = (MSymbol *) alloca (sizeof (MSymbol) * len);
      for (i = 0; i < len; i++)
	keyseq[i] = one_char_symbol[MTEXT_DATA (mt)[i]];
    }
  else
    {
      MPlist *elt;
	  
      if (MFAILP (MPLIST_PLIST_P (keylist)))
	return -1;
      elt = MPLIST_PLIST (keylist);
      len = MPLIST_LENGTH (elt);
      if (MFAILP (len > 0))
	return -1;
      keyseq = (MSymbol *) alloca (sizeof (MSymbol) * len);
      for (i = 0; i < len; i++, elt = MPLIST_NEXT (elt))
	{
	  if (MPLIST_INTEGER_P (elt))
	    {
	      int c = MPLIST_INTEGER (elt);

	      if (MFAILP (c >= 0 && c < 0x100))
		return -1;
	      keyseq[i] = one_char_symbol[c];
	    }
	  else
	    {
	      if (MFAILP (MPLIST_SYMBOL_P (elt)))
		return -1;
	      keyseq[i] = MPLIST_SYMBOL (elt);
	    }
	}
    }

  for (i = 0; i < len; i++)
    {
      MIMMap *deeper = NULL;

      if (map->submaps)
	deeper = mplist_get (map->submaps, keyseq[i]);
      else
	map->submaps = mplist ();
      if (! deeper)
	{
	  /* Fixme: It is better to make all deeper maps at once.  */
	  MSTRUCT_CALLOC (deeper, MERROR_IM);
	  mplist_put (map->submaps, keyseq[i], deeper);
	}
      map = deeper;
    }

  /* We reach a terminal map.  */
  if (map->map_actions
      || map->branch_actions)
    /* This map is already defined.  We avoid overriding it.  */
    return 0;

  if (! MPLIST_TAIL_P (map_actions))
    {
      if (parse_action_list (map_actions, macros) < 0)
	MERROR (MERROR_IM, -1);
      map->map_actions = map_actions;
    }
  if (branch_actions)
    {
      map->branch_actions = branch_actions;
      M17N_OBJECT_REF (branch_actions);
    }

  return 0;
}

/* Load a branch from PLIST into MAP.  PLIST has this form:
      PLIST ::= ( MAP-NAME BRANCH-ACTION * )  */

static int
load_branch (MInputMethodInfo *im_info, MPlist *plist, MIMMap *map)
{
  MSymbol map_name;
  MPlist *branch_actions;

  if (MFAILP (MPLIST_SYMBOL_P (plist)))
    return -1;
  map_name = MPLIST_SYMBOL (plist);
  plist = MPLIST_NEXT (plist);
  if (MPLIST_TAIL_P (plist))
    branch_actions = NULL;
  else if (MFAILP (parse_action_list (plist, im_info->macros) >= 0))
    return -1;
  else
    branch_actions = plist;
  if (map_name == Mnil)
    {
      map->branch_actions = branch_actions;
      if (branch_actions)
	M17N_OBJECT_REF (branch_actions);
    }
  else if (map_name == Mt)
    {
      map->map_actions = branch_actions;
      if (branch_actions)
	M17N_OBJECT_REF (branch_actions);
    }
  else if (im_info->maps) 
    {
      plist = (MPlist *) mplist_get (im_info->maps, map_name);
      if (! plist && im_info->configured_vars)
	{
	  MPlist *p = mplist__assq (im_info->configured_vars, map_name);

	  if (p && MPLIST_PLIST_P (p))
	    {
	      p = MPLIST_NEXT (MPLIST_NEXT (MPLIST_NEXT (MPLIST_PLIST (p))));
	      if (MPLIST_SYMBOL_P (p))
		plist = mplist_get (im_info->maps, MPLIST_SYMBOL (p));
	    }
	}
      if (plist)
	{
	  MPLIST_DO (plist, plist)
	    {
	      MPlist *keylist, *map_actions;

	      if (! MPLIST_PLIST_P (plist))
		MERROR (MERROR_IM, -1);
	      keylist = MPLIST_PLIST (plist);
	      map_actions = MPLIST_NEXT (keylist);
	      if (MPLIST_SYMBOL_P (keylist))
		{
		  MSymbol command = MPLIST_SYMBOL (keylist);
		  MPlist *pl;

		  if (MFAILP (command != Mat_reload))
		    continue;
		  pl = resolve_command (im_info->configured_cmds, command);
		  if (MFAILP (pl))
		    continue;
		  MPLIST_DO (pl, pl)
		    load_translation (map, pl, map_actions, branch_actions,
				      im_info->macros);
		}
	      else
		load_translation (map, keylist, map_actions, branch_actions,
				  im_info->macros);
	    }
	}
    }

  return 0;
}

/* Load a macro from PLIST into IM_INFO->macros.
   PLIST has this form:
      PLIST ::= ( MACRO-NAME ACTION * )
   IM_INFO->macros is a plist of macro names vs action list.  */

static int
load_macros (MInputMethodInfo *im_info, MPlist *plist)
{
  MSymbol name; 
  MPlist *pl;

  if (! MPLIST_SYMBOL_P (plist))
    MERROR (MERROR_IM, -1);
  name = MPLIST_SYMBOL (plist);
  plist = MPLIST_NEXT (plist);
  if (MFAILP (! MPLIST_TAIL_P (plist)))
    MERROR (MERROR_IM, -1);
  pl = mplist_get (im_info->macros, name);
  M17N_OBJECT_UNREF (pl);
  mplist_put (im_info->macros, name, plist);
  M17N_OBJECT_REF (plist);
  return 0;
}

/* Load an external module from PLIST, and return a pointer to
   MIMExternalModule.

   PLIST has this form:
      PLIST ::= ( MODULE-NAME FUNCTION * )
   IM_INFO->externals is a plist of MODULE-NAME vs (MIMExternalModule *).

   On error, return NULL.  */

static MIMExternalModule *
load_external_module (MPlist *plist)
{
  void *handle;
  MSymbol module;
  char *module_file;
  MIMExternalModule *external;
  MPlist *func_list;
  void *func;

  if (MPLIST_MTEXT_P (plist))
    module = msymbol ((char *) MTEXT_DATA (MPLIST_MTEXT (plist)));
  else if (MPLIST_SYMBOL_P (plist))
    module = MPLIST_SYMBOL (plist);
  module_file = alloca (strlen (M17N_MODULE_DIR) + 1
			+ strlen (MSYMBOL_NAME (module))
			+ strlen (DLOPEN_SHLIB_EXT) + 1);
  sprintf (module_file, "%s/%s%s",
	   M17N_MODULE_DIR, MSYMBOL_NAME (module), DLOPEN_SHLIB_EXT);

  handle = dlopen (module_file, RTLD_NOW);
  if (MFAILP (handle))
    return NULL;
  func_list = mplist ();
  MPLIST_DO (plist, MPLIST_NEXT (plist))
    {
      if (! MPLIST_SYMBOL_P (plist))
	MERROR_GOTO (MERROR_IM, err_label);
      func = dlsym (handle, MSYMBOL_NAME (MPLIST_SYMBOL (plist)));
      if (MFAILP (func))
	goto err_label;
      mplist_add (func_list, MPLIST_SYMBOL (plist), func);
    }

  MSTRUCT_MALLOC (external, MERROR_IM);
  external->name = module;
  external->handle = handle;
  external->func_list = func_list;
  return external;

 err_label:
  M17N_OBJECT_UNREF (func_list);
  dlclose (handle);
  return NULL;
}

static void
unload_external_module (MIMExternalModule *external)
{
  dlclose (external->handle);
  M17N_OBJECT_UNREF (external->func_list);
  free (external);
}

static void
free_map (MIMMap *map, int top)
{
  MPlist *plist;

  if (top)
    M17N_OBJECT_UNREF (map->map_actions);
  if (map->submaps)
    {
      MPLIST_DO (plist, map->submaps)
	free_map ((MIMMap *) MPLIST_VAL (plist), 0);
      M17N_OBJECT_UNREF (map->submaps);
    }
  M17N_OBJECT_UNREF (map->branch_actions);
  free (map);
}

static void
free_state (void *object)
{
  MIMState *state = object;

  M17N_OBJECT_UNREF (state->title);
  if (state->map)
    free_map (state->map, 1);
  free (state);
}

/** Load a state from PLIST into a newly allocated state object.
    PLIST has this form:
      PLIST ::= ( STATE-NAME STATE-TITLE ? BRANCH * )
      BRANCH ::= ( MAP-NAME BRANCH-ACTION * )
   Return the state object.  */

static MIMState *
load_state (MInputMethodInfo *im_info, MPlist *plist)
{
  MIMState *state;

  if (MFAILP (MPLIST_SYMBOL_P (plist)))
    return NULL;
  M17N_OBJECT (state, free_state, MERROR_IM);
  state->name = MPLIST_SYMBOL (plist);
  plist = MPLIST_NEXT (plist);
  if (MPLIST_MTEXT_P (plist))
    {
      state->title = MPLIST_MTEXT (plist);
      mtext_put_prop (state->title, 0, mtext_nchars (state->title),
		      Mlanguage, im_info->language);
      M17N_OBJECT_REF (state->title);
      plist = MPLIST_NEXT (plist);
    }
  MSTRUCT_CALLOC (state->map, MERROR_IM);
  MPLIST_DO (plist, plist)
    {
      if (MFAILP (MPLIST_PLIST_P (plist)))
	continue;
      load_branch (im_info, MPLIST_PLIST (plist), state->map);
    }
  return state;
}

/* Return a newly created IM_INFO for an input method specified by
   LANUAGE, NAME, and EXTRA.  IM_INFO is stored in PLIST.  */

static MInputMethodInfo *
new_im_info (MDatabase *mdb, MSymbol language, MSymbol name, MSymbol extra,
	     MPlist *plist)
{
  MInputMethodInfo *im_info;
  MPlist *elt;

  if (name == Mnil && extra == Mnil)
    language = Mt, extra = Mglobal;

  MDEBUG_PRINT3 ("loading %s-%s %s\n",
		 msymbol_name (language), msymbol_name (name), (mdb ? "from mdb" : ""));

  MSTRUCT_CALLOC (im_info, MERROR_IM);
  im_info->mdb = mdb;
  im_info->language = language;
  im_info->name = name;
  im_info->extra = extra;

  elt = mplist ();
  mplist_add (plist, Mplist, elt);
  M17N_OBJECT_UNREF (elt);
  elt = mplist_add (elt, Msymbol, language);
  elt = mplist_add (elt, Msymbol, name);
  elt = mplist_add (elt, Msymbol, extra);
  mplist_add (elt, Mt, im_info);

  return im_info;
}

static void
fini_im_info (MInputMethodInfo *im_info)
{
  MPlist *plist;

  M17N_OBJECT_UNREF (im_info->cmds);
  M17N_OBJECT_UNREF (im_info->configured_cmds);
  M17N_OBJECT_UNREF (im_info->bc_cmds);
  M17N_OBJECT_UNREF (im_info->vars);
  M17N_OBJECT_UNREF (im_info->configured_vars);
  M17N_OBJECT_UNREF (im_info->bc_vars);
  M17N_OBJECT_UNREF (im_info->description);
  M17N_OBJECT_UNREF (im_info->title);
  if (im_info->states)
    {
      MPLIST_DO (plist, im_info->states)
	{
	  MIMState *state = (MIMState *) MPLIST_VAL (plist);

	  M17N_OBJECT_UNREF (state);
	}
      M17N_OBJECT_UNREF (im_info->states);
    }

  if (im_info->macros)
    {
      MPLIST_DO (plist, im_info->macros)
	M17N_OBJECT_UNREF (MPLIST_VAL (plist));	
      M17N_OBJECT_UNREF (im_info->macros);
    }

  if (im_info->externals)
    {
      MPLIST_DO (plist, im_info->externals)
	{
	  unload_external_module (MPLIST_VAL (plist));
	  MPLIST_KEY (plist) = Mt;
	}
      M17N_OBJECT_UNREF (im_info->externals);
    }
  if (im_info->maps)
    {
      MPLIST_DO (plist, im_info->maps)
	{
	  MPlist *p = MPLIST_PLIST (plist);

	  M17N_OBJECT_UNREF (p);
	}
      M17N_OBJECT_UNREF (im_info->maps);
    }

  im_info->tick = 0;
}

static void
free_im_info (MInputMethodInfo *im_info)
{
  MDEBUG_PRINT2 ("freeing %s-%s\n", msymbol_name (im_info->language),
		 msymbol_name (im_info->name));
  fini_im_info (im_info);
  free (im_info);
}

static void
free_im_list (MPlist *plist)
{
  MPlist *pl, *elt;

  MPLIST_DO (pl, plist)
    {
      MInputMethodInfo *im_info;

      elt = MPLIST_NEXT (MPLIST_NEXT (MPLIST_NEXT (MPLIST_PLIST (pl))));
      im_info = MPLIST_VAL (elt);
      free_im_info (im_info);
    }
  M17N_OBJECT_UNREF (plist);
}

static MInputMethodInfo *
lookup_im_info (MPlist *plist, MSymbol language, MSymbol name, MSymbol extra)
{
  if (name == Mnil && extra == Mnil)
    language = Mt, extra = Mglobal;
  while ((plist = mplist__assq (plist, language)))
    {
      MPlist *elt = MPLIST_PLIST (plist);

      plist = MPLIST_NEXT (plist);
      elt = MPLIST_NEXT (elt);
      if (MPLIST_SYMBOL (elt) != name)
	continue;
      elt = MPLIST_NEXT (elt);
      if (MPLIST_SYMBOL (elt) != extra)
	continue;
      elt = MPLIST_NEXT (elt);
      return MPLIST_VAL (elt);
    }
  return NULL;
}

static void load_im_info (MPlist *, MInputMethodInfo *);

#define get_custom_info(im_info)				\
  (im_custom_list						\
   ? lookup_im_info (im_custom_list, (im_info)->language,	\
		     (im_info)->name, (im_info)->extra)		\
   : NULL)

#define get_config_info(im_info)				\
  (im_config_list						\
   ? lookup_im_info (im_config_list, (im_info)->language,	\
		     (im_info)->name, (im_info)->extra)		\
   : NULL)

static int
update_custom_info (void)
{
  MPlist *plist, *pl;

  if (im_custom_mdb)
    {
      if (mdatabase__check (im_custom_mdb) > 0)
	return 1;
    }
  else
    {
      MDatabaseInfo *custom_dir_info;
      char custom_path[PATH_MAX + 1];

      custom_dir_info = MPLIST_VAL (mdatabase__dir_list);
      if (! custom_dir_info->filename
	  || custom_dir_info->len + strlen (CUSTOM_FILE) > PATH_MAX)
	return -1;
      strcpy (custom_path, custom_dir_info->filename);
      strcat (custom_path, CUSTOM_FILE);
      im_custom_mdb = mdatabase_define (Minput_method, Mt, Mnil, Mconfig,
					NULL, custom_path);
    }

  if (im_custom_list)
    {
      free_im_list (im_custom_list);
      im_custom_list = NULL;
    }
  plist = mdatabase_load (im_custom_mdb);
  if (! plist)
    return -1;
  im_custom_list = mplist ();

  MPLIST_DO (pl, plist)
    {
      MSymbol language, name, extra;
      MInputMethodInfo *im_info;
      MPlist *im_data, *p;

      if (! MPLIST_PLIST_P (pl))
	continue;
      p = MPLIST_PLIST (pl);
      im_data = MPLIST_NEXT (p);
      if (! MPLIST_PLIST_P (p))
	continue;
      p = MPLIST_PLIST (p);
      if (! MPLIST_SYMBOL_P (p)
	  || MPLIST_SYMBOL (p) != Minput_method)
	continue;
      p = MPLIST_NEXT (p);
      if (! MPLIST_SYMBOL_P (p))
	continue;
      language = MPLIST_SYMBOL (p);
      p = MPLIST_NEXT (p);
      if (! MPLIST_SYMBOL_P (p))
	continue;
      name = MPLIST_SYMBOL (p);
      p = MPLIST_NEXT (p);
      if (MPLIST_TAIL_P (p))
	extra = Mnil;
      else if (MPLIST_SYMBOL_P (p))
	extra = MPLIST_SYMBOL (p);
      if (language == Mnil || (name == Mnil && extra == Mnil))
	continue;
      im_info = new_im_info (NULL, language, name, extra, im_custom_list);
      load_im_info (im_data, im_info);
    }
  M17N_OBJECT_UNREF (plist);
  return 0;
}

static int
update_global_info (void)
{
  MPlist *plist;

  if (global_info)
    {
      int ret = mdatabase__check (global_info->mdb);

      if (ret)
	return ret;
      fini_im_info (global_info);
    }
  else
    {
      MDatabase *mdb = mdatabase_find (Minput_method, Mt, Mnil, Mglobal);

      if (! mdb)
	return -1;
      global_info = new_im_info (mdb, Mt, Mnil, Mglobal, im_info_list);
    }
  if (! global_info->mdb
      || ! (plist = mdatabase_load (global_info->mdb)))
    return -1;

  load_im_info (plist, global_info);
  M17N_OBJECT_UNREF (plist);
  return 0;
}


/* Return an IM_INFO for the input method specified by LANGUAGE, NAME,
   and EXTRA.  KEY, if not Mnil, tells which kind of information about
   the input method is necessary, and the returned IM_INFO may contain
   only that information.  */

static MInputMethodInfo *
get_im_info (MSymbol language, MSymbol name, MSymbol extra, MSymbol key)
{
  MPlist *plist;
  MInputMethodInfo *im_info;
  MDatabase *mdb;

  if (name == Mnil && extra == Mnil)
    language = Mt, extra = Mglobal;
  im_info = lookup_im_info (im_info_list, language, name, extra);
  if (im_info)
    {
      if (key == Mnil ? im_info->states != NULL
	  : key == Mcommand ? im_info->cmds != NULL
	  : key == Mvariable ? im_info->vars != NULL
	  : key == Mtitle ? im_info->title != NULL
	  : key == Mdescription ? im_info->description != NULL
	  : 1)
	/* IM_INFO already contains required information.  */
	return im_info;
      /* We have not yet loaded required information.  */
    }
  else
    {
      mdb = mdatabase_find (Minput_method, language, name, extra);
      if (! mdb)
	return NULL;
      im_info = new_im_info (mdb, language, name, extra, im_info_list);
    }

  if (key == Mnil)
    {
      plist = mdatabase_load (im_info->mdb);
    }
  else
    {
      mplist_push (load_im_info_keys, key, Mt);
      plist = mdatabase__load_for_keys (im_info->mdb, load_im_info_keys);
      mplist_pop (load_im_info_keys);
    }
  im_info->tick = 0;
  if (! plist)
    MERROR (MERROR_IM, im_info);
  update_global_info ();
  load_im_info (plist, im_info);
  M17N_OBJECT_UNREF (plist);
  if (key == Mnil)
    {
      if (! im_info->cmds)
	im_info->cmds = mplist ();
      if (! im_info->vars)
	im_info->vars = mplist ();
      if (! im_info->states)
	im_info->states = mplist ();
    }
  if (! im_info->title
      && (key == Mnil || key == Mtitle))
    im_info->title = (name == Mnil ? mtext ()
		      : mtext_from_data (MSYMBOL_NAME (name),
					 MSYMBOL_NAMELEN (name),
					 MTEXT_FORMAT_US_ASCII));
  return im_info;
}

/* Check if IM_INFO->mdb is updated or not.  If not updated, return 0.
   If updated, but got unloadable, return -1.  Otherwise, update
   contents of IM_INFO from the new database, and return 1.  */

static int
reload_im_info (MInputMethodInfo *im_info)
{
  int check;
  MPlist *plist;

  update_custom_info ();
  update_global_info ();
  check = mdatabase__check (im_info->mdb);
  if (check < 0)
    return -1;
  plist = mdatabase_load (im_info->mdb);
  if (! plist)
    return -1;
  fini_im_info (im_info);
  load_im_info (plist, im_info);
  M17N_OBJECT_UNREF (plist);
  if (! im_info->cmds)
    im_info->cmds = mplist ();
  if (! im_info->vars)
    im_info->vars = mplist ();
  if (! im_info->title)
    {
      MSymbol name = im_info->name;

      im_info->title = (name == Mnil ? mtext ()
			: mtext_from_data (MSYMBOL_NAME (name),
					   MSYMBOL_NAMELEN (name),
					   MTEXT_FORMAT_US_ASCII));
    }
  return 1;
}

static MInputMethodInfo *
get_im_info_by_tags (MPlist *plist)
{
  MSymbol tag[3];
  int i;

  for (i = 0; i < 3 && MPLIST_SYMBOL_P (plist);
       i++, plist = MPLIST_NEXT (plist))
    tag[i] = MPLIST_SYMBOL (plist);
  if (i < 2)
    return NULL;
  for (; i < 3; i++)
    tag[i] = Mnil;
  return get_im_info (tag[0], tag[1], tag[2], Mnil);
}


/* Open an input method specified by LANG_NAME, create an input
   context for it, and return the input context.  */


static MInputContext *
create_ic_for_im (MPlist *lang_name, MInputMethod *current)
{
  MSymbol language, name;
  MInputMethod *im;
  MInputContext *ic;
  MInputDriver *driver = minput_driver;

  language = MPLIST_SYMBOL (lang_name);
  lang_name = MPLIST_NEXT (lang_name);
  name = MPLIST_SYMBOL (lang_name);
  if (language == current->language
      && name == current->name)
    return NULL;
  minput_driver = &minput_default_driver;
  im = minput_open_im (language, name, NULL);
  if (! im)
    {
      minput_driver = driver;
      return NULL;
    }
  ic = minput_create_ic (im, NULL);
  if (! ic)
    {
      minput_close_im (im);
      minput_driver = driver;
      return NULL;
    }
  minput_driver = driver;
  return ic;
}

static int
check_description (MPlist *plist)
{
  MText *mt;

  if (MPLIST_MTEXT_P (plist))
    return 1;
  if (MPLIST_PLIST_P (plist))
    {
      MPlist *pl = MPLIST_PLIST (plist);

      if (MFAILP (MPLIST_SYMBOL_P (pl) && MPLIST_SYMBOL (pl) == M_gettext))
	return 0;
      pl =MPLIST_NEXT (pl);
      if (MFAILP (MPLIST_MTEXT_P (pl)))
	return 0;
      mt = MPLIST_MTEXT (pl);
      M17N_OBJECT_REF (mt);
#if ENABLE_NLS
      {
	char *translated = dgettext ("m17n-db", (char *) MTEXT_DATA (mt));

	if (translated == (char *) MTEXT_DATA (mt))
	  translated = dgettext ("m17n-contrib", (char *) MTEXT_DATA (mt));
	if (translated != (char *) MTEXT_DATA (mt))
	  {
	    M17N_OBJECT_UNREF (mt);
	    mt = mtext__from_data (translated, strlen (translated),
				   MTEXT_FORMAT_UTF_8, 1);
	  }
      }
#endif
      mplist_set (plist, Mtext, mt);
      M17N_OBJECT_UNREF (mt);
      return 1;
    }
  if (MFAILP (MPLIST_SYMBOL_P (plist) && MPLIST_SYMBOL (plist) == Mnil))
    return 0;
  return 1;
}


/* Check KEYSEQ, and return 1 if it is valid as a key sequence, return
   0 if not.  */

static int
check_command_keyseq (MPlist *keyseq)
{
  if (MPLIST_PLIST_P (keyseq))
    {
      MPlist *p = MPLIST_PLIST (keyseq);

      MPLIST_DO (p, p)
	if (! MPLIST_SYMBOL_P (p) && ! MPLIST_INTEGER_P (p))
	  return 0;
      return 1;
    }
  if (MPLIST_MTEXT_P (keyseq))
    {
      MText *mt = MPLIST_MTEXT (keyseq);
      int i;
      
      for (i = 0; i < mtext_nchars (mt); i++)
	if (mtext_ref_char (mt, i) >= 256)
	  return 0;
      return 1;
    }
  return 0;
}

/* Load command defitions from PLIST into IM_INFO->cmds.

   PLIST is well-formed and has this form;
     (command (NAME [DESCRIPTION KEYSEQ ...]) ...)
   NAME is a symbol.  DESCRIPTION is an M-text or `nil'.  KEYSEQ is an
   M-text or a plist of symbols.

   The returned list has the same form, but for each element...

   (1) If DESCRIPTION and the rest are omitted, the element is not
   stored in the returned list.

   (2) If DESCRIPTION is nil, it is complemented by the corresponding
   description in global_info->cmds (if any).  */

static void
load_commands (MInputMethodInfo *im_info, MPlist *plist)
{
  MPlist *tail;

  im_info->cmds = tail = mplist ();

  MPLIST_DO (plist, MPLIST_NEXT (plist))
    {
      /* PLIST ::= ((NAME DESC KEYSEQ ...) ...) */
      MPlist *pl, *p;

      if (MFAILP (MPLIST_PLIST_P (plist)))
	continue;
      pl = MPLIST_PLIST (plist); /* PL ::= (NAME DESC KEYSEQ ...) */
      if (MFAILP (MPLIST_SYMBOL_P (pl)))
	continue;
      p = MPLIST_NEXT (pl);	/* P ::= (DESC KEYSEQ ...) */
      if (MPLIST_TAIL_P (p))	/* PL ::= (NAME) */
	{
	  if (MFAILP (im_info != global_info))
	    mplist_add (p, Msymbol, Mnil); /* PL ::= (NAME nil) */
	}
      else
	{
	  if (! check_description (p))
	    mplist_set (p, Msymbol, Mnil);
	  p = MPLIST_NEXT (p);
	  while (! MPLIST_TAIL_P (p))
	    {
	      if (MFAILP (check_command_keyseq (p)))
		mplist__pop_unref (p);
	      else
		p = MPLIST_NEXT (p);
	    }
	}
      tail = mplist_add (tail, Mplist, pl);
    }
}

static MPlist *
config_command (MPlist *plist, MPlist *global_cmds, MPlist *custom_cmds,
		MPlist *config_cmds)
{
  MPlist *global = NULL, *custom = NULL, *config = NULL;
  MSymbol name = MPLIST_SYMBOL (plist);
  MSymbol status;
  MPlist *description, *keyseq;

  if (global_cmds && (global = mplist__assq (global_cmds, name)))
    global = MPLIST_NEXT (MPLIST_PLIST (global));  

  plist = MPLIST_NEXT (plist);
  if (MPLIST_MTEXT_P (plist) || MPLIST_PLIST_P (plist))
    {
      description = plist;
      plist = MPLIST_NEXT (plist);
    }
  else
    {
      description = global;
      if (! MPLIST_TAIL_P (plist))
	plist = MPLIST_NEXT (plist);
    }
  if (MPLIST_TAIL_P (plist) && global)
    {
      keyseq = MPLIST_NEXT (global);
      status = Minherited;
    }
  else
    {
      keyseq = plist;
      status = Mnil;
    }

  if (config_cmds && (config = mplist__assq (config_cmds, name)))
    {
      status = Mconfigured;
      config = MPLIST_NEXT (MPLIST_PLIST (config));
      if (! MPLIST_TAIL_P (config))
	keyseq = config;
    }
  else if (custom_cmds && (custom = mplist__assq (custom_cmds, name)))
    {
      MPlist *this_keyseq = MPLIST_NEXT (MPLIST_NEXT (MPLIST_PLIST (custom)));

      if (MPLIST_TAIL_P (this_keyseq))
	mplist__pop_unref (custom);
      else
	{
	  status = Mcustomized;
	  keyseq = this_keyseq;
	}
    }
  
  plist = mplist ();
  mplist_add (plist, Msymbol, name);
  if (description)
    mplist_add (plist, MPLIST_KEY (description), MPLIST_VAL (description));
  else
    mplist_add (plist, Msymbol, Mnil);
  mplist_add (plist, Msymbol, status);
  mplist__conc (plist, keyseq);
  return plist;
}

static void
config_all_commands (MInputMethodInfo *im_info)
{
  MPlist *global_cmds, *custom_cmds, *config_cmds;
  MInputMethodInfo *temp;
  MPlist *tail, *plist;

  M17N_OBJECT_UNREF (im_info->configured_cmds);

  if (MPLIST_TAIL_P (im_info->cmds)
      || ! im_info->mdb)
    return;

  global_cmds = im_info != global_info ? global_info->cmds : NULL;
  custom_cmds = ((temp = get_custom_info (im_info)) ? temp->cmds : NULL);
  config_cmds = ((temp = get_config_info (im_info)) ? temp->cmds : NULL);

  im_info->configured_cmds = tail = mplist ();
  MPLIST_DO (plist, im_info->cmds)
    {
      MPlist *pl = config_command (MPLIST_PLIST (plist),
				   global_cmds, custom_cmds, config_cmds);
      if (pl)
	{
	  tail = mplist_add (tail, Mplist, pl);
	  M17N_OBJECT_UNREF (pl);
	}
    }
}

/* Check VAL's value against VALID_VALUES, and return 1 if it is
   valid, return 0 if not.  */

static int
check_variable_value (MPlist *val, MPlist *global)
{
  MSymbol type = MPLIST_KEY (val);
  MPlist *valids = MPLIST_NEXT (val);

  if (type != Minteger && type != Mtext && type != Msymbol && type != Mplist)
    return 0;
  if (global)
    {
      if (MPLIST_KEY (global) != Mt
	  && MPLIST_KEY (global) != MPLIST_KEY (val))
	return 0;
      if (MPLIST_TAIL_P (valids))
	valids = MPLIST_NEXT (global);
    }
  if (MPLIST_TAIL_P (valids))
    return 1;

  if (type == Minteger)
    {
      int n = MPLIST_INTEGER (val);

      MPLIST_DO (valids, valids)
	{
	  if (MPLIST_INTEGER_P (valids))
	    {
	      if (n == MPLIST_INTEGER (valids))
		break;
	    }
	  else if (MPLIST_PLIST_P (valids))
	    {
	      MPlist *p = MPLIST_PLIST (valids);
	      int min_bound, max_bound;

	      if (! MPLIST_INTEGER_P (p))
		MERROR (MERROR_IM, 0);
	      min_bound = MPLIST_INTEGER (p);
	      p = MPLIST_NEXT (p);
	      if (! MPLIST_INTEGER_P (p))
		MERROR (MERROR_IM, 0);
	      max_bound = MPLIST_INTEGER (p);
	      if (n >= min_bound && n <= max_bound)
		break;
	    }
	}
    }
  else if (type == Msymbol)
    {
      MSymbol sym = MPLIST_SYMBOL (val);

      MPLIST_DO (valids, valids)
	{
	  if (! MPLIST_SYMBOL_P (valids))
	    MERROR (MERROR_IM, 0);
	  if (sym == MPLIST_SYMBOL (valids))
	    break;
	}
    }
  else if (type == Mtext)
    {
      MText *mt = MPLIST_MTEXT (val);

      MPLIST_DO (valids, valids)
	{
	  if (! MPLIST_MTEXT_P (valids))
	    MERROR (MERROR_IM, 0);
	  if (mtext_cmp (mt, MPLIST_MTEXT (valids)) == 0)
	    break;
	}
    }

  return (! MPLIST_TAIL_P (valids));
}

/* Load variable definitions from PLIST into IM_INFO->vars.

   PLIST is well-formed and has this form;
     ((NAME [DESCRIPTION DEFAULT-VALUE VALID-VALUE ...])
      ...)
   NAME is a symbol.  DESCRIPTION is an M-text or `nil'.

   The returned list has the same form, but for each element...

   (1) If DESCRIPTION and the rest are omitted, the element is not
   stored in the returned list.

   (2) If DESCRIPTION is nil, it is complemented by the corresponding
   description in global_info->vars (if any).  */

static void
load_variables (MInputMethodInfo *im_info, MPlist *plist)
{
  MPlist *global_vars = ((im_info->mdb && im_info != global_info)
			 ? global_info->vars : NULL);
  MPlist *tail;

  im_info->vars = tail = mplist ();
  MPLIST_DO (plist, MPLIST_NEXT (plist))
    {
      MPlist *pl, *p;

      if (MFAILP (MPLIST_PLIST_P (plist)))
	continue;
      pl = MPLIST_PLIST (plist); /* PL ::= (NAME DESC VALUE VALID ...) */
      if (MFAILP (MPLIST_SYMBOL_P (pl)))
	continue;
      if (im_info == global_info)
	{
	  /* Loading a global variable.  */
	  p = MPLIST_NEXT (pl);
	  if (MPLIST_TAIL_P (p))
	    mplist_add (p, Msymbol, Mnil);
	  else
	    {
	      if (! check_description (p))
		mplist_set (p, Msymbol, Mnil);
	      p = MPLIST_NEXT (p);
	      if (MFAILP (! MPLIST_TAIL_P (p)
			  && check_variable_value (p, NULL)))
		mplist_set (p, Mt, NULL);
	    }
	}
      else if (im_info->mdb)
	{
	  /* Loading a local variable.  */
	  MSymbol name = MPLIST_SYMBOL (pl);
	  MPlist *global = NULL;

	  if (global_vars
	      && (p = mplist__assq (global_vars, name)))
	    {
	      /* P ::= ((NAME DESC ...) ...) */
	      p = MPLIST_PLIST (p); /* P ::= (NAME DESC ...) */
	      global = MPLIST_NEXT (p); /* P ::= (DESC VALUE ...) */
	      global = MPLIST_NEXT (global); /* P ::= (VALUE ...) */
	    }

	  p = MPLIST_NEXT (pl);	/* P ::= (DESC VALUE VALID ...) */
	  if (! MPLIST_TAIL_P (p))
	    {
	      if (! check_description (p))
		mplist_set (p, Msymbol, Mnil);
	      p = MPLIST_NEXT (p); /* P ::= (VALUE VALID ...) */
	      if (MFAILP (! MPLIST_TAIL_P (p)))
		mplist_set (p, Mt, NULL);
	      else
		{
		  MPlist *valid_values = MPLIST_NEXT (p);

		  if (! MPLIST_TAIL_P (valid_values)
		      ? MFAILP (check_variable_value (p, NULL))
		      : global && MFAILP (check_variable_value (p, global)))
		    mplist_set (p, Mt, NULL);
		}
	    }
	}
      else
	{
	  /* Loading a variable customization.  */
	  p = MPLIST_NEXT (pl);	/* P ::= (nil VALUE) */
	  if (MFAILP (! MPLIST_TAIL_P (p)))
	    continue;
	  p = MPLIST_NEXT (p);	/* P ::= (VALUE) */
	  if (MFAILP (MPLIST_INTEGER_P (p) || MPLIST_SYMBOL_P (p)
		      || MPLIST_MTEXT_P (p)))
	    continue;
	}
      tail = mplist_add (tail, Mplist, pl);
    }
}

static MPlist *
config_variable (MPlist *plist, MPlist *global_vars, MPlist *custom_vars,
		 MPlist *config_vars)
{
  MPlist *global = NULL, *custom = NULL, *config = NULL;
  MSymbol name = MPLIST_SYMBOL (plist);
  MSymbol status;
  MPlist *description = NULL, *value, *valids;

  if (global_vars)
    {
      global = mplist__assq (global_vars, name);
      if (global)
	global = MPLIST_NEXT (MPLIST_PLIST (global)); /* (DESC VALUE ...) */
    }

  plist = MPLIST_NEXT (plist);
  if (MPLIST_MTEXT_P (plist) || MPLIST_PLIST_P (plist))
    description = plist;
  else if (global)
    description = global;
  if (global)
    global = MPLIST_NEXT (global); /* (VALUE VALIDS ...) */

  if (MPLIST_TAIL_P (plist))
    {
      /* Inherit from global (if any).  */
      if (global)
	{
	  value = global;
	  if (MPLIST_KEY (value) == Mt)
	    value = NULL;
	  valids = MPLIST_NEXT (global);
	  status = Minherited;
	}
      else
	{
	  value = NULL;
	  valids = NULL;
	  status = Mnil;
	  plist = NULL;
	}
    }
  else
    {
      value = plist = MPLIST_NEXT (plist);
      valids = MPLIST_NEXT (value);
      if (MPLIST_KEY (value) == Mt)
	value = NULL;
      if (! MPLIST_TAIL_P (valids))
	global = NULL;
      else if (global)
	valids = MPLIST_NEXT (global);
      status = Mnil;
    }

  if (config_vars && (config = mplist__assq (config_vars, name)))
    {
      status = Mconfigured;
      config = MPLIST_NEXT (MPLIST_PLIST (config));
      if (! MPLIST_TAIL_P (config))
	{
	  value = config;
	  if (MFAILP (check_variable_value (value, global ? global : plist)))
	    value = NULL;
	}
    }
  else if (custom_vars && (custom = mplist__assq (custom_vars, name)))
    {
      MPlist *this_value = MPLIST_NEXT (MPLIST_NEXT (MPLIST_PLIST (custom)));

      if (MPLIST_TAIL_P (this_value))
	mplist__pop_unref (custom);
      else
	{
	  value = this_value;
	  if (MFAILP (check_variable_value (value, global ? global : plist)))
	    value = NULL;
	  status = Mcustomized;
	}
    }
  
  plist = mplist ();
  mplist_add (plist, Msymbol, name);
  if (description)
    mplist_add (plist, MPLIST_KEY (description), MPLIST_VAL (description));
  else
    mplist_add (plist, Msymbol, Mnil);
  mplist_add (plist, Msymbol, status);
  if (value)
    mplist_add (plist, MPLIST_KEY (value), MPLIST_VAL (value));
  else
    mplist_add (plist, Mt, NULL);
  if (valids && ! MPLIST_TAIL_P (valids))
    mplist__conc (plist, valids);
  return plist;
}

/* Return a configured variable definition list based on
   IM_INFO->vars.  If a variable in it doesn't contain a value, try to
   get it from global_info->vars.  */

static void
config_all_variables (MInputMethodInfo *im_info)
{
  MPlist *global_vars, *custom_vars, *config_vars;
  MInputMethodInfo *temp;
  MPlist *tail, *plist;

  M17N_OBJECT_UNREF (im_info->configured_vars);

  if (MPLIST_TAIL_P (im_info->vars)
      || ! im_info->mdb)
    return;

  global_vars = im_info != global_info ? global_info->vars : NULL;
  custom_vars = ((temp = get_custom_info (im_info)) ? temp->vars : NULL);
  config_vars = ((temp = get_config_info (im_info)) ? temp->vars : NULL);

  im_info->configured_vars = tail = mplist ();
  MPLIST_DO (plist, im_info->vars)
    {
      MPlist *pl = config_variable (MPLIST_PLIST (plist),
				    global_vars, custom_vars, config_vars);
      if (pl)
	{
	  tail = mplist_add (tail, Mplist, pl);
	  M17N_OBJECT_UNREF (pl);
	}
    }
}

/* Load an input method (LANGUAGE NAME) from PLIST into IM_INFO.
   CONFIG contains configuration information of the input method.  */

static void
load_im_info (MPlist *plist, MInputMethodInfo *im_info)
{
  MPlist *pl, *p;

  if (! im_info->cmds && (pl = mplist__assq (plist, Mcommand)))
    {
      load_commands (im_info, MPLIST_PLIST (pl));
      config_all_commands (im_info);
      pl = mplist_pop (pl);
      M17N_OBJECT_UNREF (pl);
    }

  if (! im_info->vars && (pl = mplist__assq (plist, Mvariable)))
    {
      load_variables (im_info, MPLIST_PLIST (pl));
      config_all_variables (im_info);
      pl = mplist_pop (pl);
      M17N_OBJECT_UNREF (pl);
    }

  MPLIST_DO (plist, plist)
    if (MPLIST_PLIST_P (plist))
      {
	MPlist *elt = MPLIST_PLIST (plist);
	MSymbol key;

	if (MFAILP (MPLIST_SYMBOL_P (elt)))
	  continue;
	key = MPLIST_SYMBOL (elt);
	if (key == Mtitle)
	  {
	    if (im_info->title)
	      continue;
	    elt = MPLIST_NEXT (elt);
	    if (MFAILP (MPLIST_MTEXT_P (elt)))
	      continue;
	    im_info->title = MPLIST_MTEXT (elt);
	    M17N_OBJECT_REF (im_info->title);
	  }
	else if (key == Mmap)
	  {
	    pl = mplist__from_alist (MPLIST_NEXT (elt));
	    if (MFAILP (pl))
	      continue;
	    if (! im_info->maps)
	      im_info->maps = pl;
	    else
	      {
		mplist__conc (im_info->maps, pl);
		M17N_OBJECT_UNREF (pl);
	      }
	  }
	else if (key == Mmacro)
	  {
	    if (! im_info->macros)
	      im_info->macros = mplist ();
	    MPLIST_DO (elt, MPLIST_NEXT (elt))
	      {
		if (MFAILP (MPLIST_PLIST_P (elt)))
		  continue;
		load_macros (im_info, MPLIST_PLIST (elt));
	      }
	  }
	else if (key == Mmodule)
	  {
	    if (! im_info->externals)
	      im_info->externals = mplist ();
	    MPLIST_DO (elt, MPLIST_NEXT (elt))
	      {
		MIMExternalModule *external;

		if (MFAILP (MPLIST_PLIST_P (elt)))
		  continue;
		external = load_external_module (MPLIST_PLIST (elt));
		if (external)
		  mplist_add (im_info->externals, external->name, external);
	      }
	  }
	else if (key == Mstate)
	  {
	    MPLIST_DO (elt, MPLIST_NEXT (elt))
	      {
		MIMState *state;

		if (MFAILP (MPLIST_PLIST_P (elt)))
		  continue;
		pl = MPLIST_PLIST (elt);
		if (! im_info->states)
		  im_info->states = mplist ();
		state = load_state (im_info, MPLIST_PLIST (elt));
		if (MFAILP (state))
		  continue;
		mplist_put (im_info->states, state->name, state);
	      }
	  }
	else if (key == Minclude)
	  {
	    /* elt ::= include (tag1 tag2 ...) key item ... */
	    MSymbol key;
	    MInputMethodInfo *temp;

	    elt = MPLIST_NEXT (elt);
	    if (MFAILP (MPLIST_PLIST_P (elt)))
	      continue;
	    temp = get_im_info_by_tags (MPLIST_PLIST (elt));
	    if (MFAILP (temp))
	      continue;
	    elt = MPLIST_NEXT (elt);
	    if (MFAILP (MPLIST_SYMBOL_P (elt)))
	      continue;
	    key = MPLIST_SYMBOL (elt);
	    elt = MPLIST_NEXT (elt);
	    if (key == Mmap)
	      {
		if (! temp->maps || MPLIST_TAIL_P (temp->maps))
		  continue;
		if (! im_info->maps)
		  im_info->maps = mplist ();
		MPLIST_DO (pl, temp->maps)
		  {
		    p = MPLIST_VAL (pl);
		    MPLIST_ADD_PLIST (im_info->maps, MPLIST_KEY (pl), p);
		    M17N_OBJECT_REF (p);
		  }
	      }
	    else if (key == Mmacro)
	      {
		if (! temp->macros || MPLIST_TAIL_P (temp->macros))
		  continue;
		if (! im_info->macros)
		  im_info->macros = mplist ();
		MPLIST_DO (pl, temp->macros)
		  {
		    p = MPLIST_VAL (pl);
		    MPLIST_ADD_PLIST (im_info->macros, MPLIST_KEY (pl), p);
		    M17N_OBJECT_REF (p);
		  }
	      }
	    else if (key == Mstate)
	      {
		if (! temp->states || MPLIST_TAIL_P (temp->states))
		  continue;
		if (! im_info->states)
		  im_info->states = mplist ();
		MPLIST_DO (pl, temp->states)
		  {
		    MIMState *state = MPLIST_VAL (pl);

		    mplist_add (im_info->states, MPLIST_KEY (pl), state);
		    M17N_OBJECT_REF (state);
		  }
	      }
	  }
	else if (key == Mdescription)
	  {
	    if (im_info->description)
	      continue;
	    elt = MPLIST_NEXT (elt);
	    if (! check_description (elt))
	      continue;
	    im_info->description = MPLIST_MTEXT (elt);
	    M17N_OBJECT_REF (im_info->description);
	  }
      }
  if (im_info->macros)
    {
      MPLIST_DO (pl, im_info->macros)
	parse_action_list (MPLIST_PLIST (pl), im_info->macros);
    }

  im_info->tick = time (NULL);
}



static int take_action_list (MInputContext *ic, MPlist *action_list);
static void preedit_commit (MInputContext *ic, int need_prefix);

/* Shift to the state of name STATE_NAME.  If STATE_NAME is `t', shift
   to the previous state (if any).  If STATE_NAME is `nil', shift to
   the initial state.  */

static void
shift_state (MInputContext *ic, MSymbol state_name)
{
  MInputMethodInfo *im_info = (MInputMethodInfo *) ic->im->info;
  MInputContextInfo *ic_info = (MInputContextInfo *) ic->info;
  MIMState *orig_state = ic_info->state, *state;

  /* Find a state to shift to.  If not found, shift to the initial
     state.  */
  if (state_name == Mt)
    {
      if (! ic_info->prev_state)
	return;
      state = ic_info->prev_state;
    }
  else if (state_name == Mnil)
    {
      state = (MIMState *) MPLIST_VAL (im_info->states);
    }
  else
    {
      state = (MIMState *) mplist_get (im_info->states, state_name);
      if (! state)
	state = (MIMState *) MPLIST_VAL (im_info->states);
    }

  if (MDEBUG_FLAG ())
    {
      if (orig_state)
	MDEBUG_PRINT4 ("\n  [IM:%s-%s] [%s] (shift %s)\n",
		       MSYMBOL_NAME (im_info->language),
		       MSYMBOL_NAME (im_info->name),
		       MSYMBOL_NAME (orig_state->name),
		       MSYMBOL_NAME (state->name));
      else
	MDEBUG_PRINT1 (" (shift %s)\n", MSYMBOL_NAME (state->name));
    }

  /* Enter the new state.  */
  ic_info->state = state;
  ic_info->map = state->map;
  ic_info->state_key_head = ic_info->key_head;
  if (state == (MIMState *) MPLIST_VAL (im_info->states)
      && orig_state)
    /* We have shifted to the initial state.  */
    preedit_commit (ic, 0);
  mtext_cpy (ic_info->preedit_saved, ic->preedit);
  ic_info->state_pos = ic->cursor_pos;
  if (state != orig_state || state_name == Mnil)
    {
      if (state == (MIMState *) MPLIST_VAL (im_info->states))
	{
	  /* Shifted to the initial state.  */
	  ic_info->prev_state = NULL;
	  M17N_OBJECT_UNREF (ic_info->vars_saved);
	  ic_info->vars_saved = mplist_copy (ic_info->vars);
	}
      else
	ic_info->prev_state = orig_state;

      if (state->title)
	ic->status = state->title;
      else
	ic->status = im_info->title;
      ic->status_changed = 1;
      ic_info->state_hook = ic_info->map->map_actions;
    }
}

/* Find a candidate group that contains a candidate number INDEX from
   PLIST.  Set START_INDEX to the first candidate number of the group,
   END_INDEX to the last candidate number plus 1, GROUP_INDEX to the
   candidate group number if they are non-NULL.  If INDEX is -1, find
   the last candidate group.  */

static MPlist *
find_candidates_group (MPlist *plist, int index,
		       int *start_index, int *end_index, int *group_index)
{
  int i = 0, gidx = 0, len;

  MPLIST_DO (plist, plist)
    {
      if (MPLIST_MTEXT_P (plist))
	len = mtext_nchars (MPLIST_MTEXT (plist));
      else
	len = mplist_length (MPLIST_PLIST (plist));
      if (index < 0 ? MPLIST_TAIL_P (MPLIST_NEXT (plist))
	  : i + len > index)
	{
	  if (start_index)
	    *start_index = i;
	  if (end_index)
	    *end_index = i + len;
	  if (group_index)
	    *group_index = gidx;
	  return plist;
	}
      i += len;
      gidx++;
    }
  return NULL;
}

/* Adjust markers for the change of preedit text.
   If FROM == TO, the change is insertion of INS chars.
   If FROM < TO and INS == 0, the change is deletion of the range.
   If FROM < TO and INS > 0, the change is replacement.  */

static void
adjust_markers (MInputContext *ic, int from, int to, int ins)
{
  MInputContextInfo *ic_info = ((MInputContext *) ic)->info;
  MPlist *markers;

  if (from == to)
    {
      MPLIST_DO (markers, ic_info->markers)
	if (MPLIST_INTEGER (markers) > from)
	  MPLIST_VAL (markers) = (void *) (MPLIST_INTEGER (markers) + ins);
      if (ic->cursor_pos >= from)
	ic->cursor_pos += ins;
    }
  else
    {
      MPLIST_DO (markers, ic_info->markers)
	{
	  if (MPLIST_INTEGER (markers) >= to)
	    MPLIST_VAL (markers)
	      = (void *) (MPLIST_INTEGER (markers) + ins - (to - from));
	  else if (MPLIST_INTEGER (markers) > from)
	    MPLIST_VAL (markers) = (void *) from;
	}
      if (ic->cursor_pos >= to)
	ic->cursor_pos += ins - (to - from);
      else if (ic->cursor_pos > from)
	ic->cursor_pos = from;
    }
}


static void
preedit_insert (MInputContext *ic, int pos, MText *mt, int c)
{
  int nchars = mt ? mtext_nchars (mt) : 1;

  if (mt)
    {
      mtext_ins (ic->preedit, pos, mt);
      MDEBUG_PRINT1 ("(\"%s\")", MTEXT_DATA (mt));
    }
  else
    {
      mtext_ins_char (ic->preedit, pos, c, 1);
      if (c < 0x7F)
	MDEBUG_PRINT1 ("('%c')", c);
      else
	MDEBUG_PRINT1 ("(U+%04X)", c);
    }
  adjust_markers (ic, pos, pos, nchars);
  ic->preedit_changed = 1;
}


static void
preedit_delete (MInputContext *ic, int from, int to)
{
  mtext_del (ic->preedit, from, to);
  adjust_markers (ic, from, to, 0);
  ic->preedit_changed = 1;
}

static void
preedit_replace (MInputContext *ic, int from, int to, MText *mt, int c)
{
  int ins;

  mtext_del (ic->preedit, from, to);
  if (mt)
    {
      mtext_ins (ic->preedit, from, mt);
      ins = mtext_nchars (mt);
    }
  else
    {
      mtext_ins_char (ic->preedit, from, c, 1);
      ins = 1;
    }
  adjust_markers (ic, from, to, ins);
  ic->preedit_changed = 1;
}


static void
preedit_commit (MInputContext *ic, int need_prefix)
{
  MInputContextInfo *ic_info = (MInputContextInfo *) ic->info;
  int preedit_len = mtext_nchars (ic->preedit);

  if (preedit_len > 0)
    {
      MPlist *p;

      mtext_put_prop_values (ic->preedit, 0, mtext_nchars (ic->preedit),
			     Mcandidate_list, NULL, 0);
      mtext_put_prop_values (ic->preedit, 0, mtext_nchars (ic->preedit),
			     Mcandidate_index, NULL, 0);
      mtext_cat (ic->produced, ic->preedit);
      if (MDEBUG_FLAG ())
	{
	  int i;

	  if (need_prefix)
	    {
	      MInputMethodInfo *im_info = ic->im->info;
	      MDEBUG_PRINT3 ("\n  [IM:%s-%s] [%s]",
			     MSYMBOL_NAME (im_info->language),
			     MSYMBOL_NAME (im_info->name),
			     MSYMBOL_NAME (ic_info->state->name));
	    }
	  MDEBUG_PRINT (" (commit");
	  for (i = 0; i < mtext_nchars (ic->preedit); i++)
	    MDEBUG_PRINT1 (" U+%04X", mtext_ref_char (ic->preedit, i));
	  MDEBUG_PRINT (")");
	}

      mtext_reset (ic->preedit);
      mtext_reset (ic_info->preedit_saved);
      MPLIST_DO (p, ic_info->markers)
	MPLIST_VAL (p) = 0;
      ic->cursor_pos = ic_info->state_pos = 0;
      ic->preedit_changed = 1;
      ic_info->commit_key_head = ic_info->key_head;
    }
  if (ic->candidate_list)
    {
      M17N_OBJECT_UNREF (ic->candidate_list);
      ic->candidate_list = NULL;
      ic->candidate_index = 0;
      ic->candidate_from = ic->candidate_to = 0;
      ic->candidates_changed = MINPUT_CANDIDATES_LIST_CHANGED;
      if (ic->candidate_show)
	{
	  ic->candidate_show = 0;
	  ic->candidates_changed |= MINPUT_CANDIDATES_SHOW_CHANGED;
	}
    }
}

static int
new_index (MInputContext *ic, int current, int limit, MSymbol sym, MText *mt)
{
  int code = marker_code (sym, 0);

  if (mt && (code == '[' || code == ']'))
    {
      int pos = current;

      if (code == '[' && current > 0)
	{
	  if (mtext_prop_range (mt, Mcandidate_list, pos - 1, &pos, NULL, 1)
	      && pos > 0)
	    current = pos;
	}
      else if (code == ']' && current < mtext_nchars (mt))
	{
	  if (mtext_prop_range (mt, Mcandidate_list, pos, NULL, &pos, 1))
	    current = pos;
	}
      return current;
    }
  if (code >= 0)
    return (code == '<' ? 0
	    : code == '>' ? limit
	    : code == '-' ? current - 1
	    : code == '+' ? current + 1
	    : code == '=' ? current
	    : code - '0' > limit ? limit
	    : code - '0');
  if (! ic)  
    return 0;
  return (int) mplist_get (((MInputContextInfo *) ic->info)->markers, sym);
}

static void
update_candidate (MInputContext *ic, MTextProperty *prop, int idx)
{
  int from = mtext_property_start (prop);
  int to = mtext_property_end (prop);
  int start;
  MPlist *candidate_list = mtext_property_value (prop);
  MPlist *group = find_candidates_group (candidate_list, idx, &start,
					 NULL, NULL);
  int ingroup_index = idx - start;
  MText *mt;

  candidate_list = mplist_copy (candidate_list);
  if (MPLIST_MTEXT_P (group))
    {
      mt = MPLIST_MTEXT (group);
      preedit_replace (ic, from, to, NULL, mtext_ref_char (mt, ingroup_index));
      to = from + 1;
    }
  else
    {
      int i;
      MPlist *plist;

      for (i = 0, plist = MPLIST_PLIST (group); i < ingroup_index;
	   i++, plist = MPLIST_NEXT (plist));
      mt = MPLIST_MTEXT (plist);
      preedit_replace (ic, from, to, mt, 0);
      to = from + mtext_nchars (mt);
    }
  mtext_put_prop (ic->preedit, from, to, Mcandidate_list, candidate_list);
  M17N_OBJECT_UNREF (candidate_list);
  mtext_put_prop (ic->preedit, from, to, Mcandidate_index, (void *) idx);
  ic->cursor_pos = to;
}

static MCharset *
get_select_charset (MInputContextInfo * ic_info)
{
  MPlist *plist = resolve_variable (ic_info, Mcandidates_charset);
  MSymbol sym;

  if (! MPLIST_VAL (plist))
    return NULL;
  sym = MPLIST_SYMBOL (plist);
  if (sym == Mnil)
    return NULL;
  return MCHARSET (sym);
}

/* The returned plist must be UNREFed.  */

static MPlist *
adjust_candidates (MPlist *plist, MCharset *charset)
{
  MPlist *pl;

  /* plist ::= MTEXT ... | PLIST ... */
  plist = mplist_copy (plist);
  if (MPLIST_MTEXT_P (plist))
    {
      pl = plist;
      while (! MPLIST_TAIL_P (pl))
	{
	  /* pl ::= MTEXT ... */
	  MText *mt = MPLIST_MTEXT (pl);
	  int mt_copied = 0;
	  int i, c;

	  for (i = mtext_nchars (mt) - 1; i >= 0; i--)
	    {
	      c = mtext_ref_char (mt, i);
	      if (ENCODE_CHAR (charset, c) == MCHAR_INVALID_CODE)
		{
		  if (! mt_copied)
		    {
		      mt = mtext_dup (mt);
		      mplist_set (pl, Mtext, mt);
		      M17N_OBJECT_UNREF (mt);
		      mt_copied = 1;
		    }
		  mtext_del (mt, i, i + 1);
		}
	    }
	  if (mtext_len (mt) > 0)
	    pl = MPLIST_NEXT (pl);
	  else
	    {
	      mplist_pop (pl);
	      M17N_OBJECT_UNREF (mt);
	    }
	}
    }
  else				/* MPLIST_PLIST_P (plist) */
    {
      pl = plist;
      while (! MPLIST_TAIL_P (pl))
	{
	  /* pl ::= (MTEXT ...) ... */
	  MPlist *p = MPLIST_PLIST (pl);
	  int p_copied = 0;
	  /* p ::= MTEXT ... */
	  MPlist *p0 = p;
	  int n = 0;

	  while (! MPLIST_TAIL_P (p0))
	    {
	      MText *mt = MPLIST_MTEXT (p0);
	      int i, c;

	      for (i = mtext_nchars (mt) - 1; i >= 0; i--)
		{
		  c = mtext_ref_char (mt, i);
		  if (ENCODE_CHAR (charset, c) == MCHAR_INVALID_CODE)
		    break;
		}
	      if (i < 0)
		{
		  p0 = MPLIST_NEXT (p0);
		  n++;
		}
	      else
		{
		  if (! p_copied)
		    {
		      p = mplist_copy (p);
		      mplist_set (pl, Mplist, p);
		      M17N_OBJECT_UNREF (p);
		      p_copied = 1;
		      p0 = p;
		      while (n-- > 0)
			p0 = MPLIST_NEXT (p0);
		    }	  
		  mplist_pop (p0);
		  M17N_OBJECT_UNREF (mt);
		}
	    }
	  if (! MPLIST_TAIL_P (p))
	    pl = MPLIST_NEXT (pl);
	  else
	    {
	      mplist_pop (pl);
	      M17N_OBJECT_UNREF (p);
	    }
	}
    }
  if (MPLIST_TAIL_P (plist))
    {
      M17N_OBJECT_UNREF (plist);
      return NULL;
    }      
  return plist;
}

/* The returned Plist must be UNREFed.  */

static MPlist *
get_candidate_list (MInputContextInfo *ic_info, MPlist *args)
{
  MCharset *charset = get_select_charset (ic_info);
  MPlist *plist;
  int column;
  int i, len;

  plist = resolve_variable (ic_info, Mcandidates_group_size);
  column = MPLIST_INTEGER (plist);

  plist = MPLIST_PLIST (args);
  if (! plist)
    return NULL;
  if (charset)
    {
      plist = adjust_candidates (plist, charset);
      if (! plist)
	return NULL;
    }
  else
    M17N_OBJECT_REF (plist);

  if (column == 0)
    return plist;

  if (MPLIST_MTEXT_P (plist))
    {
      MText *mt = MPLIST_MTEXT (plist);
      MPlist *next = MPLIST_NEXT (plist);

      if (MPLIST_TAIL_P (next))
	M17N_OBJECT_REF (mt);
      else
	{
	  mt = mtext_dup (mt);
	  while (! MPLIST_TAIL_P (next))
	    {
	      mt = mtext_cat (mt, MPLIST_MTEXT (next));
	      next = MPLIST_NEXT (next);
	    }
	}
      M17N_OBJECT_UNREF (plist);
      plist = mplist ();
      len = mtext_nchars (mt);
      if (len <= column)
	mplist_add (plist, Mtext, mt);
      else
	{
	  for (i = 0; i < len; i += column)
	    {
	      int to = (i + column < len ? i + column : len);
	      MText *sub = mtext_copy (mtext (), 0, mt, i, to);
						       
	      mplist_add (plist, Mtext, sub);
	      M17N_OBJECT_UNREF (sub);
	    }
	}
      M17N_OBJECT_UNREF (mt);
    }
  else if (MPLIST_PLIST_P (plist))
    {
      MPlist *tail = plist;
      MPlist *new = mplist ();
      MPlist *this = mplist ();
      int count = 0;

      MPLIST_DO (tail, tail)
	{
	  MPlist *p = MPLIST_PLIST (tail);

	  MPLIST_DO (p, p)
	    {
	      MText *mt = MPLIST_MTEXT (p);

	      if (count == column)
		{
		  mplist_add (new, Mplist, this);
		  M17N_OBJECT_UNREF (this);
		  this = mplist ();
		  count = 0;
		}
	      mplist_add (this, Mtext, mt);
	      count++;
	    }
	}
      mplist_add (new, Mplist, this);
      M17N_OBJECT_UNREF (this);
      mplist_set (plist, Mnil, NULL);
      MPLIST_DO (tail, new)
	{
	  MPlist *elt = MPLIST_PLIST (tail);

	  mplist_add (plist, Mplist, elt);
	}
      M17N_OBJECT_UNREF (new);
    }

  return plist;
}


static MPlist *
regularize_action (MPlist *action_list, MInputContextInfo *ic_info)
{
  MPlist *action = NULL;
  MSymbol name;
  MPlist *args;

  if (MPLIST_SYMBOL_P (action_list))
    {
      MSymbol var = MPLIST_SYMBOL (action_list);
      MPlist *p;

      MPLIST_DO (p, ic_info->vars)
	if (MPLIST_SYMBOL (MPLIST_PLIST (p)) == var)
	  break;
      if (MPLIST_TAIL_P (p))
	return NULL;
      action = MPLIST_NEXT (MPLIST_PLIST (p));
      mplist_set (action_list, MPLIST_KEY (action), MPLIST_VAL (action));
    }

  if (MPLIST_PLIST_P (action_list))
    {
      action = MPLIST_PLIST (action_list);
      if (MPLIST_SYMBOL_P (action))
	{
	  name = MPLIST_SYMBOL (action);
	  args = MPLIST_NEXT (action);
	  if (name == Minsert
	      && MPLIST_PLIST_P (args))
	    mplist_set (action, Msymbol, M_candidates);
	}
      else if (MPLIST_MTEXT_P (action) || MPLIST_PLIST_P (action))
	{
	  action = mplist ();
	  mplist_push (action, Mplist, MPLIST_VAL (action_list));
	  mplist_push (action, Msymbol, M_candidates);
	  mplist_set (action_list, Mplist, action);
	  M17N_OBJECT_UNREF (action);
	}
    }
  else if (MPLIST_MTEXT_P (action_list) || MPLIST_INTEGER_P (action_list))
    {
      action = mplist ();
      mplist_push (action, MPLIST_KEY (action_list), MPLIST_VAL (action_list));
      mplist_push (action, Msymbol, Minsert);
      mplist_set (action_list, Mplist, action);
      M17N_OBJECT_UNREF (action);
    }
  return action;
}

/* Perform list of actions in ACTION_LIST for the current input
   context IC.  If "unhandle" action was performed or an error
   occurred, return -1.  Otherwise, return 0, 1, 2, or 3.  See the
   comment in filter () for the detail. */

static int
take_action_list (MInputContext *ic, MPlist *action_list)
{
  MInputContextInfo *ic_info = (MInputContextInfo *) ic->info;
  MTextProperty *prop;
  int result;

  MPLIST_DO (action_list, action_list)
    {
      MPlist *action = regularize_action (action_list, ic_info);
      MSymbol name;
      MPlist *args;

      if (! action)
	continue;
      name = MPLIST_SYMBOL (action);
      args = MPLIST_NEXT (action);

      MDEBUG_PRINT1 (" %s", MSYMBOL_NAME (name));
      if (name == Minsert)
	{
	  if (MPLIST_SYMBOL_P (args))
	    {
	      args = resolve_variable (ic_info, MPLIST_SYMBOL (args));
	      if (! MPLIST_MTEXT_P (args) && ! MPLIST_INTEGER_P (args))
		continue;
	    }
	  if (MPLIST_MTEXT_P (args))
	    preedit_insert (ic, ic->cursor_pos, MPLIST_MTEXT (args), 0);
	  else			/* MPLIST_INTEGER_P (args)) */
	    preedit_insert (ic, ic->cursor_pos, NULL, MPLIST_INTEGER (args));
	}
      else if (name == M_candidates)
	{
	  MPlist *plist = get_candidate_list (ic_info, args);
	  MPlist *pl;
	  int len;

	  if (! plist)
	    continue;
	  if (MPLIST_PLIST_P (plist) && MPLIST_TAIL_P (plist))
	    {
	      M17N_OBJECT_UNREF (plist);
	      continue;
	    }
	  if (MPLIST_MTEXT_P (plist))
	    {
	      preedit_insert (ic, ic->cursor_pos, NULL,
			      mtext_ref_char (MPLIST_MTEXT (plist), 0));
	      len = 1;
	    }
	  else
	    {
	      MText * mt = MPLIST_MTEXT (MPLIST_PLIST (plist));

	      preedit_insert (ic, ic->cursor_pos, mt, 0);
	      len = mtext_nchars (mt);
	    }
	  pl = mplist_copy (plist);
	  M17N_OBJECT_UNREF (plist);
	  mtext_put_prop (ic->preedit,
			  ic->cursor_pos - len, ic->cursor_pos,
			  Mcandidate_list, pl);
	  M17N_OBJECT_UNREF (pl);
	  mtext_put_prop (ic->preedit,
			  ic->cursor_pos - len, ic->cursor_pos,
			  Mcandidate_index, (void *) 0);
	}
      else if (name == Mselect)
	{
	  int start, end;
	  int code, idx, gindex;
	  int pos = ic->cursor_pos;
	  MPlist *group;
	  int idx_decided = 0;

	  if (pos == 0
	      || ! (prop = mtext_get_property (ic->preedit, pos - 1,
					       Mcandidate_list)))
	    continue;
	  idx = (int) mtext_get_prop (ic->preedit, pos - 1, Mcandidate_index);
	  group = find_candidates_group (mtext_property_value (prop), idx,
					 &start, &end, &gindex);
	  if (MPLIST_SYMBOL_P (args))
	    {
	      code = marker_code (MPLIST_SYMBOL (args), 0);
	      if (code < 0)
		{
		  args = resolve_variable (ic_info, MPLIST_SYMBOL (args));
		  if (! MPLIST_INTEGER_P (args))
		    continue;
		  idx = start + MPLIST_INTEGER (args);
		  if (idx < start || idx >= end)
		    continue;
		  idx_decided = 1;
		}		  
	    }
	  else
	    code = -1;

	  if (code != '[' && code != ']')
	    {
	      if (! idx_decided)
		idx = (start
		       + (code >= 0
			  ? new_index (NULL, ic->candidate_index - start,
				       end - start - 1, MPLIST_SYMBOL (args),
				       NULL)
			  : MPLIST_INTEGER (args)));
	      if (idx < 0)
		{
		  find_candidates_group (mtext_property_value (prop), -1,
					 NULL, &end, NULL);
		  idx = end - 1;
		}
	      else if (idx >= end
		       && MPLIST_TAIL_P (MPLIST_NEXT (group)))
		idx = 0;
	    }
	  else
	    {
	      int ingroup_index = idx - start;
	      int len;

	      group = mtext_property_value (prop);
	      len = mplist_length (group);
	      if (code == '[')
		{
		  gindex--;
		  if (gindex < 0)
		    gindex = len - 1;;
		}
	      else
		{
		  gindex++;
		  if (gindex >= len)
		    gindex = 0;
		}
	      for (idx = 0; gindex > 0; gindex--, group = MPLIST_NEXT (group))
		idx += (MPLIST_MTEXT_P (group)
			? mtext_nchars (MPLIST_MTEXT (group))
			: mplist_length (MPLIST_PLIST (group)));
	      len = (MPLIST_MTEXT_P (group)
		     ? mtext_nchars (MPLIST_MTEXT (group))
		     : mplist_length (MPLIST_PLIST (group)));
	      if (ingroup_index >= len)
		ingroup_index = len - 1;
	      idx += ingroup_index;
	    }
	  update_candidate (ic, prop, idx);
	  MDEBUG_PRINT1 ("(%d)", idx);
	}
      else if (name == Mshow)
	ic->candidate_show = 1;
      else if (name == Mhide)
	ic->candidate_show = 0;
      else if (name == Mdelete)
	{
	  int len = mtext_nchars (ic->preedit);
	  int pos;
	  int to;

	  if (MPLIST_SYMBOL_P (args)
	      && surrounding_pos (MPLIST_SYMBOL (args), &pos))
	    {
	      to = ic->cursor_pos + pos;
	      if (to < 0)
		{
		  delete_surrounding_text (ic, to);
		  to = 0;
		}
	      else if (to > len)
		{
		  delete_surrounding_text (ic, to - len);
		  to = len;
		}
	    }
	  else
	    {
	      to = (MPLIST_SYMBOL_P (args)
		    ? new_index (ic, ic->cursor_pos, len, MPLIST_SYMBOL (args),
				 ic->preedit)
		    : MPLIST_INTEGER (args));
	      if (to < 0)
		to = 0;
	      else if (to > len)
		to = len;
	      pos = to - ic->cursor_pos;
	    }
	  MDEBUG_PRINT1 ("(%d)", pos);
	  if (to < ic->cursor_pos)
	    preedit_delete (ic, to, ic->cursor_pos);
	  else if (to > ic->cursor_pos)
	    preedit_delete (ic, ic->cursor_pos, to);
	}
      else if (name == Mmove)
	{
	  int len = mtext_nchars (ic->preedit);
	  int pos
	    = (MPLIST_SYMBOL_P (args)
	       ? new_index (ic, ic->cursor_pos, len, MPLIST_SYMBOL (args),
			    ic->preedit)
	       : MPLIST_INTEGER (args));

	  if (pos < 0)
	    pos = 0;
	  else if (pos > len)
	    pos = len;
	  if (pos != ic->cursor_pos)
	    {
	      ic->cursor_pos = pos;
	      ic->preedit_changed = 1;
	    }
	  MDEBUG_PRINT1 ("(%d)", ic->cursor_pos);
	}
      else if (name == Mmark)
	{
	  int code = marker_code (MPLIST_SYMBOL (args), 0);

	  if (code < 0)
	    {
	      mplist_put (ic_info->markers, MPLIST_SYMBOL (args),
			  (void *) ic->cursor_pos);
	      MDEBUG_PRINT1 ("(%d)", ic->cursor_pos);
	    }
	}
      else if (name == Mpushback)
	{
	  if (MPLIST_INTEGER_P (args) || MPLIST_SYMBOL_P (args))
	    {
	      int num;

	      if (MPLIST_SYMBOL_P (args))
		{
		  args = resolve_variable (ic_info, MPLIST_SYMBOL (args));
		  if (MPLIST_INTEGER_P (args))
		    num = MPLIST_INTEGER (args);
		  else
		    num = 0;
		}
	      else
		num = MPLIST_INTEGER (args);

	      if (num > 0)
		ic_info->key_head -= num;
	      else if (num == 0)
		ic_info->key_head = 0;
	      else
		ic_info->key_head = - num;
	      if (ic_info->key_head > ic_info->used)
		ic_info->key_head = ic_info->used;
	    }
	  else if (MPLIST_MTEXT_P (args))
	    {
	      MText *mt = MPLIST_MTEXT (args);
	      int i, len = mtext_nchars (mt);
	      MSymbol key;

	      ic_info->key_head--;
	      for (i = 0; i < len; i++)
		{
		  key = one_char_symbol[MTEXT_DATA (mt)[i]];
		  if (ic_info->key_head + i < ic_info->used)
		    ic_info->keys[ic_info->key_head + i] = key;
		  else
		    MLIST_APPEND1 (ic_info, keys, key, MERROR_IM);
		}
	    }
	  else
	    {
	      MPlist *plist = MPLIST_PLIST (args), *pl;
	      int i = 0;
	      MSymbol key;

	      ic_info->key_head--;

	      MPLIST_DO (pl, plist)
		{
		  key = MPLIST_SYMBOL (pl);
		  if (ic_info->key_head < ic_info->used)
		    ic_info->keys[ic_info->key_head + i] = key;
		  else
		    MLIST_APPEND1 (ic_info, keys, key, MERROR_IM);
		  i++;
		}
	    }
	}
      else if (name == Mpop)
	{
	  if (ic_info->key_head < ic_info->used)
	    MLIST_DELETE1 (ic_info, keys, ic_info->key_head, 1);
	}
      else if (name == Mcall)
	{
	  MInputMethodInfo *im_info = (MInputMethodInfo *) ic->im->info;
	  MIMExternalFunc func = NULL;
	  MSymbol module, func_name;
	  MPlist *func_args, *val;

	  result = 0;
	  module = MPLIST_SYMBOL (args);
	  args = MPLIST_NEXT (args);
	  func_name = MPLIST_SYMBOL (args);

	  if (im_info->externals)
	    {
	      MIMExternalModule *external
		= (MIMExternalModule *) mplist_get (im_info->externals,
						    module);
	      if (external)
		func = ((MIMExternalFunc)
			mplist_get_func (external->func_list, func_name));
	    }
	  if (! func)
	    continue;
	  func_args = mplist ();
	  mplist_add (func_args, Mt, ic);
	  MPLIST_DO (args, MPLIST_NEXT (args))
	    {
	      int code;

	      if (MPLIST_KEY (args) == Msymbol
		  && MPLIST_KEY (args) != Mnil
		  && (code = marker_code (MPLIST_SYMBOL (args), 0)) >= 0)
		{
		  code = new_index (ic, ic->cursor_pos, 
				    mtext_nchars (ic->preedit),
				    MPLIST_SYMBOL (args), ic->preedit);
		  mplist_add (func_args, Minteger, (void *) code);
		}
	      else
		mplist_add (func_args, MPLIST_KEY (args), MPLIST_VAL (args));
	    }
	  val = (func) (func_args);
	  M17N_OBJECT_UNREF (func_args);
	  if (val && ! MPLIST_TAIL_P (val))
	    result = take_action_list (ic, val);
	  M17N_OBJECT_UNREF (val);
	  if (result != 0)
	    return result;
	}
      else if (name == Mshift)
	{
	  shift_state (ic, MPLIST_SYMBOL (args));
	}
      else if (name == Mundo)
	{
	  int intarg = (MPLIST_TAIL_P (args)
			? ic_info->used - 2
			: integer_value (ic, args, 0));

	  mtext_reset (ic->preedit);
	  mtext_reset (ic_info->preedit_saved);
	  mtext_reset (ic->produced);
	  M17N_OBJECT_UNREF (ic_info->vars);
	  ic_info->vars = mplist_copy (ic_info->vars_saved);
	  ic->cursor_pos = ic_info->state_pos = 0;
	  ic_info->state_key_head = ic_info->key_head
	    = ic_info->commit_key_head = 0;

	  shift_state (ic, Mnil);
	  if (intarg < 0)
	    {
	      if (MPLIST_TAIL_P (args))
		{
		  ic_info->used = 0;
		  return -1;
		}
	      ic_info->used += intarg;
	    }
	  else
	    ic_info->used = intarg;
	  break;
	}
      else if (name == Mset || name == Madd || name == Msub
	       || name == Mmul || name == Mdiv)
	{
	  MSymbol sym = MPLIST_SYMBOL (args);
	  MPlist *value = resolve_variable (ic_info, sym);
	  int val1, val2;
	  char *op;

	  val1 = MPLIST_INTEGER (value);
	  args = MPLIST_NEXT (args);
	  val2 = resolve_expression (ic, args);
	  if (name == Mset)
	    val1 = val2, op = "=";
	  else if (name == Madd)
	    val1 += val2, op = "+=";
	  else if (name == Msub)
	    val1 -= val2, op = "-=";
	  else if (name == Mmul)
	    val1 *= val2, op = "*=";
	  else
	    val1 /= val2, op = "/=";
	  MDEBUG_PRINT4 ("(%s %s 0x%X(%d))",
			 MSYMBOL_NAME (sym), op, val1, val1);
	  mplist_set (value, Minteger, (void *) val1);
	}
      else if (name == Mequal || name == Mless || name == Mgreater
	       || name == Mless_equal || name == Mgreater_equal)
	{
	  int val1, val2;
	  MPlist *actions1, *actions2;

	  val1 = resolve_expression (ic, args);
	  args = MPLIST_NEXT (args);
	  val2 = resolve_expression (ic, args);
	  args = MPLIST_NEXT (args);
	  actions1 = MPLIST_PLIST (args);
	  args = MPLIST_NEXT (args);
	  if (MPLIST_TAIL_P (args))
	    actions2 = NULL;
	  else
	    actions2 = MPLIST_PLIST (args);
	  MDEBUG_PRINT3 ("(%d %s %d)? ", val1, MSYMBOL_NAME (name), val2);
	  if (name == Mequal ? val1 == val2
	      : name == Mless ? val1 < val2
	      : name == Mgreater ? val1 > val2
	      : name == Mless_equal ? val1 <= val2
	      : val1 >= val2)
	    {
	      MDEBUG_PRINT ("ok");
	      result = take_action_list (ic, actions1);
	    }
	  else
	    {
	      MDEBUG_PRINT ("no");
	      if (actions2)
		result = take_action_list (ic, actions2);
	    }
	  if (result != 0)
	    return result;
	}
      else if (name == Mcond)
	{
	  int idx = 0;

	  MPLIST_DO (args, args)
	    {
	      MPlist *cond;

	      idx++;
	      if (! MPLIST_PLIST (args))
		continue;
	      cond = MPLIST_PLIST (args);
	      if (resolve_expression (ic, cond) != 0)
		{
		  MDEBUG_PRINT1 ("(%dth)", idx);
		  result = take_action_list (ic, MPLIST_NEXT (cond));
		  if (result != 0)
		    return result;
		  break;
		}
	    }
	}
      else if (name == Mcommit)
	{
	  preedit_commit (ic, 0);
	}
      else if (name == Munhandle)
	{
	  preedit_commit (ic, 0);
	  return -1;
	}
      else if (name == Mswitch_im)
	{
	  ic_info->pushing_or_switching = args;
	  M17N_OBJECT_REF (args);
	  return 1;
	}
      else if (name == Mpush_im)
	{
	  ic_info->pushing_or_switching = args;
	  M17N_OBJECT_REF (args);
	  return 2;
	}
      else if (name == Mpop_im)
	{
	  shift_state (ic, Mnil);
	  return 3;
	}
      else
	{
	  MInputMethodInfo *im_info = (MInputMethodInfo *) ic->im->info;
	  MPlist *actions;

	  if (im_info->macros
	      && (actions = mplist_get (im_info->macros, name)))
	    {
	      result = take_action_list (ic, actions);
	      if (result != 0)
		return result;
	    };
	}
    }
  return 0;
}


/* Handle the input key KEY in the current state and map specified in
   the input context IC.  If KEY was handled correctly, return 0
   (without im switching) or a positive number (with im switching).
   Otherwise, return -1.  */

static int
handle_key (MInputContext *ic)
{
  MInputMethodInfo *im_info = (MInputMethodInfo *) ic->im->info;
  MInputContextInfo *ic_info = (MInputContextInfo *) ic->info;
  MIMMap *map = ic_info->map;
  MIMMap *submap = NULL;
  MSymbol key = ic_info->keys[ic_info->key_head];
  MSymbol alias = Mnil;
  int result;
  int i;

  if (ic_info->state_hook)
    {
      MDEBUG_PRINT3 ("  [IM:%s-%s] [%s] init-actions:",
		     MSYMBOL_NAME (im_info->language),
		     MSYMBOL_NAME (im_info->name),
		     MSYMBOL_NAME (ic_info->state->name));
      result = take_action_list (ic, ic_info->state_hook);
      mtext_cpy (ic_info->preedit_saved, ic->preedit);
      ic_info->state_pos = ic->cursor_pos;
      ic_info->state_hook = NULL;
      if (result != 0)
	{
	  MDEBUG_PRINT ("\n");
	  return result;
	}
    }

  MDEBUG_PRINT4 ("  [IM:%s-%s] [%s] handle `%s'", 
		 MSYMBOL_NAME (im_info->language),
		 MSYMBOL_NAME (im_info->name),
		 MSYMBOL_NAME (ic_info->state->name), msymbol_name (key));

  if (map->submaps)
    {
      submap = mplist_get (map->submaps, key);
      alias = key;
      while (! submap
	     && (alias = msymbol_get (alias, M_key_alias))
	     && alias != key)
	submap = mplist_get (map->submaps, alias);
    }

  if (submap)
    {
      if (! alias || alias == key)
	MDEBUG_PRINT (" submap-found");
      else
	MDEBUG_PRINT1 (" submap-found (by alias `%s')", MSYMBOL_NAME (alias));
      mtext_cpy (ic->preedit, ic_info->preedit_saved);
      ic->preedit_changed = 1;
      ic->cursor_pos = ic_info->state_pos;
      ic_info->key_head++;
      ic_info->map = map = submap;
      if (map->map_actions)
	{
	  MDEBUG_PRINT (" map-actions:");
	  result = take_action_list (ic, map->map_actions);
	  if (result != 0)
	    {
	      MDEBUG_PRINT ("\n");
	      return result;
	    }
	}
      else if (map->submaps)
	{
	  for (i = ic_info->state_key_head; i < ic_info->key_head; i++)
	    {
	      MSymbol key = ic_info->keys[i];
	      char *name = msymbol_name (key);

	      if (! name[0] || ! name[1])
		mtext_ins_char (ic->preedit, ic->cursor_pos++, name[0], 1);
	    }
	}

      /* If this is the terminal map or we have shifted to another
	 state, perform branch actions (if any).  */
      if (! map->submaps || map != ic_info->map)
	{
	  if (map->branch_actions)
	    {
	      MDEBUG_PRINT (" branch-actions:");
	      result = take_action_list (ic, map->branch_actions);
	      if (result != 0)
		{
		  MDEBUG_PRINT ("\n");
		  return result;
		}
	    }
	  /* If MAP is still not the root map, shift to the current
	     state.  */
	  if (ic_info->map != ic_info->state->map)
	    shift_state (ic, ic_info->state->name);
	}
    }
  else
    {
      /* MAP can not handle KEY.  */

      /* Perform branch actions if any.  */
      if (map->branch_actions)
	{
	  MDEBUG_PRINT (" branch-actions:");
	  result = take_action_list (ic, map->branch_actions);
	  if (result != 0)
	    {
	      MDEBUG_PRINT ("\n");
	      return result;
	    }
	}

      if (map == ic_info->map)
	{
	  /* The above branch actions didn't change the state.  */

	  /* If MAP is the root map of the initial state, and there
	     still exist an unhandled key, it means that the current
	     input method can not handle it.  */
	  if (map == ((MIMState *) MPLIST_VAL (im_info->states))->map
	      && ic_info->key_head < ic_info->used)
	    {
	      MDEBUG_PRINT (" unhandled\n");
	      ic_info->state_hook = map->map_actions;
	      return -1;
	    }

	  if (map != ic_info->state->map)
	    {
	      /* MAP is not the root map.  Shift to the root map of the
		 current state. */
	      shift_state (ic, ic_info->state->name);
	    }
	  else if (! map->branch_actions)
	    {
	      /* MAP is the root map without any default branch
		 actions.  Shift to the initial state.  */
	      shift_state (ic, Mnil);
	    }
	}
    }
  MDEBUG_PRINT ("\n");
  return 0;
}

struct MIMInputStack
{
  MInputMethodInfo *im_info;
  MInputContextInfo *ic_info;
};

static void
pop_im (MInputContext *ic)
{
  MInputContextInfo *ic_info = (MInputContextInfo *) ic->info;
  MInputMethodInfo *im_info = (MInputMethodInfo *) ic->im->info;
  int i;

  shift_state (ic, Mnil);
  for (i = 0; i < ic_info->used; i++)
    MLIST_APPEND1 (ic_info->stack->ic_info, keys, ic_info->keys[i], MERROR_IM);
  ic_info->stack->ic_info->key_head = 0;
  ic_info->stack->ic_info->state_key_head = 0;
  ic_info->stack->ic_info->commit_key_head = 0;
  ic_info->stack->ic_info->used = ic_info->used;
  ic_info->used = ic_info->key_head
    = ic_info->state_key_head = ic_info->commit_key_head = 0;

  MDEBUG_PRINT2 ("\n  [IM:%s-%s] poped",
		 MSYMBOL_NAME (im_info->language),
		 MSYMBOL_NAME (im_info->name));
  ic->im->info = ic_info->stack->im_info;
  ic->info = ic_info->stack->ic_info;
  /*ic_info = (MInputContextInfo *) ic->info;*/
  free (ic_info->stack);
  ic_info->stack = NULL;
  ic->status = ((MInputMethodInfo *) (ic->im->info))->title;
  ic->status_changed = ic->preedit_changed = ic->candidates_changed = 1;
}

static MInputContextInfo *
push_im (MInputContext *ic, MInputContext *pushing)
{
  MInputContextInfo *ic_info = (MInputContextInfo *) ic->info;
  MInputContextInfo *new_info;
  MIMInputStack *stack;
  MText *produced;
  int i;

  if (! pushing)
    {
      pushing = create_ic_for_im (ic_info->pushing_or_switching, ic->im);
      if (! pushing)
	{
	  free (stack);
	  MERROR (MERROR_IM, NULL);
	}
    }

  new_info = pushing->info;
  for (i = 0; i < ic_info->used; i++)
    MLIST_APPEND1 (new_info, keys, ic_info->keys[i], MERROR_IM);
  new_info->key_head = 0;
  new_info->state_key_head = 0;
  new_info->commit_key_head = 0;
  ic_info->used = 0;
  ic_info->key_head = ic_info->state_key_head = ic_info->commit_key_head = 0;
  MSTRUCT_CALLOC_SAFE (stack);
  if (! stack)
    MERROR (MERROR_IM, NULL);
  stack->im_info = (MInputMethodInfo *)ic->im->info;
  stack->ic_info = ic_info;
  M17N_OBJECT_UNREF (ic_info->pushing_or_switching);
  ic->im->info = pushing->im->info;
  ic->info = pushing->info;
  ic->status = pushing->status;
  ic->status_changed = 1;
  ic_info = (MInputContextInfo *) ic->info;
  ic_info->stack = stack;
  MDEBUG_PRINT2 ("\n  [IM:%s-%s] pushed", 
		 MSYMBOL_NAME (pushing->im->language),
		 MSYMBOL_NAME (pushing->im->name));
  return ic_info;
}
 
/* Initialize IC->ic_info of which members are all zero cleared now.  */

static void
init_ic_info (MInputContext *ic)
{
  MInputMethodInfo *im_info = ic->im->info;
  MInputContextInfo *ic_info = ic->info;
  MPlist *plist;
  
  MLIST_INIT1 (ic_info, keys, 8);;

  ic_info->markers = mplist ();

  ic_info->vars = mplist ();
  /* If this input method has its own setting of variables, record
     those values in ic_info->vars.  */
  if (im_info->configured_vars)
    MPLIST_DO (plist, im_info->configured_vars)
      {
	MPlist *pl = MPLIST_PLIST (plist);
	MSymbol name = MPLIST_SYMBOL (pl);

	pl = MPLIST_NEXT (MPLIST_NEXT (MPLIST_NEXT (pl)));
	if (MPLIST_KEY (pl) != Mt)
	  {
	    MPlist *p = mplist ();

	    mplist_push (ic_info->vars, Mplist, p);
	    M17N_OBJECT_UNREF (p);
	    mplist_add (p, Msymbol, name);
	    mplist_add (p, MPLIST_KEY (pl), MPLIST_VAL (pl));
	  }
      }
  /* Remember the original values.  */
  ic_info->vars_saved = mplist_copy (ic_info->vars);

  /* If this input method uses external modules, initialize them.  */
  if (im_info->externals)
    {
      MPlist *func_args = mplist (), *plist;

      mplist_add (func_args, Mt, ic);
      MPLIST_DO (plist, im_info->externals)
	{
	  MIMExternalModule *external = MPLIST_VAL (plist);
	  MIMExternalFunc func
	    = (MIMExternalFunc) mplist_get_func (external->func_list, Minit);

	  if (func)
	    (func) (func_args);
	}
      M17N_OBJECT_UNREF (func_args);
    }

  ic_info->preedit_saved = mtext ();

  if (fallback_input_methods)
    {
      /* Record MInputContext of each fallback input method in
	 ic_info->fallbacks in the preference order.  Note that the
	 input methods in fallback_input_methods are in reverse
	 preference order.  */
      MPlist *fallbacks = fallback_input_methods;

      /* To prevent this part being called recursively via
	 create_ic_for_im below.  */
      fallback_input_methods = NULL;
      ic_info->fallbacks = mplist ();
      MPLIST_DO (plist, fallbacks)
	{
	  MInputContext *fallback_ic
	    = create_ic_for_im (MPLIST_PLIST (plist), ic->im);
	  if (fallback_ic)
	    mplist_push (ic_info->fallbacks, Mt, fallback_ic);
	}
      fallback_input_methods = fallbacks;
    }

  /* Remember the tick of this input method to know when to
     re-initialize ic_info.  */
  ic_info->tick = im_info->tick;
}

/* Finalize IC->ic_info.  */

static void
fini_ic_info (MInputContext *ic)
{
  MInputMethodInfo *im_info;
  MInputContextInfo *ic_info;
  MPlist *plist;

  if (((MInputContextInfo *) ic->info)->stack)
    pop_im (ic);
  im_info = ic->im->info;
  ic_info = ic->info;
  if (ic_info->fallbacks)
    {
      MPLIST_DO (plist, ic_info->fallbacks)
	{
	  MInputContext *ic = MPLIST_VAL (plist);
	  MInputMethod *im = ic->im;

	  minput_destroy_ic (ic);
	  minput_close_im (im);
	}
      M17N_OBJECT_UNREF (ic_info->fallbacks);
    }
  if (im_info->externals)
    {
      MPlist *func_args = mplist (), *plist;

      mplist_add (func_args, Mt, ic);
      MPLIST_DO (plist, im_info->externals)
	{
	  MIMExternalModule *external = MPLIST_VAL (plist);
	  MIMExternalFunc func
	    = (MIMExternalFunc) mplist_get_func (external->func_list, Mfini);

	  if (func)
	    (func) (func_args);
	}
      M17N_OBJECT_UNREF (func_args);
    }

  MLIST_FREE1 (ic_info, keys);
  M17N_OBJECT_UNREF (ic_info->preedit_saved);
  M17N_OBJECT_UNREF (ic_info->markers);
  M17N_OBJECT_UNREF (ic_info->vars);
  M17N_OBJECT_UNREF (ic_info->vars_saved);
  M17N_OBJECT_UNREF (ic_info->preceding_text);
  M17N_OBJECT_UNREF (ic_info->following_text);
  M17N_OBJECT_UNREF (ic_info->pushing_or_switching);

  /* Note that we can clear ic_info->fallbacks and ic_info->stack too
     because they should be saved and restored by the caller if
     necessary.  */
  memset (ic_info, 0, sizeof (MInputContextInfo));
}

static void
re_init_ic (MInputContext *ic, int reload)
{
  MInputMethodInfo *im_info = (MInputMethodInfo *) ic->im->info;
  MInputContextInfo *ic_info = (MInputContextInfo *) ic->info;
  int status_changed, preedit_changed, cursor_pos_changed, candidates_changed;
  /* Remember these now.  They are cleared by fini_ic_info ().  */
  MIMInputStack *stack = ic_info->stack;

  status_changed = ic_info->state != (MIMState *) MPLIST_VAL (im_info->states);
  preedit_changed = mtext_nchars (ic->preedit) > 0;
  cursor_pos_changed = ic->cursor_pos > 0;
  candidates_changed = 0;
  if (ic->candidate_list)
    {
      candidates_changed |= MINPUT_CANDIDATES_LIST_CHANGED;
      M17N_OBJECT_UNREF (ic->candidate_list);
      ic->candidate_list = NULL;
    }
  if (ic->candidate_show)
    {
      candidates_changed |= MINPUT_CANDIDATES_SHOW_CHANGED;
      ic->candidate_show = 0;
    }
  if (ic->candidate_index > 0)
    {
      candidates_changed |= MINPUT_CANDIDATES_INDEX_CHANGED;
      ic->candidate_index = 0;
      ic->candidate_from = ic->candidate_to = 0;
    }
  if (mtext_nchars (ic->produced) > 0)
    mtext_reset (ic->produced);
  if (mtext_nchars (ic->preedit) > 0)
    mtext_reset (ic->preedit);
  ic->cursor_pos = 0;
  M17N_OBJECT_UNREF (ic->plist);
  ic->plist = mplist ();

  fini_ic_info (ic);
  if (reload)
    reload_im_info (im_info);
  if (! im_info->states)
    {
      struct MIMState *state;

      M17N_OBJECT (state, free_state, MERROR_IM);
      state->name = msymbol ("init");
      state->title = mtext__from_data ("ERROR!", 6, MTEXT_FORMAT_US_ASCII, 0);
      MSTRUCT_CALLOC (state->map, MERROR_IM);
      im_info->states = mplist ();
      mplist_add (im_info->states, state->name, state);
    }
  init_ic_info (ic);
  /* Restore them now.  */
  ic_info->stack = stack;
  shift_state (ic, Mnil);

  ic->status_changed = status_changed;
  ic->preedit_changed = preedit_changed;
  ic->cursor_pos_changed = cursor_pos_changed;
  ic->candidates_changed = candidates_changed;
}

static void
reset_ic (MInputContext *ic, MSymbol ignore)
{
  MInputMethodInfo *im_info = ic->im->info;
  MDEBUG_PRINT2 ("\n  [IM:%s-%s] reset\n", 
		 MSYMBOL_NAME (im_info->language),
		 MSYMBOL_NAME (im_info->name));
  re_init_ic (ic, 0);
}

static int
open_im (MInputMethod *im)
{
  MInputMethodInfo *im_info = get_im_info (im->language, im->name, Mnil, Mnil);

  if (! im_info || ! im_info->states || MPLIST_LENGTH (im_info->states) == 0)
    MERROR (MERROR_IM, -1);
  im->info = im_info;

  return 0;
}

static void
close_im (MInputMethod *im)
{
  im->info = NULL;
}

static int
create_ic (MInputContext *ic)
{
  MInputContextInfo *ic_info;

  MSTRUCT_CALLOC (ic_info, MERROR_IM);
  ic->info = ic_info;
  init_ic_info (ic);
  shift_state (ic, Mnil);
  return 0;
}

static void
destroy_ic (MInputContext *ic)
{
  fini_ic_info (ic);
  free (ic->info);
}


/* Return 1 if KEY is for invoking COMMAND.  Otherwise, return 0.   */

static int
check_command_key (MInputContext *ic, MSymbol key, MSymbol command)
{
  MInputMethodInfo *im_info = ic->im->info;
  MPlist *plist = resolve_command (im_info->configured_cmds, command);

  if (! plist)
    {
      plist = resolve_command (global_info->configured_cmds, command);
      if (! plist)
	return 0;
    }
  MPLIST_DO (plist, plist)
    {
      MSymbol this_key, alias;

      if (MPLIST_MTEXT_P (plist))
	{
	  MText *mt = MPLIST_MTEXT (plist);
	  int c = mtext_ref_char (mt, 0);

	  if (c >= 256)
	    continue;
	  this_key = one_char_symbol[c];
	}
      else
	{
	  MPlist *pl = MPLIST_PLIST (plist);
      
	  this_key = MPLIST_SYMBOL (pl);
	}
      alias = this_key;
      while (alias != key 
	     && (alias = msymbol_get (alias, M_key_alias))
	     && alias != this_key);
      if (alias == key)
	break;
    }
  return ! MPLIST_TAIL_P (plist);
}

static int
check_reload (MInputContext *ic, MSymbol key)
{
  MInputMethodInfo *im_info = ic->im->info;
  if (! check_command_key (ic, key, Mat_reload))
    return 0;
  MDEBUG_PRINT2 ("\n  [IM:%s-%s] reload",
		 MSYMBOL_NAME (im_info->language),
		 MSYMBOL_NAME (im_info->name));
  re_init_ic (ic, 1);
  return 1;
}

static MInputContext *
check_fallback (MInputContext *ic, MSymbol key)
{
  MInputContextInfo *ic_info = ic->info;
  MPlist *plist;

  MPLIST_DO (plist, ic_info->fallbacks)
    {
      MSymbol alias = key;
      MInputContext *this_ic = (MInputContext *) MPLIST_VAL (plist);
      MInputMethodInfo *this_im_info = (MInputMethodInfo * )this_ic->im->info;
      MIMMap *map = ((MIMState *) MPLIST_VAL (this_im_info->states))->map;
      MIMMap *submap;
      
      if (! map->submaps)
	continue;
      submap = mplist_get (map->submaps, key);
      while (! submap
	     && (alias = msymbol_get (alias, M_key_alias))
	     && alias != key)
	submap = mplist_get (map->submaps, alias);
      if (submap)
	return this_ic;
    }
  return NULL;
}


/** Handle the input key KEY in the current state and map of IC->info.
    If KEY is handled but no text is produced, return 0, otherwise
    return 1.

    Ignore ARG.  */

static int
filter (MInputContext *ic, MSymbol key, void *arg)
{
  MInputMethodInfo *im_info = (MInputMethodInfo *) ic->im->info;
  MInputContextInfo *ic_info = (MInputContextInfo *) ic->info;
  int count = 0;
  /* The result of handling keys.  
     -1: Not handled, or an error occured
      0: Handled normally without changing IM
      1: IM switch
      2: IM pushed
      3: IM poped */
  int result;

  if (check_reload (ic, key))
    return 0;

  if (! ic_info->state)
    {
      ic_info->key_unhandled = 1;
      return 0;
    }
  mtext_reset (ic->produced);
  ic->status_changed = ic->preedit_changed = ic->candidates_changed = 0;
  MLIST_APPEND1 (ic_info, keys, key, MERROR_IM);
 repeat:
  M17N_OBJECT_UNREF (ic_info->preceding_text);
  M17N_OBJECT_UNREF (ic_info->following_text);
  ic_info->preceding_text = ic_info->following_text = NULL;
  ic_info->key_unhandled = 0;

  do {
    MPlist *candidate_list = ic->candidate_list;
    int candidate_index = ic->candidate_index;
    int candidate_show = ic->candidate_show;
    MTextProperty *prop;

    result = handle_key (ic);
    if (ic->candidate_list)
      {
	M17N_OBJECT_UNREF (ic->candidate_list);
	ic->candidate_list = NULL;
      }
    if (ic->cursor_pos > 0
	&& (prop = mtext_get_property (ic->preedit, ic->cursor_pos - 1,
				       Mcandidate_list)))
      {
	ic->candidate_list = mtext_property_value (prop);
	M17N_OBJECT_REF (ic->candidate_list);
	ic->candidate_index
	  = (int) mtext_get_prop (ic->preedit, ic->cursor_pos - 1,
				  Mcandidate_index);
	ic->candidate_from = mtext_property_start (prop);
	ic->candidate_to = mtext_property_end (prop);
      }
    if (candidate_list != ic->candidate_list)
      ic->candidates_changed |= MINPUT_CANDIDATES_LIST_CHANGED;
    if (candidate_index != ic->candidate_index)
      ic->candidates_changed |= MINPUT_CANDIDATES_INDEX_CHANGED;
    if (candidate_show != ic->candidate_show)
      ic->candidates_changed |= MINPUT_CANDIDATES_SHOW_CHANGED;    

    if (count++ == 100)
      {
	/* Insane number of iteration, perhaps a bug.  */
	mdebug_hook ();
	reset_ic (ic, Mnil);
	ic_info->key_unhandled = 1;
	break;
      }
    /* Break the loop if all keys were handled.  */
  } while (result == 0 && ic_info->key_head < ic_info->used);

  /* If the current map is the root of the initial state, we should
     produce any preedit text in ic->produced, and if the current
     input method is a pushed one, pop it.  */
  if (ic_info->map == ((MIMState *) MPLIST_VAL (im_info->states))->map)
    {
      preedit_commit (ic, 1);
      if (ic_info->stack)
	result = 3;
    }

  if (mtext_nchars (ic->produced) > 0)
    {
      if (MDEBUG_FLAG ())
	{
	  int i;

	  MDEBUG_PRINT3 ("\n  [IM:%s-%s] [%s] (produced",
			 MSYMBOL_NAME (im_info->language),
			 MSYMBOL_NAME (im_info->name),
			 MSYMBOL_NAME (ic_info->state->name));
	  for (i = 0; i < mtext_nchars (ic->produced); i++)
	    MDEBUG_PRINT1 (" U+%04X", mtext_ref_char (ic->produced, i));
	  MDEBUG_PRINT (")");
	}

      mtext_put_prop (ic->produced, 0, mtext_nchars (ic->produced),
       		      Mlanguage, ic->im->language);
    }

  if (ic_info->commit_key_head > 0)
    {
      memmove (ic_info->keys, ic_info->keys + ic_info->commit_key_head,
	       sizeof (MSymbol *) * (ic_info->used - ic_info->commit_key_head));
      ic_info->used -= ic_info->commit_key_head;
      ic_info->key_head -= ic_info->commit_key_head;
      ic_info->state_key_head -= ic_info->commit_key_head;
      ic_info->commit_key_head = 0;
    }

  if (result != 0)
    {
      MInputContext *pushing = NULL;

      if (ic_info->key_head > 0)
	{
	  memmove (ic_info->keys, ic_info->keys + ic_info->key_head,
		   sizeof (MSymbol *) * (ic_info->used - ic_info->key_head));
	  ic_info->used -= ic_info->key_head;
	  ic_info->key_head = ic_info->state_key_head
	    = ic_info->commit_key_head = 0;
	}

      if (result < 0)
	{
	  /* KEY was not handled.  If the current input method was not
	     the pushed one, check if it is a key for one of fallback
	     input methods.  */
	  if (! ic_info->stack)
	    {
	      pushing = check_fallback (ic, ic_info->keys[ic_info->key_head]);
	      if (pushing)
		result = 2;
	    }
	}
      if (result < 0)
	{
	  ic_info->used = 0;
	  ic_info->key_head = ic_info->state_key_head
	    = ic_info->commit_key_head = 0;
	  ic_info->key_unhandled = 1;
	}
      else if (result == 3)	/* pop */
	{
	  pop_im (ic);
	  ic_info = ic->info;
	  im_info = ic->im->info;
	  if (ic_info->key_head < ic_info->used)
	    goto repeat;
	}
      else if (result == 2)	/* push */
	{
	  ic_info = push_im (ic, pushing);
	  if (ic_info)
	    {
	      im_info = ic->im->info;
	      goto repeat;
	    }
	}
      else			/* (result == 1) switch */
	{
	}
    }

  return (! ic_info->key_unhandled && mtext_nchars (ic->produced) == 0);
}


/** Return 1 if the last event or key was not handled, otherwise
    return 0.

    There is no need of looking up because ic->produced should already
    contain the produced text (if any).

    Ignore KEY.  */

static int
lookup (MInputContext *ic, MSymbol key, void *arg, MText *mt)
{
  mtext_cat (mt, ic->produced);
  mtext_reset (ic->produced);
  return (((MInputContextInfo *) ic->info)->key_unhandled ? -1 : 0);
}


/* Input method command handler.  */

/* List of all (global and local) commands. 
   (LANG:(IM-NAME:(COMMAND ...) ...) ...) ...
   COMMAND is CMD-NAME:(mtext:DESCRIPTION plist:KEYSEQ ...))
   Global commands are stored as (t (t COMMAND ...))  */


/* Input method variable handler.  */


/* Support functions for mdebug_dump_im.  */

static void
dump_im_map (MPlist *map_list, int indent)
{
  char *prefix;
  MSymbol key = MPLIST_KEY (map_list);
  MIMMap *map = (MIMMap *) MPLIST_VAL (map_list);

  prefix = (char *) alloca (indent + 1);
  memset (prefix, 32, indent);
  prefix[indent] = '\0';

  fprintf (mdebug__output, "(\"%s\" ", msymbol_name (key));
  if (map->map_actions)
    mdebug_dump_plist (map->map_actions, indent + 2);
  if (map->submaps)
    {
      MPLIST_DO (map_list, map->submaps)
	{
	  fprintf (mdebug__output, "\n%s  ", prefix);
	  dump_im_map (map_list, indent + 2);
	}
    }
  if (map->branch_actions)
    {
      fprintf (mdebug__output, "\n%s  (branch\n%s    ", prefix, prefix);
      mdebug_dump_plist (map->branch_actions, indent + 4);
      fprintf (mdebug__output, ")");      
    }
  fprintf (mdebug__output, ")");
}


static void
dump_im_state (MIMState *state, int indent)
{
  char *prefix;
  MPlist *map_list;

  prefix = (char *) alloca (indent + 1);
  memset (prefix, 32, indent);
  prefix[indent] = '\0';

  fprintf (mdebug__output, "(%s", msymbol_name (state->name));
  if (state->map->submaps)
    {
      MPLIST_DO (map_list, state->map->submaps)
	{
	  fprintf (mdebug__output, "\n%s  ", prefix);
	  dump_im_map (map_list, indent + 2);
	}
    }
  fprintf (mdebug__output, ")");
}



int
minput__init ()
{
  Minput_driver = msymbol ("input-driver");

  Minput_preedit_start = msymbol ("input-preedit-start");
  Minput_preedit_done = msymbol ("input-preedit-done");
  Minput_preedit_draw = msymbol ("input-preedit-draw");
  Minput_status_start = msymbol ("input-status-start");
  Minput_status_done = msymbol ("input-status-done");
  Minput_status_draw = msymbol ("input-status-draw");
  Minput_candidates_start = msymbol ("input-candidates-start");
  Minput_candidates_done = msymbol ("input-candidates-done");
  Minput_candidates_draw = msymbol ("input-candidates-draw");
  Minput_set_spot = msymbol ("input-set-spot");
  Minput_focus_move = msymbol ("input-focus-move");
  Minput_focus_in = msymbol ("input-focus-in");
  Minput_focus_out = msymbol ("input-focus-out");
  Minput_toggle = msymbol ("input-toggle");
  Minput_reset = msymbol ("input-reset");
  Minput_get_surrounding_text = msymbol ("input-get-surrounding-text");
  Minput_delete_surrounding_text = msymbol ("input-delete-surrounding-text");
  Mcustomized = msymbol ("customized");
  Mconfigured = msymbol ("configured");
  Minherited = msymbol ("inherited");

  minput_default_driver.open_im = open_im;
  minput_default_driver.close_im = close_im;
  minput_default_driver.create_ic = create_ic;
  minput_default_driver.destroy_ic = destroy_ic;
  minput_default_driver.filter = filter;
  minput_default_driver.lookup = lookup;
  minput_default_driver.callback_list = mplist ();
  mplist_put_func (minput_default_driver.callback_list, Minput_reset,
		   M17N_FUNC (reset_ic));
  minput_driver = &minput_default_driver;

  fully_initialized = 0;
  return 0;
}

void
minput__fini ()
{
  if (fully_initialized)
    {
      MDEBUG_PRINT ("freeing im_info_list\n");
      free_im_list (im_info_list);
      MDEBUG_PRINT ("freeing im_custom_list\n");
      if (im_custom_list)
	free_im_list (im_custom_list);
      MDEBUG_PRINT ("freeing im_config_list\n");
      if (im_config_list)
	free_im_list (im_config_list);
      M17N_OBJECT_UNREF (load_im_info_keys);
      M17N_OBJECT_UNREF (fallback_input_methods);
    }

  M17N_OBJECT_UNREF (minput_default_driver.callback_list);
  M17N_OBJECT_UNREF (minput_driver->callback_list);
}

MSymbol
minput__char_to_key (int c)
{
  if (c < 0 || c >= 0x100)
    return Mnil;

  return one_char_symbol[c];
}

/*** @} */
#endif /* !FOR_DOXYGEN || DOXYGEN_INTERNAL_MODULE */


/* External API */

/*** @addtogroup m17nInputMethod */
/*** @{ */
/*=*/

/***en
    @brief Symbol whose name is "input-method".
 */
/***ja
    @brief "input-method" ��̾���Ȥ��ƻ��ĥ���ܥ�.
 */
MSymbol Minput_method;

/***en
    @name Variables: Predefined symbols for callback commands.  */
/***ja
    @name �ѿ��� ������Хå����ޥ��������Ѥߥ���ܥ�.  */
/*** @{ */ 
/***en
    These are the predefined symbols that are used as the @c COMMAND
    argument of callback functions of an input method driver (see
    #MInputDriver::callback_list).  

    Most of them do not require extra argument nor return any value;
    exceptions are these:

    @b Minput_get_surrounding_text: When a callback function assigned for
    this command is called, the first element of #MInputContext::plist
    has key #Minteger and the value specifies which portion of the
    surrounding text should be retrieved.  If the value is positive,
    it specifies the number of characters following the current cursor
    position.  If the value is negative, the absolute value specifies
    the number of characters preceding the current cursor position.
    If the value is zero, it means that the caller just wants to know
    if the surrounding text is currently supported or not.

    If the surrounding text is currently supported, the callback
    function must set the key of this element to #Mtext and the value
    to the retrieved M-text.  The length of the M-text may be shorter
    than the requested number of characters, if the available text is
    not that long.  The length can be zero in the worst case.  Or, the
    length may be longer if an application thinks it is more efficient
    to return that length.

    If the surrounding text is not currently supported, the callback
    function should return without changing the first element of
    #MInputContext::plist.

    @b Minput_delete_surrounding_text: When a callback function assigned
    for this command is called, the first element of
    #MInputContext::plist has key #Minteger and the value specifies
    which portion of the surrounding text should be deleted in the
    same way as the case of Minput_get_surrounding_text.  The callback
    function must delete the specified text.  It should not alter
    #MInputContext::plist.  */ 
/***ja
    ���ϥ᥽�åɥɥ饤�ФΥ�����Хå��ؿ��ˤ����� @c COMMAND 
    �����Ȥ����Ѥ���������Ѥߥ���ܥ� (#MInputDriver::callback_list ����)��

    �ۤȤ�ɤ��ɲäΰ�����ɬ�פȤ��ʤ����ͤ��֤��ʤ������ʲ����㳰�Ǥ��롣

    Minput_get_surrounding_text: ���Υ��ޥ�ɤ˳�����Ƥ�줿������Х�
    ���ؿ����ƤФ줿�ݤˤϡ� #MInputContext::plist ��������Ǥϥ����Ȥ�
    ��#Minteger ��Ȥꡢ�����ͤϥ��饦��ǥ��󥰥ƥ����ȤΤ����ɤ���ʬ
    ���ä���뤫����ꤹ�롣�ͤ����Ǥ���С����ߤΥ���������֤�³��
    �ͤθĿ�ʬ��ʸ�����롣��Ǥ���С�����������֤���Ԥ����ͤ�����
    ��ʬ��ʸ�����롣���ߥ��饦��ɥƥ����Ȥ����ݡ��Ȥ���Ƥ��뤫�ɤ�
    �����Τꤿ�������Ǥ���С������ͤϥ���Ǥ��ɤ���

    ���饦��ǥ��󥰥ƥ����Ȥ����ݡ��Ȥ���Ƥ���С�������Хå��ؿ���
    �������ǤΥ����� #Mtext �ˡ��ͤ�������M-text �����ꤷ�ʤ��ƤϤ�
    ��ʤ����⤷�ƥ����Ȥ�Ĺ������ʬ�Ǥʤ���С����� M-text ��Ĺ������
    �ᤵ��Ƥ���ʸ�������û�����ɤ����ǰ��ξ�� 0 �Ǥ�褤�������ץꥱ��
    �����¦��ɬ�פǸ�ΨŪ���Ȼפ���Ĺ���Ƥ��ɤ���

    ���饦��ǥ��󥰥ƥ����Ȥ����ݡ��Ȥ���Ƥ��ʤ���С�������Хå���
    ���� #MInputContext::plist ��������Ǥ��ѹ����ƤϤʤ�ʤ���

    Minput_delete_surrounding_text: ���Υ��ޥ�ɤ˳�����Ƥ�줿������
    �Хå��ؿ����ƤФ줿�ݤˤϡ�#MInputContext::plist ��������Ǥϡ�����
    �Ȥ���#Minteger ��Ȥꡢ�ͤϺ������٤����饦��ǥ��󥰥ƥ����Ȥ�
    Minput_get_surrounding_text ��Ʊ�ͤΤ�����ǻ��ꤹ�롣������Хå�
    �ؿ��ϻ��ꤵ�줿�ƥ����Ȥ������ʤ���Фʤ�ʤ����ޤ�
    #MInputContext::plist ���Ѥ��ƤϤʤ�ʤ���  */ 
MSymbol Minput_preedit_start;
MSymbol Minput_preedit_done;
MSymbol Minput_preedit_draw;
MSymbol Minput_status_start;
MSymbol Minput_status_done;
MSymbol Minput_status_draw;
MSymbol Minput_candidates_start;
MSymbol Minput_candidates_done;
MSymbol Minput_candidates_draw;
MSymbol Minput_set_spot;
MSymbol Minput_toggle;
MSymbol Minput_reset;
MSymbol Minput_get_surrounding_text;
MSymbol Minput_delete_surrounding_text;
/*** @} */

/*=*/

/***en
    @name Variables: Predefined symbols for special input events.

    These are the predefined symbols that are used as the @c KEY
    argument of minput_filter ().  */ 
/***ja
    @name �ѿ�: ���̤����ϥ��٥��������Ѥߥ���ܥ�.

    minput_filter () �� @c KEY �����Ȥ����Ѥ���������Ѥߥ���ܥ롣  */ 

/*** @{ */ 
/*=*/

MSymbol Minput_focus_out;
MSymbol Minput_focus_in;
MSymbol Minput_focus_move;

/*** @} */

/*=*/
/***en
    @name Variables: Predefined symbols used in input method information.  */
/***ja
    @name �ѿ�: ���ϥ᥽�åɾ���������Ѥߥ���ܥ�.  */
/*** @{ */ 
/*=*/
/***en
    These are the predefined symbols describing status of input method
    command and variable, and are used in a return value of
    minput_get_command () and minput_get_variable ().  */
/***ja
    ���ϥ᥽�åɤΥ��ޥ�ɤ��ѿ��ξ��֤�ɽ����minput_get_command () ��
    minput_get_variable () ������ͤȤ����Ѥ���������Ѥߥ���ܥ롣  */
MSymbol Minherited;
MSymbol Mcustomized;
MSymbol Mconfigured;
/*** @} */ 

/*=*/

/***en
    @brief The default driver for internal input methods.

    The variable #minput_default_driver is the default driver for
    internal input methods.

    The member MInputDriver::open_im () searches the m17n database for
    an input method that matches the tag \< #Minput_method, $LANGUAGE,
    $NAME\> and loads it.

    The member MInputDriver::callback_list () is @c NULL.  Thus, it is
    programmers responsibility to set it to a plist of proper callback
    functions.  Otherwise, no feedback information (e.g. preedit text)
    can be shown to users.

    The macro M17N_INIT () sets the variable #minput_driver to the
    pointer to this driver so that all internal input methods use it.

    Therefore, unless @c minput_driver is set differently, the driver
    dependent arguments $ARG of the functions whose name begins with
    "minput_" are all ignored.  */
/***ja
    @brief �������ϥ᥽�å��ѥǥե���ȥɥ饤��.

    �ѿ� #minput_default_driver ���������ϥ᥽�å��ѤΥǥե���ȤΥɥ饤�Ф�ɽ����

    ���� MInputDriver::open_im () �� m17n �ǡ����١����椫�饿�� 
    \< #Minput_method, $LANGUAGE, $NAME\> 
    �˹��פ������ϥ᥽�åɤ�õ�����������ɤ��롣

    ���� MInputDriver::callback_list () �� @c NULL �Ǥ��ꡢ
    �������äơ��ץ����¦����Ǥ����ä� Ŭ�ڤʥ�����Хå��ؿ��� plist
    �����ꤷ�ʤ��ƤϤʤ�ʤ�������ʤ��ȡ�preedit 
    �ƥ����ȤʤɤΥե����ɥХå����󤬥桼����ɽ������ʤ���

    �ޥ��� M17N_INIT () ���ѿ� #minput_driver 
    �򤳤Υɥ饤�ФؤΥݥ��󥿤����ꤷ�����Ƥ��������ϥ᥽�åɤ����Υɥ饤�Ф�Ȥ��褦�ˤ��롣

    �������äơ�@c minput_driver ���ǥե�����ͤΤޤޤǤ���С�minput_ 
    �ǻϤޤ�ؿ��Υɥ饤�Ф˰�¸������� $ARG �Ϥ��٤�̵�뤵��롣  */

MInputDriver minput_default_driver;
/*=*/

/***en
    @brief The driver for internal input methods.

    The variable #minput_driver is a pointer to the input method
    driver that is used by internal input methods.  The macro
    M17N_INIT () initializes it to a pointer to #minput_default_driver
    if <m17n<EM></EM>.h> is included.  */ 
/***ja
    @brief �������ϥ᥽�å��ѥɥ饤��.

    �ѿ� #minput_driver ���������ϥ᥽�åɤˤ�äƻ��Ѥ���Ƥ������ϥ�
    ���åɥɥ饤�ФؤΥݥ��󥿤Ǥ��롣�ޥ��� M17N_INIT () �Ϥ��Υݥ���
    ����#minput_default_driver (<m17n<EM></EM>.h> �� include ����Ƥ���
    ��) �˽�������롣  */ 

MInputDriver *minput_driver;

/*=*/
/***
    The variable #Minput_driver is a symbol for a foreign input method.
    See @ref foreign-input-method "foreign input method" for the detail.  */
MSymbol Minput_driver;

/*=*/

/***en
    @name Functions
*/
/***ja
    @name �ؿ�
*/
/*** @{ */

/*=*/

/***en
    @brief Open an input method.

    The minput_open_im () function opens an input method whose
    language and name match $LANGUAGE and $NAME, and returns a pointer
    to the input method object newly allocated.

    This function at first decides a driver for the input method as
    described below.

    If $LANGUAGE is not #Mnil, the driver pointed by the variable
    #minput_driver is used.

    If $LANGUAGE is #Mnil and $NAME has the property #Minput_driver, the
    driver pointed to by the property value is used to open the input
    method.  If $NAME has no such a property, @c NULL is returned.

    Then, the member MInputDriver::open_im () of the driver is
    called.  

    $ARG is set in the member @c arg of the structure MInputMethod so
    that the driver can refer to it.  */
/***ja
    @brief ���ϥ᥽�åɤ򥪡��ץ󤹤�.

    �ؿ� minput_open_im () �ϸ��� $LANGUAGE ��̾�� $NAME 
    �˹��פ������ϥ᥽�åɤ򥪡��ץ󤷡������˳�����Ƥ�줿���ϥ᥽�åɥ��֥������ȤؤΥݥ��󥿤��֤���
    
    ���δؿ��ϡ��ޤ����ϥ᥽�å��ѤΥɥ饤�Ф�ʲ��Τ褦�ˤ��Ʒ��ꤹ�롣

    $LANGUAGE �� #Mnil �Ǥʤ���С��ѿ� #minput_driver 
    �ǻؤ���Ƥ���ɥ饤�Ф��Ѥ��롣

    $LANGUAGE �� #Mnil �Ǥ��ꡢ$NAME �� #Minput_driver
    �ץ�ѥƥ�����ľ��ˤϡ����Υץ�ѥƥ����ͤǻؤ���Ƥ������ϥɥ饤�Ф��Ѥ������ϥ᥽�åɤ򥪡��ץ󤹤롣
    $NAME �ˤ��Τ褦�ʥץ�ѥƥ���̵���ä����� @c NULL ���֤���

    �����ǡ��ɥ饤�ФΥ��� MInputDriver::open_im () ���ƤФ�롣

    $ARG �Ϲ�¤�� MInputMethod �Υ��� @c arg �����ꤵ�졢�ɥ饤�Ф��黲�ȤǤ��롣

    @latexonly \IPAlabel{minput_open} @endlatexonly

*/

MInputMethod *
minput_open_im (MSymbol language, MSymbol name, void *arg)
{
  MInputMethod *im;
  MInputDriver *driver;

  MINPUT__INIT ();

  MDEBUG_PRINT2 ("  [IM:%s-%s] opening ... ",
		 MSYMBOL_NAME (language), MSYMBOL_NAME (name));
  if (language)
    {
      if (name == Mnil)
	MERROR (MERROR_IM, NULL);
      driver = minput_driver;
    }
  else
    {
      driver = (MInputDriver *) msymbol_get (name, Minput_driver);
      if (! driver)
	MERROR (MERROR_IM, NULL);
    }

  MSTRUCT_CALLOC (im, MERROR_IM);
  im->language = language;
  im->name = name;
  im->arg = arg;
  im->driver = *driver;
  if ((*im->driver.open_im) (im) < 0)
    {
      MDEBUG_PRINT (" failed\n");
      free (im);
      return NULL;
    }
  MDEBUG_PRINT (" ok\n");
  return im;
}

/*=*/

/***en
    @brief Close an input method.

    The minput_close_im () function closes the input method $IM, which
    must have been created by minput_open_im ().  */

/***ja
    @brief ���ϥ᥽�åɤ򥯥�������.

    �ؿ� minput_close_im () �ϡ����ϥ᥽�å� $IM �򥯥������롣
    �������ϥ᥽�å� $IM �� minput_open_im () �ˤ�äƺ��줿��ΤǤʤ���Фʤ�ʤ���  */

void
minput_close_im (MInputMethod *im)
{
  MDEBUG_PRINT2 ("  [IM:%s-%s] closing ... ",
		 MSYMBOL_NAME (im->language), MSYMBOL_NAME (im->name));
  (*im->driver.close_im) (im);
  free (im);
  MDEBUG_PRINT (" done\n");
}

/*=*/

/***en
    @brief Create an input context.

    The minput_create_ic () function creates an input context object
    associated with input method $IM, and calls callback functions
    corresponding to @b Minput_preedit_start, @b Minput_status_start, and
    @b Minput_status_draw in this order.

    @return
    If an input context is successfully created, minput_create_ic ()
    returns a pointer to it.  Otherwise it returns @c NULL.  */

/***ja
    @brief ���ϥ���ƥ����Ȥ���������.

    �ؿ� minput_create_ic () �����ϥ᥽�å� $IM
    ���б��������ϥ���ƥ����ȥ��֥������Ȥ���������
    @b Minput_preedit_start, @b Minput_status_start, @b Minput_status_draw
    ���б����륳����Хå��ؿ��򤳤ν�˸Ƥ֡�

    @return
    ���ϥ���ƥ����Ȥ��������줿��硢minput_create_ic () 
    �Ϥ������ϥ���ƥ����ȤؤΥݥ��󥿤��֤������Ԥ������� @c NULL ���֤���
      */

MInputContext *
minput_create_ic (MInputMethod *im, void *arg)
{
  MInputContext *ic;

  MDEBUG_PRINT2 ("  [IM:%s-%s] creating context ... ",
		 MSYMBOL_NAME (im->language), MSYMBOL_NAME (im->name));
  MSTRUCT_CALLOC (ic, MERROR_IM);
  ic->im = im;
  ic->arg = arg;
  ic->preedit = mtext ();
  ic->candidate_list = NULL;
  ic->produced = mtext ();
  ic->spot.x = ic->spot.y = 0;
  ic->active = 1;
  ic->plist = mplist ();
  if ((*im->driver.create_ic) (ic) < 0)
    {
      MDEBUG_PRINT (" failed\n");
      M17N_OBJECT_UNREF (ic->preedit);
      M17N_OBJECT_UNREF (ic->produced);
      M17N_OBJECT_UNREF (ic->plist);
      free (ic);
      return NULL;
    };

  if (im->driver.callback_list)
    {
      minput_callback (ic, Minput_preedit_start);
      minput_callback (ic, Minput_status_start);
      minput_callback (ic, Minput_status_draw);
    }

  MDEBUG_PRINT (" ok\n");
  return ic;
}

/*=*/

/***en
    @brief Destroy an input context.

    The minput_destroy_ic () function destroys the input context $IC,
    which must have been created by minput_create_ic ().  It calls
    callback functions corresponding to @b Minput_preedit_done,
    @b Minput_status_done, and @b Minput_candidates_done in this order.  */

/***ja
    @brief ���ϥ���ƥ����Ȥ��˲�����.

    �ؿ� minput_destroy_ic () �ϡ����ϥ���ƥ����� $IC ���˲����롣
    �������ϥ���ƥ����Ȥ� minput_create_ic () 
    �ˤ�äƺ��줿��ΤǤʤ���Фʤ�ʤ������δؿ��� 
    @b Minput_preedit_done, @b Minput_status_done, @b Minput_candidates_done 
    ���б����륳����Хå��ؿ��򤳤ν�˸Ƥ֡�
  */

void
minput_destroy_ic (MInputContext *ic)
{
  MDEBUG_PRINT2 ("  [IM:%s-%s] destroying context ... ",
		 MSYMBOL_NAME (ic->im->language), MSYMBOL_NAME (ic->im->name));
  if (ic->im->driver.callback_list)
    {
      minput_callback (ic, Minput_preedit_done);
      minput_callback (ic, Minput_status_done);
      minput_callback (ic, Minput_candidates_done);
    }
  (*ic->im->driver.destroy_ic) (ic);
  M17N_OBJECT_UNREF (ic->preedit);
  M17N_OBJECT_UNREF (ic->produced);
  M17N_OBJECT_UNREF (ic->plist);
  MDEBUG_PRINT (" done\n");
  free (ic);
}

/*=*/

/***en
    @brief Filter an input key.

    The minput_filter () function filters input key $KEY according to
    input context $IC, and calls callback functions corresponding to
    @b Minput_preedit_draw, @b Minput_status_draw, and
    @b Minput_candidates_draw if the preedit text, the status, and the
    current candidate are changed respectively.

    To make the input method commit the current preedit text (if any)
    and shift to the initial state, call this function with #Mnil as
    $KEY.

    To inform the input method about the focus-out event, call this
    function with @b Minput_focus_out as $KEY.

    To inform the input method about the focus-in event, call this
    function with @b Minput_focus_in as $KEY.

    To inform the input method about the focus-move event (i.e. input
    spot change within the same input context), call this function
    with @b Minput_focus_move as $KEY.

    @return
    If $KEY is filtered out, this function returns 1.  In that case,
    the caller should discard the key.  Otherwise, it returns 0, and
    the caller should handle the key, for instance, by calling the
    function minput_lookup () with the same key.  */

/***ja
    @brief ���ϥ�����ե��륿����.

    �ؿ� minput_filter () �����ϥ��� $KEY �����ϥ���ƥ����� $IC 
    �˱����ƥե��륿����preedit �ƥ����ȡ����ơ��������������Ǥθ��䤬�Ѳ����������ǡ����줾��
    @b Minput_preedit_draw, @b Minput_status_draw,
    @b Minput_candidates_draw ���б����륳����Хå��ؿ���Ƥ֡�

    @return 
    $KEY ���ե��륿�����С����δؿ��� 1 ���֤���
    ���ξ��ƤӽФ�¦�Ϥ��Υ�����ΤƤ�٤��Ǥ��롣
    �����Ǥʤ���� 0 ���֤����ƤӽФ�¦�ϡ����Ȥ���Ʊ�������Ǵؿ� minput_lookup ()
    ��Ƥ֤ʤɤ��ơ����Υ�����������롣

    @latexonly \IPAlabel{minput_filter} @endlatexonly
*/

int
minput_filter (MInputContext *ic, MSymbol key, void *arg)
{
  int ret;

  if (! ic
      || ! ic->active)
    return 0;
  if (ic->im->driver.callback_list
      && mtext_nchars (ic->preedit) > 0)
    minput_callback (ic, Minput_preedit_draw);

  ret = (*ic->im->driver.filter) (ic, key, arg);

  if (ic->im->driver.callback_list)
    {
      if (ic->preedit_changed)
	minput_callback (ic, Minput_preedit_draw);
      if (ic->status_changed)
	minput_callback (ic, Minput_status_draw);
      if (ic->candidates_changed)
	minput_callback (ic, Minput_candidates_draw);
    }

  return ret;
}

/*=*/

/***en
    @brief Look up a text produced in the input context.

    The minput_lookup () function looks up a text in the input context
    $IC.  $KEY must be identical to the one that was used in the previous call of
    minput_filter ().

    If a text was produced by the input method, it is concatenated
    to M-text $MT.

    This function calls #MInputDriver::lookup .

    @return
    If $KEY was correctly handled by the input method, this function
    returns 0.  Otherwise, it returns -1, even though some text
    might be produced in $MT.  */

/***ja
    @brief ���ϥ���ƥ�������Υƥ����Ȥ�õ��.

    �ؿ� minput_lookup () �����ϥ���ƥ����� $IC ��Υƥ����Ȥ�õ����
    $KEY �ϴؿ� minput_filter () �ؤ�ľ���θƤӽФ����Ѥ���줿��Τ�Ʊ���Ǥʤ��ƤϤʤ�ʤ���

    �ƥ����Ȥ����ϥ᥽�åɤˤ�ä���������Ƥ���С��ƥ����Ȥ� M-text
    $MT ��Ϣ�뤵��롣

    ���δؿ��ϡ�#MInputDriver::lookup ��Ƥ֡�

    @return 
    $KEY �����ϥ᥽�åɤˤ�ä�Ŭ�ڤ˽����Ǥ���С����δؿ��� 0 ���֤���
    �����Ǥʤ���� -1 ���֤���
    ���ξ��Ǥ� $MT �˲��餫�Υƥ����Ȥ���������Ƥ��뤳�Ȥ����롣

    @latexonly \IPAlabel{minput_lookup} @endlatexonly  */

int
minput_lookup (MInputContext *ic, MSymbol key, void *arg, MText *mt)
{
  return (ic ? (*ic->im->driver.lookup) (ic, key, arg, mt) : -1);
}
/*=*/

/***en
    @brief Set the spot of the input context.

    The minput_set_spot () function sets the spot of input context $IC
    to coordinate ($X, $Y ) with the height specified by $ASCENT and $DESCENT .
    The semantics of these values depends on the input method driver.

    For instance, a driver designed to work in a CUI environment may
    use $X and $Y as the column- and row numbers, and may ignore $ASCENT and
    $DESCENT .  A driver designed to work in a window system may
    interpret $X and $Y as the pixel offsets relative to the origin of the
    client window, and may interpret $ASCENT and $DESCENT as the ascent- and
    descent pixels of the line at ($X . $Y ).

    $FONTSIZE specifies the fontsize of preedit text in 1/10 point.

    $MT and $POS are the M-text and the character position at the spot.
    $MT may be @c NULL, in which case, the input method cannot get
    information about the text around the spot.  */

/***ja
    @brief ���ϥ���ƥ����ȤΥ��ݥåȤ����ꤹ��.

    �ؿ� minput_set_spot () �ϡ����ϥ���ƥ����� $IC �Υ��ݥåȤ򡢺�ɸ ($X, $Y )
    �ΰ��֤� ���⤵ $ASCENT�� $DESCENT 
    �����ꤹ�롣 �������ͤΰ�̣�����ϥ᥽�åɥɥ饤�Ф˰�¸���롣

    ���Ȥ��� CUI �Ķ���ư���ɥ饤�Ф� $X �� $Y 
    �򤽤줾����ȹԤ��ֹ�Ȥ����Ѥ���$ASCENT �� $DESCENT 
    ��̵�뤹�뤫�⤷��ʤ��� �ޤ�������ɥ������ƥ��ѤΥɥ饤�Ф�
    $X �� $Y �򥯥饤����ȥ�����ɥ��θ�������Υ��ե��åȤ�ԥ�����ñ�̤�ɽ������ΤȤ��ư�����
    $ASCENT �� $DESCENT �� ($X . $Y )
    ����Υ�����Ȥȥǥ�����Ȥ�ԥ�����ñ�̤�ɽ������ΤȤ��ư������⤷��ʤ���

    $FONTSIZE �ˤ� preedit �ƥ����ȤΥե���ȥ������� 1/10 �ݥ����ñ�̤ǻ��ꤹ�롣

    $MT �� $POS �Ϥ��Υ��ݥåȤ� M-text ��ʸ�����֤Ǥ��롣$MT �� @c
    NULL �Ǥ�褯�����ξ��ˤ����ϥ᥽�åɤϥ��ݥåȼ��դΥƥ����Ȥ˴ؤ����������뤳�Ȥ��Ǥ��ʤ���
    */

void
minput_set_spot (MInputContext *ic, int x, int y,
		 int ascent, int descent, int fontsize,
		 MText *mt, int pos)
{
  ic->spot.x = x;
  ic->spot.y = y;
  ic->spot.ascent = ascent;
  ic->spot.descent = descent;
  ic->spot.fontsize = fontsize;
  ic->spot.mt = mt;
  ic->spot.pos = pos;
  if (ic->im->driver.callback_list)
    minput_callback (ic, Minput_set_spot);
}
/*=*/

/***en
    @brief Toggle input method.

    The minput_toggle () function toggles the input method associated
    with input context $IC.  */
/***ja
    @brief ���ϥ᥽�åɤ����ؤ���.

    �ؿ� minput_toggle () �����ϥ���ƥ����� $IC 
    ���б��դ���줿���ϥ᥽�åɤ�ȥ��뤹�롣
    */

void
minput_toggle (MInputContext *ic)
{
  if (ic->im->driver.callback_list)
    minput_callback (ic, Minput_toggle);
  ic->active = ! ic->active;
}

/*=*/

/***en
    @brief Reset an input context.

    The minput_reset_ic () function resets input context $IC by
    calling a callback function corresponding to @b Minput_reset.  It
    resets the status of $IC to its initial one.  As the
    current preedit text is deleted without commitment, if necessary,
    call minput_filter () with the arg @b key #Mnil to force the input
    method to commit the preedit in advance.  */

/***ja
    @brief ���ϥ���ƥ����Ȥ�ꥻ�åȤ���.

    �ؿ� minput_reset_ic () �� @b Minput_reset ���б����륳����Хå��ؿ�
    ��Ƥ֤��Ȥˤ�ä����ϥ���ƥ����� $IC ��ꥻ�åȤ��롣�ꥻ�åȤȤϡ�
    �ºݤˤ����ϥ᥽�åɤ������֤˰ܤ����ȤǤ��롣����������Υƥ���
    �Ȥϥ��ߥåȤ���뤳�Ȥʤ���������Τǡ����ץꥱ�������ץ���
    ��ϡ�ɬ�פʤ��ͽ�� minput_filter () ����� @b key #Mnil �ǸƤ��
    ����Ū�˥ץꥨ�ǥ��åȥƥ����Ȥ򥳥ߥåȤ����뤳�ȡ�  */

void
minput_reset_ic (MInputContext *ic)
{
  if (ic->im->driver.callback_list)
    minput_callback (ic, Minput_reset);
}

/*=*/

/***en
    @brief Get title and icon filename of an input method.

    The minput_get_title_icon () function returns a plist containing a
    title and icon filename (if any) of an input method specified by
    $LANGUAGE and $NAME.

    The first element of the plist has key #Mtext and the value is an
    M-text of the title for identifying the input method.  The second
    element (if any) has key #Mtext and the value is an M-text of the
    icon image (absolute) filename for the same purpose.

    @return
    If there exists a specified input method and it defines an title,
    a plist is returned.  Otherwise, NULL is returned.  The caller
    must free the plist by m17n_object_unref ().  */
/***ja
    @brief ���ϥ᥽�åɤΥ����ȥ�ȥ��������ѥե�����̾������.

    �ؿ� minput_get_title_icon () �ϡ� $LANGUAGE �� $NAME �ǻ��ꤵ���
    ���ϥ᥽�åɤΥ����ȥ�ȡʤ���С˥��������ѥե������ޤ� plist ��
    �֤���

    plist ��������Ǥϡ�#Mtext �򥭡��˻������ͤ����ϥ᥽�åɤ��̤���
    �����ȥ��ɽ�� M-text �Ǥ��롣�������Ǥ�����С������� #Mtext �Ǥ�
    �ꡢ�ͤϼ����ѥ�����������ե���������Хѥ���ɽ�� M-text �Ǥ��롣

    @return
    ��������ϥ᥽�åɤ�¸�ߤ��������ȥ뤬�������Ƥ����
     plist ���֤��������Ǥʤ���� NULL ���֤����ƽ�¦��
     �ؿ� m17n_object_unref () ���Ѥ��� plist ��������ʤ��ƤϤʤ�ʤ���  */

MPlist *
minput_get_title_icon (MSymbol language, MSymbol name)
{
  MInputMethodInfo *im_info;
  MPlist *plist;
  char *file = NULL;
  MText *mt;

  MINPUT__INIT ();

  im_info = get_im_info (language, name, Mnil, Mtitle);
  if (! im_info || !im_info->title)
    return NULL;
  mt = mtext_get_prop (im_info->title, 0, Mtext);
  if (mt)
    file = mdatabase__find_file ((char *) MTEXT_DATA (mt));
  else
    {
      char *buf = alloca (MSYMBOL_NAMELEN (language) + MSYMBOL_NAMELEN (name)
			  + 12);

      sprintf (buf, "icons/%s-%s.png", (char *) MSYMBOL_NAME (language), 
	       (char *) MSYMBOL_NAME (name));
      file = mdatabase__find_file (buf);
      if (! file && language == Mt)
	{
	  sprintf (buf, "icons/%s.png", (char *) MSYMBOL_NAME (name));
	  file = mdatabase__find_file (buf);
	}
    }

  plist = mplist ();
  mplist_add (plist, Mtext, im_info->title);
  if (file)
    {
      mt = mtext__from_data (file, strlen (file), MTEXT_FORMAT_UTF_8, 1);
      free (file);
      mplist_add (plist, Mtext, mt);
      M17N_OBJECT_UNREF (mt);
    }
  return plist;
}

/*=*/

/***en
    @brief Get description text of an input method.

    The minput_get_description () function returns an M-text that
    describes the input method specified by $LANGUAGE and $NAME.

    @return
    If the specified input method has a description text, a pointer to
    #MText is returned.  The caller has to free it by m17n_object_unref ().
    If the input method does not have a description text, @c NULL is
    returned.  */
/***ja
    @brief ���ϥ᥽�åɤ������ƥ����Ȥ�����.

    �ؿ� minput_get_description () �ϡ�$LANGUAGE �� $NAME �ˤ�äƻ���
    ���줿���ϥ᥽�åɤ��������� M-text ���֤���

    @return
    ���ꤵ�줿���ϥ᥽�åɤ���������ƥ����Ȥ���äƤ���С�
    #MText �ؤΥݥ��󥿤��֤����ƤӽФ�¦�ϡ������ m17n_object_unref
    () ���Ѥ��Ʋ������ʤ��ƤϤʤ�ʤ������ϥ᥽�åɤ������ƥ����Ȥ�̵��
    ���@c NULL ���֤��� */

MText *
minput_get_description (MSymbol language, MSymbol name)
{
  MInputMethodInfo *im_info;
  MSymbol extra;

  MINPUT__INIT ();

  if (name != Mnil)
    extra = Mnil;
  else
    extra = language, language = Mt;

  im_info = get_im_info (language, name, extra, Mdescription);
  if (! im_info || ! im_info->description)
    return NULL;
  M17N_OBJECT_REF (im_info->description);
  return im_info->description;
}

/*=*/

/***en
    @brief Get information about input method command(s).

    The minput_get_command () function returns information about
    the command $COMMAND of the input method specified by $LANGUAGE and
    $NAME.  An input method command is a pseudo key event to which one
    or more actual input key sequences are assigned.

    There are two kinds of commands, global and local.  A global
    command has a global definition, and the description and the key
    assignment may be inherited by a local command.  Each input method
    defines a local command which has a local key assignment.  It may
    also declare a local command that inherits the definition of a
    global command of the same name.

    If $LANGUAGE is #Mt and $NAME is #Mnil, this function returns
    information about a global command.  Otherwise information about a
    local command is returned.

    If $COMMAND is #Mnil, information about all commands is returned.

    The return value is a @e well-formed plist (@ref m17nPlist) of this
    format:
@verbatim
  ((NAME DESCRIPTION STATUS [KEYSEQ ...]) ...)
@endverbatim
    @c NAME is a symbol representing the command name.

    @c DESCRIPTION is an M-text describing the command, or #Mnil if the
    command has no description.

    @c STATUS is a symbol representing how the key assignment is decided.
    The value is #Mnil (the default key assignment), @b Mcustomized (the
    key assignment is customized by per-user customization file), or
    @b Mconfigured (the key assignment is set by the call of
    minput_config_command ()).  For a local command only, it may also
    be @b Minherited (the key assignment is inherited from the
    corresponding global command).

    @c KEYSEQ is a plist of one or more symbols representing a key
    sequence assigned to the command.  If there's no KEYSEQ, the
    command is currently disabled (i.e. no key sequence can trigger
    actions of the command).

    If $COMMAND is not #Mnil, the first element of the returned plist
    contains the information about $COMMAND.

    @return

    If the requested information was found, a pointer to a non-empty
    plist is returned.  As the plist is kept in the library, the
    caller must not modify nor free it.

    Otherwise (the specified input method or the specified command
    does not exist), @c NULL is returned.  */
/***ja
    @brief ���ϥ᥽�åɤΥ��ޥ�ɤ˴ؤ�����������.

    �ؿ� minput_get_command () �ϡ�$LANGUAGE �� $NAME �ǻ��ꤵ�������
    �᥽�åɤΥ��ޥ�� $COMMAND �˴ؤ��������֤������ϥ᥽�åɤΥ���
    ��ɤȤϡ������������٥�ȤǤ��ꡢ���İʾ�μºݤ����ϥ�����������
    �󥹤�������Ƥ��롣

    ���ޥ�ɤˤϡ������Х�ȥ�����Σ����ब���롣�����Х�ʥ��ޥ��
    �ϥ����Х��������졢������ʥ��ޥ�ɤϤ��������ȥ����������
    ��Ѿ����뤳�Ȥ��Ǥ��롣�����ϥ᥽�åɤϥ�����ʥ�����������ĥ�
    ����ʥ��ޥ�ɤ�������롣�ޤ�Ʊ̾�Υ����Х�ʥ��ޥ�ɤ�������
    �����������ʥ��ޥ�ɤ�������뤳�Ȥ�Ǥ��롣

    $LANGUAGE �� #Mt �� $NAME �� #Mnil �ξ��ϡ����δؿ��ϥ����Х륳
    �ޥ�ɤ˴ؤ��������֤��������Ǥʤ���Х����륳�ޥ�ɤ˴ؤ����
    �Τ��֤���

    $COMMAND �� #Mnil �ξ��ϡ����٤ƤΥ��ޥ�ɤ˴ؤ��������֤���

    ����ͤϰʲ��η����� @e well-formed plist (@ref m17nPlist) �Ǥ��롣

@verbatim
  ((NAME DESCRIPTION STATUS [KEYSEQ ...]) ...)
@endverbatim
    @c NAME �ϥ��ޥ��̾�򼨤�����ܥ�Ǥ��롣

    @c DESCRIPTION �ϥ��ޥ�ɤ��������� M-text �Ǥ��뤫��������̵������
    �� #Mnil �Ǥ��롣

    @c STATUS �ϥ���������Ƥ��ɤΤ褦�������뤫�򤢤�魯����ܥ�
    �Ǥ��ꡢ�����ͤ� #Mnil �ʥǥե���Ȥγ�����ơ�, @b Mcustomized �ʥ桼
    ����Υ������ޥ����ե�����ˤ�äƥ������ޥ������줿������ơ�,
    @b Mconfigured ��minput_config_command ()��Ƥ֤��Ȥˤ�ä����ꤵ���
    ������ơˤΤ����줫�Ǥ��롣�����륳�ޥ�ɤξ��ˤϡ�
    @b Minherited ���б����륰���Х륳�ޥ�ɤ���ηѾ��ˤ�������ơ�
    �Ǥ�褤��

    @c KEYSEQ �ϣ��İʾ�Υ���ܥ뤫��ʤ� plist �Ǥ��ꡢ�ƥ���ܥ�ϥ���
    ��ɤ˳�����Ƥ��Ƥ��륭�����������󥹤�ɽ����KEYSEQ ��̵�����ϡ�
    ���Υ��ޥ�ɤϸ����ǻ�����ǽ�Ǥ��롣�ʤ��ʤ�����ޥ�ɤ�ư���
    ư�Ǥ��륭�����������󥹤�̵������

    $COMMAND �� #Mnil �Ǥʤ���С��֤���� plist �κǽ�����Ǥϡ�
    $COMMAND �˴ؤ�������ޤࡣ

    @return

    ����줿���󤬸��Ĥ���С����Ǥʤ� plist �ؤΥݥ��󥿤��֤����ꥹ
    �Ȥϥ饤�֥�꤬�������Ƥ���Τǡ��ƽ�¦���ѹ���������������ꤹ��
    ���ȤϤǤ��ʤ���

    �����Ǥʤ���С����ʤ����������ϥ᥽�åɤ䥳�ޥ�ɤ�¸�ߤ��ʤ����
    @c NULL ���֤���  */

#if EXAMPLE_CODE
MText *
get_im_command_description (MSymbol language, MSymbol name, MSymbol command)
{
  /* Return a description of the command COMMAND of the input method
     specified by LANGUAGE and NAME.  */
  MPlist *cmd = minput_get_command (language, name, command);
  MPlist *plist;

  if (! cmds)
    return NULL;
  plist = mplist_value (cmds);	/* (NAME DESCRIPTION STATUS KEY-SEQ ...) */
  plist = mplist_next (plist);	/* (DESCRIPTION STATUS KEY-SEQ ...) */
  return  (mplist_key (plist) == Mtext
	   ? (MText *) mplist_value (plist)
	   : NULL);
}
#endif

MPlist *
minput_get_command (MSymbol language, MSymbol name, MSymbol command)
{
  MInputMethodInfo *im_info;

  MINPUT__INIT ();

  im_info = get_im_info (language, name, Mnil, Mcommand);
  if (! im_info
      || ! im_info->configured_cmds
      || MPLIST_TAIL_P (im_info->configured_cmds))
    return NULL;
  if (command == Mnil)
    return im_info->configured_cmds;
  return mplist__assq (im_info->configured_cmds, command);
}

/*=*/

/***en
    @brief Configure the key sequence of an input method command.

    The minput_config_command () function assigns a list of key
    sequences $KEYSEQLIST to the command $COMMAND of the input method
    specified by $LANGUAGE and $NAME.

    If $KEYSEQLIST is a non-empty plist, it must be a list of key
    sequences, and each key sequence must be a plist of symbols.

    If $KEYSEQLIST is an empty plist, any configuration and
    customization of the command are cancelled, and default key
    sequences become effective.

    If $KEYSEQLIST is NULL, the configuration of the command is
    canceled, and the original key sequences (what saved in per-user
    customization file, or the default one) become effective.

    In the latter two cases, $COMMAND can be #Mnil to make all the
    commands of the input method the target of the operation.

    If $NAME is #Mnil, this function configures the key assignment of a
    global command, not that of a specific input method.

    The configuration takes effect for input methods opened or
    re-opened later in the current session.  In order to make the
    configuration take effect for the future session, it must be saved
    in a per-user customization file by the function
    minput_save_config ().

    @return
    If the operation was successful, this function returns 0,
    otherwise returns -1.  The operation fails in these cases:
    <ul>
    <li>$KEYSEQLIST is not in a valid form.
    <li>$COMMAND is not available for the input method.
    <li>$LANGUAGE and $NAME do not specify an existing input method.
    </ul>

    @seealso
    minput_get_commands (), minput_save_config ().
*/
/***ja
    @brief ���ϥ᥽�åɤΥ��ޥ�ɤΥ������������󥹤����ꤹ��.

    �ؿ� minput_config_command () �ϥ������������󥹤Υꥹ��
    $KEYSEQLIST ��$LANGUAGE �� $NAME �ˤ�äƻ��ꤵ������ϥ᥽�åɤ�
    ���ޥ�� $COMMAND �˳�����Ƥ롣

    $KEYSEQLIST �����ꥹ�ȤǤʤ���С��������������󥹤Υꥹ�ȤǤ��ꡢ
    �ƥ������������󥹤ϥ���ܥ�� plist �Ǥ��롣

    $KEYSEQLIST ������ plist �ʤ�С����Υ��ޥ�ɤ�����䥫�����ޥ�����
    ���٤ƥ���󥻥뤵�졢�ǥե���ȤΥ������������󥹤�ͭ���ˤʤ롣

    $KEYSEQLIST �� NULL �Ǥ���С����Υ��ޥ�ɤ�����ϥ���󥻥뤵�졢
    ���Υ������������󥹡ʥ桼����Υ������ޥ����ե��������¸����Ƥ�
    ���Ρ����뤤�ϥǥե���ȤΤ�Ρˤ�ͭ���ˤʤ롣

    ��Τդ��Ĥξ��ˤϡ�$COMMAND �� #Mnil ��Ȥ뤳�Ȥ��Ǥ����������
    �ϥ᥽�åɤ����ƤΥ��ޥ������Υ���󥻥���̣���롣

    $NAME �� #Mnil �ʤ�С����δؿ��ϸġ������ϥ᥽�åɤǤϤʤ�������
    ��ʥ��ޥ�ɤΥ���������Ƥ����ꤹ�롣

    ����������ϡ����ԤΥ��å����������ϥ᥽�åɤ������ץ�ʤޤ���
    �ƥ����ץ�ˤ��줿������ͭ���ˤʤ롣����Υ��å������Ǥ�ͭ���ˤ�
    �뤿��ˤϡ��ؿ� minput_save_config () ���Ѥ��ƥ桼����Υ������ޥ�
    ���ե��������¸���ʤ��ƤϤʤ�ʤ���

    @return

    ���δؿ��ϡ���������������� 0 �򡢼��Ԥ���� -1 ���֤������ԤȤϰʲ��ξ��Ǥ��롣
    <ul>
    <li>$KEYSEQLIST ��ͭ���ʷ����Ǥʤ���
    <li>$COMMAND ����������ϥ᥽�åɤ����ѤǤ��ʤ���
    <li>$LANGUAGE �� $NAME �ǻ��ꤵ������ϥ᥽�åɤ�¸�ߤ��ʤ���
    </ul>

    @seealso
    minput_get_commands (), minput_save_config ().
*/

#if EXAMPLE_CODE
/* Add "C-x u" to the "start" command of Unicode input method.  */
{
  MSymbol start_command = msymbol ("start");
  MSymbol unicode = msymbol ("unicode");
  MPlist *cmd, *plist, *key_seq_list, *key_seq;

  /* At first get the current key-sequence assignment.  */
  cmd = minput_get_command (Mt, unicode, start_command);
  if (! cmd)
    {
      /* The input method does not have the command "start".  Here
	 should come some error handling code.  */
    }
  /* Now CMD == ((start DESCRIPTION STATUS KEY-SEQUENCE ...) ...).
     Extract the part (KEY-SEQUENCE ...).  */
  plist = mplist_next (mplist_next (mplist_next (mplist_value (cmd))));
  /* Copy it because we should not modify it directly.  */
  key_seq_list = mplist_copy (plist);
  
  key_seq = mplist ();
  mplist_add (key_seq, Msymbol, msymbol ("C-x"));
  mplist_add (key_seq, Msymbol, msymbol ("u"));
  mplist_add (key_seq_list, Mplist, key_seq);
  m17n_object_unref (key_seq);

  minput_config_command (Mt, unicode, start_command, key_seq_list);
  m17n_object_unref (key_seq_list);
}
#endif

int
minput_config_command (MSymbol language, MSymbol name, MSymbol command,
		       MPlist *keyseqlist)
{
  MInputMethodInfo *im_info, *config;
  MPlist *plist;

  MINPUT__INIT ();

  im_info = get_im_info (language, name, Mnil, Mcommand);
  if (! im_info)
    MERROR (MERROR_IM, -1);
  if (command == Mnil ? (keyseqlist && ! MPLIST_TAIL_P (keyseqlist))
      : (! im_info->cmds
	 || ! mplist__assq (im_info->configured_cmds, command)))
    MERROR (MERROR_IM, -1);
  if (keyseqlist && ! MPLIST_TAIL_P (keyseqlist))
    {
      MPLIST_DO (plist, keyseqlist)
	if (! check_command_keyseq (plist))
	  MERROR (MERROR_IM, -1);
    }

  config = get_config_info (im_info);
  if (! config)
    {
      if (! im_config_list)
	im_config_list = mplist ();
      config = new_im_info (NULL, language, name, Mnil, im_config_list);
      config->cmds = mplist ();
      config->vars = mplist ();
    }

  if (! keyseqlist && MPLIST_TAIL_P (config->cmds))
    /* Nothing to do.  */
    return 0;

  if (command == Mnil)
    {
      if (! keyseqlist)
	{
	  /* Cancal the configuration. */
	  if (MPLIST_TAIL_P (config->cmds))
	    return 0;
	  mplist_set (config->cmds, Mnil, NULL);
	}
      else
	{
	  /* Cancal the customization. */
	  MInputMethodInfo *custom = get_custom_info (im_info);

	  if (MPLIST_TAIL_P (config->cmds)
	      && (! custom || ! custom->cmds || MPLIST_TAIL_P (custom->cmds)))
	    /* Nothing to do.  */
	    return 0;
	  mplist_set (config->cmds, Mnil, NULL);
	  MPLIST_DO (plist, custom->cmds)
	    {
	      command = MPLIST_SYMBOL (MPLIST_PLIST (plist));
	      plist = mplist ();
	      mplist_add (plist, Msymbol, command);
	      mplist_push (config->cmds, Mplist, plist);
	      M17N_OBJECT_UNREF (plist);
	    }
	}
    }
  else
    {
      plist = mplist__assq (config->cmds, command);
      if (! keyseqlist)
	{
	  /* Cancel the configuration.  */
	  if (! plist)
	    return 0;
	  mplist__pop_unref (plist);
	}
      else if (MPLIST_TAIL_P (keyseqlist))
	{
	  /* Cancel the customization.  */
	  MInputMethodInfo *custom = get_custom_info (im_info);
	  int no_custom = (! custom || ! custom->cmds
			   || ! mplist__assq (custom->cmds, command));
	  if (! plist)
	    {
	      if (no_custom)
		return 0;
	      plist = mplist ();
	      mplist_add (config->cmds, Mplist, plist);
	      M17N_OBJECT_UNREF (plist);
	      plist = mplist_add (plist, Msymbol, command);
	    }
	  else
	    {
	      if (no_custom)
		mplist__pop_unref (plist);
	      else
		{
		  plist = MPLIST_PLIST (plist); /* (NAME nil KEYSEQ ...) */
		  plist = MPLIST_NEXT (plist);
		  mplist_set (plist, Mnil, NULL);
		}
	    }
	}
      else
	{
	  MPlist *pl;

	  if (plist)
	    {
	      plist = MPLIST_NEXT (MPLIST_PLIST (plist));
	      if (! MPLIST_TAIL_P (plist))
		mplist_set (plist, Mnil, NULL);
	    }
	  else
	    {
	      plist = mplist ();
	      mplist_add (config->cmds, Mplist, plist);
	      M17N_OBJECT_UNREF (plist);
	      plist = mplist_add (plist, Msymbol, command);
	      plist = MPLIST_NEXT (plist);
	    }
	  MPLIST_DO (keyseqlist, keyseqlist)
	    {
	      pl = mplist_copy (MPLIST_VAL (keyseqlist));
	      plist = mplist_add (plist, Mplist, pl);
	      M17N_OBJECT_UNREF (pl);
	    }
	}
    }
  config_all_commands (im_info);
  im_info->tick = time (NULL);
  return 0;
}

/*=*/

/***en
    @brief Get information about input method variable(s).

    The minput_get_variable () function returns information about
    variable $VARIABLE of the input method specified by $LANGUAGE and $NAME.
    An input method variable controls behavior of an input method.

    There are two kinds of variables, global and local.  A global
    variable has a global definition, and the description and the value
    may be inherited by a local variable.  Each input method defines a
    local variable which has local value.  It may also declare a
    local variable that inherits definition of a global variable of
    the same name.

    If $LANGUAGE is #Mt and $NAME is #Mnil, information about a global
    variable is returned.  Otherwise information about a local variable
    is returned.

    If $VARIABLE is #Mnil, information about all variables is
    returned.

    The return value is a @e well-formed plist (@ref m17nPlist) of this
    format:
@verbatim
  ((NAME DESCRIPTION STATUS VALUE [VALID-VALUE ...]) ...)
@endverbatim
    @c NAME is a symbol representing the variable name.

    @c DESCRIPTION is an M-text describing the variable, or #Mnil if the
    variable has no description.

    @c STATUS is a symbol representing how the value is decided.  The
    value is #Mnil (the default value), @b Mcustomized (the value is
    customized by per-user customization file), or @b Mconfigured (the
    value is set by the call of minput_config_variable ()).  For a
    local variable only, it may also be @b Minherited (the value is
    inherited from the corresponding global variable).

    @c VALUE is the initial value of the variable.  If the key of this
    element is #Mt, the variable has no initial value.  Otherwise, the
    key is #Minteger, #Msymbol, or #Mtext and the value is of the
    corresponding type.

    @c VALID-VALUEs (if any) specify which values the variable can have.
    They have the same type (i.e. having the same key) as @c VALUE except
    for the case that VALUE is an integer.  In that case, @c VALID-VALUE
    may be a plist of two integers specifying the range of possible
    values.

    If there no @c VALID-VALUE, the variable can have any value as long
    as the type is the same as @c VALUE.

    If $VARIABLE is not #Mnil, the first element of the returned plist
    contains the information about $VARIABLE.

    @return

    If the requested information was found, a pointer to a non-empty
    plist is returned.  As the plist is kept in the library, the
    caller must not modify nor free it.

    Otherwise (the specified input method or the specified variable
    does not exist), @c NULL is returned.  */
/***ja
    @brief ���ϥ᥽�åɤ��ѿ��˴ؤ�����������.

    �ؿ� minput_get_variable () �ϡ�$LANGUAGE �� $NAME �ǻ��ꤵ�������
    �᥽�åɤ��ѿ� $VARIABLE �˴ؤ��������֤������ϥ᥽�åɤ��ѿ��Ȥϡ�
    ���ϥ᥽�åɤο�������椹���ΤǤ��롣

    �ѿ��ˤϡ������Х�ȥ�����Σ����ब���롣�����Х���ѿ��ϥ�
    ���Х��������졢��������ѿ��Ϥ����������ͤ�Ѿ����뤳�Ȥ���
    ���롣�����ϥ᥽�åɤϥ�������ͤ���ĥ�������ѿ���������롣
    �ޤ�Ʊ̾�Υ����Х���ѿ��������Ѿ������������ѿ����������
    ���Ȥ�Ǥ��롣

    $LANGUAGE �� #Mt �� $NAME �� #Mnil �ξ��ϡ����δؿ��ϥ����Х���
    ���˴ؤ��������֤��������Ǥʤ���Х������ѿ��˴ؤ����Τ��֤���

    $VARIABLE �� #Mnil �ξ��ϡ����٤ƤΥ��ޥ�ɤ˴ؤ��������֤���

    ����ͤϰʲ��η����� @e well-formed plist (@ref m17nPlist) �Ǥ��롣
@verbatim
  ((NAME DESCRIPTION STATUS VALUE [VALID-VALUE ...]) ...)
@endverbatim

    @c NAME ���ѿ���̾���򼨤�����ܥ�Ǥ��롣

    @c DESCRIPTION ���ѿ����������� M-text �Ǥ��뤫��������̵�����ˤ�
    #Mnil �Ǥ��롣

    @c STATUS ���ͤ��ɤΤ褦�������뤫�򤢤�魯����ܥ�Ǥ��ꡢ
    @c STATUS ���ͤ� #Mnil �ʥǥե���Ȥ��͡�, @b Mcustomized �ʥ桼�����
    �������ޥ����ե�����ˤ�äƥ������ޥ������줿�͡�, @b Mconfigured
    ��minput_config_variable ()��Ƥ֤��Ȥˤ�ä����ꤵ����͡ˤΤ�����
    ���Ǥ��롣�������ѿ��ξ��ˤϡ�@b Minherited ���б����륰���Х�
    �ѿ�����Ѿ������͡ˤǤ�褤��

    @c VALUE ���ѿ��ν���ͤǤ��롣�������ǤΥ�����#Mt �Ǥ���н���ͤ��
    ���ʤ��������Ǥʤ���С������� #Minteger, #Msymbol, #Mtext �Τ�����
    ���Ǥ��ꡢ�ͤϤ��줾���б����뷿�Τ�ΤǤ��롣

    @c VALID-VALUE �Ϥ⤷����С��ѿ��μ�������ͤ���ꤹ�롣����� @c VALUE
    ��Ʊ����(���ʤ��Ʊ�����������) �Ǥ��뤬���㳰�Ȥ��� @c VALUE ��
    integer �ξ��� @c VALID-VALUE �ϲ�ǽ���ͤ��ϰϤ򼨤���Ĥ���������
    �ʤ� plist �Ȥʤ뤳�Ȥ��Ǥ��롣

    @c VALID-VALUE ���ʤ���С��ѿ��� @c VALUE ��Ʊ�����Ǥ���¤ꤤ���ʤ��ͤ�
    �Ȥ뤳�Ȥ��Ǥ��롣

    $VARIABLE �� #Mnil �Ǥʤ���С��֤���� plist �κǽ�����Ǥ�
    $VARIABLE �˴ؤ�������ޤࡣ

    @return

    ����줿���󤬸��Ĥ���С����Ǥʤ� plist �ؤΥݥ��󥿤��֤����ꥹ
    �Ȥϥ饤�֥�꤬�������Ƥ���Τǡ��ƽ�¦���ѹ���������������ꤹ��
    ���ȤϤǤ��ʤ���

    �����Ǥʤ���С����ʤ����������ϥ᥽�åɤ��ѿ���¸�ߤ��ʤ����
    @c NULL ���֤��� */

MPlist *
minput_get_variable (MSymbol language, MSymbol name, MSymbol variable)
{
  MInputMethodInfo *im_info;

  MINPUT__INIT ();

  im_info = get_im_info (language, name, Mnil, Mvariable);
  if (! im_info || ! im_info->configured_vars)
    return NULL;
  if (variable == Mnil)
    return im_info->configured_vars;
  return mplist__assq (im_info->configured_vars, variable);
}

/*=*/

/***en
    @brief Configure the value of an input method variable.

    The minput_config_variable () function assigns $VALUE to the
    variable $VARIABLE of the input method specified by $LANGUAGE and
    $NAME.

    If $VALUE is a non-empty plist, it must be a plist of one element
    whose key is #Minteger, #Msymbol, or #Mtext, and the value is of
    the corresponding type.  That value is assigned to the variable.

    If $VALUE is an empty plist, any configuration and customization
    of the variable are canceled, and the default value is assigned to
    the variable.

    If $VALUE is NULL, the configuration of the variable is canceled,
    and the original value (what saved in per-user customization file,
    or the default value) is assigned to the variable.

    In the latter two cases, $VARIABLE can be #Mnil to make all the
    variables of the input method the target of the operation.

    If $NAME is #Mnil, this function configures the value of global
    variable, not that of a specific input method.

    The configuration takes effect for input methods opened or
    re-opened later in the current session.  To make the configuration
    take effect for the future session, it must be saved in a per-user
    customization file by the function minput_save_config ().

    @return

    If the operation was successful, this function returns 0,
    otherwise returns -1.  The operation fails in these cases:
    <ul>
    <li>$VALUE is not in a valid form, the type does not match the
    definition, or the value is our of range.
    <li>$VARIABLE is not available for the input method.
    <li>$LANGUAGE and $NAME do not specify an existing input method.  
    </ul>

    @seealso
    minput_get_variable (), minput_save_config ().  */
/***ja
    @brief ���ϥ᥽�åɤ��ѿ����ͤ����ꤹ��.

    �ؿ� minput_config_variable () ���� $VALUE ��$LANGUAGE �� $NAME
    �ˤ�äƻ��ꤵ������ϥ᥽�åɤ��ѿ� $VARIABLE �˳�����Ƥ롣

    $VALUE �� ���ꥹ�ȤǤʤ���С������Ǥ� plist �Ǥ��ꡢ���Υ�����
    #Minteger, #Msymbol, #Mtext �Τ����줫���ͤ��б����뷿�Τ�ΤǤ��롣
    �����ͤ��ѿ� $VARIABLE �˳�����Ƥ��롣

    $VALUE �� ���ꥹ�ȤǤ���С��ѿ�������ȥ������ޥ���������󥻥뤵
    �졢�ǥե�����ͤ��ѿ� $VARIABLE �˳�����Ƥ��롣

    $VALUE �� NULL �Ǥ���С��ѿ�������ϥ���󥻥뤵�졢�����͡ʥ桼��
    ��Υ������ޥ����ե���������͡��ޤ��ϥǥե���Ȥ��͡ˤ�������Ƥ��롣

    ��Τդ��Ĥξ��ˤϡ�$VARIABLE �� #Mnil ��Ȥ뤳�Ȥ��Ǥ������ꤵ��
    �����ϥ᥽�åɤ����Ƥ��ѿ�����Υ���󥻥���̣���롣

    $NAME �� #Mnil �ʤ�С����δؿ��ϸġ������ϥ᥽�åɤǤϤʤ�������
    ����ѿ����ͤ����ꤹ�롣

    ����������ϡ����ԤΥ��å����������ϥ᥽�åɤ������ץ�ʤޤ���
    �ƥ����ץ�ˤ��줿������ͭ���ˤʤ롣����Υ��å������Ǥ�ͭ���ˤ�
    �뤿��ˤϡ��ؿ� minput_save_config () ���Ѥ��ƥ桼����Υ������ޥ�
    ���ե��������¸���ʤ��ƤϤʤ�ʤ���

    @return

    ���δؿ��ϡ���������������� 0 �򡢼��Ԥ���� -1 ���֤������ԤȤϰʲ��ξ��Ǥ��롣
    <ul>
    <li>$VALUE��ͭ���ʷ����Ǥʤ�����������˹��ʤ����ޤ����ͤ��ϰϳ��Ǥ��롣
    <li>$VARIABLE ����������ϥ᥽�åɤ����ѤǤ��ʤ���
    <li>$LANGUAGE �� $NAME �ǻ��ꤵ������ϥ᥽�åɤ�¸�ߤ��ʤ���
    </ul>

    @seealso
    minput_get_commands (), minput_save_config ().
*/
int
minput_config_variable (MSymbol language, MSymbol name, MSymbol variable,
			MPlist *value)
{
  MInputMethodInfo *im_info, *config;
  MPlist *plist;

  MINPUT__INIT ();

  im_info = get_im_info (language, name, Mnil, Mvariable);
  if (! im_info)
    MERROR (MERROR_IM, -1);
  if (variable == Mnil ? (value && ! MPLIST_TAIL_P (value))
      : (! im_info->vars
	 || ! (plist = mplist__assq (im_info->configured_vars, variable))))
    MERROR (MERROR_IM, -1);

  if (value && ! MPLIST_TAIL_P (value))
    {
      plist = MPLIST_PLIST (plist);
      plist = MPLIST_NEXT (plist); /* (DESC STATUS VALUE VALIDS ...) */
      plist = MPLIST_NEXT (plist); /* (STATUS VALUE VALIDS ...) */
      plist = MPLIST_NEXT (plist); /* (VALUE VALIDS ...) */
      if (MPLIST_KEY (plist) != Mt
	  && ! check_variable_value (value, plist))
	MERROR (MERROR_IM, -1);
    }

  config = get_config_info (im_info);
  if (! config)
    {
      if (! im_config_list)
	im_config_list = mplist ();
      config = new_im_info (NULL, language, name, Mnil, im_config_list);
      config->cmds = mplist ();
      config->vars = mplist ();
    }

  if (! value && MPLIST_TAIL_P (config->vars))
    /* Nothing to do.  */
    return 0;

  if (variable == Mnil)
    {
      if (! value)
	{
	  /* Cancel the configuration.  */
	  if (MPLIST_TAIL_P (config->vars))
	    return 0;
	  mplist_set (config->vars, Mnil, NULL);
	}
      else
	{
	  /* Cancel the customization.  */
	  MInputMethodInfo *custom = get_custom_info (im_info);

	  if (MPLIST_TAIL_P (config->vars)
	      && (! custom || ! custom->vars || MPLIST_TAIL_P (custom->vars)))
	    /* Nothing to do.  */
	    return 0;
	  mplist_set (config->vars, Mnil, NULL);
	  MPLIST_DO (plist, custom->vars)
	    {
	      variable = MPLIST_SYMBOL (MPLIST_PLIST (plist));
	      plist = mplist ();
	      mplist_add (plist, Msymbol, variable);
	      mplist_push (config->vars, Mplist, plist);
	      M17N_OBJECT_UNREF (plist);
	    }
	}
    }
  else
    {
      plist = mplist__assq (config->vars, variable);
      if (! value)
	{
	  /* Cancel the configuration.  */
	  if (! plist)
	    return 0;
	  mplist__pop_unref (plist);
	}
      else if (MPLIST_TAIL_P (value))
	{
	  /* Cancel the customization.  */
	  MInputMethodInfo *custom = get_custom_info (im_info);
	  int no_custom = (! custom || ! custom->vars
			   || ! mplist__assq (custom->vars, variable));
	  if (! plist)
	    {
	      if (no_custom)
		return 0;
	      plist = mplist ();
	      mplist_add (config->vars, Mplist, plist);
	      M17N_OBJECT_UNREF (plist);
	      plist = mplist_add (plist, Msymbol, variable);
	    }
	  else
	    {
	      if (no_custom)
		mplist__pop_unref (plist);
	      else
		{
		  plist = MPLIST_PLIST (plist); /* (NAME nil VALUE) */
		  plist = MPLIST_NEXT (plist);	/* ([nil VALUE]) */
		  mplist_set (plist, Mnil ,NULL);
		}
	    }
	}
      else
	{
	  if (plist)
	    {
	      plist = MPLIST_NEXT (MPLIST_PLIST (plist));
	      if (! MPLIST_TAIL_P (plist))
		mplist_set (plist, Mnil, NULL);
	    }
	  else
	    {
	      plist = mplist ();
	      mplist_add (config->vars, Mplist, plist);
	      M17N_OBJECT_UNREF (plist);
	      plist = mplist_add (plist, Msymbol, variable);
	      plist = MPLIST_NEXT (plist);
	    }
	  mplist_add (plist, MPLIST_KEY (value), MPLIST_VAL (value));
	}
    }
  config_all_variables (im_info);
  im_info->tick = time (NULL);
  return 0;
}

/*=*/

/***en
    @brief Get the name of per-user customization file.
    
    The minput_config_file () function returns the absolute path name
    of per-user customization file into which minput_save_config ()
    save configurations.  It is usually @c config.mic under the
    directory <tt>${HOME}/.m17n.d</tt> (${HOME} is user's home
    directory).  It is not assured that the file of the returned name
    exists nor is readable/writable.  If minput_save_config () fails
    and returns -1, an application program might check the file, make
    it writable (if possible), and try minput_save_config () again.

    @return

    This function returns a string.  As the string is kept in the
    library, the caller must not modify nor free it.

    @seealso
    minput_save_config ()
*/
/***ja
    @brief �桼����Υ������ޥ����ե������̾��������.
    
    �ؿ� minput_config_file () �ϡ��ؿ� minput_save_config () �������
    ��¸����桼����Υ������ޥ����ե�����ؤ����Хѥ�̾���֤����̾�ϡ��桼��
    �Υۡ���ǥ��쥯�ȥ�β��Υǥ��쥯�ȥ� @c ".m17n.d" �ˤ���@c
    "config.mic" �Ȥʤ롣�֤��줿̾���Υե����뤬¸�ߤ��뤫���ɤ߽񤭤�
    ���뤫���ݾڤ���ʤ����ؿ�minput_save_config () �����Ԥ��� -1 ����
    �������ˤϡ����ץꥱ�������ץ����ϥե������¸�ߤ��ǧ����
    �ʤǤ���С˽񤭹��߲�ǽ�ˤ�����minput_save_config () �����Ȥ�
    �Ǥ��롣

    @return

    ���δؿ���ʸ������֤���ʸ����ϥ饤�֥�꤬�������Ƥ���Τǡ��ƽ�
    ¦��������������������ꤹ�뤳�ȤϤǤ��ʤ���

    @seealso
    minput_save_config ()
*/

char *
minput_config_file ()
{
  MINPUT__INIT ();

  return mdatabase__file (im_custom_mdb);
}

/*=*/

/***en
    @brief Save configurations in per-user customization file.

    The minput_save_config () function saves the configurations done
    so far in the current session into the per-user customization
    file.

    @return

    If the operation was successful, 1 is returned.  If the per-user
    customization file is currently locked, 0 is returned.  In that
    case, the caller may wait for a while and try again.  If the
    configuration file is not writable, -1 is returned.  In that case,
    the caller may check the name of the file by calling
    minput_config_file (), make it writable if possible, and try
    again.

    @seealso
    minput_config_file ()  */
/***ja
    @brief �����桼����Υ������ޥ����ե��������¸����.

    �ؿ� minput_save_config () �ϸ��ԤΥ��å����Ǥ���ޤǤ˹Ԥä�����
    ��桼����Υ������ޥ����ե��������¸���롣

    @return

    ��������� 1 ���֤����桼����Υ������ޥ����ե����뤬��å�����Ƥ�
    ��� 0 ���֤������ξ�硢�ƽ�¦�Ϥ��Ф餯�Ԥäƺƻ�ԤǤ��롣����ե�
    ���뤬�񤭹����ԲĤξ�硢-1 ���֤������ξ�硢minput_config_file
    () ��Ƥ�ǥե�����̾������å������Ǥ���н񤭹��߲�ǽ�ˤ����ƻ��
    �Ǥ��롣

    @seealso
    minput_config_file ()  */

int
minput_save_config (void)
{
  MPlist *data, *tail, *plist, *p, *elt;
  int ret;

  MINPUT__INIT ();
  ret = mdatabase__lock (im_custom_mdb);
  if (ret <= 0)
    return ret;
  if (! im_config_list)
    return 1;
  update_custom_info ();
  if (! im_custom_list)
    im_custom_list = mplist ();

  /* At first, reflect configuration in customization.  */
  MPLIST_DO (plist, im_config_list)
    {
      MPlist *pl = MPLIST_PLIST (plist);
      MSymbol language, name, extra, command, variable;
      MInputMethodInfo *custom, *config;

      language = MPLIST_SYMBOL (pl);
      pl = MPLIST_NEXT (pl);
      name = MPLIST_SYMBOL (pl);
      pl = MPLIST_NEXT (pl);
      extra = MPLIST_SYMBOL (pl);
      pl = MPLIST_NEXT (pl);
      config = MPLIST_VAL (pl);
      custom = get_custom_info (config);
      if (! custom)
	custom = new_im_info (NULL, language, name, extra, im_custom_list);
      if (config->cmds)
	MPLIST_DO (pl, config->cmds)
	  {
	    elt = MPLIST_PLIST (pl);
	    command = MPLIST_SYMBOL (elt);
	    if (custom->cmds)
	      p = mplist__assq (custom->cmds, command);
	    else
	      custom->cmds = mplist (), p = NULL;
	    elt = MPLIST_NEXT (elt);
	    if (p)
	      {
		p = MPLIST_NEXT (MPLIST_NEXT (MPLIST_PLIST (p)));
		mplist_set (p, Mnil, NULL);
	      }
	    else
	      {
		p = mplist ();
		mplist_add (custom->cmds, Mplist, p);
		M17N_OBJECT_UNREF (p);
		mplist_add (p, Msymbol, command);
		p = mplist_add (p, Msymbol, Mnil);
		p = MPLIST_NEXT (p);
	      }
	    mplist__conc (p, elt);
	  }
      if (config->vars)
	MPLIST_DO (pl, config->vars)
	  {
	    elt = MPLIST_PLIST (pl);
	    variable = MPLIST_SYMBOL (elt);
	    if (custom->vars)
	      p = mplist__assq (custom->vars, variable);
	    else
	      custom->vars = mplist (), p = NULL;
	    elt = MPLIST_NEXT (elt);
	    if (p)
	      {
		p = MPLIST_NEXT (MPLIST_NEXT (MPLIST_PLIST (p)));
		mplist_set (p, Mnil, NULL);
	      }
	    else
	      {
		p = mplist ();
		mplist_add (custom->vars, Mplist, p);
		M17N_OBJECT_UNREF (p);
		mplist_add (p, Msymbol, variable);
		p = mplist_add (p, Msymbol, Mnil);
		p = MPLIST_NEXT (p);
	      }
	    mplist__conc (p, elt);
	  }
    }
  free_im_list (im_config_list);
  im_config_list = NULL;

  /* Next, reflect customization to the actual plist to be written.  */
  data = tail = mplist ();
  MPLIST_DO (plist, im_custom_list)
    {
      MPlist *pl = MPLIST_PLIST (plist);
      MSymbol language, name, extra;
      MInputMethodInfo *custom, *im_info;

      language = MPLIST_SYMBOL (pl);
      pl  = MPLIST_NEXT (pl);
      name = MPLIST_SYMBOL (pl);
      pl = MPLIST_NEXT (pl);
      extra = MPLIST_SYMBOL (pl);
      pl = MPLIST_NEXT (pl);
      custom = MPLIST_VAL (pl);
      if ((! custom->cmds || MPLIST_TAIL_P (custom->cmds))
	  && (! custom->vars || MPLIST_TAIL_P (custom->vars)))
	continue;
      im_info = lookup_im_info (im_info_list, language, name, extra);
      if (im_info)
	{
	  if (im_info->cmds)
	    config_all_commands (im_info);
	  if (im_info->vars)
	    config_all_variables (im_info);
	}
      
      elt = NULL;
      if (custom->cmds && ! MPLIST_TAIL_P (custom->cmds))
	{
	  MPLIST_DO (p, custom->cmds)
	    if (! MPLIST_TAIL_P (MPLIST_NEXT (MPLIST_PLIST (p))))
	      break;
	  if (! MPLIST_TAIL_P (p))
	    {
	      elt = mplist ();
	      pl = mplist ();
	      mplist_add (elt, Mplist, pl);
	      M17N_OBJECT_UNREF (pl);
	      pl = mplist_add (pl, Msymbol, Mcommand);
	      MPLIST_DO (p, custom->cmds)
		if (! MPLIST_TAIL_P (MPLIST_NEXT (MPLIST_PLIST (p))))
		  pl = mplist_add (pl, Mplist, MPLIST_PLIST (p));
	    }
	}
      if (custom->vars && ! MPLIST_TAIL_P (custom->vars))
	{
	  MPLIST_DO (p, custom->vars)
	    if (! MPLIST_TAIL_P (MPLIST_NEXT (MPLIST_PLIST (p))))
	      break;
	  if (! MPLIST_TAIL_P (p))
	    {
	      if (! elt)
		elt = mplist ();
	      pl = mplist ();
	      mplist_add (elt, Mplist, pl);
	      M17N_OBJECT_UNREF (pl);
	      pl = mplist_add (pl, Msymbol, Mvariable);
	      MPLIST_DO (p, custom->vars)
		if (! MPLIST_TAIL_P (MPLIST_NEXT (MPLIST_PLIST (p))))
		  pl = mplist_add (pl, Mplist, MPLIST_PLIST (p));
	    }
	}
      if (elt)
	{
	  pl = mplist ();
	  mplist_push (elt, Mplist, pl);
	  M17N_OBJECT_UNREF (pl);
	  pl = mplist_add (pl, Msymbol, Minput_method);
	  pl = mplist_add (pl, Msymbol, language);
	  pl = mplist_add (pl, Msymbol, name);
	  if (extra != Mnil)
	    pl = mplist_add (pl, Msymbol, extra);
	  tail = mplist_add (tail, Mplist, elt);
	  M17N_OBJECT_UNREF (elt);
	}
    }

  mplist_push (data, Mstring, ";; -*- mode:lisp; coding:utf-8 -*-");
  ret = mdatabase__save (im_custom_mdb, data);
  mdatabase__unlock (im_custom_mdb);
  M17N_OBJECT_UNREF (data);
  return (ret < 0 ? -1 : 1);
}

/***en
    @brief List available input methods.

    The minput_list () function returns a list of currently available
    input methods whose language is $LANGUAGE.  If $LANGUAGE is #Mnil,
    all input methods are listed.

    @return
    The returned value is a plist of this form:
	((LANGUAGE-NAME INPUT-METHOD-NAME SANE) ...)
    The third element SANE of each input method is #Mt if it can be
    successfully used, or #Mnil if it has some problem (e.g. syntax
    error of MIM file, unavailable external module, unavailable
    including input method).  */

#if EXAMPLE_CODE
#include <stdio.h>
#include <string.h>
#include <m17n.h>

int
main (int argc, char **argv)
{
  MPlist *imlist, *pl;

  M17N_INIT ();
  imlist = minput_list ((argc > 1) ? msymbol (argv[1]) : Mnil);
  for (pl = imlist; mplist_key (pl) != Mnil; pl = mplist_next (pl))
    {
      MPlist *p = mplist_value (pl);
      MSymbol lang, name, sane;

      lang = mplist_value (p);
      p = mplist_next (p);
      name = mplist_value (p);
      p = mplist_next (p);
      sane = mplist_value (p);

      printf ("%s %s %s\n", msymbol_name (lang), msymbol_name (name),
	      sane == Mt ? "ok" : "no");
    }

  m17n_object_unref (imlist);
  M17N_FINI ();
  exit (0);
}
#endif

MPlist *
minput_list (MSymbol language)
{
  MPlist *plist, *pl;
  MPlist *imlist = mplist ();
  
  MINPUT__INIT ();
  plist = mdatabase_list (Minput_method, language, Mnil, Mnil);
  if (! plist)
    return imlist;
  MPLIST_DO (pl, plist)
    {
      MDatabase *mdb = MPLIST_VAL (pl);
      MSymbol *tag = mdatabase_tag (mdb);
      MPlist *imdata, *p, *elm;
      int num_maps = 0, num_states = 0;

      if (tag[2] == Mnil)
	continue;
      imdata = mdatabase_load (mdb);
      if (! imdata)
	continue;
      MPLIST_DO (p, imdata)
	if (MPLIST_PLIST_P (p))
	  {
	    /* Check these basic functionarity:
	       All external modules (if any) are loadable.
	       All included input method (if any) are loadable.
	       At least one map is defined or included.
	       At least one state is defined or included. */
	    MPlist *elt = MPLIST_PLIST (p);
	    MSymbol key;

	    if (MFAILP (MPLIST_SYMBOL_P (elt)))
	      break;
	    key = MPLIST_SYMBOL (elt);
	    if (key == Mmap)
	      num_maps++;
	    else if (key == Mstate)
	      num_states++;
	    else if (key == Mmodule)
	      {
		MPLIST_DO (elt, MPLIST_NEXT (elt))
		  {
		    MIMExternalModule *external;

		    if (MFAILP (MPLIST_PLIST_P (elt)))
		      break;
		    external = load_external_module (MPLIST_PLIST (elt));
		    if (MFAILP (external))
		      break;
		    unload_external_module (external);
		  }
		if (! MPLIST_TAIL_P (elt))
		  break;
	      }
	    else if (key == Minclude)
	      {
		MInputMethodInfo *im_info;

		elt = MPLIST_NEXT (elt);
		if (MFAILP (MPLIST_PLIST_P (elt)))
		  break;
		im_info = get_im_info_by_tags (MPLIST_PLIST (elt));
		if (MFAILP (im_info))
		  break;
		elt = MPLIST_NEXT (elt);
		if (MFAILP (MPLIST_SYMBOL_P (elt)))
		  break;
		key = MPLIST_SYMBOL (elt);
		if (key == Mmap)
		  {
		    if (! im_info->maps)
		      break;
		    num_maps++;
		  }
		else if (key == Mstate)
		  {
		    if (! im_info->states)
		      break;
		    num_states++;
		  }
	      }
	  }
      elm = mplist ();
      mplist_add (elm, Msymbol, tag[1]);
      mplist_add (elm, Msymbol, tag[2]);
      if (MPLIST_TAIL_P (p) && num_maps > 0 && num_states > 0)
	mplist_add (elm, Msymbol, Mt);
      else
	mplist_add (elm, Msymbol, Mnil);
      mplist_push (imlist, Mplist, elm);
      M17N_OBJECT_UNREF (elm);
      M17N_OBJECT_UNREF (imdata);
    }
  M17N_OBJECT_UNREF (plist);
  return imlist;
}

/*=*/
/*** @} */
/*=*/
/***en
    @name Obsolete functions
*/
/***ja
    @name Obsolete �ʴؿ�
*/
/*** @{ */

/*=*/
/***en
    @brief Get a list of variables of an input method (obsolete).

    This function is obsolete.  Use minput_get_variable () instead.

    The minput_get_variables () function returns a plist (#MPlist) of
    variables used to control the behavior of the input method
    specified by $LANGUAGE and $NAME.  The plist is @e well-formed
    (@ref m17nPlist) of the following format:

@verbatim
    (VARNAME (DOC-MTEXT DEFAULT-VALUE [ VALUE ... ] )
     VARNAME (DOC-MTEXT DEFAULT-VALUE [ VALUE ... ] )
     ...)
@endverbatim

    @c VARNAME is a symbol representing the variable name.

    @c DOC-MTEXT is an M-text describing the variable.

    @c DEFAULT-VALUE is the default value of the variable.  It is a
    symbol, integer, or M-text.

    @c VALUEs (if any) specifies the possible values of the variable.
    If @c DEFAULT-VALUE is an integer, @c VALUE may be a plist (@c FROM
    @c TO), where @c FROM and @c TO specifies a range of possible
    values.

    For instance, suppose an input method has the variables:

    @li name:intvar, description:"value is an integer",
         initial value:0, value-range:0..3,10,20

    @li name:symvar, description:"value is a symbol",
         initial value:nil, value-range:a, b, c, nil

    @li name:txtvar, description:"value is an M-text",
         initial value:empty text, no value-range (i.e. any text)

    Then, the returned plist is as follows.

@verbatim
    (intvar ("value is an integer" 0 (0 3) 10 20)
     symvar ("value is a symbol" nil a b c nil)
     txtvar ("value is an M-text" ""))
@endverbatim

    @return
    If the input method uses any variables, a pointer to #MPlist is
    returned.  As the plist is kept in the library, the caller must not
    modify nor free it.  If the input method does not use any
    variable, @c NULL is returned.  */
/***ja
    @brief ���ϥ᥽�åɤ��ѿ��ꥹ�Ȥ�����.

    �ؿ� minput_get_variables () �ϡ�$LANGUAGE �� $NAME �ˤ�äƻ��ꤵ
    �줿���ϥ᥽�åɤο����񤤤����椹���ѿ��Υץ�ѥƥ��ꥹ��
    (#MPlist) ���֤������Υꥹ�Ȥ� @e well-formed �Ǥ���(@ref m17nPlist) ��
    ���η����Ǥ��롣

@verbatim
    (VARNAME (DOC-MTEXT DEFAULT-VALUE [ VALUE ... ] )
     VARNAME (DOC-MTEXT DEFAULT-VALUE [ VALUE ... ] )
     ...)
@endverbatim

    @c VARNAME ���ѿ���̾���򼨤�����ܥ�Ǥ��롣

    @c DOC-MTEXT ���ѿ����������� M-text �Ǥ��롣

    @c DEFAULT-VALUE ���ѿ��Υǥե�����ͤǤ��ꡢ����ܥ롢�����⤷����
    M-text �Ǥ��롣

    @c VALUE �ϡ��⤷���ꤵ��Ƥ�����ѿ��μ�������ͤ򼨤����⤷
    @c DEFAULT-VALUE �������ʤ顢 @c VALUE �� (@c FROM @c TO) �Ȥ�����
    �Υꥹ�ȤǤ��ɤ������ξ�� @c FROM �� @c TO �ϲ�ǽ���ͤ��ϰϤ򼨤���

    ��Ȥ��ơ��������ϥ᥽�åɤ����Τ褦���ѿ�����ľ���ͤ��褦��

    @li name:intvar, ����:"value is an integer",
        �����:0, �ͤ��ϰ�:0..3,10,20

    @li name:symvar, ����:"value is a symbol",
         �����:nil, �ͤ��ϰ�:a, b, c, nil

    @li name:txtvar, ����:"value is an M-text",
        �����:empty text, �ͤ��ϰϤʤ�(�ɤ�� M-text �Ǥ��)

    ���ξ�硢�֤����ꥹ�Ȥϰʲ��Τ褦�ˤʤ롣

@verbatim
    (intvar ("value is an integer" 0 (0 3) 10 20)
     symvar ("value is a symbol" nil a b c nil)
     txtvar ("value is an M-text" ""))
@endverbatim

    @return 
    ���ϥ᥽�åɤ����餫���ѿ�����Ѥ��Ƥ���� #MPlist �ؤΥݥ��󥿤��֤���
    �֤����ץ�ѥƥ��ꥹ�Ȥϥ饤�֥��ˤ�äƴ�������Ƥ��ꡢ�ƤӽФ�¦���ѹ���������������ꤷ�ƤϤʤ�ʤ���
    ���ϥ᥽�åɤ��ѿ�����ڻ��Ѥ��Ƥʤ���С�@c NULL ���֤���  */

MPlist *
minput_get_variables (MSymbol language, MSymbol name)
{
  MInputMethodInfo *im_info;
  MPlist *vars;

  MINPUT__INIT ();

  im_info = get_im_info (language, name, Mnil, Mvariable);
  if (! im_info || ! im_info->configured_vars)
    return NULL;

  M17N_OBJECT_UNREF (im_info->bc_vars);
  im_info->bc_vars = mplist ();
  MPLIST_DO (vars, im_info->configured_vars)
    {
      MPlist *plist = MPLIST_PLIST (vars);
      MPlist *elt = mplist ();

      mplist_push (im_info->bc_vars, Mplist, elt);
      mplist_add (elt, Msymbol, MPLIST_SYMBOL (plist));
      elt = MPLIST_NEXT (elt);
      mplist_set (elt, Mplist, mplist_copy (MPLIST_NEXT (plist)));
      M17N_OBJECT_UNREF (elt);
    }
  return im_info->bc_vars;
}

/*=*/

/***en
    @brief Set the initial value of an input method variable.

    The minput_set_variable () function sets the initial value of
    input method variable $VARIABLE to $VALUE for the input method
    specified by $LANGUAGE and $NAME.

    By default, the initial value is 0.

    This setting gets effective in a newly opened input method.

    @return
    If the operation was successful, 0 is returned.  Otherwise -1 is
    returned, and #merror_code is set to @c MERROR_IM.  */
/***ja
    @brief ���ϥ᥽�å��ѿ��ν���ͤ����ꤹ��.

    �ؿ� minput_set_variable () �ϡ�$LANGUAGE �� $NAME 
    �ˤ�äƻ��ꤵ�줿���ϥ᥽�åɤ����ϥ᥽�å��ѿ� $VARIABLE
    �ν���ͤ� $VALUE �����ꤹ�롣

    �ǥե���Ȥν���ͤ� 0 �Ǥ��롣

    ��������ϡ������������ץ󤵤줿���ϥ᥽�åɤ���ͭ���Ȥʤ롣

    @return
    ��������������� 0 ���֤��������Ǥʤ���� -1 ���֤���
    #merror_code �� @c MERROR_IM �����ꤹ�롣  */

int
minput_set_variable (MSymbol language, MSymbol name,
		     MSymbol variable, void *value)
{
  MPlist *plist, *pl;
  MInputMethodInfo *im_info;
  int ret;

  MINPUT__INIT ();

  if (variable == Mnil)
    MERROR (MERROR_IM, -1);
  plist = minput_get_variable (language, name, variable);
  plist = MPLIST_PLIST (plist);
  plist = MPLIST_NEXT (plist);
  pl = mplist ();
  mplist_add (pl, MPLIST_KEY (plist), value);
  ret = minput_config_variable (language, name, variable, pl);
  M17N_OBJECT_UNREF (pl);
  if (ret == 0)
    {
      im_info = get_im_info (language, name, Mnil, Mvariable);
      im_info->tick = 0;
    }
  return ret;
}

/*=*/

/***en
    @brief Get information about input method commands.

    The minput_get_commands () function returns information about
    input method commands of the input method specified by $LANGUAGE
    and $NAME.  An input method command is a pseudo key event to which
    one or more actual input key sequences are assigned.

    There are two kinds of commands, global and local.  Global
    commands are used by multiple input methods for the same purpose,
    and have global key assignments.  Local commands are used only by
    a specific input method, and have only local key assignments.

    Each input method may locally change key assignments for global
    commands.  The global key assignment for a global command is
    effective only when the current input method does not have local
    key assignments for that command.

    If $NAME is #Mnil, information about global commands is returned.
    In this case $LANGUAGE is ignored.

    If $NAME is not #Mnil, information about those commands that have
    local key assignments in the input method specified by $LANGUAGE
    and $NAME is returned.

    @return
    If no input method commands are found, this function returns @c NULL.

    Otherwise, a pointer to a plist is returned.  The key of each
    element in the plist is a symbol representing a command, and the
    value is a plist of the form COMMAND-INFO described below.

    The first element of COMMAND-INFO has the key #Mtext, and the
    value is an M-text describing the command.

    If there are no more elements, that means no key sequences are
    assigned to the command.  Otherwise, each of the remaining
    elements has the key #Mplist, and the value is a plist whose keys are
    #Msymbol and values are symbols representing input keys, which are
    currently assigned to the command.

    As the returned plist is kept in the library, the caller must not
    modify nor free it.  */
/***ja
    @brief ���ϥ᥽�åɤΥ��ޥ�ɤ˴ؤ�����������.

    �ؿ� minput_get_commands () �ϡ� $LANGUAGE �� $NAME �ˤ�äƻ��ꤵ
    �줿���ϥ᥽�åɤ����ϥ᥽�åɥ��ޥ�ɤ˴ؤ��������֤������ϥ᥽��
    �ɥ��ޥ�ɤȤϡ������������٥�ȤǤ��ꡢ���줾��ˣ��İʾ�μºݤ�
    ���ϥ������������󥹤�������Ƥ��Ƥ����Τ�ؤ���

    ���ޥ�ɤˤϥ����Х�ȥ�����Σ����ब���롣�����Х륳�ޥ��
    ��ʣ�������ϥ᥽�åɤˤ����ơ�Ʊ����Ū�ǡ������Х�ʥ����������
    ���Ѥ����롣�����륳�ޥ�ɤ���������ϥ᥽�åɤǤΤߡ�������
    �ʥ��������ǻ��Ѥ���롣

    �ġ������ϥ᥽�åɤϥ����Х륳�ޥ�ɤΥ����������ѹ����뤳�Ȥ��
    ���롣�����Х륳�ޥ���ѤΥ����Х륭��������Ƥϡ����Ѥ�������
    �᥽�åɤˤ����Ƥ��Υ��ޥ���ѤΥ�����ʥ���������¸�ߤ��ʤ����
    �ˤΤ�ͭ���Ǥ��롣

    $NAME �� #Mnil �Ǥ���С������Х륳�ޥ�ɤ˴ؤ��������֤�������
    ��硢$LANGUAGE ��̵�뤵��롣

    $NAME �� #Mnil �Ǥʤ���С�$LANGUAGE �� $NAME �ˤ�äƻ��ꤵ�����
    �ϥ᥽�åɤ��֤��������ʥ���������Ƥ���ĥ��ޥ�ɤ˴ؤ������
    ���֤���

    @return
    ���ϥ᥽�åɥ��ޥ�ɤ����Ĥ���ʤ���С����δؿ��� @c NULL ���֤���

    �����Ǥʤ���Хץ�ѥƥ��ꥹ�ȤؤΥݥ��󥿤��֤����ꥹ�Ȥγ����Ǥ�
    �����ϸġ��Υ��ޥ�ɤ򼨤�����ܥ�Ǥ��ꡢ�ͤϲ����� COMMAND-INFO
    �η����Υץ�ѥƥ��ꥹ�ȤǤ��롣

    COMMAND-INFO ��������ǤΥ����� #Mtext �ޤ��� #Msymbol �Ǥ��롣����
    �� #Mtext �ʤ顢�ͤϤ��Υ��ޥ�ɤ��������� M-text �Ǥ��롣������
    #Msymbol �ʤ��ͤ� #Mnil �Ǥ��ꡢ���Υ��ޥ�ɤ������ƥ����Ȥ������
    �����Ȥˤʤ롣

    ����ʳ������Ǥ�̵����С����Υ��ޥ�ɤ��Ф��ƥ������������󥹤���
    �����Ƥ��Ƥ��ʤ����Ȥ��̣���롣�����Ǥʤ���С��Ĥ�γ����Ǥϥ�
    ���Ȥ���#Mplist ���ͤȤ��ƥץ�ѥƥ��ꥹ�Ȥ���ġ����Υץ�ѥƥ�
    �ꥹ�ȤΥ����� #Msymbol �Ǥ��ꡢ�ͤϸ��ߤ��Υ��ޥ�ɤ˳�����Ƥ��
    �Ƥ������ϥ�����ɽ������ܥ�Ǥ��롣

    �֤����ץ�ѥƥ��ꥹ�Ȥϥ饤�֥��ˤ�äƴ�������Ƥ��ꡢ�Ƥӽ�
    ��¦���ѹ���������������ꤷ�ƤϤʤ�ʤ���*/

MPlist *
minput_get_commands (MSymbol language, MSymbol name)
{
  MInputMethodInfo *im_info;
  MPlist *cmds;

  MINPUT__INIT ();

  im_info = get_im_info (language, name, Mnil, Mcommand);
  if (! im_info || ! im_info->configured_vars)
    return NULL;
  M17N_OBJECT_UNREF (im_info->bc_cmds);
  im_info->bc_cmds = mplist ();
  MPLIST_DO (cmds, im_info->configured_cmds)
    {
      MPlist *plist = MPLIST_PLIST (cmds);
      MPlist *elt = mplist ();

      mplist_push (im_info->bc_cmds, Mplist, elt);
      mplist_add (elt, MPLIST_SYMBOL (plist),
		  mplist_copy (MPLIST_NEXT (plist)));
      M17N_OBJECT_UNREF (elt);
    }
  return im_info->bc_cmds;
}

/*=*/

/***en
    @brief Assign a key sequence to an input method command (obsolete).

    This function is obsolete.  Use minput_config_command () instead.

    The minput_assign_command_keys () function assigns input key
    sequence $KEYSEQ to input method command $COMMAND for the input
    method specified by $LANGUAGE and $NAME.  If $NAME is #Mnil, the
    key sequence is assigned globally no matter what $LANGUAGE is.
    Otherwise the key sequence is assigned locally.

    Each element of $KEYSEQ must have the key $Msymbol and the value
    must be a symbol representing an input key.

    $KEYSEQ may be @c NULL, in which case, all assignments are deleted
    globally or locally.

    This assignment gets effective in a newly opened input method.

    @return
    If the operation was successful, 0 is returned.  Otherwise -1 is
    returned, and #merror_code is set to @c MERROR_IM.  */
/***ja
    @brief ���ϥ᥽�åɥ��ޥ�ɤ˥������������󥹤������Ƥ�.

    �ؿ� minput_assign_command_keys () �ϡ� $LANGUAGE �� $NAME �ˤ�ä�
    ���ꤵ�줿���ϥ᥽�å��Ѥ����ϥ᥽�åɥ��ޥ�� $COMMAND ���Ф��ơ�
    ���ϥ������������� $KEYSEQ �������Ƥ롣 $NAME �� #Mnil �ʤ�С�
    $LANGUAGE �˴ط��ʤ������ϥ������������󥹤ϥ����Х�˳�����Ƥ�
    ��롣�����Ǥʤ�С�������Ƥϥ�����Ǥ��롣

    $KEYSEQ �γ����Ǥϥ����Ȥ��� $Msymbol ���ͤȤ������ϥ�����ɽ����
    ��ܥ������ʤ��ƤϤʤ�ʤ���

    $KEYSEQ �� @c NULL �Ǥ�褤�����ξ�硢�����Х�⤷���ϥ������
    ���٤Ƥγ�����Ƥ��õ��롣

    ���γ�����Ƥϡ�������ưʹ߿����������ץ󤵤줿���ϥ᥽�åɤ���ͭ
    ���ˤʤ롣

    @return 
    ��������������� 0 ���֤��������Ǥʤ���� -1 ���֤���
    #merror_code �� @c MERROR_IM �����ꤹ�롣  */

int
minput_assign_command_keys (MSymbol language, MSymbol name,
			    MSymbol command, MPlist *keyseq)
{
  int ret;

  MINPUT__INIT ();

  if (command == Mnil)
    MERROR (MERROR_IM, -1);
  if (keyseq)
    {
      MPlist *plist;

      if  (! check_command_keyseq (keyseq))
	MERROR (MERROR_IM, -1);
      plist = mplist ();
      mplist_add (plist, Mplist, keyseq);
      keyseq = plist;
    }  
  else
    keyseq = mplist ();
  ret = minput_config_command (language, name, command, keyseq);
  M17N_OBJECT_UNREF (keyseq);
  return ret;
}

/*=*/

/***en
    @brief Parse input method names

    The minput_parse_im_names () function parses M-text $MT and returns
    a list of input method names.  Input method names in $MT must be
    separated by comma (",").  Input methods whose language is #Mt can
    be specified by its name only (i.e. just "latn-post" instead of
    "t-latn-post").

    @return
    The minput_parse_im_names () returns a plist of which elements are
    plist of LANGUAGE and NAME of input methods as below:
	((LANGUAGE1 NAME1) (LANGUAGE2 NAME2) ...)
    Both LANGUAGEn and NAMEn are symbols.  LANGUAGEn is #Mt if the
    corresponding input method is not limited to a specific language.
    If a specified input method doesn't exist, the corresponding
    element in the above plist is a sub-part of $MT for that
    non-existing input method name.
    For instance, if "symbol,unknown,unicode" is specified as $MT and
    "unknown" doesn't exist, the return value is:
	((t symbol) "unknown" (t unicode))	
  */

MPlist *
minput_parse_im_names (MText *mt)
{
  int from = 0, to = mtext_len (mt), i, start = 0;
  MPlist *plist = mplist ();;
  char *buf;
  
  MINPUT__INIT ();

  MTABLE_MALLOC (buf, to + 1, MERROR_IM);
  i = 0;
  while (1)
    {
      MSymbol lang = Mnil, name = Mnil;
      int c;
      int idx = 0, name_idx = 0;
      MInputMethodInfo *im_info;

      while (i < to && ((c = mtext_ref_char (mt, i)) == ' ' || c >= 128))
	i++;
      if (i >= to)
	break;
      buf[idx++] = c;
      i++;
      while (i < to && ((c = mtext_ref_char (mt, i)) != ',' && c != ' ')) 
	{
	  if (c == '-' && name_idx == 0)
	    {
	      buf[idx] = '\0';
	      lang = msymbol (buf);
	      name_idx = idx + 1;
	    }
	  buf[idx++] = c;
	  i++;
	}
      buf[idx] = '\0';
      name = msymbol (buf + name_idx);
      if (name_idx == 0)
	lang = Mt;

      im_info = get_im_info (lang, name, Mnil, Mnil);
      if (! im_info && lang != Mt)
	{
	  lang = Mt;
	  name = msymbol (buf);
	  im_info = get_im_info (lang, name, Mnil, Mnil);	  
	}
      if (im_info)
	{
	  MPlist *p = mplist ();

	  mplist_add (p, Msymbol, lang);
	  mplist_add (p, Msymbol, name);
	  mplist_add (plist, Mplist, p);
	  M17N_OBJECT_UNREF (p);
	}
      else
	{
	  MText *err = mtext__from_data (buf, idx, MTEXT_FORMAT_US_ASCII, 1);
	  mplist_add (plist, Mtext, err);
	  M17N_OBJECT_UNREF (err);
	}
      i++;
    }
  free (buf);
  return plist;
}


/*=*/

/***en
    @brief Call a callback function

    The minput_callback () functions calls a callback function
    $COMMAND assigned for the input context $IC.  The caller must set
    specific elements in $IC->plist if the callback function requires.

    @return
    If there exists a specified callback function, 0 is returned.
    Otherwise -1 is returned.  By side effects, $IC->plist may be
    modified.  */

int
minput_callback (MInputContext *ic, MSymbol command)
{
  MInputCallbackFunc func;

  if (! ic->im->driver.callback_list)
    return -1;
  func = ((MInputCallbackFunc)
	  mplist_get_func (ic->im->driver.callback_list, command));
  if (! func)
    return -1;
  (func) (ic, command);
  return 0;
}

/*** @} */ 
/*** @} */
/*=*/
/*** @addtogroup m17nDebug */
/*=*/
/*** @{  */
/*=*/

/***en
    @brief Dump an input method.

    The mdebug_dump_im () function prints the input method $IM in a
    human readable way to the stderr or to what specified by the
    environment variable MDEBUG_OUTPUT_FILE.  $INDENT specifies how
    many columns to indent the lines but the first one.

    @return
    This function returns $IM.  */
/***ja
    @brief ���ϥ᥽�åɤ����פ���.

    �ؿ� mdebug_dump_im () �����ϥ᥽�å� $IM ��ɸ�२�顼���Ϥ⤷����
    �Ķ��ѿ� MDEBUG_DUMP_FONT �ǻ��ꤵ�줿�ե�����˿ʹ֤˲��ɤʷ��ǽ�
    �Ϥ��롣$INDENT �ϣ����ܰʹߤΥ���ǥ�Ȥ���ꤹ�롣

    @return
    ���δؿ��� $IM ���֤���  */

MInputMethod *
mdebug_dump_im (MInputMethod *im, int indent)
{
  MInputMethodInfo *im_info = (MInputMethodInfo *) im->info;
  char *prefix;

  prefix = (char *) alloca (indent + 1);
  memset (prefix, 32, indent);
  prefix[indent] = '\0';

  fprintf (mdebug__output, "(input-method %s %s ", msymbol_name (im->language),
	   msymbol_name (im->name));
  mdebug_dump_mtext (im_info->title, 0, 0);
  if (im->name != Mnil)
    {
      MPlist *state;

      MPLIST_DO (state, im_info->states)
	{
	  fprintf (mdebug__output, "\n%s  ", prefix);
	  dump_im_state (MPLIST_VAL (state), indent + 2);
	}
    }
  fprintf (mdebug__output, ")");
  return im;
}

/*** @} */ 

/*
  Local Variables:
  coding: euc-japan
  End:
*/
