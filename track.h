/* Code heavily inspired by Track_Copy.c from Amiga Devices Manual */

#pragma once

#include <exec/types.h>
#include <devices/trackdisk.h>
#include <clib/exec_protos.h>

#define TRACK_SIZE      ((int32_t)(NUMSECS * TD_SECTOR))

void
track_motorOff(struct IOExtTD *disk);

void
track_motorOn(struct IOExtTD *disk);

int16_t
track_read(struct IOExtTD *disk, uint8_t *buffer, int16_t track);

int16_t
track_write(struct IOExtTD *disk, uint8_t *buffer, int16_t track);

int16_t
track_findNumTracks(struct IOExtTD *disk);

void
track_diskBusy(uint8_t *drive, int32_t onflag);
