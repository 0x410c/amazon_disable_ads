
all: build

build:
	ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.mk

push: build
	adb shell ls /data/local/tmp
	adb push libs/armeabi/getroot /data/local/tmp/getroot

clean:
	rm -rf libs
	rm -rf obj

