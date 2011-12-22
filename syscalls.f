
: [compile-ioctl-preamble] immediate
    ' dcreset ,
    ' lit , DC_CALL_C_ELLIPSIS ,
    ' dcmode ,
    ' lit , SYS_ioctl ,
    ' dcint ,
;

: [compile-ioctl-call] immediate
    ' lit , syscall-fn ,
    ' dccallint ,
    ' lit , DC_CALL_C_DEFAULT ,
    ' dcmode ,
;

: i2c-slave
    [compile-ioctl-preamble]
    dcint
    hex 707 decimal dcint
    dcint
    [compile-ioctl-call]
;
