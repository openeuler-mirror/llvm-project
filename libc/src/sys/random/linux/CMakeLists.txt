add_entrypoint_object(
  getrandom
  SRCS
    getrandom.cpp
  HDRS
    ../getrandom.h
  DEPENDS
    libc.include.sys_random
    libc.include.sys_syscall
    libc.src.__support.OSUtil.osutil
    libc.src.errno.errno
)
