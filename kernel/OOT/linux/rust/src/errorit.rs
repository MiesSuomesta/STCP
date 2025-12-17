
// Errorit
// Quick & Dirty Linux errno constants (kernel-style negative errors)

pub const EPERM:        i32 = 1;
pub const ENOENT:       i32 = 2;
pub const ESRCH:        i32 = 3;
pub const EINTR:        i32 = 4;
pub const EIO:          i32 = 5;
pub const ENXIO:        i32 = 6;
pub const E2BIG:        i32 = 7;
pub const ENOEXEC:      i32 = 8;
pub const EBADF:        i32 = 9;
pub const ECHILD:       i32 = 10;
pub const EAGAIN:       i32 = 11;   // EWOULDBLOCK
pub const ENOMEM:       i32 = 12;
pub const EACCES:       i32 = 13;
pub const EFAULT:       i32 = 14;
pub const EBUSY:        i32 = 16;
pub const EEXIST:       i32 = 17;
pub const EXDEV:        i32 = 18;
pub const ENODEV:       i32 = 19;
pub const ENOTDIR:      i32 = 20;
pub const EISDIR:       i32 = 21;
pub const EINVAL:       i32 = 22;
pub const ENFILE:       i32 = 23;
pub const EMFILE:       i32 = 24;
pub const ENOTTY:       i32 = 25;
pub const ETXTBSY:      i32 = 26;
pub const EFBIG:        i32 = 27;
pub const ENOSPC:       i32 = 28;
pub const ESPIPE:       i32 = 29;
pub const EROFS:        i32 = 30;
pub const EMLINK:       i32 = 31;
pub const EPIPE:        i32 = 32;
pub const EDOM:         i32 = 33;
pub const ERANGE:       i32 = 34;


pub const ENOTCONN:     i32 = 107;
pub const ECONNRESET:   i32 = 104;
pub const ECONNREFUSED: i32 = 111;
pub const ECONNABORTED: i32 = 103;
pub const EHOSTUNREACH: i32 = 113;
pub const ETIMEDOUT:    i32 = 110;
pub const EINPROGRESS:  i32 = 115;

