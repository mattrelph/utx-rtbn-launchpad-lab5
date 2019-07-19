// eFile.c
// Runs on either TM4C123 or MSP432
// High-level implementation of the file system implementation.
// Daniel and Jonathan Valvano
// August 29, 2016
#include <stdint.h>
#include "eDisk.h"

uint8_t Buff[512]; // temporary buffer used during file I/O
uint8_t Directory[256], FAT[256];
int32_t bDirectoryLoaded =0; // 0 means disk on ROM is complete, 1 means RAM version active

// Return the larger of two integers.
int16_t max(int16_t a, int16_t b){
  if(a > b){
    return a;
  }
  return b;
}
//*****MountDirectory******
// if directory and FAT are not loaded in RAM,
// bring it into RAM from disk
void MountDirectory(void){ 
// if bDirectoryLoaded is 0, 
//    read disk sector 255 and populate Directory and FAT
//    set bDirectoryLoaded=1
// if bDirectoryLoaded is 1, simply return
// **write this function**
	if (bDirectoryLoaded == 0)
	{
		eDisk_ReadSector(Buff, 255);	//Load directory
		for (int i =0; i<256; i++)
		{
			Directory[i] = Buff[i];
			FAT[i] = Buff[i+256]; 
			//Directory[i] = 255;
			//FAT[i] = 255; 
			
		}
		bDirectoryLoaded=1;
	}
	return;
}
		

// Return the index of the last sector in the file
// associated with a given starting sector.
// Note: This function will loop forever without returning
// if the file has no end (i.e. the FAT is corrupted).
uint8_t lastsector(uint8_t start){
// **write this function**
	MountDirectory();
	uint8_t count = 0;
	uint8_t current = start;
	
	while (FAT[current]!=255)
	{
		count++;	//Count keeps track of how many sectors we are scrolling through.
		current = FAT[current];
	}
	
  return current; // Returns last sector
}

// Return the index of the first free sector.
// Note: This function will loop forever without returning
// if a file has no end or if (Directory[255] != 255)
// (i.e. the FAT is corrupted).
uint8_t findfreesector(void){
// **write this function**
	MountDirectory();

	uint8_t current = 0;
	int8_t fs = -1;
	uint8_t ls = lastsector(Directory[current]);
	while ((ls != 255))
	{
		fs = max(ls, fs);
		current++;
		ls = lastsector(Directory[current]);
	}
  
  return (fs+1); // Returns last sector found (Hopefully free)
}

// Append a sector index 'n' at the end of file 'num'.
// This helper function is part of OS_File_Append(), which
// should have already verified that there is free space,
// so it always returns 0 (successful).
// Note: This function will loop forever without returning
// if the file has no end (i.e. the FAT is corrupted).
uint8_t appendfat(uint8_t num, uint8_t n){
// **write this function**
	MountDirectory();
	uint8_t i = Directory[num];
	if (i == 255)	//If the directory number =255, we assign the file the first free sector (n)
	{
		Directory[num] = n;
		return 0;
	}
	//uint8_t m = FAT[i];
	while (1)	//Hunt for the tail of the file
	{
		uint8_t m = FAT[i]; //m is now the new next location
		if (m == 255)	//If we get here, we have gotten to the old last sector of the file. We will now add a new sector.
		{
			FAT[i] = n;	//Assign the first free sector to the tail of the file
			return 0;
		}			
		else
		{
			i=m;
		}		
	}
}

//********OS_File_New*************
// Returns a file number of a new file for writing
// Inputs: none
// Outputs: number of a new file
// Errors: return 255 on failure or disk full
uint8_t OS_File_New(void){
// **write this function**
 MountDirectory(); 
	uint8_t i = 0;
	while (Directory[i] != 255)
	{
		i++;
		if (i == 255)
		{
			return 255;			
		}
	}
	return i;

}

//********OS_File_Size*************
// Check the size of this file
// Inputs:  num, 8-bit file number, 0 to 254
// Outputs: 0 if empty, otherwise the number of sectors
// Errors:  none
uint8_t OS_File_Size(uint8_t num){
// **write this function**
	MountDirectory();
	uint8_t count = 0;
	uint8_t current = Directory[num];
	if (current != 255)
	{
		count++;
		while (FAT[current]!=255)
		{
			count++;	//Count keeps track of how many sectors we are scrolling through.
			current = FAT[current];
		}
	}
  return count; // Returns file size
}

//********OS_File_Append*************
// Save 512 bytes into the file
// Inputs:  num, 8-bit file number, 0 to 254
//          buf, pointer to 512 bytes of data
// Outputs: 0 if successful
// Errors:  255 on failure or disk full
uint8_t OS_File_Append(uint8_t num, uint8_t buf[512]){
// **write this function**
  MountDirectory();
	uint8_t n = findfreesector();
	if (n == 255)
	{
		return 255;	//No space - Disk full
	}
	else
	{
		if (eDisk_WriteSector (buf, n) == RES_OK)
		{
			if (appendfat(num, n) == 0)
			{
				return 0; // Success!
			}
			else
			{
				return 255;
			}
		}
		else
		{
			return 255;
		}
		
	}

}

//********OS_File_Read*************
// Read 512 bytes from the file
// Inputs:  num, 8-bit file number, 0 to 254
//          location, logical address, 0 to 254
//          buf, pointer to 512 empty spaces in RAM
// Outputs: 0 if successful
// Errors:  255 on failure because no data
uint8_t OS_File_Read(uint8_t num, uint8_t location,
                     uint8_t buf[512]){
// **write this function**
	MountDirectory();
	//First we find the physical address of the sector
	uint8_t current = 0;
	uint8_t physical_location= Directory[num];	//Get first sector from the Directory
	while ((current < location)&&(FAT[physical_location]!=255))
	{
		if (FAT[physical_location] == 255)
		{
			return 255;
		}
		current++;
		//Now go through the FAT until we are in the desired physical sector
		physical_location = FAT[physical_location];
	}
	if(current < location)
	{
			return 255;		
	}
	//Now read from the disk
	if (eDisk_ReadSector(buf, physical_location) == RES_OK)
	{
		  return 0; // Success
	}
	else
	{
		return 255;	//Something went wrong
	}

}

//********OS_File_Flush*************
// Update working buffers onto the disk
// Power can be removed after calling flush
// Inputs:  none
// Outputs: 0 if success
// Errors:  255 on disk write failure
uint8_t OS_File_Flush(void){
// **write this function**
	MountDirectory();
	uint8_t buf[512];
	for (int i = 0; i < 256; i++)
	{
		buf[i]=Directory[i];
		buf[i+256] = FAT[i];
	}
	if (eDisk_WriteSector(buf, 255) == RES_OK)
	{
		  return 0;	//Success!
	}
	else
	{
		return 255; //Something went wrong
	}
}

//********OS_File_Format*************
// Erase all files and all data
// Inputs:  none
// Outputs: 0 if success
// Errors:  255 on disk write failure
uint8_t OS_File_Format(void){
// call eDiskFormat
// clear bDirectoryLoaded to zero
// **write this function**
	bDirectoryLoaded = 0;
	if (eDisk_Format()== RES_OK)
	{
		return 0; //Format success
	}
	else
	{
		return 255; //Something went wrong
	}
	

}
