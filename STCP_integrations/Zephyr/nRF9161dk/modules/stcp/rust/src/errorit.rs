// Errorit
// Quick & Dirty Linux errno constants (kernel-style negative errors)
pub(crate) const EPERM:        i32 = 1;
pub(crate) const ENOENT:       i32 = 2;
pub(crate) const ESRCH:        i32 = 3;
pub(crate) const EINTR:        i32 = 4;
pub(crate) const EIO:          i32 = 5;
pub(crate) const ENXIO:        i32 = 6;
pub(crate) const E2BIG:        i32 = 7;
pub(crate) const ENOEXEC:      i32 = 8;
pub(crate) const EBADF:        i32 = 9;
pub(crate) const ECHILD:       i32 = 10;
pub(crate) const EAGAIN:       i32 = 11;   // EWOULDBLOCK
pub(crate) const ENOMEM:       i32 = 12;
pub(crate) const EACCES:       i32 = 13;
pub(crate) const EFAULT:       i32 = 14;
pub(crate) const EBUSY:        i32 = 16;
pub(crate) const EEXIST:       i32 = 17;
pub(crate) const EXDEV:        i32 = 18;
pub(crate) const ENODEV:       i32 = 19;
pub(crate) const ENOTDIR:      i32 = 20;
pub(crate) const EISDIR:       i32 = 21;
pub(crate) const EINVAL:       i32 = 22;
pub(crate) const ENFILE:       i32 = 23;
pub(crate) const EMFILE:       i32 = 24;
pub(crate) const ENOTTY:       i32 = 25;
pub(crate) const ETXTBSY:      i32 = 26;
pub(crate) const EFBIG:        i32 = 27;
pub(crate) const ENOSPC:       i32 = 28;
pub(crate) const ESPIPE:       i32 = 29;
pub(crate) const EROFS:        i32 = 30;
pub(crate) const EMLINK:       i32 = 31;
pub(crate) const EPIPE:        i32 = 32;
pub(crate) const EDOM:         i32 = 33;
pub(crate) const ERANGE:       i32 = 34;

pub(crate) const EMSGSIZE:     i32 = 90;
pub(crate) const EPROTO:       i32 = 71;

pub(crate) const ENOTCONN:     i32 = 107;
pub(crate) const ECONNRESET:   i32 = 104;
pub(crate) const ECONNREFUSED: i32 = 111;
pub(crate) const ECONNABORTED: i32 = 103;
pub(crate) const ETIMEDOUT:    i32 = 116; // Linuksilla 110
pub(crate) const EINPROGRESS:  i32 = 115;

pub(crate) const EHOSTUNREACH: i32 = 113;
pub(crate) const ENETUNREACH:  i32 = 128;

