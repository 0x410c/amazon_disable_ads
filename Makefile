TOOLCHAIN=~/toolchain
TOOLROOT=$(TOOLCHAIN)/bin/arm-linux-androideabi-
LDFLAGS += -L$(TOOLCHAIN)/sysroot/usr/lib 
CFLAGS += -I$(TOOLCHAIN)/sysroot/usr/include
ifdef DEBUG
    LDFLAGS += -llog
endif

%: %.c
	$(TOOLROOT)gcc -march=armv7-a -fPIE -pie $^ -o $@
