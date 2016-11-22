TOOLCHAIN=~/toolchain
TOOLROOT=$(TOOLCHAIN)/bin/arm-linux-androideabi-
ifdef DEBUG
    CFLAGS += -DDEBUG
    LDFLAGS += -llog
endif

%: %.c
	$(TOOLROOT)gcc $(CFLAGS) -march=armv7-a -fPIE -pie $^ -o $@ $(LDFLAGS)
