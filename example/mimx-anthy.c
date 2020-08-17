/* mimx-anthy.c -- Anthy input method external module.	-*- coding: euc-jp; -*-
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
    @enpage mimx-anthy external module for the input method <ja, anthy>

    @section mimx-anthy-description DESCRIPTION

    The shared library mimx-anthy.so is an external module used by the
    input method <ja, anthy>.  It exports these functions.

    <ul>
    <li> init

    Initialize this module.

    <li> fini

    Finalize this module.

    <li> convert

    Convert the current preedit text (Hiragana sequence) into
    Kana-Kanji mixed text.

    <li> change

    Record the change of candidate of the current segment.

    <li> resize

    Enlarge or shorten the length of the current segment.

    <li> commit

    Commit the lastly selected candidates of all the segments.

    </ul>

    @section mimx-anthy-seealso See also
    @ref mdbIM
*/
/***ja
    @japage mimx-anthy ���ϥ᥽�å� <ja, anthy> �ѳ����⥸�塼��.

    @section mimx-anthy-description DESCRIPTION

    ��ͭ�饤�֥�� mimx-anthy.so �����ϥ᥽�å�<ja, anthy> ���Ѥ���� 
    �볰���⥸�塼��Ǥ��ꡢ�ʲ��δؿ��� export ���Ƥ��롣

    <ul>
    <li> init

    �⥸�塼��ν������ 

    <li> fini

    �⥸�塼��ν�λ�� 

    <li> convert

    ���ߤ� preedit �ƥ����� (�Ҥ餬����) �򤫤ʴ����ƥ����Ȥ��Ѵ����롣 

    <li> change

    ���ߤΥ������Ȥθ����������Ͽ���롣 

    <li> resize

    ���ߤΥ������Ȥ�Ĺ�����ѹ����롣 

    <li> commit

    ���������Ȥκǿ��θ���򥳥ߥåȤ��롣

    </ul>

    @section mimx-anthy-seealso ����
    @ref mdbIM
*/

#ifndef FOR_DOXYGEN

#include <stdio.h>
#include <string.h>
#include <m17n.h>

#ifdef HAVE_ANTHY

#include <anthy/anthy.h>

static int initialized;
static MSymbol Manthy, Msegment;

/* A structure to record in MInputContext->plist with key Manthy.  */

typedef struct {
  MInputContext *ic;
  anthy_context_t ac;
  /* Which candidate is selected in each segment.  */
  int *candidate_numbers;
  /* Size of the above array.  */
  int num_segments;
  /* Converter for this context.  */
  MConverter *converter;
} AnthyContext;

static AnthyContext *
new_context (MInputContext *ic)
{
  AnthyContext *context;
  anthy_context_t ac;
  MSymbol euc_jp = msymbol ("euc-jp");
  /* Rebound to an actual buffer just before being used.  */
  MConverter *converter = mconv_buffer_converter (euc_jp, NULL, 0);

  if (! converter)
    return NULL;
  ac = anthy_create_context ();
  if (! ac)
    return NULL;
  context = calloc (1, sizeof (AnthyContext));
  context->ic = ic;
  context->ac = ac;
  context->num_segments = 0;
  context->candidate_numbers = NULL;
  context->converter = converter;
  return context;
}

static AnthyContext *
get_context (MInputContext *ic)
{
  MPlist *plist = ic->plist;
  AnthyContext *context;

  for (; plist && mplist_key (plist) != Mnil; plist = mplist_next (plist))
    {
      if (mplist_key (plist) != Manthy)
	continue;
      context = mplist_value (plist);
      if (context->ic == ic)
	return context;
    }
  return NULL;
}


static void
free_context (AnthyContext *context)
{
  anthy_release_context (context->ac);
  if (context->candidate_numbers)
    free (context->candidate_numbers);
  mconv_free_converter (context->converter);
  free (context);
}

static void
allocate_candidate_numbers (AnthyContext *context, int num)
{
  if (context->num_segments < num)
    {
      if (context->num_segments == 0)
	context->candidate_numbers = malloc (sizeof (int) * num);
      else
	context->candidate_numbers = realloc (context->candidate_numbers,
					      sizeof (int) * num);
      context->num_segments = num;
    }
}

static void
add_action (MPlist *actions, MSymbol name, MSymbol key, void *val)
{
  MPlist *action = mplist ();

  mplist_add (action, Msymbol, name);
  mplist_add (action, key, val);
  mplist_add (actions, Mplist, action);
  m17n_object_unref (action);
}

/* Return a list of all candidates of the Nth segment.  The return
   value is a plist whose elements are plists who contains at most 5
   candidates.  */

static MPlist *
make_candidate_list (AnthyContext *context, int n)
{
  MPlist *plist = mplist (), *pl;
  int i;
  char buf[1024];
  struct anthy_segment_stat ss;
  MText *mt;
  
  anthy_get_segment_stat (context->ac, n, &ss);
  for (i = 0, pl = mplist (); i < ss.nr_candidate; i++)
    {
      anthy_get_segment (context->ac, n, i, buf, sizeof (buf));
      mconv_rebind_buffer (context->converter,
			   (unsigned char *) buf, strlen (buf));
      mt = mconv_decode (context->converter, mtext ());
      mtext_put_prop (mt, 0, mtext_len (mt), Msegment, (void *) (n + 1));
      mplist_add (pl, Mtext, mt);
      m17n_object_unref (mt);
      if (i % 5 == 4)
	{
	  mplist_add (plist, Mplist, pl);
	  m17n_object_unref (pl);
	  pl = mplist ();
	}
    }
  if (mplist_key (pl) != Mnil)
    mplist_add (plist, Mplist, pl);
  m17n_object_unref (pl);
  return plist;
}

MPlist *
init (MPlist *args)
{
  MInputContext *ic = mplist_value (args);
  AnthyContext *context;

  if (! initialized++)
    {
      anthy_init ();
      Manthy = msymbol (" anthy");
      Msegment = msymbol (" segment");
    }
  context = new_context (ic);
  if (context)
    mplist_push (ic->plist, Manthy, context);
  return NULL;
}

MPlist *
fini (MPlist *args)
{
  MInputContext *ic = mplist_value (args);
  AnthyContext *context = get_context (ic);

  if (context)
    free_context (context);
  return NULL;
}

MPlist *
convert (MPlist *args)
{
  MInputContext *ic = mplist_value (args);
  AnthyContext *context = get_context (ic);
  struct anthy_conv_stat cs;
  MPlist *action, *actions;
  int i;
  unsigned char buf[1024];

  if (! context)
    return NULL;

  mconv_rebind_buffer (context->converter, buf, sizeof (buf));
  mconv_encode (context->converter, ic->preedit);
  buf[context->converter->nbytes] = '\0';
  anthy_set_string (context->ac, (char *) buf);
  anthy_get_stat (context->ac, &cs);
  allocate_candidate_numbers (context, cs.nr_segment);

  actions = mplist ();
  add_action (actions, msymbol ("move"), Msymbol, msymbol ("@<"));
  add_action (actions, msymbol ("delete"), Msymbol, msymbol ("@>"));
  for (i = 0; i < cs.nr_segment; i++)
    {
      context->candidate_numbers[i] = 0;
      if (i == 1)
	add_action (actions, msymbol ("mark"), Msymbol, msymbol ("@anthy"));
      action = make_candidate_list (context, i);
      mplist_add (actions, Mplist, action);
      m17n_object_unref (action);
    }
  if (cs.nr_segment > 1)
    add_action (actions, msymbol ("move"), Msymbol, msymbol ("@anthy"));

  return actions;
}

MPlist *
change (MPlist *args)
{
  MInputContext *ic = mplist_value (args);
  AnthyContext *context = get_context (ic);
  int segment;

  if (! context)
    return NULL;
  if (! ic->candidate_list || ic->cursor_pos == 0)
    return NULL;
  segment = (int) mtext_get_prop (ic->preedit, ic->cursor_pos - 1, Msegment);
  if (segment == 0)
    return NULL;
  segment--;
  context->candidate_numbers[segment] = ic->candidate_index;
  return NULL;
}

MPlist *
resize (MPlist *args)
{
  MInputContext *ic = mplist_value (args);
  AnthyContext *context = get_context (ic);
  struct anthy_conv_stat cs;
  MSymbol shorten;
  int segment;
  MPlist *actions, *action;
  int i;

  if (! context)
    return NULL;
  if (! ic->candidate_list || ic->cursor_pos == 0)
    return NULL;
  segment = (int) mtext_get_prop (ic->preedit, ic->cursor_pos - 1, Msegment);
  if (segment == 0)
    return NULL;
  segment--;
  args = mplist_next (args);
  shorten = mplist_value (args);
  anthy_resize_segment (context->ac, segment, shorten == Mt ? -1 : 1);
  anthy_get_stat (context->ac, &cs);
  allocate_candidate_numbers (context, cs.nr_segment);

  actions = mplist ();
  if (segment == 0)
    add_action (actions, msymbol ("move"), Msymbol, msymbol ("@<"));
  else
    add_action (actions, msymbol ("move"), Msymbol, msymbol ("@["));
  add_action (actions, msymbol ("delete"), Msymbol, msymbol ("@>"));
  for (i = segment; i < cs.nr_segment; i++)
    {
      context->candidate_numbers[i] = 0;
      if (i == segment + 1)
	add_action (actions, msymbol ("mark"), Msymbol, msymbol ("@anthy"));
      action = make_candidate_list (context, i);
      mplist_add (actions, Mplist, action);
      m17n_object_unref (action);
    }
  if (segment + 1 < cs.nr_segment)
    add_action (actions, msymbol ("move"), Msymbol, msymbol ("@anthy"));
  return actions;
}

MPlist *
commit (MPlist *args)
{
  MInputContext *ic = mplist_value (args);
  AnthyContext *context = get_context (ic);
  struct anthy_conv_stat cs;
  int i;

  if (! context)
    return NULL;
  anthy_get_stat (context->ac, &cs);
  for (i = 0; i < cs.nr_segment; i++)
    anthy_commit_segment (context->ac, i, context->candidate_numbers[i]);
  return NULL;
}

#else  /* not HAVE_ANTHY */

MPlist *convert (MPlist *args) { return NULL; }
MPlist *change (MPlist *args) { return NULL; }
MPlist *resize (MPlist *args) { return NULL; }
MPlist *commit (MPlist *args) { return NULL; }

#endif /* not HAVE_ANTHY */
#endif /* not FOR_DOXYGEN */
