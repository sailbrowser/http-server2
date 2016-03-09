#include <unistd.h>

int get_num_cores() {
  return sysconf(_SC_NPROCESSORS_ONLN);
}
