



CC=gcc.exe

# -fomit-frame-pointer 

CFLAGS = -Iinclude -Werror -Wall -DIPPROTOCOL=0 -D_WIN32_WINNT=0x0500 -D__DEBUG__=0 -D_WIN32_IE=0x501 -D__WIN32__=1 -mmmx -std=gnu99 -march=i686 -mtune=generic -fgnu89-inline -funroll-loops -ffast-math -finline-functions -fomit-frame-pointer 
LIBS = -L"lib" 


all : wvs.exe


# lib\liblglcd.a 

wvs.exe: wa.c 
	$(CC) -c wa.c -o wvs.o $(CFLAGS)
	$(CC) wvs.o -o wvs.exe $(LIBS) -mwindows -lm lib\libmylcd.a -liphlpapi -lwininet -lwsock32 -lws2_32 -lwinmm -lpsapi -lhid -lsetupapi lib\libhook.a
	strip.exe wvs.exe

%.exe: %.o
	$(CC) $(LIBS) -o $@ $^
	strip.exe $@

%.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)


clean :
	rm -f *.exe *.o *.bak


### PHONY define
.PHONY: all all-before all-after clean clean-custom



