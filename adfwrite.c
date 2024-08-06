/* Code heavily inspired by Track_Copy.c from Amiga Devices Manual */

#include "adf.h"
#include "track.h"

#include <clib/alib_protos.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

char* g_program = 0;
static uint16_t g_verify = 0;

static int
disk_copy(const char* src, int unit, int fd, struct IOExtTD *diskreq0)
{
  int success = FALSE;  
  uint8_t *buffer = 0;
  uint8_t *verify = 0;  
  int16_t track;
  int16_t adfFail = FALSE;  
  int16_t numTracks;
  int16_t retryCount = 0;  
  
  if ((buffer = AllocMem(TRACK_SIZE, MEMF_CHIP|MEMF_PUBLIC)))  {
    if ((verify = AllocMem(TRACK_SIZE, MEMF_CHIP|MEMF_PUBLIC)))  {    
      track_motorOn(diskreq0);
      success = TRUE;    
      numTracks = track_findNumTracks(diskreq0);

      for (track=0; (track < numTracks) && success; track++) {
	if ((success = (read(fd, buffer, TRACK_SIZE)) == TRACK_SIZE)) {
	retry:
	  if (retryCount < 3) {
	    success = g_verify || track_write(diskreq0, buffer, track);
	    if (success) {
	      success = track_read(diskreq0, verify, track);
	      if (success) {
		success = memcmp(buffer, verify, TRACK_SIZE) == 0;
	      }
	    }
	    if (!success && !g_verify) {
	      retryCount++;
	      goto retry;
	    }
	  }
	} else {
	  adfFail = TRUE;
	}

#ifdef SQUIRTD_OUTPUT
	printf("\n");
#else
	printf("\r");
#endif
	printf("%s %c%c DF%d:%02d %c ", src, success ? '=' : '!', g_verify ? '=' : '>', unit, (track+2)>>1, track & 1 ? '\\' : '/');

	if (retryCount) {
	  printf(" %d retries", retryCount);
	}
	
	if (adfFail) {
	  printf(" *** failed to read track from ADF***\n\n");	  	  
	} else if (g_verify && !success) {
	  printf(" *** FAILED TO VERIFY *** \n\n");
	}

	fflush(stdout);	
      }
            
      if (success) {
	printf("\n\n%s %d tracks (%d bytes)\n\n", g_verify ? "verified" : "wrote", numTracks>>1, TRACK_SIZE*numTracks);
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
main_writeADF(struct IOExtTD *diskreq0, const char* src, uint32_t unit)
{
  uint8_t drive[] = "DFx:";
  int fd;
  int success = FALSE;
  
  drive[2] = unit + '0';
  
  if (!OpenDevice((void*)TD_NAME, unit, (struct IORequest *)diskreq0, 0L)) {
    if ((fd = open(src, O_RDONLY)) >= 0) {
      track_diskBusy(drive, TRUE);
      disk_copy(src, unit, fd, diskreq0);
      close(fd);
      success = TRUE;
    } else {
      printf("%s: failed to open %s\n", g_program, src);
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

  if (argc >= 3) {  
    if (strnicmp(argv[2],"df",2) == 0) {
      if (argv[2][2] >= '0' && argv[2][2] <= '3' && (argv[2][3] == '\0' || (argv[2][3] == ':' && argv[2][4] == '\0'))) {
	result = argv[2][2] - 0x30;
      }
    }
  }

  if (argc == 4) {
    g_verify = strcmp(argv[3], "verify") == 0;
  }

  if (result < 0) {
    printf("usage: %s filename.adf drive (df0 - df3) [verify]\n", g_program);       
  }
  
  return result;
}
 

int
main(int argc, char **argv)
{
  struct IOExtTD *diskreq0;
  struct MsgPort *diskPort;
  uint16_t fail = 0;
  int32_t unit;

  g_program = argv[0];

  if (!g_program) {
    printf("invalid execution context\n");
  } else {
    if ((unit = main_parseArgs(argc, argv)) >= 0) {
      if ((diskPort = CreatePort(NULL, 0))) {
	if ((diskreq0 = (struct IOExtTD *)CreateExtIO(diskPort, sizeof(struct IOExtTD)))) {
	  main_writeADF(diskreq0, argv[1], unit);
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
