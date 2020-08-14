/*
 * ADBD -- AO Database dumper.
 *
 * $Id: ctree.cc 1150 2010-05-13 12:30:54Z os $
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
#define CTREE_CPP
#include "ctree.h"

// Handle of the Ctree DLL.  NULL until it is loaded.
static HINSTANCE dll = NULL;

#define DYNLINK(ptr,name) \
  if ((addr = GetProcAddress(dll, name)) == NULL) \
    goto fail; \
  memcpy(&ptr, &addr, sizeof(void *))

// Perform dynamic linking of ctreestd.dll functions
int CTreeStd_LinkDll(const char *DllPath)
{
  FARPROC addr;

  if (dll)
    return 0;

  // Load the dll file into memory
  dll = LoadLibrary(DllPath);
  if (dll == NULL)
    return 1;

  // Look up functions in the dll and set their pointers
  // On failure, jump to fail.
  DYNLINK(InitISAM,      "INTISAM");
  DYNLINK(CloseISAM,     "CLISAM");
  DYNLINK(OpenIFile,     "OPNIFIL");
  DYNLINK(CloseIFile,    "CLIFIL");
  DYNLINK(GetRecord,     "EQLREC");
  DYNLINK(GetVRecord,    "EQLVREC");
  DYNLINK(VRecordLength, "GETVLEN");
  DYNLINK(ReReadVRecord, "REDVREC");
  DYNLINK(NextInVSet,    "NXTVSET");
  DYNLINK(FirstInVSet,   "FRSVSET");
  DYNLINK(NextInSet,     "NXTSET");
  DYNLINK(FirstInSet,    "FRSSET");
  DYNLINK(FirstKey,      "FRSKEY");
  DYNLINK(NextKey,       "NXTKEY");

  return 0;

fail:
  // Function name not found in the dynamic link table by
  // GetProcAddress.  Unload the dll and signal failure.
  FreeLibrary(dll);
  dll = NULL;

  return 2;
}


// Unload ctreestd.dll
int CTreeStd_UnlinkDll(void)
{
  if (!dll)
    return 1;
  FreeLibrary(dll);
  dll = NULL;
  return 0;
}
