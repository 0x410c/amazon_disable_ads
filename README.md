# Disable Amazon Ads (Using Exploit CVE-2016-5195)

Code got way out of hand, because I was under the pump for Androids next security update to remove dirty cow exploit.
There is a small chance that future OTA's will not renable the ad application after it is disabled using this exploit.

This git repo is half of the story, but I checked in some of my files historical purposes.
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
$ /data/local/tmp/dirtycow /system/bin/run-as /data/local/tmp/shell
$ run-as
> rm /data/data/com.android.systemui/files/boot.ad
> rm /data/data/com.android.systemui/files/boot.ad
```


