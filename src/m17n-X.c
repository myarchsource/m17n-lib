/* m17n-X.c -- implementation of the GUI API on X Windows.
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

#include "config.h"

#ifdef HAVE_X11

#if !defined (FOR_DOXYGEN) || defined (DOXYGEN_INTERNAL_MODULE)
/*** @addtogroup m17nInternal
     @{ */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <locale.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xlocale.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xatom.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>

#ifdef HAVE_XFT2
#include <X11/Xft/Xft.h>
#include <fontconfig/fcfreetype.h>
#endif	/* HAVE_XFT2 */

#include "m17n-gui.h"
#include "m17n-X.h"
#include "m17n-misc.h"
#include "internal.h"
#include "internal-gui.h"
#include "symbol.h"
#include "input.h"
#include "font.h"
#include "fontset.h"
#include "face.h"

typedef struct
{
  /* Common header for the m17n object.  */
  M17NObject control;

  Display *display;

  /* If nonzero, <display> is opened by this library.  Thus it should
     be closed on freeing this structure.  */
  int auto_display;

  /** List of available X-core fonts on the display.  Keys are
      registries and values are plists whose keys are families and
      values are pointers to MFont.  */
  MPlist *font_list;

  /** Nonzero means that <font_list> already contains all available
      fonts on the display.  */
  int all_fonts_scaned;

 /** Modifier bit masks of the display.  */
  int meta_mask;
  int alt_mask;
  int super_mask;
  int hyper_mask;
  int altgr_mask;

  Atom MULE_BASELINE_OFFSET;
  Atom AVERAGE_WIDTH;
} MDisplayInfo;

/* Anchor of the chain of MDisplayInfo objects.  */
static MPlist *display_info_list;


/* Color value and the corresponding GC.  */
typedef struct
{
  unsigned int rgb;	       /* (red << 16) | (green << 8) | blue */
  GC gc;
} RGB_GC;

enum gc_index
  {
    GC_INVERSE,
    GC_NORMAL = GC_INVERSE + 7,
    GC_HLINE,
    GC_BOX_TOP,
    GC_BOX_BOTTOM,
    GC_BOX_LEFT,
    GC_BOX_RIGHT,
    GC_MAX
  };

typedef struct
{
  int rgb_fore;
  int rgb_back;
  /* The first 8 elements are indexed by an intensity for
     anti-aliasing.  The 2nd to 7th are created on demand.  */
  GC gc[GC_MAX];
#ifdef HAVE_XFT2
  XftColor xft_color_fore, xft_color_back;
#endif
} GCInfo;

typedef struct
{
  /* Common header for the m17n object.  */
  M17NObject control;

  MDisplayInfo *display_info;

  int screen_num;

  Drawable drawable;

  unsigned depth;

  Colormap cmap;

  GC scratch_gc;

  int resy;

#ifdef HAVE_XFT2
  XftDraw *xft_draw;
#endif

  /** List of pointers to realized faces on the frame.  */
  MPlist *realized_face_list;

  /* List of single element whose value is a root of chain of realized
     fonts.  */
  MPlist *realized_font_list;

  /** List of pointers to realized fontsets on the frame.  */
  MPlist *realized_fontset_list;

  /** List of XColors vs GCs on the frame.  */
  MPlist *gc_list;
} MWDevice;

static MPlist *device_list;

static MSymbol M_iso8859_1, M_iso10646_1;

#define FRAME_DEVICE(frame) ((MWDevice *) (frame)->device)
#define FRAME_DISPLAY(frame) (FRAME_DEVICE (frame)->display_info->display)
#define FRAME_SCREEN(frame) (FRAME_DEVICE (frame)->screen_num)
#define FRAME_CMAP(frame) (FRAME_DEVICE (frame)->cmap)
#define FRAME_VISUAL(frame) DefaultVisual (FRAME_DISPLAY (frame), \
					   FRAME_SCREEN (frame))

#define DEFAULT_FONT "-*-*-medium-r-normal--13-*-*-*-c-*-iso8859-1"

typedef struct
{
  String font;
  String foreground;
  String background;
  Boolean reverse_video;
} AppData, *AppDataPtr;

static void
free_display_info (void *object)
{
  MDisplayInfo *disp_info = (MDisplayInfo *) object;
  MPlist *plist, *pl;

  MPLIST_DO (plist, disp_info->font_list)
    {
      MPLIST_DO (pl, MPLIST_VAL (plist))
	free (MPLIST_VAL (pl));
      M17N_OBJECT_UNREF (MPLIST_VAL (plist));
    }
  M17N_OBJECT_UNREF (disp_info->font_list);

  if (disp_info->auto_display)
    XCloseDisplay (disp_info->display);

  free (object);
}

static void
free_device (void *object)
{
  MWDevice *device = object;
  MPlist *plist;

  for (plist = device->realized_fontset_list;
       mplist_key (plist) != Mnil; plist = mplist_next (plist))
    mfont__free_realized_fontset ((MRealizedFontset *) mplist_value (plist));
  M17N_OBJECT_UNREF (device->realized_fontset_list);

  if (MPLIST_VAL (device->realized_font_list))
    mfont__free_realized (MPLIST_VAL (device->realized_font_list));
  M17N_OBJECT_UNREF (device->realized_font_list);

  MPLIST_DO (plist, device->realized_face_list)
    {
      MRealizedFace *rface = MPLIST_VAL (plist);

      free (rface->info);
      mface__free_realized (rface);
    }
  M17N_OBJECT_UNREF (device->realized_face_list);

  MPLIST_DO (plist, device->gc_list)
    {
      XFreeGC (device->display_info->display,
	       ((RGB_GC *) MPLIST_VAL (plist))->gc);
      free (MPLIST_VAL (plist));
    }
  M17N_OBJECT_UNREF (device->gc_list);
  XFreeGC (device->display_info->display, device->scratch_gc);

#ifdef HAVE_XFT2
  XftDrawDestroy (device->xft_draw);
#endif

  XFreePixmap (device->display_info->display, device->drawable);
  M17N_OBJECT_UNREF (device->display_info);
  free (object);
}


static void
find_modifier_bits (MDisplayInfo *disp_info)
{
  Display *display = disp_info->display;
  XModifierKeymap *mods;
  KeyCode meta_l = XKeysymToKeycode (display, XK_Meta_L);
  KeyCode meta_r = XKeysymToKeycode (display, XK_Meta_R);
  KeyCode alt_l = XKeysymToKeycode (display, XK_Alt_L);
  KeyCode alt_r = XKeysymToKeycode (display, XK_Alt_R);
  KeyCode super_l = XKeysymToKeycode (display, XK_Super_L);
  KeyCode super_r = XKeysymToKeycode (display, XK_Super_R);
  KeyCode hyper_l = XKeysymToKeycode (display, XK_Hyper_L);
  KeyCode hyper_r = XKeysymToKeycode (display, XK_Hyper_R);
#ifdef XK_XKB_KEYS
  KeyCode altgr = XKeysymToKeycode (display, XK_ISO_Level3_Shift);
#endif
  int i, j;

  mods = XGetModifierMapping (display);
  /* We skip the first three sets for Shift, Lock, and Control.  The
     remaining sets are for Mod1, Mod2, Mod3, Mod4, and Mod5.  */
  for (i = 3; i < 8; i++)
    for (j = 0; j < mods->max_keypermod; j++)
      {
	KeyCode code = mods->modifiermap[i * mods->max_keypermod + j];

	if (! code)
	  continue;
	if (code == meta_l || code == meta_r)
	  disp_info->meta_mask |= (1 << i);
	else if (code == alt_l || code == alt_r)
	  disp_info->alt_mask |= (1 << i);
	else if (code == super_l || code == super_r)
	  disp_info->super_mask |= (1 << i);
	else if (code == hyper_l || code == hyper_r)
	  disp_info->hyper_mask |= (1 << i);
#ifdef XK_XKB_KEYS
	else if (code == altgr)
	  disp_info->altgr_mask |= (1 << i);
#endif
      }

  /* If meta keys are not in any modifier, use alt keys as meta
     keys.  */
  if (! disp_info->meta_mask)
    {
      disp_info->meta_mask = disp_info->alt_mask;
      disp_info->alt_mask = 0;
    }
  /* If both meta and alt are assigned to the same modifier, give meta
     keys priority.  */
  if (disp_info->meta_mask & disp_info->alt_mask)
    disp_info->alt_mask &= ~disp_info->meta_mask;

  XFreeModifiermap (mods);
}

static RGB_GC *
get_rgb_gc (MWDevice *device, XColor *xcolor)
{
  int rgb = (((xcolor->red >> 8) << 16) | ((xcolor->green >> 8) << 8)
	     | (xcolor->blue >> 8));
  MPlist *plist;
  RGB_GC *rgb_gc;
  unsigned long valuemask = GCForeground;
  XGCValues values;

  MPLIST_DO (plist, device->gc_list)
    {
      rgb_gc = MPLIST_VAL (plist);

      if (rgb_gc->rgb == rgb)
	return rgb_gc;
      if (rgb_gc->rgb > rgb)
	break;
    }

  if (! XAllocColor (device->display_info->display, device->cmap, xcolor))
    return NULL;

  rgb_gc = malloc (sizeof (RGB_GC));
  rgb_gc->rgb = rgb;
  values.foreground = xcolor->pixel;
  rgb_gc->gc = XCreateGC (device->display_info->display,
			  device->drawable, valuemask, &values);
  mplist_push (plist, Mt, rgb_gc);
  return rgb_gc;
}

static GC
get_gc (MFrame *frame, MSymbol color, int for_foreground, int *rgb_ret)
{
  MWDevice *device = FRAME_DEVICE (frame);
  XColor xcolor;
  RGB_GC *rgb_gc;

  if (color == Mnil)
    {
      if (frame->rface)
	goto no_color;
      color = for_foreground ? frame->foreground : frame->background;
    }
  if (! XParseColor (FRAME_DISPLAY (frame), device->cmap,
		     msymbol_name (color), &xcolor))
    goto no_color;
  rgb_gc = get_rgb_gc (device, &xcolor);
  if (! rgb_gc)
    goto no_color;
  if (rgb_ret)
    *rgb_ret = rgb_gc->rgb;
  return rgb_gc->gc;

 no_color:
  {
    GCInfo *info = frame->rface->info;
    GC gc;
    int rgb;

    if (for_foreground)
      rgb = info->rgb_fore, gc = info->gc[GC_NORMAL];
    else
      rgb = info->rgb_back, gc = info->gc[GC_INVERSE];
    if (rgb_ret)
      *rgb_ret = rgb;
    return gc;
  }
}

static GC
get_gc_for_anti_alias (MWDevice *device, GCInfo *info, int intensity)
{
  int rgb_fore, rgb_back;
  XColor xcolor;
  RGB_GC *rgb_gc;
  GC gc;

  if (info->gc[intensity])
    return info->gc[intensity];

  rgb_fore = info->rgb_fore, rgb_back = info->rgb_back;
  xcolor.red = ((((rgb_fore & 0xFF0000) >> 16) * intensity
		 + ((rgb_back & 0xFF0000) >> 16) * (7 - intensity)) / 7) << 8;
  xcolor.green = ((((rgb_fore & 0xFF00) >> 8) * intensity
		   + ((rgb_back & 0xFF00) >> 8) * (7 - intensity)) / 7) << 8;
  xcolor.blue = (((rgb_fore & 0xFF) * intensity
		  + (rgb_back & 0xFF) * (7 - intensity)) / 7) << 8;
  rgb_gc = get_rgb_gc (device, &xcolor);
  if (rgb_gc)
    gc = rgb_gc->gc;
  else
    gc =get_gc_for_anti_alias (device, info,
			       intensity < 4 ? intensity - 1 : intensity + 1);
  return (info->gc[intensity] = gc);
}

static GC
set_region (MFrame *frame, GC gc, MDrawRegion region)
{
  unsigned long valuemask = GCForeground;

  XCopyGC (FRAME_DISPLAY (frame), gc, valuemask,
	   FRAME_DEVICE (frame)->scratch_gc);
  XSetRegion (FRAME_DISPLAY (frame), FRAME_DEVICE (frame)->scratch_gc, region);
  return FRAME_DEVICE (frame)->scratch_gc;
}


/** X font handler */

static MFont *xfont_select (MFrame *, MFont *, int);
static MRealizedFont *xfont_open (MFrame *, MFont *, MFont *, MRealizedFont *);
static void xfont_find_metric (MRealizedFont *, MGlyphString *, int, int);
static int xfont_has_char (MFrame *, MFont *, MFont *, int, unsigned);
static unsigned xfont_encode_char (MFrame *, MFont *, MFont *, unsigned);
static void xfont_render (MDrawWindow, int, int, MGlyphString *,
			  MGlyph *, MGlyph *, int, MDrawRegion);
static int xfont_list (MFrame *, MPlist *, MFont *, int);
static void xfont_list_family_names (MFrame *, MPlist *);
static int xfont_check_capability (MRealizedFont *rfont, MSymbol capability);

static MFontDriver xfont_driver =
  { xfont_select, xfont_open,
    xfont_find_metric, xfont_has_char, xfont_encode_char,
    xfont_render, xfont_list, xfont_list_family_names, xfont_check_capability
  };

static int
font_compare (const void *p1, const void *p2)
{
  return strcmp (*(char **) p1, *(char **) p2);
}

static MPlist *
xfont_registry_list (MFrame *frame, MSymbol registry)
{
  MDisplayInfo *disp_info = FRAME_DEVICE (frame)->display_info;
  MPlist *font_list = disp_info->font_list;
  MPlist *plist, *p;
  char pattern[1024];
  char **font_names, **names;
  int nfonts;
  int i, j;
  MFont font;
  int for_full_width;

  plist = mplist_get (font_list, registry);
  if (plist)
    return plist;
  p = plist = mplist ();
  mplist_add (font_list, registry, plist);
  sprintf (pattern, "-*-*-*-*-*-*-*-*-*-*-*-*-%s", msymbol_name (registry));
  font_names = XListFonts (disp_info->display, pattern, 0x8000, &nfonts);
  if (nfonts == 0)
    return plist;
  {
    char *reg_name = msymbol_name (registry);

    for_full_width = (strncmp (reg_name, "jis", 3) == 0
		      || strncmp (reg_name, "gb", 2) == 0
		      || strncmp (reg_name, "big5", 4) == 0
		      || strncmp (reg_name, "ksc", 3) == 0);
  }
  names = alloca (sizeof (char *) * nfonts);
  memcpy (names, font_names, sizeof (char *) * nfonts);
  qsort (names, nfonts, sizeof (char *), font_compare);
  MFONT_INIT (&font);
  for (i = 0; i < nfonts; i++)
    if (mfont__parse_name_into_font (names[i], Mx, &font) == 0
	&& (font.size > 0 || font.property[MFONT_RESY] == 0))
      {
	MSymbol family = FONT_PROPERTY (&font, MFONT_FAMILY);
	MFont *fontx;
	unsigned sizes[256];
	int nsizes = 0;
	int limit;
	int size, normal_size;
	char *base_end;
	int base_len;
	int fields;
	
	/* Calculate how many bytes to compare to detect fonts of the
	   same base name.  */
	for (base_end = names[i], fields = 0; *base_end; base_end++)
	  if (*base_end == '-'
	      && ++fields == 7	/* PIXEL_SIZE */)
	    break;
	base_len = base_end - names[i] + 1;

	size = font.size / 10;
	sizes[nsizes++] = size;
	normal_size = (size >= 6 && size <= 29);
	limit = (i + 256 < nfonts ? i + 256 : nfonts);
	for (j = i + 1; j < limit && ! memcmp (names[i], names[j], base_len);
	     i = j++)
	  if (mfont__parse_name_into_font (names[j], Mx, &font) == 0
	      && (font.size > 0 || font.property[MFONT_RESY] == 0))
	    {
	      size = font.size / 10;
	      sizes[nsizes++] = size;
	      normal_size |= (size >= 6 && size <= 29);
	    }

	font.for_full_width = for_full_width;
	font.type = MFONT_TYPE_OBJECT;
	font.source = MFONT_SOURCE_X;
	if (normal_size)
	  {
	    MSTRUCT_CALLOC (fontx, MERROR_WIN);
	    *fontx = font;
	    fontx->multiple_sizes = 1;
	    fontx->size = 0;
	    for (j = 0; j < nsizes; j++)
	      if (sizes[j] >= 6 && sizes[j] <= 29)
		fontx->size |= 1 << (sizes[j] - 6);
	    p = mplist_add (p, family, fontx);
	  }
	for (j = 0; j < nsizes; j++)
	  if (sizes[j] < 6 || sizes[j] > 29)
	    {
	      MSTRUCT_CALLOC (fontx, MERROR_WIN);
	      *fontx = font;
	      fontx->multiple_sizes = 0;
	      fontx->size = sizes[j] * 10;
	      if (sizes[j] == 0)
		fontx->property[MFONT_RESY] = 0;
	      p = mplist_add (p, family, fontx);
	    }
      }
  XFreeFontNames (font_names);
  return plist;
}

static void
xfont_list_all (MFrame *frame)
{
  MDisplayInfo *disp_info = FRAME_DEVICE (frame)->display_info;
  MPlist *font_encoding_list, *p;

  if (disp_info->all_fonts_scaned)
    return;
  disp_info->all_fonts_scaned = 1;
  font_encoding_list = mfont__encoding_list ();
  if (! font_encoding_list)
    return;
  MPLIST_DO (p, font_encoding_list)
    xfont_registry_list (frame, MPLIST_KEY (p));
}

typedef struct
{
  M17NObject control;
  Display *display;
  XFontStruct *xfont;
} MRealizedFontX;

/* The X font driver function SELECT.  */

static MFont *
xfont_select (MFrame *frame, MFont *font, int limited_size)
{
  MPlist *plist = mplist (), *pl;
  int num = xfont_list (frame, plist, font, 0);
  MFont *found = NULL;

  if (num > 0)
    MPLIST_DO (pl, plist)
      {
	font = MPLIST_VAL (pl);
	if (limited_size == 0
	    || font->size == 0
	    || font->size <= limited_size)
	  {
	    found = font;
	    break;
	  }
      }
  M17N_OBJECT_UNREF (plist);
  return found;
}

/* The X font driver function CLOSE.  */

static void
close_xfont (void *object)
{
  MRealizedFontX *x_rfont = object;

  XFreeFont (x_rfont->display, x_rfont->xfont);
  free (x_rfont);
}

/* The X font driver function OPEN.  */

static MRealizedFont *
xfont_open (MFrame *frame, MFont *font, MFont *spec, MRealizedFont *rfont)
{
  int size;
  MRealizedFontX *x_rfont;
  char *name;
  Display *display = FRAME_DISPLAY (frame);
  XFontStruct *xfont;
  int mdebug_flag = MDEBUG_FONT;
  MFont this;

  size = spec->size;
  if (size)
    {
      int ratio = mfont_resize_ratio (font);

      if (ratio != 100)
	size = size * ratio / 100;
    }
  else
    size = 120;

  if (font->size)
    {
      /* non-scalable font */
      if (font->multiple_sizes)
	{
	  int i;

	  if (size < 60)
	    size = 60;
	  else if (size > 290)
	    size = 290;
	  for (i = size / 10 - 6; i >= 0; i--)
	    if (font->size & (1 << i))
	      break;
	  if (i == 0)
	    for (i = size / 10 - 5; i < 24; i++)
	      if (font->size & (1 << i))
		break;
	  size = (i + 6) * 10;
	}
      else
	size = font->size;
    }

  if (rfont)
    {
      for (; rfont; rfont = rfont->next)
	if (rfont->font == font && rfont->spec.size == size)
	  return rfont;
    }

  this = *font;
  this.multiple_sizes = 0;
  this.size = size;
  /* This never fail to generate a valid fontname.  */
  name = mfont_unparse_name (&this, Mx);
  xfont = XLoadQueryFont (FRAME_DISPLAY (frame), name);
  if (! xfont)
    {
      MDEBUG_PRINT1 (" [XFONT] x %s\n", name);
      free (name);
      font->type = MFONT_TYPE_FAILURE;
      return NULL;
    }
  M17N_OBJECT (x_rfont, close_xfont, MERROR_FONT_X);
  x_rfont->display = display;
  x_rfont->xfont = xfont;
  MSTRUCT_CALLOC (rfont, MERROR_FONT_X);
  rfont->id = msymbol (name);
  rfont->spec = this;
  rfont->spec.type = MFONT_TYPE_REALIZED;
  rfont->spec.source = MFONT_SOURCE_X;
  rfont->frame = frame;
  rfont->font = font;
  rfont->driver = &xfont_driver;
  rfont->info = x_rfont;
  {
    MDisplayInfo *disp_info = FRAME_DEVICE (frame)->display_info;
    unsigned long value;

    rfont->baseline_offset
      = (XGetFontProperty (xfont, disp_info->MULE_BASELINE_OFFSET, &value)
	 ? (int) (value << 6) : 0);
    rfont->average_width
      = (XGetFontProperty (xfont, disp_info->AVERAGE_WIDTH, &value)
	 ? (int) (value << 6) / 10 : 0);
  }
  rfont->ascent = (xfont->ascent << 6) + rfont->baseline_offset;
  rfont->descent = (xfont->descent << 6) - rfont->baseline_offset;
  rfont->max_advance = xfont->max_bounds.width << 6;
  rfont->x_ppem = rfont->y_ppem = size / 10;
  rfont->fontp = xfont;
  rfont->next = MPLIST_VAL (frame->realized_font_list);
  MPLIST_VAL (frame->realized_font_list) = rfont;
  MDEBUG_PRINT1 (" [XFONT] o %s\n", name);
  free (name);
  return rfont;
}


/* The X font driver function FIND_METRIC.  */

static void
xfont_find_metric (MRealizedFont *rfont, MGlyphString *gstring,
		   int from, int to)
{
  XFontStruct *xfont = rfont->fontp;
  MGlyph *g = MGLYPH (from), *gend = MGLYPH (to);

  for (; g != gend; g++)
    if (! g->g.measured)
      {
	if (g->g.code == MCHAR_INVALID_CODE)
	  {
	    g->g.lbearing = xfont->max_bounds.lbearing << 6;
	    g->g.rbearing = xfont->max_bounds.rbearing << 6;
	    g->g.xadv = xfont->max_bounds.width << 6;
	    g->g.ascent = xfont->ascent << 6;
	    g->g.descent = xfont->descent << 6;
	  }
	else
	  {
	    int byte1 = g->g.code >> 8, byte2 = g->g.code & 0xFF;
	    XCharStruct *pcm = NULL;

	    if (xfont->per_char != NULL)
	      {
		if (xfont->min_byte1 == 0 && xfont->max_byte1 == 0)
		  {
		    if (byte1 == 0
			&& byte2 >= xfont->min_char_or_byte2
			&& byte2 <= xfont->max_char_or_byte2)
		      pcm = xfont->per_char + byte2 - xfont->min_char_or_byte2;
		  }
		else
		  {
		    if (byte1 >= xfont->min_byte1
			&& byte1 <= xfont->max_byte1
			&& byte2 >= xfont->min_char_or_byte2
			&& byte2 <= xfont->max_char_or_byte2)
		      {
			pcm = (xfont->per_char
			       + ((xfont->max_char_or_byte2
				   - xfont->min_char_or_byte2 + 1)
				  * (byte1 - xfont->min_byte1))
			       + (byte2 - xfont->min_char_or_byte2));
		      }
		  }
	      }

	    if (pcm)
	      {
		g->g.lbearing = pcm->lbearing << 6;
		g->g.rbearing = pcm->rbearing << 6;
		g->g.xadv = pcm->width << 6;
		g->g.ascent = pcm->ascent << 6;
		g->g.descent = pcm->descent << 6;
	      }
	    else
	      {
		/* If the per_char pointer is null, all glyphs between
		   the first and last character indexes inclusive have
		   the same information, as given by both min_bounds and
		   max_bounds.  */
		g->g.lbearing = 0;
		g->g.rbearing = xfont->max_bounds.width << 6;
		g->g.xadv = xfont->max_bounds.width << 6;
		g->g.ascent = xfont->ascent << 6;
		g->g.descent = xfont->descent << 6;
	      }
	  }
	g->g.yadv = 0;
	g->g.ascent += rfont->baseline_offset;
	g->g.descent -= rfont->baseline_offset;
	g->g.measured = 1;
      }
}


static int
xfont_has_char (MFrame *frame, MFont *font, MFont *spec, int c, unsigned code)
{
  return (xfont_encode_char (frame, font, spec, code) != MCHAR_INVALID_CODE);
}

/* The X font driver function GET_GLYPH_ID.  */

static unsigned
xfont_encode_char (MFrame *frame, MFont *font, MFont *spec, unsigned code)
{
  MRealizedFont *rfont;
  XFontStruct *xfont;
  unsigned min_byte1, max_byte1, min_byte2, max_byte2;
  int all_chars_exist;

  if (font->type == MFONT_TYPE_REALIZED)
    rfont = (MRealizedFont *) font;
  else if (font->type == MFONT_TYPE_OBJECT)
    {
      for (rfont = MPLIST_VAL (frame->realized_font_list); rfont;
	   rfont = rfont->next)
	if (rfont->font == font)
	  break;
      if (! rfont)
	{
	  rfont = xfont_open (frame, font, spec, NULL);
	  if (! rfont)
	    return MCHAR_INVALID_CODE;
	}
    }
  else
    MFATAL (MERROR_FONT_X);
  xfont = rfont->fontp;
  all_chars_exist = (! xfont->per_char || xfont->all_chars_exist == True);
  min_byte1 = xfont->min_byte1;
  max_byte1 = xfont->max_byte1;
  min_byte2 = xfont->min_char_or_byte2;
  max_byte2 = xfont->max_char_or_byte2;

  if (min_byte1 == 0 && max_byte1 == 0)
    {
      XCharStruct *pcm;

      if (code < min_byte2 || code > max_byte2)
	return MCHAR_INVALID_CODE;
      if (all_chars_exist)
	return code;
      pcm = xfont->per_char + (code - min_byte2);
      return ((pcm->width > 0 || pcm->rbearing != pcm->lbearing)
	      ? code : MCHAR_INVALID_CODE);
    }
  else
    {
      unsigned byte1 = code >> 8, byte2 = code & 0xFF;
      XCharStruct *pcm;

      if (byte1 < min_byte1 || byte1 > max_byte1
	  || byte2 < min_byte2 || byte2 > max_byte2)
	return MCHAR_INVALID_CODE;

      if (all_chars_exist)
	return code;
      pcm = xfont->per_char + ((byte1 - min_byte1) * (max_byte2 - min_byte2 + 1)
			   + (byte2 - min_byte2));
      return ((pcm->width > 0 || pcm->rbearing != pcm->lbearing)
	      ? code : MCHAR_INVALID_CODE);
    }
}

/* The X font driver function RENDER.  */

static void
xfont_render (MDrawWindow win, int x, int y, MGlyphString *gstring,
	      MGlyph *from, MGlyph *to, int reverse, MDrawRegion region)
{
  MRealizedFace *rface = from->rface;
  Display *display = FRAME_DISPLAY (rface->frame);
  XChar2b *code;
  GC gc = ((GCInfo *) rface->info)->gc[reverse ? GC_INVERSE : GC_NORMAL];
  MGlyph *g;
  int i;
  int baseline_offset;

  if (from == to)
    return;

  baseline_offset = rface->rfont->baseline_offset >> 6;
  if (region)
    gc = set_region (rface->frame, gc, region);
  XSetFont (display, gc, ((XFontStruct *) rface->rfont->fontp)->fid);
  code = (XChar2b *) alloca (sizeof (XChar2b) * (to - from));
  for (i = 0, g = from; g < to; i++, g++)
    {
      code[i].byte1 = g->g.code >> 8;
      code[i].byte2 = g->g.code & 0xFF;
    }

  g = from;
  while (g < to)
    {
      if (g->type == GLYPH_PAD)
	x += g++->g.xadv;
      else if (g->type == GLYPH_SPACE)
	for (; g < to && g->type == GLYPH_SPACE; g++)
	  x += g->g.xadv;
      else if (! g->rface->rfont)
	{
	  if ((g->g.c >= 0x200B && g->g.c <= 0x200F)
	      || (g->g.c >= 0x202A && g->g.c <= 0x202E))
	    x += g++->g.xadv;
	  else
	    {
	      /* As a font is not found for this character, draw an
		 empty box.  */
	      int box_width = g->g.xadv;
	      int box_height = gstring->ascent + gstring->descent;

	      if (box_width > 4)
		box_width -= 2;
	      if (box_height > 4)
		box_height -= 2;
	      XDrawRectangle (display, (Window) win, gc,
			      x, y - gstring->ascent, box_width, box_height);
	      x += g++->g.xadv;
	    }
	}
      else if (g->g.xoff != 0 || g->g.yoff != 0 || g->right_padding)
	{
	  XDrawString16 (display, (Window) win, gc,
			 x + g->g.xoff, y + g->g.yoff - baseline_offset,
			 code + (g - from), 1);
	  x += g->g.xadv;
	  g++;
	}
      else
	{
	  int orig_x = x;
	  int code_idx = g - from;

	  for (i = 0;
	       g < to && g->type == GLYPH_CHAR && g->g.xoff == 0 && g->g.yoff == 0;
	       i++, g++)
	      x += g->g.xadv;
	  XDrawString16 (display, (Window) win, gc,
			 orig_x, y - baseline_offset, code + code_idx, i);
	}
    }
}

static int
xfont_list (MFrame *frame, MPlist *plist, MFont *font, int maxnum)
{
  MDisplayInfo *disp_info = FRAME_DEVICE (frame)->display_info;
  MSymbol registry = font ? FONT_PROPERTY (font, MFONT_REGISTRY) : Mnil;
  MSymbol family = font ? FONT_PROPERTY (font, MFONT_FAMILY) : Mnil;
  int size = font ? font->size : 0;
  MPlist *pl, *p;
  int num = 0;
  int mdebug_flag = MDEBUG_FONT;

  MDEBUG_PRINT2 (" [X-FONT] listing %s-%s...",
		 family ? msymbol_name (family) : "*",
		 registry ? msymbol_name (registry) : "*");

  if (registry == Mnil)
    xfont_list_all (frame);
  else
    xfont_registry_list (frame, registry);

  MPLIST_DO (pl, disp_info->font_list)
    if (registry == Mnil || registry == MPLIST_KEY (pl))
      {
	MPLIST_DO (p, MPLIST_VAL (pl))
	  if (family == Mnil || family == MPLIST_KEY (p))
	    {
	      MFont *fontx = MPLIST_VAL (p);

	      if (! font || (mfont__match_p (fontx, font, MFONT_REGISTRY)))
		{
		  if (fontx->size != 0 && size)
		    {
		      if (fontx->multiple_sizes)
			{
			  if (size < 60 || size > 290
			      || ! (fontx->size & (1 << (size / 10 - 6))))
			    continue;
			}
		      else if (fontx->size != size)
			continue;
		    }
		  mplist_push (plist, MPLIST_KEY (p), fontx);
		  num++;
		  if (maxnum > 0 && maxnum == num)
		    goto done;
		}
	    }
      }

 done:
  MDEBUG_PRINT1 (" %d found\n", num);
  return num;
}

static void
xfont_list_family_names (MFrame *frame, MPlist *plist)
{
  MDisplayInfo *disp_info = FRAME_DEVICE (frame)->display_info;
  char **font_names;
  int i, nfonts;
  MSymbol last_family = Mnil;

  font_names = XListFonts (disp_info->display,
			   "-*-*-*-*-*-*-*-*-*-*-*-*-*-*", 0x8000, &nfonts);
  for (i = 0; i < nfonts; i++)
    {
      MSymbol family;
      char foundry[256], fam[256];
      MPlist *p;
      
      if (sscanf (font_names[i], "-%s-%s-", foundry, fam) < 2)
	continue;
      family = msymbol (fam);
      if (family == last_family)
	continue;
      last_family = family;

      MPLIST_DO (p, plist)
	{
	  MSymbol sym = MPLIST_SYMBOL (p);

	  if (sym == family)
	    break;
	  if (strcmp (MSYMBOL_NAME (sym), fam) > 0)
	    {
	      mplist_push (p, Msymbol, family);
	      break;
	    }
	}
      if (MPLIST_TAIL_P (p))
	mplist_push (p, Msymbol, family);
    }
  if (font_names)
    XFreeFontNames (font_names);
}

static int 
xfont_check_capability (MRealizedFont *rfont, MSymbol capability)
{
  /* Currently X font driver doesn't support any capability.  */
  return -1;
}


/* Xft Handler */

#ifdef HAVE_XFT2

typedef struct
{
  M17NObject control;
  FT_Face ft_face;		/* This must be the 2nd member. */
  Display *display;
  XftFont *font_aa;
  XftFont *font_no_aa;
  /* Pointer to MRealizedFontFT */
  void *info;
} MRealizedFontXft;

static MRealizedFont *xft_open (MFrame *frame, MFont *font, MFont *spec,
				MRealizedFont *);
static int xft_has_char (MFrame *frame, MFont *font, MFont *spec,
			 int c, unsigned code);
static unsigned xft_encode_char (MFrame *frame, MFont *font, MFont *spec,
				 unsigned code);
static void xft_find_metric (MRealizedFont *, MGlyphString *, int, int);
static void xft_render (MDrawWindow, int, int, MGlyphString *,
			MGlyph *, MGlyph *, int, MDrawRegion);
static int xft_check_capability (MRealizedFont *rfont, MSymbol capability);
static int xft_check_otf (MFLTFont *font, MFLTOtfSpec *spec);
static int xft_drive_otf (MFLTFont *font, MFLTOtfSpec *spec,
			  MFLTGlyphString *in, int from, int to,
			  MFLTGlyphString *out,
			  MFLTGlyphAdjustment *adjustment);
static int xft_try_otf (MFLTFont *font, MFLTOtfSpec *spec,
			MFLTGlyphString *in, int from, int to);
static int xft_iterate_otf_feature (struct _MFLTFont *font, MFLTOtfSpec *spec,
				    int from, int to, unsigned char *table);


static MFontDriver xft_driver =
  { NULL, xft_open,
    xft_find_metric, xft_has_char, xft_encode_char, xft_render, NULL, NULL,
    xft_check_capability, NULL, NULL, xft_check_otf, xft_drive_otf, xft_try_otf,
#ifdef HAVE_OTF
    xft_iterate_otf_feature
#endif	/* HAVE_OTF */
  };

static void
close_xft (void *object)
{
  MRealizedFontXft *rfont_xft = object;

  if (rfont_xft->font_aa)
    XftFontClose (rfont_xft->display, rfont_xft->font_aa);
  if (rfont_xft->font_no_aa)
    XftFontClose (rfont_xft->display, rfont_xft->font_no_aa);
  M17N_OBJECT_UNREF (rfont_xft->info);
  free (rfont_xft);
}


static XftFont *
xft_open_font (Display *display, MSymbol file, double size,
	       FcBool anti_alias)
{
  FcPattern *pattern;
  XftFont *font;

  pattern = FcPatternCreate ();
  FcPatternAddString (pattern, FC_FILE, (FcChar8 *) msymbol_name (file));
  FcPatternAddDouble (pattern, FC_PIXEL_SIZE, size);
  FcPatternAddBool (pattern, FC_ANTIALIAS, anti_alias);
  font = XftFontOpenPattern (display, pattern);
  return font;
}


static MRealizedFont *
xft_open (MFrame *frame, MFont *font, MFont *spec, MRealizedFont *rfont)
{
  Display *display = FRAME_DISPLAY (frame);
  int reg = spec->property[MFONT_REGISTRY];
  FT_Face ft_face;
  MRealizedFontXft *rfont_xft;
  FcBool anti_alias = FRAME_DEVICE (frame)->depth > 1 ? FcTrue : FcFalse;
  int size;
  XftFont *xft_font;
  int ascent, descent, max_advance, average_width, baseline_offset;

  if (font->size)
    /* non-scalable font */
    size = font->size;
  else if (spec->size)
    {
      int ratio = mfont_resize_ratio (font);

      size = ratio == 100 ? spec->size : spec->size * ratio / 100;
    }
  else
    size = 120;

  if (rfont)
    {
      MRealizedFont *save = NULL;

      for (; rfont; rfont = rfont->next)
	if (rfont->font == font
	    && (rfont->font->size ? rfont->font->size == size
		: rfont->spec.size == size)
	    && rfont->spec.property[MFONT_REGISTRY] == reg)
	  {
	    if (! save)
	      save = rfont;
	    if (rfont->driver == &xft_driver)
	      return rfont;
	  }
      rfont = save;
    }
  rfont = (mfont__ft_driver.open) (frame, font, spec, rfont);
  if (! rfont)
    return NULL;
  ascent = rfont->ascent;
  descent = rfont->descent;
  max_advance = rfont->max_advance;
  average_width = rfont->average_width;
  baseline_offset = rfont->baseline_offset;
  spec = &rfont->spec;
  ft_face = rfont->fontp;
  xft_font = xft_open_font (display, font->file, size / 10, anti_alias);
  if (! xft_font)
    return NULL;
  M17N_OBJECT (rfont_xft, close_xft, MERROR_WIN);
  rfont_xft->display = display;
  if (anti_alias == FcTrue)
    rfont_xft->font_aa = xft_font;
  else
    rfont_xft->font_no_aa = xft_font;
  rfont_xft->ft_face = ft_face;
  rfont_xft->info = rfont->info;
  M17N_OBJECT_REF (rfont->info);
  MSTRUCT_CALLOC (rfont, MERROR_FONT_X);
  rfont->id = font->file;
  rfont->spec = *spec;
  rfont->spec.size = size;
  rfont->frame = frame;
  rfont->font = font;
  rfont->driver = &xft_driver;
  rfont->info = rfont_xft;
  rfont->ascent = ascent;
  rfont->descent = descent;
  rfont->max_advance = max_advance;
  rfont->average_width = average_width;
  rfont->baseline_offset = baseline_offset;
  rfont->x_ppem = ft_face->size->metrics.x_ppem;
  rfont->y_ppem = ft_face->size->metrics.y_ppem;
  rfont->fontp = xft_font;
  rfont->next = MPLIST_VAL (frame->realized_font_list);
  MPLIST_VAL (frame->realized_font_list) = rfont;
  return rfont;
}

static void
xft_find_metric (MRealizedFont *rfont, MGlyphString *gstring,
		int from, int to)
{
  Display *display = FRAME_DISPLAY (rfont->frame);
  XftFont *xft_font = rfont->fontp;
  MGlyph *g = MGLYPH (from), *gend = MGLYPH (to);

  for (; g != gend; g++)
    if (! g->g.measured)
      {
	if (g->g.code == MCHAR_INVALID_CODE)
	  {
	    g->g.lbearing = 0;
	    g->g.rbearing = xft_font->max_advance_width << 6;
	    g->g.xadv = g->g.rbearing << 6;
	    g->g.ascent = xft_font->ascent << 6;
	    g->g.descent = xft_font->descent << 6;
	  }
	else
	  {
	    XGlyphInfo extents;

	    XftGlyphExtents (display, xft_font, &g->g.code, 1, &extents);
	    g->g.lbearing = (- extents.x) << 6;
	    g->g.rbearing = (extents.width - extents.x) << 6;
	    g->g.xadv = extents.xOff << 6;
	    g->g.ascent = extents.y << 6;
	    g->g.descent = (extents.height - extents.y) << 6;
	  }
	g->g.yadv = 0;
	g->g.measured = 1;
      }
}

static int
xft_has_char (MFrame *frame, MFont *font, MFont *spec, int c, unsigned code)
{
  int result;

  if (font->type == MFONT_TYPE_REALIZED)
    {
      MRealizedFont *rfont = (MRealizedFont *) font;
      MRealizedFontXft *rfont_xft = rfont->info;
      
      rfont->info = rfont_xft->info;
      result = mfont__ft_driver.has_char (frame, font, spec, c, code);
      rfont->info = rfont_xft;
    }
  else
    result = mfont__ft_driver.has_char (frame, font, spec, c, code);
  return result;
}

static unsigned
xft_encode_char (MFrame *frame, MFont *font, MFont *spec, unsigned code)
{
  if (font->type == MFONT_TYPE_REALIZED)
    {
      MRealizedFont *rfont = (MRealizedFont *) font;
      MRealizedFontXft *rfont_xft = rfont->info;
      
      rfont->info = rfont_xft->info;
      code = mfont__ft_driver.encode_char (frame, font, spec, code);
      rfont->info = rfont_xft;
    }
  else
    code = mfont__ft_driver.encode_char (frame, font, spec, code);
  return code;
}

static void 
xft_render (MDrawWindow win, int x, int y,
	    MGlyphString *gstring, MGlyph *from, MGlyph *to,
	    int reverse, MDrawRegion region)
{
  MRealizedFace *rface = from->rface;
  MFrame *frame = rface->frame;
  Display *display = FRAME_DISPLAY (frame);
  MRealizedFont *rfont = rface->rfont;
  MRealizedFontXft *rfont_xft = rfont->info;
  XftDraw *xft_draw = FRAME_DEVICE (frame)->xft_draw;
  XftColor *xft_color = (! reverse
			 ? &((GCInfo *) rface->info)->xft_color_fore
			 : &((GCInfo *) rface->info)->xft_color_back);
  int anti_alias = (gstring->control.anti_alias
		    && FRAME_DEVICE (frame)->depth > 1);
  XftFont *xft_font;
  MGlyph *g;
  FT_UInt *glyphs;
  int last_x;
  int nglyphs;

  if (from == to)
    return;

  if (anti_alias)
    {
      if (rfont_xft->font_aa)
	xft_font = rfont_xft->font_aa;
      else
	{
	  double size = rfont->spec.size;

	  xft_font = xft_open_font (display, rfont->spec.file, size / 10,
				    FcTrue);
	  if (xft_font)
	    rfont_xft->font_aa = xft_font;
	  else
	    xft_font = rfont->fontp;
	}
    }
  else
    {
      if (rfont_xft->font_no_aa)
	xft_font = rfont_xft->font_no_aa;
      else
	{
	  double size = rfont->spec.size;

	  xft_font = xft_open_font (display, rfont->spec.file, size / 10,
				    FcTrue);
	  if (xft_font)
	    rfont_xft->font_no_aa = xft_font;
	  else
	    xft_font = rfont->fontp;
	}
    }

  XftDrawChange (xft_draw, (Drawable) win);
  XftDrawSetClip (xft_draw, (Region) region);
      
  y -= rfont->baseline_offset >> 6;
  glyphs = alloca (sizeof (FT_UInt) * (to - from));
  for (last_x = x, nglyphs = 0, g = from; g < to; x += g++->g.xadv)
    {
      if (! g->g.adjusted && !g->left_padding && !g->right_padding)
	glyphs[nglyphs++] = g->g.code;
      else
	{
	  if (nglyphs > 0)
	    XftDrawGlyphs (xft_draw, xft_color, xft_font,
			   last_x, y, glyphs, nglyphs);
	  nglyphs = 0;
	  XftDrawGlyphs (xft_draw, xft_color, xft_font,
			 x + g->g.xoff, y + g->g.yoff, (FT_UInt *) &g->g.code, 1);
	  last_x = x + g->g.xadv;
	}
    }
  if (nglyphs > 0)
    XftDrawGlyphs (xft_draw, xft_color, xft_font, last_x, y, glyphs, nglyphs);
}

static int
xft_check_capability (MRealizedFont *rfont, MSymbol capability)
{
  MRealizedFontXft *rfont_xft = rfont->info;
  int result;
      
  rfont->info = rfont_xft->info;
  result = mfont__ft_driver.check_capability (rfont, capability);
  rfont->info = rfont_xft;
  return result;
}

static int
xft_check_otf (MFLTFont *font, MFLTOtfSpec *spec)
{
  MRealizedFont *rfont = ((MFLTFontForRealized *) font)->rfont;
  MRealizedFontXft *rfont_xft = rfont->info;
  int result;
      
  rfont->info = rfont_xft->info;
  result = mfont__ft_driver.check_otf (font, spec);
  rfont->info = rfont_xft;
  return result;
}

static int
xft_drive_otf (MFLTFont *font, MFLTOtfSpec *spec,
	       MFLTGlyphString *in, int from, int to,
	       MFLTGlyphString *out,
	       MFLTGlyphAdjustment *adjustment)
{
  MRealizedFont *rfont = ((MFLTFontForRealized *) font)->rfont;
  MRealizedFontXft *rfont_xft = rfont->info;
  int result;
      
  rfont->info = rfont_xft->info;
  result = mfont__ft_driver.drive_otf (font, spec, in, from, to, out,
				       adjustment);
  rfont->info = rfont_xft;
  return result;
}

static int
xft_try_otf (MFLTFont *font, MFLTOtfSpec *spec,
	     MFLTGlyphString *in, int from, int to)
{
  return xft_drive_otf (font, spec, in, from, to, NULL, NULL);
}

#ifdef HAVE_OTF

static int
xft_iterate_otf_feature (struct _MFLTFont *font, MFLTOtfSpec *spec,
			 int from, int to, unsigned char *table)
{
  MRealizedFont *rfont = ((MFLTFontForRealized *) font)->rfont;
  MRealizedFontXft *rfont_xft = rfont->info;
  int result;
      
  rfont->info = rfont_xft->info;
  result = mfont__ft_driver.iterate_otf_feature (font, spec, from, to, table);
  rfont->info = rfont_xft;
  return result;
}
#endif	/* HAVE_OTF */

#endif	/* HAVE_XFT2 */


/* Functions for the device driver.  */

static void
mwin__close_device (MFrame *frame)
{
  MWDevice *device = FRAME_DEVICE (frame);

  M17N_OBJECT_UNREF (device);
}

static void *
mwin__device_get_prop (MFrame *frame, MSymbol key)
{
  MWDevice *device = FRAME_DEVICE (frame);

  if (key == Mdisplay)
    return (void *) device->display_info->display;
  if (key == Mscreen)
    return (void *) ScreenOfDisplay(device->display_info->display,
				    device->screen_num);
  if (key == Mcolormap)
    return (void *) device->cmap;
  if (key == Mdepth)
    return (void *) device->depth;
  return NULL;
}

static void
mwin__realize_face (MRealizedFace *rface)
{
  MFrame *frame;
  MSymbol foreground, background, videomode;
  MFaceHLineProp *hline;
  MFaceBoxProp *box;
  GCInfo *info;

  if (rface != rface->ascii_rface)
    {
      rface->info = rface->ascii_rface->info;
      return;
    }

  frame = rface->frame;
  MSTRUCT_CALLOC (info, MERROR_WIN);

  foreground = rface->face.property[MFACE_FOREGROUND];
  background = rface->face.property[MFACE_BACKGROUND];
  videomode = rface->face.property[MFACE_VIDEOMODE];
  if (! videomode)
    videomode = frame->videomode;
  if (videomode != Mreverse)
    {
      info->gc[GC_NORMAL] = get_gc (frame, foreground, 1, &info->rgb_fore);
      info->gc[GC_INVERSE] = get_gc (frame, background, 0, &info->rgb_back);
    }
  else
    {
      info->gc[GC_NORMAL] = get_gc (frame, background, 0, &info->rgb_fore);
      info->gc[GC_INVERSE] = get_gc (frame, foreground, 1, &info->rgb_back);
    }
#ifdef HAVE_XFT2
  if (foreground == Mnil)
    foreground = frame->foreground;
  if (background == Mnil)
    background = frame->background;
  if (videomode == Mreverse)
    {
      MSymbol temp = foreground;
      foreground = background;
      background = temp;
    }
  if (! XftColorAllocName (FRAME_DISPLAY (frame),
			   FRAME_VISUAL (frame),
			   FRAME_CMAP (frame),
			   MSYMBOL_NAME (foreground),
			   &info->xft_color_fore))
    mdebug_hook ();
  if (! XftColorAllocName (FRAME_DISPLAY (frame),
			   FRAME_VISUAL (frame),
			   FRAME_CMAP (frame),
			   MSYMBOL_NAME (background),
			   &info->xft_color_back))
    mdebug_hook ();
#endif	/* HAVE_XFT2 */

  hline = rface->hline;
  if (hline)
    {
      if (hline->color)
	info->gc[GC_HLINE] = get_gc (frame, hline->color, 1, NULL);
      else
	info->gc[GC_HLINE] = info->gc[GC_NORMAL];
    }

  box = rface->box;
  if (box)
    {
      if (box->color_top)
	info->gc[GC_BOX_TOP] = get_gc (frame, box->color_top, 1, NULL);
      else
	info->gc[GC_BOX_TOP] = info->gc[GC_NORMAL];

      if (box->color_left && box->color_left != box->color_top)
	info->gc[GC_BOX_LEFT] = get_gc (frame, box->color_left, 1, NULL);
      else
	info->gc[GC_BOX_LEFT] = info->gc[GC_BOX_TOP];

      if (box->color_bottom && box->color_bottom != box->color_top)
	info->gc[GC_BOX_BOTTOM] = get_gc (frame, box->color_bottom, 1, NULL);
      else
	info->gc[GC_BOX_BOTTOM] = info->gc[GC_BOX_TOP];

      if (box->color_right && box->color_right != box->color_bottom)
	info->gc[GC_BOX_RIGHT] = get_gc (frame, box->color_right, 1, NULL);
      else
	info->gc[GC_BOX_RIGHT] = info->gc[GC_BOX_BOTTOM];
    }

  rface->info = info;
}


static void
mwin__free_realized_face (MRealizedFace *rface)
{
  if (rface == rface->ascii_rface)
    free (rface->info);
}


static void
mwin__fill_space (MFrame *frame, MDrawWindow win, MRealizedFace *rface,
		  int reverse,
		  int x, int y, int width, int height, MDrawRegion region)
{
  GC gc = ((GCInfo *) rface->info)->gc[reverse ? GC_NORMAL : GC_INVERSE];

  if (region)
    gc = set_region (frame, gc, region);

  XFillRectangle (FRAME_DISPLAY (frame), (Window) win, gc,
		  x, y, width, height);
}


static void
mwin__draw_empty_boxes (MDrawWindow win, int x, int y,
			MGlyphString *gstring, MGlyph *from, MGlyph *to,
			int reverse, MDrawRegion region)
{
  MRealizedFace *rface = from->rface;
  Display *display = FRAME_DISPLAY (rface->frame);
  GC gc = ((GCInfo *) rface->info)->gc[reverse ? GC_INVERSE : GC_NORMAL];

  if (from == to)
    return;

  if (region)
    gc = set_region (rface->frame, gc, region);
  for (; from < to; from++)
    {
      XDrawRectangle (display, (Window) win, gc,
		      x, y - gstring->ascent + 1, from->g.xadv - 1,
		      gstring->ascent + gstring->descent - 2);
      x += from->g.xadv;
    }
}


static void
mwin__draw_hline (MFrame *frame, MDrawWindow win, MGlyphString *gstring,
		 MRealizedFace *rface, int reverse,
		 int x, int y, int width, MDrawRegion region)
{
  enum MFaceHLineType type = rface->hline->type;
  GCInfo *info = rface->info;
  GC gc = gc = info->gc[GC_HLINE];
  int i;

  y = (type == MFACE_HLINE_BOTTOM
       ? y + gstring->text_descent - rface->hline->width
       : type == MFACE_HLINE_UNDER
       ? y + 1
       : type == MFACE_HLINE_STRIKE_THROUGH
       ? y - ((gstring->ascent + gstring->descent) / 2)
       : y - gstring->text_ascent);
  if (region)
    gc = set_region (frame, gc, region);

  for (i = 0; i < rface->hline->width; i++)
    XDrawLine (FRAME_DISPLAY (frame), (Window) win, gc,
	       x, y + i, x + width - 1, y + i);
}


static void
mwin__draw_box (MFrame *frame, MDrawWindow win, MGlyphString *gstring,
		MGlyph *g, int x, int y, int width, MDrawRegion region)
{
  Display *display = FRAME_DISPLAY (frame);
  MRealizedFace *rface = g->rface;
  MFaceBoxProp *box = rface->box;
  GCInfo *info = rface->info;
  GC gc_top, gc_left, gc_right, gc_btm;
  int y0, y1;
  int i;

  y0 = y - (gstring->text_ascent
	    + rface->box->inner_vmargin + rface->box->width);
  y1 = y + (gstring->text_descent
	    + rface->box->inner_vmargin + rface->box->width - 1);

  gc_top = info->gc[GC_BOX_TOP];
  if (region)
    gc_top = set_region (frame, gc_top, region);
  if (info->gc[GC_BOX_TOP] == info->gc[GC_BOX_BOTTOM])
    gc_btm = gc_top;
  else
    gc_btm = info->gc[GC_BOX_BOTTOM];

  if (g->type == GLYPH_BOX)
    {
      int x0, x1;

      if (g->left_padding)
	x0 = x + box->outer_hmargin, x1 = x + g->g.xadv - 1;
      else
	x0 = x, x1 = x + g->g.xadv - box->outer_hmargin - 1;

      /* Draw the top side.  */
      for (i = 0; i < box->width; i++)
	XDrawLine (display, (Window) win, gc_top, x0, y0 + i, x1, y0 + i);

      /* Draw the bottom side.  */
      if (region && gc_btm != gc_top)
	gc_btm = set_region (frame, gc_btm, region);
      for (i = 0; i < box->width; i++)
	XDrawLine (display, (Window) win, gc_btm, x0, y1 - i, x1, y1 - i);

      if (g->left_padding > 0)
	{
	  /* Draw the left side.  */
	  if (info->gc[GC_BOX_LEFT] == info->gc[GC_BOX_TOP])
	    gc_left = gc_top;
	  else
	    {
	      gc_left = info->gc[GC_BOX_LEFT];
	      if (region)
		gc_left = set_region (frame, gc_left, region);
	    }
	  for (i = 0; i < rface->box->width; i++)
	    XDrawLine (display, (Window) win, gc_left,
		       x0 + i, y0 + i, x0 + i, y1 - i);
	}
      else
	{
	  /* Draw the right side.  */
	  if (info->gc[GC_BOX_RIGHT] == info->gc[GC_BOX_TOP])
	    gc_right = gc_top;
	  else
	    {
	      gc_right = info->gc[GC_BOX_RIGHT];
	      if (region)
		gc_right = set_region (frame, gc_right, region);
	    }
	  for (i = 0; i < rface->box->width; i++)
	    XDrawLine (display, (Window) win, gc_right,
		       x1 - i, y0 + i, x1 - i, y1 - i);
	}
    }
  else
    {
      /* Draw the top side.  */
      for (i = 0; i < box->width; i++)
	XDrawLine (display, (Window) win, gc_top,
		   x, y0 + i, x + width - 1, y0 + i);

      /* Draw the bottom side.  */
      if (region && gc_btm != gc_top)
	gc_btm = set_region (frame, gc_btm, region);
      for (i = 0; i < box->width; i++)
	XDrawLine (display, (Window) win, gc_btm,
		   x, y1 - i, x + width - 1, y1 - i);
    }
}


#if 0
static void
mwin__draw_bitmap (MFrame *frame, MDrawWindow win, MRealizedFace *rface,
		   int reverse, int x, int y,
		   int width, int height, int row_bytes, unsigned char *bmp,
		   MDrawRegion region)
{
  Display *display = FRAME_DISPLAY (frame);
  int i, j;
  GC gc = ((GCInfo *) rface->info)->gc[reverse ? GC_INVERSE : GC_NORMAL];

  if (region)
    gc = set_region (frame, gc, region);

  for (i = 0; i < height; i++, bmp += row_bytes)
    for (j = 0; j < width; j++)
      if (bmp[j / 8] & (1 << (7 - (j % 8))))
	XDrawPoint (display, (Window) win, gc, x + j, y + i);
}
#endif

static void
mwin__draw_points (MFrame *frame, MDrawWindow win, MRealizedFace *rface,
		   int intensity, MDrawPoint *points, int num,
		   MDrawRegion region)
{
  GCInfo *info = rface->info;
  GC gc;

  if (! (gc = info->gc[intensity]))
    gc = info->gc[intensity] = get_gc_for_anti_alias (FRAME_DEVICE (frame),
						      info, intensity);
  if (region)
    gc = set_region (frame, gc, region);

  XDrawPoints (FRAME_DISPLAY (frame), (Window) win, gc,
	       (XPoint *) points, num, CoordModeOrigin);
}


static MDrawRegion
mwin__region_from_rect (MDrawMetric *rect)
{
  MDrawRegion region1 = XCreateRegion ();
  MDrawRegion region2 = XCreateRegion ();
  XRectangle xrect;

  xrect.x = rect->x;
  xrect.y = rect->y;
  xrect.width = rect->width;
  xrect.height = rect->height;
  XUnionRectWithRegion (&xrect, region1, region2);
  XDestroyRegion (region1);
  return region2;
}

static void
mwin__union_rect_with_region (MDrawRegion region, MDrawMetric *rect)
{
  MDrawRegion region1 = XCreateRegion ();
  XRectangle xrect;

  xrect.x = rect->x;
  xrect.y = rect->y;
  xrect.width = rect->width;
  xrect.height = rect->height;

  XUnionRegion (region, region, region1);
  XUnionRectWithRegion (&xrect, region1, region);
  XDestroyRegion (region1);
}

static void
mwin__intersect_region (MDrawRegion region1, MDrawRegion region2)
{
  MDrawRegion region = XCreateRegion ();

  XUnionRegion (region1, region1, region);
  XIntersectRegion (region, region2, region1);
  XDestroyRegion (region);
}

static void
mwin__region_add_rect (MDrawRegion region, MDrawMetric *rect)
{
  MDrawRegion region1 = XCreateRegion ();
  XRectangle xrect;

  xrect.x = rect->x;
  xrect.y = rect->y;
  xrect.width = rect->width;
  xrect.height = rect->height;
  XUnionRectWithRegion (&xrect, region1, region);
  XDestroyRegion (region1);
}

static void
mwin__region_to_rect (MDrawRegion region, MDrawMetric *rect)
{
  XRectangle xrect;

  XClipBox (region, &xrect);
  rect->x = xrect.x;
  rect->y = xrect.y;
  rect->width = xrect.width;
  rect->height = xrect.height;
}

static void
mwin__free_region (MDrawRegion region)
{
  XDestroyRegion (region);
}

static void
mwin__dump_region (MDrawRegion region)
{
  XRectangle rect;
  XClipBox (region, &rect);
  fprintf (mdebug__output, "(%d %d %d %d)\n", rect.x, rect.y, rect.width, rect.height);
}


static MDrawWindow
mwin__create_window (MFrame *frame, MDrawWindow parent)
{
  Display *display = FRAME_DISPLAY (frame);
  Window win;
  XWMHints wm_hints = { InputHint, False };
  XClassHint class_hints = { "M17N-IM", "m17n-im" };
  XSetWindowAttributes set_attrs;
  unsigned long mask;
  XGCValues values;
  GCInfo *info = frame->rface->info;

  if (! parent)
    parent = (MDrawWindow) RootWindow (display, FRAME_SCREEN (frame));
  mask = GCForeground;
  XGetGCValues (display, info->gc[GC_INVERSE], mask, &values);
  set_attrs.background_pixel = values.foreground;
  set_attrs.backing_store = Always;
  set_attrs.override_redirect = True;
  set_attrs.save_under = True;
  mask = CWBackPixel | CWBackingStore | CWOverrideRedirect | CWSaveUnder;
  win = XCreateWindow (display, (Window) parent, 0, 0, 1, 1, 0,
		       CopyFromParent, InputOutput, CopyFromParent,
		       mask, &set_attrs);
  XSetWMProperties (display, (Window) win, NULL, NULL, NULL, 0,
		    NULL, &wm_hints, &class_hints);
  XSelectInput (display, (Window) win, StructureNotifyMask | ExposureMask);
  return (MDrawWindow) win;
}

static void
mwin__destroy_window (MFrame *frame, MDrawWindow win)
{
#ifdef HAVE_XFT2
  XftDraw *xft_draw = FRAME_DEVICE (frame)->xft_draw;

  if (XftDrawDrawable (xft_draw) == (Drawable) win)
    XftDrawChange (xft_draw, FRAME_DEVICE (frame)->drawable);
#endif	/* HAVE_XFT2 */
  XDestroyWindow (FRAME_DISPLAY (frame), (Window) win);
}

#if 0
static MDrawWindow
mwin__event_window (void *event)
{
  return ((MDrawWindow) ((XEvent *) event)->xany.window);
}

static void
mwin__print_event (void *arg, char *win_name)
{
  char *event_name;
  XEvent *event = (XEvent *) arg;

  switch (event->xany.type)
    {
    case 2: event_name = "KeyPress"; break;
    case 3: event_name = "KeyRelease"; break;
    case 4: event_name = "ButtonPress"; break;
    case 5: event_name = "ButtonRelease"; break;
    case 6: event_name = "MotionNotify"; break;
    case 7: event_name = "EnterNotify"; break;
    case 8: event_name = "LeaveNotify"; break;
    case 9: event_name = "FocusIn"; break;
    case 10: event_name = "FocusOut"; break;
    case 11: event_name = "KeymapNotify"; break;
    case 12: event_name = "Expose"; break;
    case 13: event_name = "GraphicsExpose"; break;
    case 14: event_name = "NoExpose"; break;
    case 15: event_name = "VisibilityNotify"; break;
    case 16: event_name = "CreateNotify"; break;
    case 17: event_name = "DestroyNotify"; break;
    case 18: event_name = "UnmapNotify"; break;
    case 19: event_name = "MapNotify"; break;
    case 20: event_name = "MapRequest"; break;
    case 21: event_name = "ReparentNotify"; break;
    case 22: event_name = "ConfigureNotify"; break;
    case 23: event_name = "ConfigureRequest"; break;
    case 24: event_name = "GravityNotify"; break;
    case 25: event_name = "ResizeRequest"; break;
    case 26: event_name = "CirculateNotify"; break;
    case 27: event_name = "CirculateRequest"; break;
    case 28: event_name = "PropertyNotify"; break;
    case 29: event_name = "SelectionClear"; break;
    case 30: event_name = "SelectionRequest"; break;
    case 31: event_name = "SelectionNotify"; break;
    case 32: event_name = "ColormapNotify"; break;
    case 33: event_name = "ClientMessage"; break;
    case 34: event_name = "MappingNotify"; break;
    default: event_name = "unknown";
    }

  fprintf (mdebug__output, "%s: %s\n", win_name, event_name);
}
#endif

static void
mwin__map_window (MFrame *frame, MDrawWindow win)
{
  XMapRaised (FRAME_DISPLAY (frame), (Window) win);
}

static void
mwin__unmap_window (MFrame *frame, MDrawWindow win)
{
  XUnmapWindow (FRAME_DISPLAY (frame), (Window) win);
}

static void
mwin__window_geometry (MFrame *frame, MDrawWindow win, MDrawWindow parent_win,
		       MDrawMetric *geometry)
{
  Display *display = FRAME_DISPLAY (frame);
  XWindowAttributes attr;
  Window parent = (Window) parent_win, root;

  XGetWindowAttributes (display, (Window) win, &attr);
  geometry->x = attr.x + attr.border_width;
  geometry->y = attr.y + attr.border_width;
  geometry->width = attr.width;
  geometry->height = attr.height; 

  if (! parent)
    parent = RootWindow (display, FRAME_SCREEN (frame));
  while (1)
    {
      Window this_parent, *children;
      unsigned n;

      XQueryTree (display, (Window) win, &root, &this_parent, &children, &n);
      if (children)
	XFree (children);
      if (this_parent == parent || this_parent == root)
	break;
      win = (MDrawWindow) this_parent;
      XGetWindowAttributes (display, (Window) win, &attr);
      geometry->x += attr.x + attr.border_width;
      geometry->y += attr.y + attr.border_width;
    }
}

static void
mwin__adjust_window (MFrame *frame, MDrawWindow win,
		     MDrawMetric *current, MDrawMetric *new)
{
  Display *display = FRAME_DISPLAY (frame);
  unsigned int mask = 0;
  XWindowChanges values;

  if (current->width != new->width)
    {
      mask |= CWWidth;
      if (new->width <= 0)
	new->width = 1;
      values.width = current->width = new->width;
    }
  if (current->height != new->height)
    {
      mask |= CWHeight;
      if (new->height <= 0)
	new->height = 1;
      values.height = current->height = new->height;
    }
  if (current->x != new->x)
    {
      mask |= CWX;
      values.x = current->x = new->x;
    }
  if (current->y != new->y)
    {
      mask |= CWY;
      current->y = new->y;
      values.y = current->y = new->y;
    }
  if (mask)
    XConfigureWindow (display, (Window) win, mask, &values);
  XClearWindow (display, (Window) win);
}

static MSymbol
mwin__parse_event (MFrame *frame, void *arg, int *modifiers)
{
  XEvent *event = (XEvent *) arg;
  MDisplayInfo *disp_info = FRAME_DEVICE (frame)->display_info;
  int len;
  char buf[512];
  KeySym keysym;
  MSymbol key;

  *modifiers = 0;
  if (event->xany.type != KeyPress
      /* && event->xany.type != KeyRelease */
      )
    return Mnil;
  len = XLookupString ((XKeyEvent *) event, (char *) buf, 512, &keysym, NULL);
  if (len > 1)
    return Mnil;
  if (keysym >= XK_Shift_L && keysym <= XK_Hyper_R)
    return Mnil;
#ifdef XK_XKB_KEYS
  if ((keysym & 0xff00) == 0xfe00)
    return Mnil;
#endif
  if (len == 1 && keysym >= XK_space && keysym <= XK_asciitilde)
    {
      int c = keysym;

      key = minput__char_to_key (c);
      if (c == ' ' && ((XKeyEvent *) event)->state & ShiftMask)
	*modifiers |= MINPUT_KEY_SHIFT_MODIFIER;
    }
  else
    {
      char *str = XKeysymToString (keysym);

      if (! str)
	return Mnil;
      key = msymbol (str);
      if (((XKeyEvent *) event)->state & ShiftMask)
	*modifiers |= MINPUT_KEY_SHIFT_MODIFIER;
    }
  if (((XKeyEvent *) event)->state & ControlMask)
    *modifiers |= MINPUT_KEY_CONTROL_MODIFIER;
  if (((XKeyEvent *) event)->state & disp_info->meta_mask)
    *modifiers |= MINPUT_KEY_META_MODIFIER;
  if (((XKeyEvent *) event)->state & disp_info->alt_mask)
    *modifiers |= MINPUT_KEY_ALT_MODIFIER;
  if (((XKeyEvent *) event)->state & disp_info->super_mask)
    *modifiers |= MINPUT_KEY_SUPER_MODIFIER;
  if (((XKeyEvent *) event)->state & disp_info->hyper_mask)
    *modifiers |= MINPUT_KEY_HYPER_MODIFIER;
#ifdef XK_XKB_KEYS
  if (((XKeyEvent *) event)->state & disp_info->altgr_mask)
    *modifiers |= MINPUT_KEY_ALTGR_MODIFIER;
#endif
  return key;
}


void
mwin__dump_gc (MFrame *frame, MRealizedFace *rface)
{
  unsigned long valuemask = GCForeground | GCBackground | GCClipMask;
  XGCValues values;
  Display *display = FRAME_DISPLAY (frame);
  GCInfo *info = rface->info;
  int i;

  for (i = 0; i <= GC_INVERSE; i++)
    {
      XGetGCValues (display, info->gc[i], valuemask, &values);
      fprintf (mdebug__output, "GC%d: fore/#%lX back/#%lX", i,
	       values.foreground, values.background);
      fprintf (mdebug__output, "\n");
    }
}

static MDeviceDriver x_driver =
  {
    mwin__close_device,
    mwin__device_get_prop,
    mwin__realize_face,
    mwin__free_realized_face,
    mwin__fill_space,
    mwin__draw_empty_boxes,
    mwin__draw_hline,
    mwin__draw_box,
    mwin__draw_points,
    mwin__region_from_rect,
    mwin__union_rect_with_region,
    mwin__intersect_region,
    mwin__region_add_rect,
    mwin__region_to_rect,
    mwin__free_region,
    mwin__dump_region,
    mwin__create_window,
    mwin__destroy_window,
    mwin__map_window,
    mwin__unmap_window,
    mwin__window_geometry,
    mwin__adjust_window,
    mwin__parse_event
  };

/* Functions to be stored in MDeviceLibraryInterface by dlsym ().  */

int
device_init ()
{
  M_iso8859_1 = msymbol ("iso8859-1");
  M_iso10646_1 = msymbol ("iso10646-1");

  display_info_list = mplist ();
  device_list = mplist ();

#ifdef HAVE_XFT2
  xft_driver.select = mfont__ft_driver.select;
  xft_driver.list = mfont__ft_driver.list;
  xft_driver.list_family_names = mfont__ft_driver.list_family_names;
#endif

  Mxim = msymbol ("xim");
  msymbol_put (Mxim, Minput_driver, &minput_xim_driver);

  return 0;
}

int
device_fini ()
{
  M17N_OBJECT_UNREF (display_info_list);
  M17N_OBJECT_UNREF (device_list);
  return 0;
}


#ifdef X_SET_ERROR_HANDLER
static int
x_error_handler (Display *display, XErrorEvent *error)
{
  mdebug_hook ();
  return 0;
}

static int
x_io_error_handler (Display *display)
{
  mdebug_hook ();
  return 0;
}
#endif

/** Return an MWDevice object corresponding to a display specified in
    PLIST.

    It searches device_list for a device matching the display.  If
    found, return the found object.  Otherwise, return a newly created
    object.  */

int
device_open (MFrame *frame, MPlist *param)
{
  Display *display = NULL;
  Screen *screen = NULL;
  int screen_num;
  Drawable drawable = 0;
  Widget widget = NULL;
  Colormap cmap = 0;
  int auto_display = 0;
  MDisplayInfo *disp_info = NULL;
  MWDevice *device = NULL;
  MSymbol key;
  XWindowAttributes attr;
  unsigned depth = 0;
  MPlist *plist;
  AppData app_data;
  MFont *font = NULL;
  MFace *face;
  int use_xfont = 0, use_freetype = 0, use_xft = 0;

  for (plist = param; (key = mplist_key (plist)) != Mnil;
       plist = mplist_next (plist))
    {
      if (key == Mdisplay)
	display = (Display *) mplist_value (plist);
      else if (key == Mscreen)
	screen = mplist_value (plist);
      else if (key == Mdrawable)
	drawable = (Drawable) mplist_value (plist);
      else if (key == Mdepth)
	depth = (unsigned) mplist_value (plist);
      else if (key == Mwidget)
	widget = (Widget) mplist_value (plist);
      else if (key == Mcolormap)
	cmap = (Colormap) mplist_value (plist);
      else if (key == Mfont)
	{
	  MSymbol val = MPLIST_SYMBOL (plist);

	  if (val == Mx)
	    use_xfont = 1;
#ifdef HAVE_FREETYPE
	  else if (val == Mfreetype)
	    use_freetype = 1;
#ifdef HAVE_XFT2
	  else if (val == Mxft)
	    use_xft = 1;
#endif
#endif
	}
    }

  /* If none of them is specified, use all of them.  */
  if (! use_xfont && ! use_freetype && ! use_xft)
    use_xfont = use_freetype = use_xft = 1;

  if (widget)
    {
      display = XtDisplay (widget);
      screen_num = XScreenNumberOfScreen (XtScreen (widget));
      depth = DefaultDepth (display, screen_num);
    }
  else if (drawable)
    {
      Window root_window;
      int x, y;
      unsigned width, height, border_width;

      if (! display)
	MERROR (MERROR_WIN, -1);
      XGetGeometry (display, drawable, &root_window,
		    &x, &y, &width, &height, &border_width, &depth);
      XGetWindowAttributes (display, root_window, &attr);
      screen_num = XScreenNumberOfScreen (attr.screen);
    }
  else
    {
      if (screen)
	display = DisplayOfScreen (screen);
      else
	{
	  if (! display)
	    {
	      display = XOpenDisplay (NULL);
	      if (! display)
		MERROR (MERROR_WIN, -1);
	      auto_display = 1;
	    }
	  screen = DefaultScreenOfDisplay (display);
	}
      screen_num = XScreenNumberOfScreen (screen);
      if (! depth)
	depth = DefaultDepth (display, screen_num);
    }

  if (! cmap)
    cmap = DefaultColormap (display, screen_num);

  for (plist = display_info_list; mplist_key (plist) != Mnil;
       plist = mplist_next (plist))
    {
      disp_info = (MDisplayInfo *) mplist_value (plist);
      if (disp_info->display == display)
	break;
    }

  if (mplist_key (plist) != Mnil)
    M17N_OBJECT_REF (disp_info);
  else
    {
      M17N_OBJECT (disp_info, free_display_info, MERROR_WIN);
      disp_info->display = display;
      disp_info->auto_display = auto_display;
      disp_info->font_list = mplist ();
      find_modifier_bits (disp_info);
      disp_info->MULE_BASELINE_OFFSET
	= XInternAtom (display, "_MULE_BASELINE_OFFSET", False);
      disp_info->AVERAGE_WIDTH
	= XInternAtom (display, "AVERAGE_WIDTH", False);
      mplist_add (display_info_list, Mt, disp_info);
    }  

  for (plist = device_list; mplist_key (plist) != Mnil;
       plist = mplist_next (plist))
    {
      device = (MWDevice *) mplist_value (plist);
      if (device->display_info == disp_info
	  && device->depth == depth
	  && device->cmap == cmap
	  && device->screen_num == screen_num)
	break;
    }

  if (mplist_key (plist) != Mnil)
    M17N_OBJECT_REF (device);
  else
    {
      unsigned long valuemask = GCForeground;
      XGCValues values;
      double pixels, mm;

      M17N_OBJECT (device, free_device, MERROR_WIN);
      device->display_info = disp_info;
      device->screen_num = screen_num;
      /* A drawable on which to create GCs.  */
      device->drawable = XCreatePixmap (display,
					RootWindow (display, screen_num),
					1, 1, depth);
      device->depth = depth;
      device->cmap = cmap;
      pixels = DisplayHeight (display, screen_num);
      mm = DisplayHeightMM (display, screen_num);
      device->resy = (mm < 1) ? 100 : pixels * 25.4 / mm;
      device->realized_face_list = mplist ();
      device->realized_font_list = mplist ();
      mplist_add (device->realized_font_list, Mt, NULL);
      device->realized_fontset_list = mplist ();
      device->gc_list = mplist ();
      values.foreground = BlackPixel (display, screen_num);
      device->scratch_gc = XCreateGC (display, device->drawable,
				      valuemask, &values);
#ifdef HAVE_XFT2
      device->xft_draw = XftDrawCreate (display, device->drawable,
					DefaultVisual (display, screen_num),
					cmap);
#endif
    }

  frame->device = device;
  frame->device_type = MDEVICE_SUPPORT_OUTPUT | MDEVICE_SUPPORT_INPUT;
  frame->dpi = device->resy;
  frame->driver = &x_driver;
  frame->font_driver_list = mplist ();
#ifdef HAVE_XFT2
  if (use_xft)
    {
      mplist_add (frame->font_driver_list, Mfreetype, &xft_driver);
      use_freetype = 0;
    }
#endif	/* HAVE_XFT2 */
#ifdef HAVE_FREETYPE
  if (use_freetype)
    mplist_add (frame->font_driver_list, Mfreetype, &mfont__ft_driver);
#endif	/* HAVE_FREETYPE */
  if (use_xfont || MPLIST_TAIL_P (frame->font_driver_list))
    mplist_add (frame->font_driver_list, Mx, &xfont_driver);

  frame->realized_font_list = device->realized_font_list;
  frame->realized_face_list = device->realized_face_list;
  frame->realized_fontset_list = device->realized_fontset_list;

  if (widget)
    {
      XtResource resources[] = {
	{ XtNfont, XtCFont, XtRString, sizeof (String),
	  XtOffset (AppDataPtr, font), XtRString, DEFAULT_FONT },
	{ XtNforeground, XtCForeground, XtRString, sizeof (String),
	  XtOffset (AppDataPtr, foreground), XtRString, "black" },
	{ XtNbackground, XtCBackground, XtRString, sizeof (String),
	  XtOffset (AppDataPtr, background), XtRString, "white" },
	{ XtNreverseVideo, XtCReverseVideo, XtRBoolean, sizeof (Boolean),
	  XtOffset (AppDataPtr, reverse_video), XtRImmediate, (caddr_t) FALSE }
      };

      XtGetApplicationResources (widget, &app_data,
				 resources, XtNumber (resources), NULL, 0);
      frame->foreground = msymbol (app_data.foreground);
      frame->background = msymbol (app_data.background);
      frame->videomode = app_data.reverse_video == True ? Mreverse : Mnormal;
    }
  else
    {
      app_data.font = DEFAULT_FONT;
      frame->foreground = msymbol ("black");
      frame->background = msymbol ("white");
      frame->videomode = Mnormal;
    }

  if (strcmp (app_data.font, DEFAULT_FONT) != 0)
    {
      XFontStruct *xfont = XLoadQueryFont (display, app_data.font);
      unsigned long value;
      char *name;

      if (xfont)
	{
	  font = mfont_parse_name (app_data.font, Mx);
	  if (! font
	      && XGetFontProperty (xfont, XA_FONT, &value)
	      && (name = ((char *) XGetAtomName (display, (Atom) value))))
	    font = mfont_parse_name (name, Mx);
	  XFreeFont (display, xfont);
	}
    }
  if (! font)
      font = mfont_parse_name (DEFAULT_FONT, Mx);
  else if (! font->size)
    font->size = 130;
  face = mface_from_font (font);
  free (font);
  face->property[MFACE_FONTSET] = mfontset (NULL);
  face->property[MFACE_FOREGROUND] = frame->foreground;
  face->property[MFACE_BACKGROUND] = frame->background;
  mface_put_prop (face, Mhline, mface_get_prop (mface__default, Mhline));
  mface_put_prop (face, Mbox, mface_get_prop (mface__default, Mbox));
  face->property[MFACE_VIDEOMODE] = frame->videomode;
  mface_put_prop (face, Mhook_func, 
		  mface_get_prop (mface__default, Mhook_func));
  face->property[MFACE_RATIO] = (void *) 100;
  mplist_push (param, Mface, face);
  M17N_OBJECT_UNREF (face);

#ifdef X_SET_ERROR_HANDLER
  XSetErrorHandler (x_error_handler);
  XSetIOErrorHandler (x_io_error_handler);
#endif
  return 0;
}



/* XIM (X Input Method) handler */

typedef struct MInputXIMMethodInfo
{
  Display *display;
  XIM xim;
  MSymbol language;
  MSymbol coding;
} MInputXIMMethodInfo;

typedef struct MInputXIMContextInfo
{
  XIC xic;
  Window win;
  MConverter *converter;
} MInputXIMContextInfo;

static int
xim_open_im (MInputMethod *im)
{
  MInputXIMArgIM *arg = (MInputXIMArgIM *) im->arg;
  MLocale *saved, *this;
  char *save_modifier_list;
  XIM xim;
  MInputXIMMethodInfo *im_info;

  saved = mlocale_set (LC_CTYPE, NULL);
  this = mlocale_set (LC_CTYPE, arg->locale ? arg->locale : "");
  if (! this)
    /* The specified locale is not supported.  */
    MERROR (MERROR_LOCALE, -1);
  if (mlocale_get_prop (this, Mcoding) == Mnil)
    {
      /* Unable to decode the output of XIM.  */
      mlocale_set (LC_CTYPE, msymbol_name (mlocale_get_prop (saved, Mname)));
      MERROR (MERROR_LOCALE, -1);
    }

  if (arg->modifier_list)
    save_modifier_list = XSetLocaleModifiers (arg->modifier_list);
  else
    save_modifier_list = XSetLocaleModifiers ("");
  if (! save_modifier_list)
    {
      /* The specified locale is not supported by X.  */
      mlocale_set (LC_CTYPE, msymbol_name (mlocale_get_prop (saved, Mname)));
      MERROR (MERROR_LOCALE, -1);
    }

  xim = XOpenIM (arg->display, arg->db, arg->res_name, arg->res_class);
  if (! xim)
    {
      /* No input method is available in the current locale.  */
      XSetLocaleModifiers (save_modifier_list);
      mlocale_set (LC_CTYPE, msymbol_name (mlocale_get_prop (saved, Mname)));
      MERROR (MERROR_WIN, -1);
    }

  MSTRUCT_MALLOC (im_info, MERROR_WIN);
  im_info->display = arg->display;
  im_info->xim = xim;
  im_info->language = mlocale_get_prop (this, Mlanguage);
  im_info->coding = mlocale_get_prop (this, Mcoding);
  im->info = im_info;

  XSetLocaleModifiers (save_modifier_list);
  mlocale_set (LC_CTYPE, msymbol_name (mlocale_get_prop (saved, Mname)));

  return 0;
}

static void
xim_close_im (MInputMethod *im)
{
  MInputXIMMethodInfo *im_info = (MInputXIMMethodInfo *) im->info;

  XCloseIM (im_info->xim);
  free (im_info);
}

static int
xim_create_ic (MInputContext *ic)
{
  MInputXIMArgIC *arg = (MInputXIMArgIC *) ic->arg;
  MInputXIMMethodInfo *im_info = (MInputXIMMethodInfo *) ic->im->info;
  MInputXIMContextInfo *ic_info;
  XIC xic;

  if (! arg->input_style)
    {
      /* By default, use Root style.  */
      arg->input_style = XIMPreeditNothing | XIMStatusNothing;
      arg->preedit_attrs = NULL;
      arg->status_attrs = NULL;
    }

  if (! arg->preedit_attrs && ! arg->status_attrs)
    xic = XCreateIC (im_info->xim,
		     XNInputStyle, arg->input_style,
		     XNClientWindow, arg->client_win,
		     XNFocusWindow, arg->focus_win,
		     NULL);
  else if (arg->preedit_attrs && ! arg->status_attrs)
    xic = XCreateIC (im_info->xim,
		     XNInputStyle, arg->input_style,
		     XNClientWindow, arg->client_win,
		     XNFocusWindow, arg->focus_win,
		     XNPreeditAttributes, arg->preedit_attrs,
		     NULL);
  else if (! arg->preedit_attrs && arg->status_attrs)
    xic = XCreateIC (im_info->xim,
		     XNInputStyle, arg->input_style,
		     XNClientWindow, arg->client_win,
		     XNFocusWindow, arg->focus_win,
		     XNStatusAttributes, arg->status_attrs,
		     NULL);
  else
    xic = XCreateIC (im_info->xim,
		     XNInputStyle, arg->input_style,
		     XNClientWindow, arg->client_win,
		     XNFocusWindow, arg->focus_win,
		     XNPreeditAttributes, arg->preedit_attrs,
		     XNStatusAttributes, arg->status_attrs,
		     NULL);
  if (! xic)
    MERROR (MERROR_WIN, -1);

  MSTRUCT_MALLOC (ic_info, MERROR_WIN);
  ic_info->xic = xic;
  ic_info->win = arg->focus_win;
  ic_info->converter = mconv_buffer_converter (im_info->coding, NULL, 0);
  ic->info = ic_info;
  return 0;
}

static void
xim_destroy_ic (MInputContext *ic)
{
  MInputXIMContextInfo *ic_info = (MInputXIMContextInfo *) ic->info;

  XDestroyIC (ic_info->xic);
  mconv_free_converter (ic_info->converter);
  free (ic_info);
  ic->info = NULL;
}

static int
xim_filter (MInputContext *ic, MSymbol key, void *event)
{
  MInputXIMContextInfo *ic_info = (MInputXIMContextInfo *) ic->info;

  return (XFilterEvent ((XEvent *) event, ic_info->win) == True);
}


static int
xim_lookup (MInputContext *ic, MSymbol key, void *arg, MText *mt)
{
  MInputXIMMethodInfo *im_info = (MInputXIMMethodInfo *) ic->im->info;
  MInputXIMContextInfo *ic_info = (MInputXIMContextInfo *) ic->info;
  XKeyPressedEvent *ev = (XKeyPressedEvent *) arg;
  KeySym keysym;
  Status status;
  char *buf;
  int len;

  buf = (char *) alloca (512);
  len = XmbLookupString (ic_info->xic, ev, buf, 512, &keysym, &status);
  if (status == XBufferOverflow)
    {
      buf = (char *) alloca (len);
      len = XmbLookupString (ic_info->xic, ev, buf, len, &keysym, &status);
    }

  mtext_reset (ic->produced);
  if (len == 0)
    return 1;

  mconv_reset_converter (ic_info->converter);
  mconv_rebind_buffer (ic_info->converter, (unsigned char *) buf, len);
  mconv_decode (ic_info->converter, ic->produced);
  mtext_put_prop (ic->produced, 0, mtext_nchars (ic->produced),
		  Mlanguage, (void *) im_info->language);
  mtext_cpy (mt, ic->produced);
  mtext_reset (ic->produced);  
  return 0;
}


/*=*/

/*** @} */
#endif /* !FOR_DOXYGEN || DOXYGEN_INTERNAL_MODULE */

/* External API */

/*** @addtogroup m17nInputMethodWin */

/*** @{ */
/*=*/
/***en
    @brief Input method driver for XIM.

    The driver #minput_xim_driver is for the foreign input method of
    name #Mxim.  It uses XIM (X Input Methods) as a background input
    engine.

    As the symbol #Mxim has property #Minput_driver whose value is
    a pointer to this driver, the input method of language #Mnil
    and name #Mxim uses this driver.

    Therefore, for such input methods, the driver dependent arguments
    to the functions whose name begin with minput_ must be as follows.

    The argument $ARG of the function minput_open_im () must be a
    pointer to the structure #MInputXIMArgIM.  See the documentation
    of #MInputXIMArgIM for more details.

    The argument $ARG of the function minput_create_ic () must be a
    pointer to the structure #MInputXIMArgIC. See the documentation
    of #MInputXIMArgIC for more details.

    The argument $ARG of the function minput_filter () must be a
    pointer to the structure @c XEvent.  The argument $KEY is ignored.

    The argument $ARG of the function minput_lookup () must be the
    same one as that of the function minput_filter ().  The argument
    $KEY is ignored.  */

/***ja
    @brief XIM�����ϥɥ饤��.

    �ɥ饤�� #minput_xim_driver �� #Mxim ��̾���Ȥ��ƻ��ĳ������ϥ᥽�å��ѤǤ��ꡢ
    XIM (X Input Methods) ��Хå����饦��ɤ����ϥ��󥸥�Ȥ��ƻ��Ѥ��롣

    ����ܥ� #Mxim �Ϥ��Υɥ饤�ФؤΥݥ��󥿤��ͤȤ���ץ�ѥƥ�
    #Minput_driver �������LANGUAGE �� #Mnil ��̾���� #Mxim 
    �Ǥ������ϥ᥽�åɤϤ��Υɥ饤�Ф����Ѥ��롣

    �������äơ����������ϥ᥽�åɤǤϡ�minput_ 
    �ǻϤޤ�̾������Ĵؿ��Υɥ饤�Ф˰�¸��������ϼ��Τ褦�ʤ�ΤǤʤ��ƤϤʤ�ʤ���

    �ؿ� minput_open_im () �ΰ��� $ARG �Ϲ�¤�� #MInputXIMArgIM 
    �ؤΥݥ��󥿤Ǥʤ��ƤϤʤ�ʤ����ܺ٤ˤĤ��Ƥ� #MInputXIMArgIM �������򻲾ȡ�

    �ؿ� minput_create_ic () �ΰ��� $ARG �Ϲ�¤�� #MInputXIMArgIC 
    �ؤΥݥ��󥿤Ǥʤ��ƤϤʤ�ʤ����ܺ٤ˤĤ��Ƥ� #MInputXIMArgIC �������򻲾ȡ�

    �ؿ� minput_filter () �ΰ��� $ARG �Ϲ�¤�� @c XEvent 
    �ؤΥݥ��󥿤Ǥʤ��ƤϤʤ�ʤ������� $KEY ��̵�뤵��롣

    �ؿ� minput_lookup () �ΰ��� $ARG �ϴؿ� function minput_filter () 
    �ΰ��� $ARG ��Ʊ����ΤǤʤ��ƤϤʤ�ʤ��� ���� $KEY �ϡ�̵�뤵��롣  */

MInputDriver minput_xim_driver =
  { xim_open_im, xim_close_im, xim_create_ic, xim_destroy_ic,
    xim_filter, xim_lookup, NULL };
/*=*/
/*** @} */ 
/*=*/
#else  /* not HAVE_X11 */

int device_open () { return -1; }

#endif	/* not HAVE_X11 */

/*
  Local Variables:
  coding: euc-japan
  End:
*/
