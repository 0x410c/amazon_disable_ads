#include <stdio.h>
#include <unistd.h>

#ifdef SETCAP
#include <sys/capability.h>
#endif

int main(int argc, char **argv)
{
  printf("running as uid %d\n", getuid());

#ifdef SETCAP
  struct __user_cap_header_struct capheader;
  struct __user_cap_data_struct capdata[2];

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

  if(setresgid(0,0,0) || setresuid(0,0,0)) {
    printf("setresgid/setresuid failed\n");
  }
#endif

  printf("uid %d\n", getuid());
  return 0;
}