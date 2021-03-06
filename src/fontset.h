/* fontset.h -- header file for the fontset module.
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

#ifndef _M17N_FONTSET_H_
#define _M17N_FONTSET_H_

extern MRealizedFontset *mfont__realize_fontset (MFrame *frame,
						 MFontset *fontset,
						 MFace *face, MFont *spec);

void mfont__free_realized_fontset (MRealizedFontset *realized);

extern MRealizedFont *mfont__lookup_fontset (MRealizedFontset *realized,
					     MGlyph *g, int *num,
					     MSymbol script, MSymbol language,
					     MSymbol charset, int size,
					     int ignore_fallback);

extern MRealizedFont *mfontset__get_font (MFrame *frame, MFontset *fontset,
					  MSymbol script, MSymbol language,
					  MFont *font, int *best);


#endif /* _M17N_FONTSET_H_ */
