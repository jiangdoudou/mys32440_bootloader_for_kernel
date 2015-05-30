# /*
# * Platform  for: s3c2440
# * author  by jiangdou
# * you  have anything,plese to QQ:344283973
# * time  at: 2009-0801
# *
# */



CC      = arm-linux-gcc
LD      = arm-linux-ld
AR      = arm-linux-ar
OBJCOPY = arm-linux-objcopy
OBJDUMP = arm-linux-objdump

CFLAGS 		:= -Wall -O2
CPPFLAGS   	:= -nostdinc -nostdlib -fno-builtin
LIB 	:= $(obj)liblcd.a
objs := start.o init.o main.o boot_zImage.o lcd_logo.o logo.o

u-boot.bin: $(objs)
	${LD} -Tboot.lds -o boot.elf $^
	${OBJCOPY} -O binary -S boot.elf $@
	${OBJDUMP} -D -m arm boot.elf > u_boot.dis

%.o:%.c
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o:%.S
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o *.bin *.elf *.dis


