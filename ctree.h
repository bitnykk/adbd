/*
 * ADBD -- AO Database dumper.
 *
 * $Id: ctree.h 1150 2010-05-13 12:30:54Z os $
 *
 * Copyright (C) 2002-2010 Oskari Saarenmaa <auno@auno.org>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _ADBD_CTREE_H
#define _ADBD_CTREE_H

#include "aodb.h"

typedef struct iseg
{
  short soffset; // segment position (offset)
  short slength; // segment length
  short segmode; // segment mode
} ISEG, *pISEG;

typedef struct iidx
{
  short ikeylen;          // key length
  short ikeytyp;          // key type
  short ikeydup;          // duplicate flag
  short inulkey;          // NULL key flag
  short iempchr;          // empty character
  short inumseg;          // number of segments
  pISEG seg;              // segment information
  const char *ridxnam;    // r-tree symbolic name
  const char *aidxnam;    // optional index file name
  unsigned short *altseq; // optional alternate sequence
  unsigned char *pvbyte;  // optional pointer to pad byte
} IIDX, *pIIDX;

typedef struct ifil
{
  const char *pfilnam;    // file name (w/o ext)
  short dfilno;           // data file number
  unsigned short dreclen; // data record length
  unsigned short dxtdsiz; // data file ext size
  short dfilmod;          // data file mode
  short dnumidx;          // number of indices
  unsigned short ixtdsiz; // index file ext size
  short ifilmod;          // index file mode
  pIIDX ix;               // index information
  const char *rfstfld;    // r-tree 1st fld name
  const char *rlstfld;    // r-tree last fld name
  short tfilno;           // temporary file number
} IFIL, *pIFIL;

#ifdef CTREE_CPP
#  define DECLARE
#else // CTREE_CPP
#  define DECLARE extern
#endif // CTREE_CPP

DECLARE short (__cdecl * InitISAM)(short bufs, short fils, short sect);
DECLARE short (__cdecl * CloseISAM)();
DECLARE short (__cdecl * OpenIFile)(pIFIL ifilptr);
DECLARE short (__cdecl * CloseIFile)(pIFIL ifilptr);
DECLARE short (__cdecl * GetRecord)(short keyno, void *target, void *recptr);
DECLARE short (__cdecl * GetVRecord)(short keyno, void *target, void *recptr, unsigned long *plen);
DECLARE long  (__cdecl * VRecordLength)(short datno);
DECLARE short (__cdecl * ReReadVRecord)(short datno, void *recptr, unsigned long bufsiz);
DECLARE short (__cdecl * FirstInVSet)(short keyno, void *target, void *recptr, short siglen, unsigned long *plen);
DECLARE short (__cdecl * NextInVSet)(short keyno, void *target, unsigned long *plen);
DECLARE short (__cdecl * FirstInSet)(short keyno, void *target, void *recptr, short siglen);
DECLARE short (__cdecl * NextInSet)(short keyno, void *target);
DECLARE long  (__cdecl * FirstKey)(unsigned long keyno, void * idxval);
DECLARE long  (__cdecl * NextKey)(unsigned long keyno, void * idxval);

DECLARE int CTreeStd_LinkDll(const char *DllPath);
DECLARE int CTreeStd_UnlinkDll(void);

#endif // !_ADBD_CTREE_H
