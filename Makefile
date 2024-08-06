#SQUIRTD_OUTPUT=-DSQUIRTD_OUTPUT
COMMON_DEPS=Makefile
CC=/usr/local/amiga/bebbo/bin/m68k-amigaos-gcc $(SQUIRTD_OUTPUT)
CFLAGS=-Os -DAMIGA -noixemul -fomit-frame-pointer -Wall 
LDFLAGS=-s
LIBS=-lamiga
READ_SRCS=adfread.c track.c
WRITE_SRCS=adfwrite.c track.c
COPY_SRCS=adfcopy.c track.c
READ_OBJS=$(addprefix build/, $(READ_SRCS:.c=.o))
WRITE_OBJS=$(addprefix build/, $(WRITE_SRCS:.c=.o))
COPY_OBJS=$(addprefix build/, $(COPY_SRCS:.c=.o))

all: build/adw build/adr build/adc

build/adw: $(WRITE_OBJS)
build/adr: $(READ_OBJS)
build/adc: $(COPY_OBJS)

build/%.o: %.c $(HEADERS) $(COMMON_DEPS) 
	@mkdir -p build
	$(CC) -c $(CFLAGS) $*.c -o build/$*.o

build/adw: $(WRITE_OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(WRITE_OBJS) -o build/adw $(LIBS)

build/adr: $(READ_OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(READ_OBJS) -o build/adr $(LIBS)

build/adc: $(COPY_OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(COPY_OBJS) -o build/adc $(LIBS)

clean:
	rm -rf build

