# Disable Amazon Ads (Using Exploit CVE-2016-5195)

Androids next Dec/Nov security update removes the exploit used here. I wanted to push out this software to as many people as possible before the next security update/OTAs arrived for the device.

This git repo is half of the story, but I checked in some of my files for historical purposes.
The other half of the story is the app I developed to do disable the ads without using the shell context
TODO upload that source. 

```
# edit the Makefile to point to your NDK toolchain or use Android.mk
$ make run-as
$ make dirtycow
$ adb push run-as /data/local/tmp
$ adb push dirtycow /data/local/tmp
$ adb shell
$ /data/local/tmp/dirtycow /system/bin/run-as /data/local/tmp/run-as
$ run-as
Package com.amazon.phoenix new state: disabled

$ echo now remove the wallpaper images and intent left behind by com.amazon.phoenix
$ adb shell
$ cp /system/bin/app_process32 /data/local/tmp/app_process32old
$ /data/local/tmp/dirtycow /system/bin/app_process32 /data/local/tmp/appfix
$ sleep 5
$ /data/local/tmp/dirtycow /system/bin/app_process32 /data/local/tmp/app_process32old
$ /data/local/tmp/dirtycow /system/bin/run-as /data/local/tmp/shell
$ run-as
> rm /data/data/com.android.systemui/files/boot.ad
> rm /data/data/com.android.systemui/files/boot.ad.image
```


