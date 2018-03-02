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

#define SB_LOAD_CALL_ID                                                        \
  BPF_STMT(BPF_LD | BPF_W | BPF_ABS, (offsetof(struct seccomp_data, nr)))

#define SB_RETURN_KILL BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL)

#define SB_ALLOW_IF_MATCH(call)                                                \
  BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, call, 0, 1),                             \
      BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW)

#ifndef SECCOMP_FILTER_FLAG_LOG
#define SECCOMP_FILTER_FLAG_LOG 0
#endif

#ifndef SYS_getrandom
#define SYS_getrandom 318
#endif

namespace {
thread_local sock_filter FILTER_PROFILE[] = {SB_LOAD_CALL_ID,
#include "sandbox_linux_profile.prog"
                                             SB_RETURN_KILL};

thread_local sock_filter DISSECTOR_PROFILE[] = {SB_LOAD_CALL_ID,
#include "sandbox_linux_profile.prog"
                                                SB_RETURN_KILL};

thread_local sock_filter FILE_PROFILE[] = {SB_LOAD_CALL_ID,
#include "sandbox_linux_profile.prog"
                                           SB_RETURN_KILL};
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
  case PROFILE_FILE:
    prog.len = sizeof(FILE_PROFILE) / sizeof(sock_filter);
    prog.filter = FILE_PROFILE;
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
}
} // namespace plugkit
