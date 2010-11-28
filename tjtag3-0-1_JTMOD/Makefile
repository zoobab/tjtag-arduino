CFLAGS += -Wall -O2 

WRT54GMEMOBJS := tjtag.o jt_mods.o

all: tjtag

tjtag : $(WRT54GMEMOBJS)
	gcc $(CFLAGS) -o $@ $(WRT54GMEMOBJS)

test: jt_mods.o test.o
	gcc $(CFLAGS) -o $@ jt_mods.o test.o

clean:
	rm -rf *.o tjtag test
