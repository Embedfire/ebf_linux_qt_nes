CC = arm-linux-gnueabihf-gcc
TARBALL = InfoNES08J

# InfoNES
.CFILES =	./../K6502.cpp \
			./../InfoNES.cpp \
			./../InfoNES_Mapper.cpp \
			./../InfoNES_pAPU.cpp \
			./InfoNES_System_Linux.cpp \
			joypad.cpp \
			keyboard.cpp \
			gamedev.cpp \
			usb_dev.cpp


.OFILES	=	$(.CFILES:.cpp=.o) 

CCFLAGS =  -O2 -fsigned-char 
LDFILGS = -lstdc++		# gcc3.x.x

all: InfoNES

InfoNES: $(.OFILES)
	${CC} -o InfoNes -I /usr/local/libnes-arm/libasound-arm/include ${LDFILGS} ${.OFILES} -lm -lpthread -L /usr/local/libnes-arm/libzlib-arm/lib -lz -L /usr/local/libnes-arm/libasound-arm/lib -lasound

.cpp.o:
	$(CC) $(INCLUDES) -c $(CCFLAGS) $*.cpp  -o $@ -I /usr/local/libnes-arm/libasound-arm/include

clean:
	rm -f $(.OFILES) ../*~ ../*/*~ core

cleanall:
	rm -f $(.OFILES) ../*~ ../*/*~ core InfoNES

release: clean all

tar:
	( cd ..; \
	tar cvf $(TARBALL).tar ./*; \
	gzip $(TARBALL).tar \
	)

install:
	install ./InfoNES /usr/local/bin
