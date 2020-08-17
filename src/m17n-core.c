/* m17n-core.c -- body of the CORE API.
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
    @addtogroup m17nIntro
    @brief Introduction to the m17n library.

    <em>API LEVELS</em>

    The API of the m17n library is divided into these five.

    <ol>
    <li> CORE API

    It provides basic modules to handle M-texts.  To use this API, an
    application program must include <m17n-core<EM></EM>.h> and be
    linked with -lm17n-core.

    <li> SHELL API

    It provides modules for character properties, character set
    handling, code conversion, etc.  They load various kinds of
    data from the database on demand.  To use this API, an application
    program must include <m17n<EM></EM>.h> and be linked with
    -lm17n-core -lm17n.

    When you use this API, CORE API is also available.

    <li> FLT API

    It provides modules for text shaping using @ref mdbFLT.  To use
    this API, an application program must include <m17n<EM></EM>.h>
    and be linked with -lm17n-core -lm17n-flt.

    When you use this API, CORE API is also available.

    <li> GUI API

    It provides GUI modules such as drawing and inputting M-texts on a
    graphic device.  This API itself is independent of graphic
    devices, but most functions require an argument MFrame that is
    created for a specific type of graphic devices.  The currently
    supported graphic devices are null device, the X Window System,
    and image data (gdImagePtr) of the GD library.

    On a frame of a null device, you cannot draw text nor use input
    methods.  However, functions like mdraw_glyph_list (), etc. are
    available.

    On a frame of the X Window System, you can use the whole GUI API.

    On a frame of the GD library, you can use all drawing API but
    cannot use input methods.

    To use this API, an application program must include
    <m17n-gui<EM></EM>.h> and be linked with -lm17n-core -lm17n
    -lm17n-gui.

    When you use this API, CORE, SHELL, and FLT APIs are also
    available.

    <li> MISC API

    It provides miscellaneous functions to support error handling and
    debugging.  This API cannot be used standalone; it must be used
    with one or more APIs listed above.  To use this API, an
    application program must include <m17n-misc<EM></EM>.h> in
    addition to one of the header files described above.

    </ol>

    See also the section @ref m17n-config "m17n-config(1)".

    <em>ENVIRONMENT VARIABLES</em>

    The m17n library pays attention to the following environment
    variables.

    <ul>
    <li> @c M17NDIR

    The name of the directory that contains data of the m17n database.
    See @ref m17nDatabase for details.

    <li> @c MDEBUG_XXX

    Environment variables whose names start with "MDEBUG_" control
    debug information output.  See @ref m17nDebug for details.

    </ul>

    <em>API NAMING CONVENTION</em>

    The m17n library exports functions, variables, macros, and types.
    All of them start with the letter 'm' or 'M', and are followed by
    an object name (e.g. "symbol", "plist") or a module name
    (e.g. draw, input).  Note that the name of M-text objects start
    with "mtext" and not with "mmtext".

    <ul>

    <li> functions -- mobject () or mobject_xxx ()

    They start with 'm' and are followed by an object name in lower
    case.  Words are separated by '_'.  For example, msymbol (),
    mtext_ref_char (), mdraw_text ().

    <li> non-symbol variables -- mobject, or mobject_xxx
    
    The naming convention is the same as functions (e.g. mface_large).

    <li> symbol variables -- Mname

    Variables of the type MSymbol start with 'M' and are followed by
    their names.  Words are separated by '_'.  For example, Mlanguage
    (the name is "language"), Miso_2022 (the name is "iso-2022").

    <li> macros -- MOBJECT_XXX

    They start with 'M' and are followed by an object name in upper
    case.  Words are separated by '_'.

    <li> types -- MObject or MObjectXxx

    They start with 'M' and are followed by capitalized object names.
    Words are concatenated directly and no '_' are used.  For example,
    MConverter, MInputDriver.

    </ul>

  */

/***ja
    @addtogroup m17nIntro
    @brief m17n �饤�֥�� ����ȥ���������.

    @em API�Υ�٥�

    m17n �饤�֥��� API �ϰʲ��Σ����ʬ�व��Ƥ��롣

    <ol>
    <li> ���� API

    M-text �򰷤�����δ���Ū�ʥ⥸�塼����󶡤��롣
    ���� API �����Ѥ��뤿��ˤϡ����ץꥱ�������ץ�����
    <m17n-core<EM></EM>.h> �� include ���� -lm17n-core
    �ǥ�󥯤���ʤ��ƤϤʤ�ʤ���

    <li> ������ API

    ʸ���ץ�ѥƥ���ʸ���������������Ѵ����Τ���Υ⥸�塼����󶡤��롣
    �����Υ⥸�塼��ϡ��ǡ����١�������ɬ�פ˱�����¿�ͤʥǡ�������ɤ��롣
    ���� API �����Ѥ��뤿��ˤϡ����ץꥱ�������ץ�����
    <m17n<EM></EM>.h> �� include ���� -lm17n-core -lm17n
    �ǥ�󥯤���ʤ��ƤϤʤ�ʤ���

    ���� API ����Ѥ���С����� API �⼫ưŪ�˻��Ѳ�ǽ�Ȥʤ롣

    <li> FLT API

    ʸ����ɽ���� @ref mdbFLT ���Ѥ���⥸�塼����󶡤��롣���� API
    �����Ѥ��뤿��ˤϡ����ץꥱ�������ץ����� <m17n<EM></EM>.h> 
    �� include ���� -lm17n-core -lm17n-flt �ǥ�󥯤���ʤ��ƤϤʤ�ʤ���

    ���� API ����Ѥ���С����� API �⼫ưŪ�˻��Ѳ�ǽ�Ȥʤ롣

    <li> GUI API

    ����ե��å��ǥХ������ M-text ��ɽ�����������Ϥ����ꤹ�뤿���
    GUI �⥸�塼����󶡤��롣���� API
    ���Τϥ���ե��å��ǥХ����Ȥ���Ω�Ǥ��뤬��
    ¿���δؿ�������Υ���ե��å��ǥХ����Ѥ˺������줿 
    MFrame ������˼�롣
    �������ǥ��ݡ��Ȥ���Ƥ��륰��ե��å��ǥХ����ϡ��̥�ǥХ�����X
    ������ɥ������ƥࡢ����� GD �饤�֥��Υ��᡼���ǡ���
    (gdImagePtr) �Ǥ��롣

    �̥�ǥХ����Υե졼���Ǥ�ɽ�������Ϥ�Ǥ��ʤ���������
    mdraw_glyph_list () �ʤɤδؿ��ϻ��Ѳ�ǽ�Ǥ��롣

    X ������ɥ������ƥ�Υե졼���ǤϤ��٤Ƥ� GUI API �����ѤǤ��롣

    GD �饤�֥��Υե졼���Ǥϡ������Ѥ� API
    �Ϥ��٤ƻ��ѤǤ��뤬�����ϤϤǤ��ʤ���

    ���� API ����Ѥ��뤿��ˤϡ����ץꥱ�������ץ�����
    <m17n-gui<EM></EM>.h> �� include ����-lm17n-core -lm17n -lm17n-gui
    �ǥ�󥯤���ʤ��ƤϤʤ�ʤ���

    ���� API ����Ѥ���С����� API�������� API������� FLT API
    �⼫ưŪ�˻��Ѳ�ǽ�Ȥʤ롣

    <li> ����¾�� API

    ���顼�������ǥХå��ѤΤ���¾�δؿ����󶡤��롣���� API
    �Ϥ�������Ǥϻ��ѤǤ������嵭��¾�� API
    �ȶ��˻Ȥ������Ѥ��뤿��ˤϡ��嵭�Τ����줫��include
    �ե�����˲ä��ơ� <m17n-misc<EM></EM>.h> ��include
    ���ʤ��ƤϤʤ�ʤ���

    </ol>

    @ref m17n-config "m17n-config(1)" ��⻲�ȡ�

    @em �Ķ��ѿ�

    m17n �饤�֥��ϰʲ��δĶ��ѿ��򻲾Ȥ��롣

    <ul>
    <li> @c M17NDIR

    m17n �ǡ����١����Υǡ������Ǽ�����ǥ��쥯�ȥ��̾�����ܺ٤� @ref
    m17nDatabase ���ȡ�

    <li> @c MDEBUG_XXX

    "MDEBUG_" �ǻϤޤ�̾������ĴĶ��ѿ��ϥǥХå�����ν��Ϥ����椹�롣
    �ܺ٤� @ref m17nDebug ���ȡ�

    </ul>

    @em API @em ��̿̾��§

    m17n �饤�֥��ϡ��ؿ����ѿ����ޥ������� export ���롣������ 'm' 
    �ޤ��� 'M' �Τ��Ȥ˥��֥�������̾ ("symbol"��"plist" �ʤ�)
    �ޤ��ϥ⥸�塼��̾ (draw, input �ʤ�) ��³������ΤǤ��롣
    M-text ���֥������Ȥ�̾���� "mmtext" �ǤϤʤ��� "mtext"
    �ǻϤޤ뤳�Ȥ���ա�
    
    <ul>

    <li> �ؿ� -- mobject () �ޤ��� mobject_xxx ()

    'm' �Τ��Ȥ˾�ʸ���ǥ��֥�������̾��³����ñ��֤� '_'
    �Ƕ��ڤ��롣���Ȥ��С�msymbol (),
     mtext_ref_char (), mdraw_text () �ʤɡ�

    <li> ����ܥ�Ǥʤ��ѿ� -- mobject,  �ޤ��� mobject_xxx
    
    �ؿ���Ʊ��̿̾��§�˽��������Ȥ���  mface_large �ʤɡ�

    <li> ����ܥ��ѿ� -- Mname

    MSymbol ���ѿ��ϡ�'M' �θ��̾����³����ñ��֤� '_'
    �Ƕ��ڤ��롣���Ȥ��� Mlanguage (̾���� "language"), Miso_2022
    (̾����"iso-2022") �ʤɡ�

    <li> �ޥ��� -- MOBJECT_XXX

    'M' �θ����ʸ���ǥ��֥�������̾��³����ñ��֤� '_' �Ƕ��ڤ��롣

    <li> ������ -- MObject �ޤ��� MObjectXxx

    'M' �θ����ʸ���ǻϤޤ륪�֥�������̾��³����ñ���Ϣ³���ƽ񤫤졢
    '_' ���Ѥ����ʤ������Ȥ��� MConverter, MInputDriver �ʤɡ�

    </ul>
    
    */
/*=*/
/*** @{ */
#ifdef FOR_DOXYGEN
/***en
    The #M17NLIB_MAJOR_VERSION macro gives the major version number
    of the m17n library.  */
/***ja
    �ޥ��� #M17NLIB_MAJOR_VERSION �� m17n 
    �饤�֥��Υ᥸�㡼�С�������ֹ��Ϳ����.  */

#define M17NLIB_MAJOR_VERSION

/*=*/

/***en
    The #M17NLIB_MINOR_VERSION macro gives the minor version number
    of the m17n library.  */

/***ja
    �ޥ��� #M17NLIB_MINOR_VERSION �� m17n 
    �饤�֥��Υޥ��ʡ��С�������ֹ��Ϳ����.  */

#define M17NLIB_MINOR_VERSION

/*=*/

/***en
    The #M17NLIB_PATCH_LEVEL macro gives the patch level number
    of the m17n library.  */

/***ja
    �ޥ��� #M17NLIB_PATCH_LEVEL �� m17n 
    �饤�֥��Υѥå���٥��ֹ��Ϳ����.  */

#define M17NLIB_PATCH_LEVEL

/*=*/

/***en
    The #M17NLIB_VERSION_NAME macro gives the version name of the
    m17n library as a string.  */

/***ja
    �ޥ��� #M17NLIB_VERSION_NAME �� m17n 
    �饤�֥��ΥС������̾��ʸ����Ȥ���Ϳ����.  */

#define M17NLIB_VERSION_NAME

/*=*/

/***en
    @brief Initialize the m17n library.

    The macro M17N_INIT () initializes the m17n library.  This macro
    must be called before any m17n functions are used.

    It is safe to call this macro multiple times, but in that case,
    the macro M17N_FINI () must be called the same times to free the
    memory.

    If the initialization was successful, the external variable
    #merror_code is set to 0.  Otherwise it is set to -1.

    @seealso
    M17N_FINI (), m17n_status ()  */

/***ja
    @brief m17n �饤�֥�����������.

    �ޥ��� M17N_INIT () �� m17n �饤�֥����������롣m17n 
    �δؿ������Ѥ������ˡ����Υޥ����ޤ��ƤФʤ��ƤϤʤ�ʤ���
    
    ���Υޥ����ʣ����Ƥ�Ǥ�����Ǥ��뤬�����ξ������������뤿��˥ޥ���
    M17N_FINI () ��Ʊ������Ƥ�ɬ�פ����롣

    �����ѿ� #merror_code �ϡ����������������� 0 �ˡ������Ǥʤ���� 
    -1 �����ꤵ��롣  

    @seealso
    M17N_FINI (), m17n_status ()  */

#define M17N_INIT()

/*=*/

/***en
    @brief Finalize the m17n library.

    The macro M17N_FINI () finalizes the m17n library.  It frees all the
    memory area used by the m17n library.  Once this macro is
    called, no m17n functions should be used until the
    macro M17N_INIT () is called again.

    If the macro M17N_INIT () was called N times, the Nth call of this
    macro actually free the memory. 

    @seealso
    M17N_INIT (), m17n_status ()  */
/***ja
    @brief m17n �饤�֥���λ����. 

    �ޥ��� M17N_FINI () �� m17n �饤�֥���λ���롣m17n 
    �饤�֥�꤬�Ȥä����ƤΥ����ΰ�ϲ�������롣���٤��Υޥ����ƤФ줿�顢�ޥ���
    M17N_INIT () �����ٸƤФ��ޤ� m17n �ؿ��ϻȤ��٤��Ǥʤ���

    �ޥ��� M17N_INIT () �� N ��ƤФ�Ƥ������ˤϡ����Υޥ��� N 
    ��ƤФ�ƽ��ƥ��꤬��������롣

    @seealso
    M17N_INIT (), m17n_status ()  */

#define M17N_FINI()
#endif /* FOR_DOXYGEN */
/*=*/
/*** @} */ 
/*=*/

#if !defined (FOR_DOXYGEN) || defined (DOXYGEN_INTERNAL_MODULE)
/*** @addtogroup m17nInternal
     @{ */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#include "m17n-core.h"
#include "m17n-misc.h"
#include "internal.h"
#include "symbol.h"

static void
default_error_handler (enum MErrorCode err)
{
  exit (err);
}

static struct timeval time_stack[16];
static int time_stack_index;

static M17NObjectArray *object_array_root;

static void
report_object_array ()
{
  fprintf (mdebug__output, "%16s %7s %7s %7s\n",
	   "object", "created", "freed", "alive");
  fprintf (mdebug__output, "%16s %7s %7s %7s\n",
	   "------", "-------", "-----", "-----");
  for (; object_array_root; object_array_root = object_array_root->next)
    {
      M17NObjectArray *array = object_array_root;

      fprintf (mdebug__output, "%16s %7d %7d %7d\n", array->name,
	       array->used, array->used - array->count, array->count);
      if (array->count > 0)
	{
	  int i;
	  for (i = 0; i < array->used && ! array->objects[i]; i++);

	  if (strcmp (array->name, "M-text") == 0)
	    {
	      MText *mt = (MText *) array->objects[i];

	      if (mt->format <= MTEXT_FORMAT_UTF_8)
		fprintf (mdebug__output, "\t\"%s\"\n", (char *) mt->data);
	    }
	  else if (strcmp (array->name, "Plist") == 0)
	    {
	      MPlist *plist = (MPlist *) array->objects[i];

	      mdebug_dump_plist (plist, 8);
	      fprintf (mdebug__output, "\n");
	    }
	}

      if (array->objects != NULL)
	{
	  free (array->objects);
	  array->count = array->used = 0;
	}
    }
}



/* Internal API */

int m17n__core_initialized;
int m17n__shell_initialized;
int m17n__gui_initialized;

int mdebug__flags[MDEBUG_MAX];
FILE *mdebug__output;

void
mdebug__push_time ()
{
  struct timezone tz;

  gettimeofday (time_stack + time_stack_index++, &tz);
}

void
mdebug__pop_time ()
{
  time_stack_index--;
}

void
mdebug__print_time ()
{
  struct timeval tv;
  struct timezone tz;
  long diff;

  gettimeofday (&tv, &tz);
  diff = ((tv.tv_sec - time_stack[time_stack_index - 1].tv_sec) * 1000000
	  + (tv.tv_usec - time_stack[time_stack_index - 1].tv_usec));
  fprintf (mdebug__output, "%8ld ms.", diff);
  time_stack[time_stack_index - 1] = tv;
}

static void
SET_DEBUG_FLAG (char *env_name, enum MDebugFlag flag)
{
  char *env_value = getenv (env_name);

  if (env_value)
    {
      int int_value = atoi (env_value);

      if (flag == MDEBUG_ALL)
	{
	  int i;
	  for (i = 0; i < MDEBUG_MAX; i++)
	    mdebug__flags[i] = int_value;
	}
      else
	mdebug__flags[flag] = int_value;
    }
}

void
mdebug__add_object_array (M17NObjectArray *array, char *name)
{
  array->name = name;
  array->count = 0;
  array->next = object_array_root;
  object_array_root = array;
}


void
mdebug__register_object (M17NObjectArray *array, void *object)
{
  if (array->objects == NULL)
    MLIST_INIT1 (array, objects, 256);
  array->count++;
  MLIST_APPEND1 (array, objects, object, MERROR_OBJECT);
}

void
mdebug__unregister_object (M17NObjectArray *array, void *object)
{
  array->count--;
  if (array->count >= 0)
    {
      int i;

      for (i = array->used - 1; i >= 0 && array->objects[i] != object; i--);
      if (i >= 0)
	{
	  if (i == array->used - 1)
	    array->used--;
	  array->objects[i] = NULL;
	}
      else
	mdebug_hook ();
    }
  else									\
    mdebug_hook ();
}


/* External API */

/* The following two are actually not exposed to a user but concealed
   by the macro M17N_INIT (). */

void
m17n_init_core (void)
{
  int mdebug_flag = MDEBUG_INIT;

  merror_code = MERROR_NONE;
  if (m17n__core_initialized++)
    return;

  m17n_memory_full_handler = default_error_handler;

  SET_DEBUG_FLAG ("MDEBUG_ALL", MDEBUG_ALL);
  SET_DEBUG_FLAG ("MDEBUG_INIT", MDEBUG_INIT);
  SET_DEBUG_FLAG ("MDEBUG_FINI", MDEBUG_FINI);
  SET_DEBUG_FLAG ("MDEBUG_CHARSET", MDEBUG_CHARSET);
  SET_DEBUG_FLAG ("MDEBUG_CODING", MDEBUG_CODING);
  SET_DEBUG_FLAG ("MDEBUG_DATABASE", MDEBUG_DATABASE);
  SET_DEBUG_FLAG ("MDEBUG_FONT", MDEBUG_FONT); 
  SET_DEBUG_FLAG ("MDEBUG_FLT", MDEBUG_FLT);
  SET_DEBUG_FLAG ("MDEBUG_FONTSET", MDEBUG_FONTSET);
  SET_DEBUG_FLAG ("MDEBUG_INPUT", MDEBUG_INPUT);
  /* for backward compatibility... */
  SET_DEBUG_FLAG ("MDEBUG_FONT_FLT", MDEBUG_FLT);
  SET_DEBUG_FLAG ("MDEBUG_FONT_OTF", MDEBUG_FLT);
  {
    char *env_value = getenv ("MDEBUG_OUTPUT_FILE");

    mdebug__output = NULL;
    if (env_value)
      {
	if (strcmp (env_value, "stdout") == 0)
	  mdebug__output = stdout;
	else
	  mdebug__output = fopen (env_value, "a");
      }
    if (! mdebug__output)
      mdebug__output = stderr;
  }

  MDEBUG_PUSH_TIME ();
  MDEBUG_PUSH_TIME ();
  if (msymbol__init () < 0)
    goto err;
  MDEBUG_PRINT_TIME ("INIT", (mdebug__output, " to initialize symbol module."));
  if  (mplist__init () < 0)
    goto err;
  MDEBUG_PRINT_TIME ("INIT", (mdebug__output, " to initialize plist module."));
  if (mchar__init () < 0)
    goto err;
  MDEBUG_PRINT_TIME ("INIT",
		     (mdebug__output, " to initialize character module."));
  if  (mchartable__init () < 0)
    goto err;
  MDEBUG_PRINT_TIME ("INIT",
		     (mdebug__output, " to initialize chartable module."));
  if (mtext__init () < 0 || mtext__prop_init () < 0)
    goto err;
  MDEBUG_PRINT_TIME ("INIT", (mdebug__output, " to initialize mtext module."));
  if (mdatabase__init () < 0)
    goto err;
  MDEBUG_PRINT_TIME ("INIT",
		     (mdebug__output, " to initialize database module."));

#if ENABLE_NLS
  bindtextdomain ("m17n-lib", GETTEXTDIR);
  bindtextdomain ("m17n-db", GETTEXTDIR);
  bindtextdomain ("m17n-contrib", GETTEXTDIR);
  bind_textdomain_codeset ("m17n-lib", "UTF-8");
  bind_textdomain_codeset ("m17n-db", "UTF-8");
  bind_textdomain_codeset ("m17n-contrib", "UTF-8");
#endif

 err:
  MDEBUG_POP_TIME ();
  MDEBUG_PRINT_TIME ("INIT",
		     (mdebug__output, " to initialize the core modules."));
  MDEBUG_POP_TIME ();
}

void
m17n_fini_core (void)
{
  int mdebug_flag = MDEBUG_FINI;

  if (m17n__core_initialized == 0
      || --m17n__core_initialized > 0)
    return;

  MDEBUG_PUSH_TIME ();
  MDEBUG_PUSH_TIME ();
  mchartable__fini ();
  MDEBUG_PRINT_TIME ("FINI", (mdebug__output, " to finalize chartable module."));
  mtext__fini ();
  MDEBUG_PRINT_TIME ("FINI", (mdebug__output, " to finalize mtext module."));
  msymbol__fini ();
  MDEBUG_PRINT_TIME ("FINI", (mdebug__output, " to finalize symbol module."));
  mplist__fini ();
  MDEBUG_PRINT_TIME ("FINI", (mdebug__output, " to finalize plist module."));
  /* We must call this after the aboves because it frees interval
     pools.  */
  mtext__prop_fini ();
  MDEBUG_PRINT_TIME ("FINI", (mdebug__output, " to finalize textprop module."));
  MDEBUG_POP_TIME ();
  MDEBUG_PRINT_TIME ("FINI", (mdebug__output, " to finalize the core modules."));
  MDEBUG_POP_TIME ();
  if (mdebug__flags[MDEBUG_FINI])
    report_object_array ();
  msymbol__free_table ();
  if (mdebug__output != stderr)
    fclose (mdebug__output);
}

/*** @} */
#endif /* !FOR_DOXYGEN || DOXYGEN_INTERNAL_MODULE */
/*=*/

/*** @addtogroup m17nIntro */

/*** @{  */
/*=*/

/***en
    @brief Report which part of the m17n library is initialized.

    The m17n_status () function returns one of these values depending
    on which part of the m17n library is initialized:

	#M17N_NOT_INITIALIZED, #M17N_CORE_INITIALIZED,
	#M17N_SHELL_INITIALIZED, #M17N_GUI_INITIALIZED  */

/***ja
    @brief m17n �饤�֥��Τɤ���ʬ����������줿����𤹤�.

    �ؿ� m17n_status () �� 
    m17n �饤�֥��Τɤ���ʬ����������줿���˱����ơ��ʲ����ͤΤ����줫���֤���

	#M17N_NOT_INITIALIZED, #M17N_CORE_INITIALIZED,
	#M17N_SHELL_INITIALIZED, #M17N_GUI_INITIALIZED  */

enum M17NStatus
m17n_status (void)
{
  return (m17n__gui_initialized ? M17N_GUI_INITIALIZED
	  : m17n__shell_initialized ? M17N_SHELL_INITIALIZED
	  : m17n__core_initialized ? M17N_CORE_INITIALIZED
	  : M17N_NOT_INITIALIZED);
}

/*** @} */

/*=*/
/***en
    @addtogroup m17nObject
    @brief Managed objects are objects managed by the reference count.

    There are some types of m17n objects that are managed by their
    reference count.  Those objects are called @e managed @e objects.
    When created, the reference count of a managed object is
    initialized to one.  The m17n_object_ref () function increments
    the reference count of a managed object by one, and the
    m17n_object_unref () function decrements by one.  A managed
    object is automatically freed when its reference count becomes
    zero.

    A property whose key is a managing key can have only a managed
    object as its value.  Some functions, for instance msymbol_put ()
    and mplist_put (), pay special attention to such a property.

    In addition to the predefined managed object types, users can
    define their own managed object types.  See the documentation of
    the m17n_object () for more details.  */
/***ja
    @addtogroup m17nObject
    @brief ���������֥������ȤȤϻ��ȿ��ˤ�äƴ�������Ƥ��륪�֥������ȤǤ���.

    m17n ���֥������ȤΤ��뷿�Τ�Τϡ����ȿ��ˤ�äƴ�������Ƥ��롣
    �����Υ��֥������Ȥ� @e ���������֥������� �ȸƤФ�롣�������줿�����Ǥλ��ȿ���
    1 �˽��������Ƥ��롣�ؿ� m17n_object_ref () �ϴ��������֥������Ȥλ��ȿ���
    1 ���䤷���ؿ�m17n_object_unref () �� 1 ���餹�����ȿ���
    0 �ˤʤä����������֥������Ȥϼ�ưŪ�˲�������롣

    ���������������Ǥ���ץ�ѥƥ��ϡ��ͤȤ��ƴ��������֥������Ȥ������롣
    �ؿ� msymbol_put () �� mplist_put () �ʤɤϤ����Υץ�ѥƥ������̰������롣

    ����Ѥߴ��������֥������ȥ����פ�¾�ˡ��桼����ɬ�פʴ��������֥������ȥ����פ�ʬ��������뤳�Ȥ��Ǥ��롣�ܺ٤�
    m17n_object () �������򻲾ȡ�  */

/*** @{  */
/*=*/
/***en
    @brief Allocate a managed object.

    The m17n_object () function allocates a new managed object of
    $SIZE bytes and sets its reference count to 1.  $FREER is the
    function that is used to free the object when the reference count
    becomes 0.  If $FREER is NULL, the object is freed by the free ()
    function.

    The heading bytes of the allocated object is occupied by
    #M17NObjectHead.  That area is reserved for the m17n library and
    application programs should never touch it.

    @return
    This function returns a newly allocated object.

    @errors
    This function never fails.  */

/***ja
    @brief ���������֥������Ȥ������Ƥ�.

    �ؿ� m17n_object () ��$SIZE �Х��Ȥο��������������֥������Ȥ������ơ����λ��ȿ���
    1 �Ȥ��롣 $FREER �ϻ��ȿ��� 0 
    �ˤʤä��ݤˤ��Υ��֥������Ȥ�������뤿����Ѥ�����ؿ��Ǥ��롣$FREER
    �� NULL�ʤ�С����֥������Ȥϴؿ� free () �ˤ�äƲ�������롣

    ������Ƥ�줿���֥���������Ƭ�ΥХ��Ȥϡ�#M17NObjectHead 
    �����롣�����ΰ�� m17n �饤�֥�꤬���Ѥ���Τǡ����ץꥱ�������ץ����Ͽ���ƤϤʤ�ʤ���

    @return
    ���δؿ��Ͽ�����������Ƥ�줿���֥������Ȥ��֤���

    @errors
    ���δؿ��ϼ��Ԥ��ʤ���    */

#if EXAMPLE_CODE
typedef struct
{
  M17NObjectHead head;
  int mem1;
  char *mem2;
} MYStruct;

void
my_freer (void *obj)
{
  free (((MYStruct *) obj)->mem2);
  free (obj);
}

void
my_func (MText *mt, MSymbol key, int num, char *str)
{
  MYStruct *st = m17n_object (sizeof (MYStruct), my_freer);

  st->mem1 = num;
  st->mem2 = strdup (str);
  /* KEY must be a managing key.   */
  mtext_put_prop (mt, 0, mtext_len (mt), key, st);
  /* This sets the reference count of ST back to 1.  */
  m17n_object_unref (st);
}
#endif

void *
m17n_object (int size, void (*freer) (void *))
{
  M17NObject *obj = malloc (size);

  obj->ref_count = 1;
  obj->ref_count_extended = 0;
  obj->flag = 0;
  obj->u.freer = freer;
  return obj;
}

/*=*/

/***en
    @brief Increment the reference count of a managed object.

    The m17n_object_ref () function increments the reference count of
    the managed object pointed to by $OBJECT.

    @return
    This function returns the resulting reference count if it fits in
    a 16-bit unsigned integer (i.e. less than 0x10000).  Otherwise, it
    return -1.

    @errors
    This function never fails.  */
/***ja
    @brief ���������֥������Ȥλ��ȿ��� 1 ���䤹.

    �ؿ� m17n_object_ref () �� $OBJECT 
    �ǻؤ������������֥������Ȥλ��ȿ��� 1 ���䤹��

    @return 
    ���δؿ��ϡ����䤷�����ȿ��� 16 �ӥåȤ����̵��������(���ʤ�� 
    0x10000 ̤��)�ˤ����ޤ�С�������֤��������Ǥʤ���� -1 ���֤���

    @errors
    ���δؿ��ϼ��Ԥ��ʤ���    */

int
m17n_object_ref (void *object)
{
  M17NObject *obj = (M17NObject *) object;
  M17NObjectRecord *record;
  unsigned *count;

  if (! obj->ref_count_extended)
    {
      if (++obj->ref_count)
	return (int) obj->ref_count;
      MSTRUCT_MALLOC (record, MERROR_OBJECT);
      record->freer = obj->u.freer;
      MLIST_INIT1 (record, counts, 1);
      MLIST_APPEND1 (record, counts, 0, MERROR_OBJECT);
      obj->u.record = record;
      obj->ref_count_extended = 1;
    }
  else
    record = obj->u.record;

  count = record->counts;
  while (*count == 0xFFFFFFFF)
    *(count++) = 0;
  (*count)++;
  if (*count == 0xFFFFFFFF)
    MLIST_APPEND1 (record, counts, 0, MERROR_OBJECT);
  return -1;
}

/*=*/

/***en
    @brief Decrement the reference count of a managed object.

    The m17n_object_unref () function decrements the reference count
    of the managed object pointed to by $OBJECT.  When the reference
    count becomes zero, the object is freed by its freer function.

    @return
    This function returns the resulting reference count if it fits in
    a 16-bit unsigned integer (i.e. less than 0x10000).  Otherwise, it
    returns -1.  Thus, the return value zero means that $OBJECT is
    freed.

    @errors
    This function never fails.  */
/***ja
    @brief ���������֥������Ȥλ��ȿ��� 1 ���餹.

    �ؿ� m17n_object_unref () �� $OBJECT �ǻؤ������������֥������Ȥλ��ȿ���
    1 ���餹�����ȿ��� 0 �ˤʤ�С����֥������Ȥϲ����ؿ��ˤ�äƲ�������롣

    @return 
    ���δؿ��ϡ����餷�����ȿ��� 16 �ӥåȤ����̵��������(���ʤ�� 
    0x10000 ̤��)�ˤ����ޤ�С�������֤��������Ǥʤ���� -1 
    ���֤����Ĥޤꡢ0 ���֤ä��褿����$OBJECT �ϲ�������Ƥ��롣

    @errors
    ���δؿ��ϼ��Ԥ��ʤ���    */
int
m17n_object_unref (void *object)
{
  M17NObject *obj = (M17NObject *) object;
  M17NObjectRecord *record;
  unsigned *count;

  if (! obj->ref_count_extended)
    {
      if (! --obj->ref_count)
	{
	  if (obj->u.freer)
	    (obj->u.freer) (object);
	  else
	    free (object);
	  return 0;
	}
      return (int) obj->ref_count;
    }

  record = obj->u.record;
  count = record->counts;
  while (! *count)
    *(count++) = 0xFFFFFFFF;
  (*count)--;
  if (! record->counts[0])
    {
      obj->ref_count_extended = 0;
      obj->ref_count--;
      obj->u.freer = record->freer;
      MLIST_FREE1 (record, counts);
      free (record);
    }
  return -1;
}

/*=*/

/*** @} */

/***en
    @addtogroup m17nError Error Handling
    @brief Error handling of the m17n library.

    There are two types of errors that may happen in a function of
    the m17n library.

    The first type is argument errors.  When a library function is
    called with invalid arguments, it returns a value that indicates
    error and at the same time sets the external variable #merror_code
    to a non-zero integer.

    The second type is memory allocation errors.  When the required
    amount of memory is not available on the system, m17n library
    functions call a function pointed to by the external variable @c
    m17n_memory_full_handler.  The default value of the variable is a
    pointer to the default_error_handle () function, which just calls
    <tt> exit ()</tt>.  */

/***ja
    @addtogroup m17nError ���顼����
    @brief m17n �饤�֥��Υ��顼����.

    m17n �饤�֥��δؿ��Ǥϡ����Ĥμ���Υ��顼�����������롣

    ��Ĥϰ����Υ��顼�Ǥ��롣
    �饤�֥��δؿ��������Ǥʤ������ȤȤ�˸ƤФ줿��硢���δؿ��ϥ��顼���̣�����ͤ��֤���Ʊ���˳����ѿ� 
    #merror_code �˥���Ǥʤ������򥻥åȤ��롣

    �⤦��Ĥμ���ϥ�������ƥ��顼�Ǥ��롣
    �����ƥबɬ�פ��̤Υ��������Ƥ뤳�Ȥ��Ǥ��ʤ���硢�饤�֥��ؿ��ϳ����ѿ� 
    @c m17n_memory_full_handler ���ؤ��ؿ���Ƥ֡��ǥե���ȤǤϡ��ؿ� 
    default_error_handle () ��ؤ��Ƥ��ꡢ���δؿ���ñ�� <tt>exit
    ()</tt> ��Ƥ֡�
*/

/*** @{ */

/*=*/

/***en 
    @brief External variable to hold error code of the m17n library.

    The external variable #merror_code holds an error code of the
    m17n library.  When a library function is called with an invalid
    argument, it sets this variable to one of @c enum #MErrorCode.

    This variable initially has the value 0.  */

/***ja 
    @brief m17n �饤�֥��Υ��顼�����ɤ��ݻ����볰���ѿ�.

    �����ѿ� #merror_code �ϡ�m17n �饤�֥��Υ��顼�����ɤ��ݻ����롣
    �饤�֥��ؿ��������Ǥʤ������ȤȤ�˸ƤФ줿�ݤˤϡ������ѿ��� 
    @c enum #MErrorCode �ΰ�Ĥ˥��åȤ��롣

    �����ѿ��ν���ͤ� 0 �Ǥ��롣  */

int merror_code;

/*=*/

/***en 
    @brief Memory allocation error handler.

    The external variable #m17n_memory_full_handler holds a pointer
    to the function to call when a library function failed to allocate
    memory.  $ERR is one of @c enum #MErrorCode indicating in which
    function the error occurred.

    This variable initially points a function that simply calls the
    <tt>exit </tt>() function with $ERR as an argument.

    An application program that needs a different error handling can
    change this variable to point a proper function.  */

/***ja 
    @brief ��������ƥ��顼�ϥ�ɥ�.

    �ѿ� #m17n_memory_full_handler 
    �ϡ��饤�֥��ؿ�����������Ƥ˼��Ԥ����ݤ˸Ƥ֤٤��ؿ��ؤΥݥ��󥿤Ǥ��롣
    $ERR �� @c enum #MErrorCode 
    �Τ����Τ����줫�Ǥ��ꡢ�ɤΥ饤�֥��ؿ��ǥ��顼�����ä����򼨤���

    @anchor test

    �������Ǥϡ������ѿ���ñ�� <tt>exit ()</tt> �� $ERR 
    ������Ȥ��ƸƤִؿ���ؤ��Ƥ��롣

    ����Ȥϰۤʤ륨�顼������ɬ�פȤ��륢�ץꥱ�������ϡ������ѿ���Ŭ���ʴؿ������ꤹ�뤳�Ȥǡ���Ū��ã���Ǥ��롣  */

void (*m17n_memory_full_handler) (enum MErrorCode err);

/*** @} */

/*=*/

/***en
    @addtogroup m17nDebug
    @brief Support for m17n library users to debug their programs.

    The m17n library provides the following facilities to support the
    library users to debug their programs.

    <ul>

    <li> Environment variables to control printing of various
    information to stderr.

    <ul>

    <li> MDEBUG_INIT -- If set to 1, print information about the
    library initialization on the call of M17N_INIT ().

    <li> MDEBUG_FINI -- If set to 1, print counts of objects that are
    not yet freed on the call of M17N_FINI ().

    <li> MDEBUG_CHARSET -- If set to 1, print information about
    charsets being loaded from the m17n database.

    <li> MDEBUG_CODING -- If set to 1, print information about coding
    systems being loaded from the m17n database.

    <li> MDEBUG_DATABASE -- If set to 1, print information about
    data being loaded from the m17n database.

    <li> MDEBUG_FONT -- If set to 1, print information about fonts
    being selected and opened.

    <li> MDEBUG_FLT -- If set to 1, 2, or 3, print information about
    which command of Font Layout Table are being executed.  The bigger
    number prints the more detailed information.

    <li> MDEBUG_INPUT -- If set to 1, print information about how an
    input method is running.

    <li> MDEBUG_ALL -- Setting this variable to 1 is equivalent to
    setting all the above variables to 1.

    <li> MDEBUG_OUTPUT_FILE -- If set to a file name, the above
    debugging information is appended to the file.  If set to
    "stdout", the information is printed to stdout.

    </ul>

    <li> Functions to print various objects in a human readable way.
    See the documentation of mdebug_dump_XXXX () functions.

    <li> The hook function called on an error.  See the documentation
    of mdebug_hook ().

    </ul>
*/
/***ja
    @addtogroup m17nDebug
    @brief m17n �饤�֥��桼���Τ���Υץ����ǥХå����ݡ���.

    m17n �饤�֥��ϡ����Υ桼������ʬ�Υץ�����ǥХå����뤿��ˡ��ʲ��ε�ǽ�򥵥ݡ��Ȥ��Ƥ��롣

    <ul>

    <li> ���ޤ��ޤʾ����ɸ�२�顼���ϤؤΥץ��Ȥ����椹��Ķ��ѿ���

    <ul>

    <li> MDEBUG_INIT -- 1 �ʤ�С�M17N_INIT () 
    ���ƤФ줿�����ǡ��饤�֥��ν�����˴ؤ�������ץ��Ȥ��롣

    <li> MDEBUG_FINI -- 1 �ʤ�С�M17N_FINI () 
    ���ƤФ줿�����ǡ��ޤ���������Ƥ��ʤ����֥������Ȥλ��ȿ���ץ��Ȥ��롣

    <li> MDEBUG_CHARSET -- 1 �ʤ�С�m17n
    �ǡ����١���������ɤ��줿ʸ�����åȤˤĤ��Ƥξ����ץ��Ȥ��롣

    <li> MDEBUG_CODING --  1 �ʤ�С�m17n 
    �ǡ����١���������ɤ��줿�����ɷϤˤĤ��Ƥξ����ץ��Ȥ��롣

    <li> MDEBUG_DATABASE -- 1 �ʤ�С�m17n
    �ǡ����١���������ɤ��줿�ǡ����ˤĤ��Ƥξ����ץ��Ȥ��롣

    <li> MDEBUG_FONT -- 1 �ʤ�С����򤵤�ƥ����ץ󤵤줿�ե���Ȥˤ�
    ���Ƥξ����ץ��Ȥ��롣

    <li> MDEBUG_FLT -- 1��2���⤷���� 3 �ʤ�С�Font Layout Table �Τ�
    �Υ��ޥ�ɤ��¹��椫�ˤĤ��ƤΤ�ץ��Ȥ��롣����礭����������
    ���������ץ��Ȥ��롣

    <li> MDEBUG_INPUT -- 1 �ʤ�С��¹�������ϥ᥽�åɤξ��֤��դ��Ƥ�
    �����ץ��Ȥ��롣

    <li> MDEBUG_ALL -- 1 �ʤ�С��嵭���٤Ƥ��ѿ��� 1 
    �ˤ����Τ�Ʊ�����̤���ġ�

    <li> MDEBUG_OUTPUT_FILE -- �⤷�ե�����̾�ʤ顢�嵭�ǥХå�����Ϥ�
    �Υե�������ɲä���롣�⤷ "stdout" �ʤ餽�ξ����ɸ����Ϥ˽���
    ����롣

    </ul>

    <li> ��Υ��֥������Ȥ�ʹ֤˲��ɤʷ��ǥץ��Ȥ���ؿ����ܺ٤ϴؿ�
    mdebug_dump_XXXX () ���������ȡ�

    <li> ���顼ȯ�����˸ƤФ��եå��ؿ���mdebug_hook () ���������ȡ�

    </ul>
*/

/*=*/
/*** @{ */
/*=*/

/***en
    @brief Hook function called on an error.

    The mdebug_hook () function is called when an error happens.  It
    returns -1 without doing anything.  It is useful to set a break
    point on this function in a debugger.  */ 
/***ja
    @brief ���顼�κݤ˸ƤФ��եå��ؿ�.

    �ؿ� mdebug_hook () �ϥ��顼�������ä��ݤ˸ƤФ졢���⤻����-1 
    ���֤����ǥХå���ǥ֥졼���ݥ���Ȥ����ꤹ�뤿����Ѥ��뤳�Ȥ��Ǥ��롣
    */ 

int
mdebug_hook ()
{
  return -1;
}

/*=*/

/*** @} */ 

/*
  Local Variables:
  coding: euc-japan
  End:
*/
