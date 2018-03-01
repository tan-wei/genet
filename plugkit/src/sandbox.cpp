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

namespace {
const uint32_t FILTER_ALLOWED_CALLS[] = {
    SYS_open,  SYS_write,           SYS_madvise,
    SYS_futex, SYS_mprotect,        SYS_munmap,
    SYS_mmap,  SYS_getpid,          SYS_exit,
    SYS_clone, SYS_set_robust_list, SYS_gettid,
    SYS_stat,  SYS_setpriority,     SYS_prctl,
};
}

void activate(Profile profile) {
  prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);

  std::vector<sock_filter> filter{
      BPF_STMT(BPF_LD | BPF_W | BPF_ABS, (offsetof(struct seccomp_data, nr)))};

  if (profile == PROFILE_FILTER) {
    for (uint32_t call : FILTER_ALLOWED_CALLS) {
      filter.push_back(BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, call, 0, 1));
      filter.push_back(BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW));
    }
  }

  filter.push_back(BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL));

  sock_fprog prog = {static_cast<unsigned short>(filter.size()), filter.data()};

  syscall(SYS_seccomp, SECCOMP_SET_MODE_FILTER, SECCOMP_FILTER_FLAG_LOG, &prog);
}
#else
void activate(Profile profile) {}
#endif
};
} // namespace plugkit
