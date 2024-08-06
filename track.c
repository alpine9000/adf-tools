/* Code heavily inspired by Track_Copy.c from Amiga Devices Manual */

#include "adf.h"
#include "track.h"
#include <stdio.h>
#include <dos/dosextens.h>
#include <clib/dos_protos.h>

void
track_diskBusy(uint8_t *drive, int32_t onflag)
{
  struct StandardPacket *pk;
  struct Process *tsk;
  
  tsk = (struct Process *)FindTask(NULL);
  if ((pk = AllocMem(sizeof(struct StandardPacket), MEMF_PUBLIC|MEMF_CLEAR))) {
    pk->sp_Msg.mn_Node.ln_Name = (void *)&(pk->sp_Pkt);    
    pk->sp_Pkt.dp_Link =& (pk->sp_Msg);
    pk->sp_Pkt.dp_Port =& (tsk->pr_MsgPort);
    pk->sp_Pkt.dp_Type = ACTION_INHIBIT;
    pk->sp_Pkt.dp_Arg1 = (onflag ? -1L : 0L);
    
    PutMsg(DeviceProc(drive), (struct Message *)pk);
    WaitPort(&(tsk->pr_MsgPort));
    GetMsg(&(tsk->pr_MsgPort));
    FreeMem(pk,(long)sizeof(*pk));
  }
}


void
track_motorOff(struct IOExtTD *disk)
{
  disk->iotd_Req.io_Length = 0;
  disk->iotd_Req.io_Command = TD_MOTOR;

  DoIO((struct IORequest *)disk);
}


void
track_motorOn(struct IOExtTD *disk)
{
  disk->iotd_Req.io_Length = 1;
  disk->iotd_Req.io_Command = TD_MOTOR;

  DoIO((struct IORequest *)disk);
}


int16_t
track_read(struct IOExtTD *disk, uint8_t *buffer, int16_t track)
{
  int16_t success = TRUE;
  
  disk->iotd_Req.io_Length = TRACK_SIZE;
  disk->iotd_Req.io_Data = (APTR)buffer;
  disk->iotd_Req.io_Command = CMD_READ;
  disk->iotd_Req.io_Offset = (uint32_t)(TRACK_SIZE * track);

  DoIO((struct IORequest *)disk);

  if (disk->iotd_Req.io_Error) {
    success = FALSE;
    printf("\n%s: failed reading track %d (error:%d)\n", g_program, track, disk->iotd_Req.io_Error);
  }

  return(success);
}


int16_t
track_write(struct IOExtTD *disk, uint8_t *buffer, int16_t track)
{
  int16_t success = TRUE;
  
  disk->iotd_Req.io_Length = TRACK_SIZE;
  disk->iotd_Req.io_Data = (APTR)buffer;
  disk->iotd_Req.io_Command = TD_FORMAT;
  disk->iotd_Req.io_Offset = (uint32_t)(TRACK_SIZE * track);

  DoIO((struct IORequest *)disk);

  if (disk->iotd_Req.io_Error) {
    success = FALSE;
    printf("\n%s: failed writing track %d (error:%d)\n", g_program, track, disk->iotd_Req.io_Error);
  }
  return(success);
}


int16_t
track_findNumTracks(struct IOExtTD *disk)
{
  disk->iotd_Req.io_Command = TD_GETNUMTRACKS;

  DoIO((struct IORequest *)disk);

  return ((int16_t)disk->iotd_Req.io_Actual);
}

