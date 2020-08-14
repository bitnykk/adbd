/*
 * ADBD -- AO Database dumper.
 *
 * $Id: aodb.h 1150 2010-05-13 12:30:54Z os $
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

#ifndef _ADBD_AODB_H
#define _ADBD_AODB_H

// Global constants
#define AODB_ERR_NOERROR        0
#define AODB_ERR_NODLL          1
#define AODB_ERR_NOINIT         2
#define AODB_ERR_NODB           3
#define AODB_TYP_ITEM           0xf4254
#define AODB_TYP_NANO           0xfde85
#define AODB_TYP_TEXTURE        0xf6954
#define AODB_TYP_ICON           0xf6958
#define AODB_TYP_JPEG           0xf6959

typedef unsigned __int64 uint64_t;
typedef unsigned int     uint32_t;
typedef unsigned short   uint16_t;
typedef unsigned char    uint8_t;

// Functions
int InitAODatabase(const char *adbd_dir, const char *ao_dir);
void ReleaseAODatabase(void);

typedef struct aodb_record
{
  unsigned int decode, first;
  unsigned long highid, lowid;
  unsigned long datalen;
  unsigned char data[0x7fffff];
} aodb_record_t;

aodb_record_t *dataset_init(uint32_t type, unsigned int decode);
void dataset_free(aodb_record_t *r);
int dataset_get(aodb_record_t *r);

#endif // !_ADBD_AODB_H
