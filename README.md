Write/Read/Copy ADF files to Amiga Floppies
===========================================

/* Code heavily inspired by Track_Copy.c from Amiga Devices Manual */

Note: These are Amiga programs designed to run on Amiga computers.

adw
---
```
usage: adw filename.adf drive (df0..df3) [verify]
```
Write the file _filename.adf_ to the floppy drive _drive_. Writes are verified as written. If the optional _verify_ argument is supplied, only the verification step is performed.

adr
---
```
usage: adr drive (df0..df3) filename.adf
```
Create a ADF image of the the floppy drive _drive_ and save it to the file _filename.adf_.

adc
---
```
usage: adc src_drive (df0..df3) dest_drive (df0..df3)
```
Duplicate the data on disk _src_drive_ to _dest_drive_. Data is verified as written.
