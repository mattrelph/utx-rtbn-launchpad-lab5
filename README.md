# utx-rtbn-launchpad-lab5
UTX Real Time Blue Tooth Networks - TM4C123 - Lab 5

* The goal of Lab 5 is to implement the write-once / read many file system 
  * Usage
    * The 128k flash memory is erased only once at the time of downloading the project;
    * The act of erasing the entire flash is equivalent to "formatting" the disk;
    * The disk is partitioned into 256 sectors of 512 bytes/sector;
    * We can append data to a file but cannot delete data or files;
    * We append data to a file in chunks of 512 bytes;
    * We will read data in a sequential fashion;
    * We assign file names as single 8-bit numbers (0 to 254);
    * We limit the file system to a maximum of 255 files;
    * We will mount (initialize the driver) the file system on startup;
    * We will call flush (backup to disk) the file system before powering down.
  * Allocation (Directory, FAT, Disk)
  * Append
  * Free Space Management
  
https://youtu.be/g9V5fuTLB7g
