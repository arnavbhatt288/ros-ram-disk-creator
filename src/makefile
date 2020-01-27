CC=gcc
CFLAGS=-I.
DEPS = install.h volume.h misc.h main_dlg.h ini_data.h fat.h fat32.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ramdisk-creator: main.o install.o volume.o main_dlg.o misc.o fat.o fat32.o
	windres -i resource.rc -o resource.o
	$(CC) -o ramdisk-creator.exe main.o install.o volume.o main_dlg.o misc.o fat.o fat32.o resource.o -s -lcomdlg32 -municode