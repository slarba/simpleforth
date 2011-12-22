
: [compile-syscall-preamble] immediate
    ' dcreset ,
    ' lit , DC_CALL_C_ELLIPSIS ,
    ' dcmode ,
;

: [compile-syscall-call] immediate
    ' lit , syscall-fn ,
    ' dccallint ,
    ' lit , DC_CALL_C_DEFAULT ,
    ' dcmode ,
;

(
: i2c-slave
    [compile-syscall-preamble]
    SYS_ioctl dcint
    dcint
    hex 707 decimal dcint
    dcint
    [compile-ioctl-call]
;
)

0 constant O_RDONLY
1 constant O_WRONLY
2 constant O_RDWR

: open
    [compile-syscall-preamble]
    SYS_open dcint
    dcptr   \ pathname
    dcint   \ flags
    [compile-syscall-call]
;

: close
    [compile-syscall-preamble]
    SYS_close dcint
    dcint
    [compile-syscall-call]
;

: read
    [compile-syscall-preamble]
    SYS_read dcint
    dcint
    dcptr
    dcint
    [compile-syscall-call]
;

: getcwd
    [compile-syscall-preamble]
    SYS_getcwd dcint
    dcptr
    dcint
    [compile-syscall-call]
;
