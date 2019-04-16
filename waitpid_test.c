#include "types.h"
#include "user.h"

int main(void) {
  printf(0, "My pid is %d.\n", (int)getpid());
  int rc, status, child_pid;

  rc = fork();
  if (rc < 0) {
    exit(1);
  } else if (rc == 0) {
    printf(0, "Child w/ pid %d.\n", (int)getpid());
    exit(1000);
  } else {
    child_pid = waitpid(rc, &status, 0);

    if (child_pid != rc) {
      printf(0, "Child PID values do not match! Exiting...");
      exit(1);
    }

    printf(0, "Parent of %d w/ pid %d. Child exited with %d.\n", child_pid, (int)getpid(), status);
    exit(0);
  }
}
