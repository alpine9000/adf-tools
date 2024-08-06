/* Code heavily inspired by Track_Copy.c from Amiga Devices Manual */

#include "adf.h"
#include "track.h"

#include <clib/alib_protos.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

char* g_program = 0;

static int
disk_copy(const char* dest, int unit, int fd, struct IOExtTD *diskreq0)
{
  int success = FALSE;  
  uint8_t *buffer = 0;
  uint8_t *verify = 0;  
  int16_t track;
  int16_t adfFail = FALSE;
  int16_t trackFail = FALSE;    
  int16_t numTracks;
  
  if ((buffer = AllocMem(TRACK_SIZE, MEMF_CHIP|MEMF_PUBLIC)))  {
    if ((verify = AllocMem(TRACK_SIZE, MEMF_CHIP|MEMF_PUBLIC)))  {    
      track_motorOn(diskreq0);
      success = TRUE;    
      numTracks = track_findNumTracks(diskreq0);

      for (track=0; (track < numTracks) && success; track++) {
	success = track_read(diskreq0, buffer, track);

	if (success) {
	  if (!(success = ((write(fd, buffer, TRACK_SIZE) == TRACK_SIZE)))) {
	    adfFail = TRUE;
	  }
	} else {
	  trackFail = TRUE;
	}

#ifdef SQUIRTD_OUTPUT
	printf("\n");
#else
	printf("\r");
#endif
	printf("DF%d:%02d %c  %c%c %s", unit, (track+2)>>1, track & 1 ? '\\' : '/',  success ? '=' : '!', '>', dest);

	
	if (adfFail) {
	  printf(" *** failed to write track to ADF***\n\n");	  	  
	} else if (trackFail) {
	  printf(" *** failed to read track ***\n\n");
	}

	fflush(stdout);	
      }
            
      if (success) {
	printf("\n\nsaved %d tracks (%d bytes)\n\n", numTracks>>1, TRACK_SIZE*numTracks);
      }
      
      track_motorOff(diskreq0);
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
main_readADF(struct IOExtTD *diskreq0, const char* dest, uint32_t unit)
{
  uint8_t drive[] = "DFx:";
  int fd;
  int success = FALSE;
  
  drive[2] = unit + '0';
  
  if (!OpenDevice((void*)TD_NAME, unit, (struct IORequest *)diskreq0, 0L)) {
    if ((fd = open(dest, O_WRONLY|O_CREAT)) >= 0) {
      track_diskBusy(drive, TRUE);
      disk_copy(dest, unit, fd, diskreq0);
      close(fd);
      success = TRUE;
    } else {
      printf("%s: failed to open %s\n", g_program, dest);
    }
    track_diskBusy(drive, FALSE);
    CloseDevice((struct IORequest *)diskreq0);
  } else {
    printf("%s: failed to open %s\n", g_program, drive);
  }

  return success;
}


static int16_t
main_parseArgs(int argc, char **argv)
{
  int16_t result = -1;

  if (argc == 3) {  
    if (strnicmp(argv[1],"df",2) == 0) {
      if (argv[1][2] >= '0' && argv[1][2] <= '3' && (argv[1][3] == '\0' || (argv[1][3] == ':' && argv[1][4] == '\0'))) {
	result = argv[1][2] - 0x30;
      }
    }
  }


  if (result < 0) {
    printf("usage: %s drive (df0 - df3) filename.adf\n", g_program);       
  }
  
  return result;
}
 

int
main(int argc, char **argv)
{
  struct IOExtTD *diskreq0;
  struct MsgPort *diskPort;
  int32_t unit;
  uint16_t fail = 0;

  g_program = argv[0];

  if (!g_program) {
    printf("invalid execution context\n");
  } else {
    if ((unit = main_parseArgs(argc, argv)) >= 0) {
      if ((diskPort = CreatePort(NULL, 0))) {
	if ((diskreq0 = (struct IOExtTD *)CreateExtIO(diskPort, sizeof(struct IOExtTD)))) {
	  main_readADF(diskreq0, argv[2], unit);
	  DeleteExtIO((struct IORequest *)diskreq0);	  
	} else  {
	  fail = 1;
	}
	DeletePort(diskPort);	
      } else {
	fail = 1;
      }
    }

    if (fail) {
      printf("%s: failed to allocate resources\n", g_program);
    }
  }

  return 0;
}
