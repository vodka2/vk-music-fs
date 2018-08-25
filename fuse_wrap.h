#pragma once

#ifdef __WIN32__
#include <fuse/fuse.h>

#define S_IFDIR 0040000u
#define S_IFREG 0100000u
#else

#define FUSE_USE_VERSION 28

#include <fuse.h>

#define fuse_uid_t                      uid_t
#define fuse_gid_t                      gid_t
#define fuse_pid_t                      pid_t

#define fuse_dev_t                      dev_t
#define fuse_ino_t                      ino_t
#define fuse_mode_t                     mode_t
#define fuse_nlink_t                    nlink_t
#define fuse_off_t                      off_t

#define fuse_fsblkcnt_t                 fsblkcnt_t
#define fuse_fsfilcnt_t                 fsfilcnt_t
#define fuse_blksize_t                  blksize_t
#define fuse_blkcnt_t                   blkcnt_t

#define fuse_utimbuf                    utimbuf
#define fuse_timespec                   timespec
#define fuse_stat                       stat

#endif
