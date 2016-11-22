#include <unistd.h>
#include <stdio.h>
#include <dlfcn.h>
#include <sys/capability.h>

#ifdef DEBUG
#include <android/log.h>
#define LOG_TAG "removeads"
#define ALOG(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__); printf(__VA_ARGS__); fflush(stdout);
#else
#define ALOG(...) ;
#endif

#define CONTEXT_SYS "u:r:shell:s0"

int set_context_new(const char* cont) {
    typedef int (*_setcon_ptr)(const char* context);
    typedef int (*_getcon_ptr)(char** context);
    ALOG("Read selinux.so");
    void *handle = dlopen("libselinux.so",RTLD_LAZY);
    ALOG("Done.");
    if (handle)
    {
        _getcon_ptr _getcon = dlsym(handle, "getcon");
        _setcon_ptr _setcon = dlsym(handle, "setcon");
        char *conn = 0;
        if (_getcon) {
            int ret = _getcon(&conn);
            if (ret) {
                ALOG("Failed getting context: %d\n",ret);
                return 2;
            } else  {
                ALOG("Current context: %s\n",conn);
            }
        }



        if (_setcon)
        {
            int ret = _setcon(cont);
            if (ret) {
                ALOG("setcon error... %d\n", ret);
                return 2;
            } else {
                ALOG("setcon success!\n");
            }
        }
        else {
            ALOG("setcon() not found\n");
            return 2;
        }
        if (conn) {
            ALOG("Old Context WAS: %s",conn);
        }
        char *connNew = 0;
        if (_getcon) {
            int ret = _getcon(&connNew);
            if (ret) {
                ALOG("Failed getting context: %d\n",ret);
                return 2;
            } else  {
                ALOG("Current context: %s\n",connNew);
            }
        }
        if (connNew) {
            ALOG("New Context  IS: %s",connNew);
        }

    }
    else {
        ALOG("libselinux.so not found\n");
        return 2;
    }

    return 0;
}

int main(int argc, char **argv)
{
  struct __user_cap_header_struct capheader;
  struct __user_cap_data_struct capdata[2];

  ALOG("running as uid %d\n", getuid());

  memset(&capheader, 0, sizeof(capheader));
  memset(&capdata, 0, sizeof(capdata));
  capheader.version = _LINUX_CAPABILITY_VERSION_3;
  capdata[CAP_TO_INDEX(CAP_SETUID)].effective |= CAP_TO_MASK(CAP_SETUID);
  capdata[CAP_TO_INDEX(CAP_SETGID)].effective |= CAP_TO_MASK(CAP_SETGID);
  capdata[CAP_TO_INDEX(CAP_SETUID)].permitted |= CAP_TO_MASK(CAP_SETUID);
  capdata[CAP_TO_INDEX(CAP_SETGID)].permitted |= CAP_TO_MASK(CAP_SETGID);
  if (capset(&capheader, &capdata[0]) < 0) {
    ALOG("Could not set capabilities: %s\n", strerror(errno));
  }

  if(setresgid(0,0,0) || setresuid(0,0,0)) {
    ALOG("setresgid/setresuid failed\n");
  }
  ALOG("uid %d\n", getuid());

  if(set_context_new(CONTEXT_SYS)) {
      ALOG("FAILURE\n");
      return 2;
  } 
  ALOG("SUCCESS\n");

#ifdef DEBUG
  system("id");
#endif
  system("pm disable com.amazon.phoenix");
  return 0;
}
