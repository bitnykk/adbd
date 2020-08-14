/*
 * ADBD -- AO Database dumper.
 *
 * $Id: main.cc 1150 2010-05-13 12:30:54Z os $
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
#include <stdbool.h>
#include <io.h>
#include "aodb.h"

#define ADBD_VERSION   "0.3.1"

#define ADBD_DECODE    0x0001
#define ADBD_DUMPITEM  0x1000
#define ADBD_DUMPNANO  0x2000
#define ADBD_DUMPICON  0x4000
#define ADBD_DUMPTEXR  0x8000

static bool get_all_entries(unsigned long highid, FILE *fd, int flags);
static bool get_all_images(unsigned long highid, const char *suffix);

static const char strErrorNoDLL[]  = "Failed to load Ctreestd.dll";
static const char strErrorNoInit[] = "Failed to initialise Ctree driver";
static const char strErrorNoDB[]   = "Failed to open AO Database";

static void
show_usage(const char *cmd)
{
  fprintf(stderr, "ADBD version %s.\n", ADBD_VERSION);
  fprintf(stderr, "\n");
  fprintf(stderr, "Usage: %s [-d] <ao path> <type>\n", cmd);
  fprintf(stderr, "       -d   - decode items (for version <= 13.91 db)\n");
  fprintf(stderr, "       type - 'item' 'nano' 'icon' 'texture' or 'all'\n");
  fprintf(stderr, "\n");
  exit(-1);
}

int
main(int argc, char *argv[])
{
  const char *dumptype = NULL, *ao_dir = NULL;
  char *adbd_dir = NULL;
  char fn[MAX_PATH], ep[32], *p, *q;
  long  i, j,
        ret   = 0,
        flags = 0,
        argid = 0,
        dumps = 0,
        aover = 0;
  FILE *fp;

  if (argc != 3 && argc != 4)
    show_usage(argv[0]);

  argid = 1;
  if (strcmp(argv[argid], "-d") == 0)
    {
      if (argc != 4)
        show_usage(argv[0]);
      flags |= ADBD_DECODE;
      argid++;
    }
  else if (argc != 3)
    {
      show_usage(argv[0]);
    }

  adbd_dir = strdup(argv[0]);
  p = strrchr(adbd_dir, '/');
  if (p == NULL)
    p = strrchr(adbd_dir, '\\');
  if (p)
    *p = 0;

  ao_dir = argv[argid++];
  dumptype = argv[argid++];

  if (stricmp(dumptype, "item") == 0)
    {
      dumps |= ADBD_DUMPITEM;
    }
  else if (stricmp(dumptype, "nano") == 0)
    {
      dumps |= ADBD_DUMPNANO;
    }
  else if (stricmp(dumptype, "icon") == 0)
    {
      dumps |= ADBD_DUMPICON;
    }
  else if (stricmp(dumptype, "texture") == 0)
    {
      dumps |= ADBD_DUMPTEXR;
    }
  else if (stricmp(dumptype, "all") == 0)
    {
      dumps |= ADBD_DUMPITEM | ADBD_DUMPNANO | ADBD_DUMPICON | ADBD_DUMPTEXR;
    }

  if (dumps == 0)
    {
      fprintf(stderr, "%s: Nothing speicifed for dumping\n", argv[0]);
      exit(-1);
    }

  ret = InitAODatabase(adbd_dir, ao_dir);
  if (ret)
    {
      const char *err = NULL;

      switch(ret)
        {
        case AODB_ERR_NODLL:
          err = strErrorNoDLL;
          break;

        case AODB_ERR_NOINIT:
          err = strErrorNoInit;
          break;

        case AODB_ERR_NODB:
          err = strErrorNoDB;
          break;
        }

      fprintf(stderr, err);

      exit(-1);
    }

  snprintf(fn, sizeof(fn), "%s\\version.id", ao_dir);
  fp = fopen(fn, "r");
  fgets(fn, sizeof(fn), fp);
  fclose(fp);
  ep[0] = 0;
  if ((q = strstr(fn, "_EP")) != NULL)
    snprintf(ep, sizeof(ep), "ep%d", (int) atoi(q+3));
  for (i=0,p=fn,j=1000000; p!=NULL; i++, j/=100)
    {
      if ((q = strchr(p, '.')) != NULL)
        *q++ = 0;
      aover += atoi(p)*(j>0?j:1);
      p = q;
    }

  printf("patch: %ld\n", aover);

  if (dumps & ADBD_DUMPITEM)
    {
      fprintf(stderr, "Starting to dump items (%ld)...\n", flags);
      snprintf(fn, sizeof(fn), "items-%ld%s.dat", aover, ep);
      fp = fopen(fn, "wb");
      get_all_entries(AODB_TYP_ITEM, fp, flags);
      fclose(fp);
    }

  if (dumps & ADBD_DUMPNANO)
    {
      fprintf(stderr, "Starting to dump nanos (%ld)...\n", flags);
      snprintf(fn, sizeof(fn), "nanos-%ld%s.dat", aover, ep);
      fp = fopen(fn, "wb");
      get_all_entries(AODB_TYP_NANO, fp, flags);
      fclose(fp);
    }

  if (dumps & ADBD_DUMPICON)
    {
      fprintf(stderr, "Starting to dump icons (%ld)...\n", flags);
      get_all_images(AODB_TYP_ICON, "png");
      fprintf(stderr, "Done\n");
    }

  if (dumps & ADBD_DUMPTEXR)
    {
      fprintf(stderr, "Starting to dump textures (%ld)...\n", flags);
      get_all_images(AODB_TYP_JPEG, "jpeg");
      get_all_images(AODB_TYP_TEXTURE, "png");
      fprintf(stderr, "Done\n");
    }
  fprintf(stderr, "All done\n");

  ReleaseAODatabase();

  return 0;
}

static bool
get_all_entries(unsigned long highid, FILE *fp, int flags)
{
  unsigned long records=0;
  aodb_record_t *r=NULL;

  fprintf(stderr, "Reading records of type %lu...\n", highid);
  r = dataset_init(highid, flags & ADBD_DECODE);
  while(dataset_get(r) == 0)
    {
      records++;
      fprintf(stderr, "%lu ", r->lowid);
      fprintf(fp, "AOID: %lu LEN: %lu\n==========\n", r->lowid, r->datalen);
      fwrite(r->data, sizeof(BYTE), r->datalen, fp);
      fprintf(fp, "\n==========\n");
    }
  dataset_free(r);
  fprintf(stderr, "Done! %lu records.\n", records);

  return true;
}

static bool
get_all_images(unsigned long highid, const char *suffix)
{
  FILE *fp;
  char iconfile[25];
  unsigned long records=0;
  aodb_record_t *r=NULL;

  fprintf(stderr, "Reading records of type %lu...\n", highid);
  r = dataset_init(highid, 0);
  for (;;)
    {
      int res = dataset_get(r);
      if (res != 0 && res != 3 && res != 4)
        break;
      if (res != 0)
        continue;
      records++;
      fprintf(stderr, "%lu ", r->lowid);
      sprintf(iconfile, "%lu-%lu.%s", r->highid, r->lowid, suffix);
      fp = fopen(iconfile, "wb");
      fwrite(r->data + 0x18, sizeof(BYTE), r->datalen - 0x18, fp);
      fclose(fp);
    }
  dataset_free(r);
  fprintf(stderr, "Done! %lu records.\n", records);

  return false;
}
