/*
Copyright (C) 1999 by Toby Jones.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <stdio.h>
#include <bios.h>

const unsigned int sector_size = 512;
unsigned char sector[sector_size];

//---------------------------------------------------------------------------
int get_sector(int drive, int head, int track, int sector, void* data)
{
   int result;

   result = biosdisk(_DISK_RESET, drive, head, track, sector, 1, data);
   if((result >> 8) == 0)
   {
      result = biosdisk(_DISK_READ, drive, head, track, sector, 1, data);
   }

   return result >> 8;
}

//---------------------------------------------------------------------------
// On success, zero is returned.  On failure, an errorcode between 1-0x12.
static const char *file_name = "mbr.bin";
int main(int, char** argv)
{
   FILE* file;

   int result = get_sector(0x80, 0, 0, 1, sector);

   if(0 != result)
   {
      fprintf(stderr, "%s: error reading sector\n", argv[0]);
      result = 1;
   }
   else if((file = fopen(file_name, "wb")) == 0)
   {
      fprintf(stderr, "%s: error opening %s\n", argv[0], file_name);
      result = 1;
   }
   else
   {
      fwrite(sector, sector_size, 1, file);
      fclose(file);
      result = 0;
   }
   return result;
}

