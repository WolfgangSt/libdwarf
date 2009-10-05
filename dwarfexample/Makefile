

LIBDIR= -L../libdwarf
LIBS= -ldwarf -lelf
CFLAGS= -Wall -I../libdwarf

all: simplereader

simplereader: simplereader.c
	$(CC) $(CFLAGS) simplereader.c -o simplereader $(LIBDIR) $(LIBS)

clean:
	rm -f simplereader.o
	rm -f simplereader
