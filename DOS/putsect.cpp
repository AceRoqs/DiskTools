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
int put_sector(int drive, int head, int track, int sector, void* data)
{
   int result;

   result = biosdisk(_DISK_RESET, drive, head, track, sector, 1, data);
   if((result >> 8) == 0)
   {
      result = biosdisk(_DISK_WRITE, drive, head, track, sector, 1, data);
   }

   return result >> 8;
}

//---------------------------------------------------------------------------
// PutSect - Load first sector_size bytes of a file onto a disk sector.
// On success, zero is returned.  On failure, an errorcode between 1-0x12.
#include <stdlib.h>
int main(int argc, char** argv)
{
   int result;
   FILE* file;

   if(6 != argc)
   {
      printf("Usage: %s drive head track sector filename\n", argv[0]);
      printf("To write MBR: %s 128 0 0 1 mbr.bin\n", argv[0]);
      printf("To write boot sector: %s 128 1 0 1 bs.bin\n", argv[0]);
      return 0;
   }

   if((file = fopen(argv[5], "rb")) == 0)
   {
      fprintf(stderr, "%s: error opening %s\n", argv[0], argv[5]);
      return 0x12;
   }

   size_t bytes = fread(sector, sector_size, 1, file);
   fclose(file);

   if(sector_size != bytes)
   {
      fprintf(stderr, "%s: error reading %s\n", argv[0], argv[5]);
      result = 0x12;
   }
   else if((result = put_sector(atoi(argv[1]),
                                atoi(argv[2]),
                                atoi(argv[3]),
                                atoi(argv[4]),
                                sector)) != 0)
   {
      fprintf(stderr, "%s: error writing sector\n", argv[0]);
   }

   return result;
}

