/*
 * ADBD -- AO Database dumper.
 *
 * $Id: aodb.cc 1150 2010-05-13 12:30:54Z os $
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

#include <windows.h>
#include <stdio.h>
#include "aodb.h"
#include "ctree.h"

// Globals
static IFIL aodb_handle;

// Initialise AO Database
int
InitAODatabase(const char *adbd_dir, const char *ao_dir)
{
  IIDX aodb_index;
  ISEG aodb_seg[2];
  char aodb_path[MAX_PATH];
  char ctree_lib[MAX_PATH];
  int err = 0;

  wsprintf(ctree_lib, "%s%s", adbd_dir, "\\ctreestd.dll");
  wsprintf(aodb_path, "%s%s", ao_dir, "\\cd_image\\data\\db\\ResourceDatabase");

  fprintf(stderr, "dll: %s\n db: %s\n", ctree_lib, aodb_path);

  if (CTreeStd_LinkDll(ctree_lib))
    return AODB_ERR_NODLL;

  if (InitISAM(6,4,32))
    {
      CTreeStd_UnlinkDll();
      return AODB_ERR_NOINIT;
    }

  aodb_handle.pfilnam = aodb_path;
  aodb_handle.dfilno  = -1;
  aodb_handle.dreclen = 8;
  aodb_handle.dxtdsiz = 4096;
  aodb_handle.dfilmod = 6;
  aodb_handle.dnumidx = 1;
  aodb_handle.ixtdsiz = 4096;
  aodb_handle.ifilmod = 2;
  aodb_handle.ix      = &aodb_index;
  aodb_handle.rfstfld = "Type";
  aodb_handle.rlstfld = "Blob";
  aodb_handle.tfilno  = 0;

  aodb_index.ikeylen = 8;
  aodb_index.ikeytyp = 0;
  aodb_index.ikeydup = 0;
  aodb_index.inulkey = 0;
  aodb_index.iempchr = 0;
  aodb_index.inumseg = 2;
  aodb_index.seg     = aodb_seg;
  aodb_index.ridxnam = "ItemIdx";
  aodb_index.aidxnam = 0;
  aodb_index.altseq  = 0;
  aodb_index.pvbyte  = 0;

  aodb_seg[0].soffset = 0;
  aodb_seg[0].slength = 4;
  aodb_seg[0].segmode = 1;
  aodb_seg[1].soffset = 4;
  aodb_seg[1].slength = 4;
  aodb_seg[1].segmode = 1;

  if ((err = OpenIFile(&aodb_handle)) == 0)
    return AODB_ERR_NOERROR;

  fprintf(stderr, "opening database failed (%d)!\n", err);
  CloseISAM();
  CTreeStd_UnlinkDll();

  return AODB_ERR_NODB;
}

// Release the AO Database
void
ReleaseAODatabase(void)
{
  CloseIFile(&aodb_handle);
  CloseISAM();
  CTreeStd_UnlinkDll();
}

// Decode data, required for items/nanos from version <= 13.91
static void
decode_item_data(unsigned char *datap, uint32_t len)
{
  uint32_t i;
  uint64_t seed;

  seed   = *((int*)(datap + 4));
  datap += 0x18;

  for (i=0x18; i<len; i++)
    {
      seed *= 0x1012003;
      seed %= 0x4e512dc8fULL;
      *datap++ ^= (uint8_t)(seed);
    }
}

aodb_record_t *
dataset_init(uint32_t type, unsigned int decode)
{
  aodb_record_t *r = (aodb_record_t *)malloc(sizeof(aodb_record_t));
  r->first = 1;
  r->highid = type;
  r->decode = decode;
  return r;
}

void
dataset_free(aodb_record_t *r)
{
  free(r);
}

static inline unsigned int
uint32_endian_swap(unsigned int val)
{
  return ((val & 0xff000000) >> 24) |
         ((val & 0x00ff0000) >>  8) |
         ((val & 0x0000ff00) <<  8) |
         ((val & 0x000000ff) << 24) ;
}

int
dataset_get(aodb_record_t *r)
{
  if (r->first)
    {
      unsigned long type = uint32_endian_swap(r->highid);
      if (FirstInSet(aodb_handle.tfilno+1, &type, r->data, sizeof(type)) != 0)
        return 1;
      r->first = 0;
    }
  else if (NextInSet(aodb_handle.tfilno+1, r->data) != 0)
    {
      return 2;
    }

  r->datalen = VRecordLength(aodb_handle.tfilno);
  if (r->datalen < 1 || r->datalen > 0x7fffff)
    {
      fprintf(stderr, "Illegal data length (%lu).\n", r->datalen);
      return 3;
    }
  if (ReReadVRecord(aodb_handle.tfilno, r->data, r->datalen))
    {
      fprintf(stderr, "ReReadVRecord failed :(\n");
      return 4;
    }
  if (r->decode)
    {
      decode_item_data(r->data, r->datalen);
    }
  r->lowid = *((unsigned long *)r->data+1);
  return 0;
}
