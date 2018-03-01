#include "sandbox.hpp"
#include <cstdint>
#include <vector>

#if defined(PLUGKIT_OS_LINUX)
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <stddef.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace plugkit {
namespace Sandbox {

#if defined(PLUGKIT_OS_LINUX)

#define ALLOW_IF_MATCH(call)                                                   \
  BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, call, 0, 1),                             \
      BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW)

namespace {
thread_local sock_filter FILTER_PROFILE[] = {
    BPF_STMT(BPF_LD | BPF_W | BPF_ABS, (offsetof(struct seccomp_data, nr))),
    ALLOW_IF_MATCH(SYS_open),
    ALLOW_IF_MATCH(SYS_write),
    ALLOW_IF_MATCH(SYS_madvise),
    ALLOW_IF_MATCH(SYS_futex),
    ALLOW_IF_MATCH(SYS_mprotect),
    ALLOW_IF_MATCH(SYS_munmap),
    ALLOW_IF_MATCH(SYS_mmap),
    ALLOW_IF_MATCH(SYS_getpid),
    ALLOW_IF_MATCH(SYS_exit),
    ALLOW_IF_MATCH(SYS_clone),
    ALLOW_IF_MATCH(SYS_set_robust_list),
    ALLOW_IF_MATCH(SYS_gettid),
    ALLOW_IF_MATCH(SYS_stat),
    ALLOW_IF_MATCH(SYS_setpriority),
    ALLOW_IF_MATCH(SYS_prctl),
    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL)};

thread_local sock_filter DISSECTOR_PROFILE[] = {
    BPF_STMT(BPF_LD | BPF_W | BPF_ABS, (offsetof(struct seccomp_data, nr))),
    ALLOW_IF_MATCH(SYS_open),
    ALLOW_IF_MATCH(SYS_write),
    ALLOW_IF_MATCH(SYS_madvise),
    ALLOW_IF_MATCH(SYS_futex),
    ALLOW_IF_MATCH(SYS_mprotect),
    ALLOW_IF_MATCH(SYS_munmap),
    ALLOW_IF_MATCH(SYS_mmap),
    ALLOW_IF_MATCH(SYS_getpid),
    ALLOW_IF_MATCH(SYS_exit),
    ALLOW_IF_MATCH(SYS_clone),
    ALLOW_IF_MATCH(SYS_set_robust_list),
    ALLOW_IF_MATCH(SYS_gettid),
    ALLOW_IF_MATCH(SYS_stat),
    ALLOW_IF_MATCH(SYS_setpriority),
    ALLOW_IF_MATCH(SYS_prctl),
    ALLOW_IF_MATCH(SYS_getrandom),
    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL)};
} // namespace

void activate(Profile profile) {
  sock_fprog prog = {0, nullptr};

  switch (profile) {
  case PROFILE_FILTER:
    prog.len = sizeof(FILTER_PROFILE) / sizeof(sock_filter);
    prog.filter = FILTER_PROFILE;
    break;
  case PROFILE_DISSECTOR:
    prog.len = sizeof(DISSECTOR_PROFILE) / sizeof(sock_filter);
    prog.filter = DISSECTOR_PROFILE;
    break;
  default:
    return;
  }

  prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
  syscall(SYS_seccomp, SECCOMP_SET_MODE_FILTER, SECCOMP_FILTER_FLAG_LOG, &prog);
}
#else
void activate(Profile profile) {}
#endif
};
} // namespace plugkit
