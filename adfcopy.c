/* Code heavily inspired by Track_Copy.c from Amiga Devices Manual */

#include "adf.h"
#include "track.h"

#include <clib/alib_protos.h>
#include <stdio.h>

char* g_program = 0;

static int
disk_copy(int srcUnit, int destUnit, struct IOExtTD *diskreq0, struct IOExtTD *diskreq1)
{
  int success = FALSE;  
  uint8_t *buffer = 0;
  uint8_t *verify = 0;  
  int16_t track;
  int16_t readFail = FALSE;
  int16_t writeFail = FALSE;  
  int16_t numTracks;
  
  if ((buffer = AllocMem(TRACK_SIZE, MEMF_CHIP|MEMF_PUBLIC)))  {
    if ((verify = AllocMem(TRACK_SIZE, MEMF_CHIP|MEMF_PUBLIC)))  {    
      track_motorOn(diskreq0);
      track_motorOn(diskreq1);      
      numTracks = track_findNumTracks(diskreq0);
      if (track_findNumTracks(diskreq1) != numTracks) {
	printf("%s: incompatible src and dest disks\n", g_program);
	return success;
      }
      success = TRUE;    	  
      for (track=0; (track < numTracks) && success; track++) {
	success = track_read(diskreq0, buffer, track);
	if (success) {
	  success = track_write(diskreq1, buffer, track);
	  if (!success) {
	    writeFail = TRUE;
	  }
	} else {
	  readFail = TRUE;
	}

#ifdef SQUIRTD_OUTPUT
	printf("\n");
#else
	printf("\r");
#endif
	printf("DF%d:%02d %c %c%c DF%d:%02d %c",
	       srcUnit, (track+2)>>1, track & 1 ? '\\' : '/',
	       success ? '=' : '!', '>',
	       destUnit, (track+2)>>1, track & 1 ? '\\' : '/');

	
	if (readFail) {
	  printf(" *** failed to read track ***\n\n");
	}


	if (writeFail) {
	  printf(" *** failed to read track ***\n\n");
	}
	
	fflush(stdout);	
      }
            
      if (success) {
	printf("\n\ncopied %d tracks (%d bytes)\n\n", numTracks>>1, TRACK_SIZE*numTracks);
      }
      
      track_motorOff(diskreq0);
      track_motorOff(diskreq1);      
    } else {
      printf("%s: failed to allocate %d bytes of chip ram\n", g_program, TRACK_SIZE);
    }
  } else {
    printf("%s: failed to allocate %d bytes of chip ram\n", g_program, TRACK_SIZE);
  }

  if (buffer) {
    FreeMem(buffer, TRACK_SIZE);
  }

  if (verify) {
    FreeMem(verify, TRACK_SIZE);
  }
  
  return success;
}


static int
main_copyADF(struct IOExtTD *diskreq0, struct IOExtTD *diskreq1, uint32_t srcUnit, uint32_t destUnit)
{
  uint8_t drive[] = "DFx:";
  int success = FALSE;  
  
  if (!OpenDevice((void*)TD_NAME, srcUnit, (struct IORequest *)diskreq0, 0L)) {
    drive[2] = srcUnit + '0';        
    track_diskBusy(drive, TRUE);
    if (!OpenDevice((void*)TD_NAME, destUnit, (struct IORequest *)diskreq1, 0L)) {
      drive[2] = destUnit + '0';
      track_diskBusy(drive, TRUE);      
      disk_copy(srcUnit, destUnit, diskreq0, diskreq1);
      track_diskBusy(drive, FALSE);
      CloseDevice((struct IORequest *)diskreq1);
      success = TRUE;
    } else {
      printf("%s: failed to open %s\n", g_program, drive);      
    }
    drive[2] = srcUnit + '0';        
    track_diskBusy(drive, FALSE);
    CloseDevice((struct IORequest *)diskreq0);
  } else {
    printf("%s: failed to open %s\n", g_program, drive);
  }

  return success;
}


static int16_t
main_parseArgs(int argc, char **argv, uint32_t* srcUnit, uint32_t* destUnit)
{
  int16_t result = 0;

  if (argc == 3) {  
    if (strnicmp(argv[1],"df",2) == 0) {
      if (argv[1][2] >= '0' && argv[1][2] <= '3' && (argv[1][3] == '\0' || (argv[1][3] == ':' && argv[1][4] == '\0'))) {
	*srcUnit = argv[1][2] - 0x30;
	result++;
      } 
    }
    if (strnicmp(argv[2],"df",2) == 0) {
      if (argv[2][2] >= '0' && argv[2][2] <= '3' && (argv[2][3] == '\0' || (argv[2][3] == ':' && argv[2][4] == '\0'))) {
	*destUnit = argv[2][2] - 0x30;
	result++;	
      }
    }    
  }


  if (result != 2) {
    printf("usage: %s src_drive (df0 - df3) dest_drive (df0 - df3)\n", g_program);       
  }
  
  return result == 2;
}
 

int
main(int argc, char **argv)
{
  struct IOExtTD *diskreq0;
  struct IOExtTD *diskreq1;  
  struct MsgPort *diskPort;
  uint32_t srcUnit = 0, destUnit = 1;
  uint32_t fail = 0;
  
  g_program = argv[0];

  if (!g_program) {
    printf("invalid execution context\n");
  } else {
    if (main_parseArgs(argc, argv, &srcUnit, &destUnit)) {
      if ((diskPort = CreatePort(NULL, 0))) {
	if ((diskreq0 = (struct IOExtTD *)CreateExtIO(diskPort, sizeof(struct IOExtTD)))) {
	  if ((diskreq1 = (struct IOExtTD *)CreateExtIO(diskPort, sizeof(struct IOExtTD)))) {	  
	    main_copyADF(diskreq0, diskreq1, srcUnit, destUnit);
	    DeleteExtIO((struct IORequest *)diskreq1);	    
	  } else {
	    fail = TRUE;
	  }
	  DeleteExtIO((struct IORequest *)diskreq0);	  
	} else  {
	  fail = TRUE;
	}
	DeletePort(diskPort);	
      } else {
	fail = TRUE;
      }
    }

    if (fail) {
      printf("%s: failed to allocate resources\n", g_program);
    }
  }

  return 0;
}
