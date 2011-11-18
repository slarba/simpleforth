
( dynamic c function calls )

: c-library ( libpath -- library )
    dcloadlib
    ?dup not if
	." cannot open dynamic library"
	die
    then
;

: c-fn ( library symbolname -- library funptr callvm )
    word create
    2dup swap dcsymbol
    ?dup not if
	." no such symbol name"
	2drop
	die
    else
	( library symbolname funptr )
	nip         ( library funptr )
	1024 dcnew  ( library funptr callvm ) \ compile callvm
	dup ' lit , , ' dcreset ,
    then
;

: c-bool
    dup ' lit , ,
    ' dcbool ,
;

: c-char
    dup ' lit , ,
    ' dcchar ,
;

: c-short
    dup ' lit , ,
    ' dcshort ,
;

: c-int
    dup ' lit , ,
    ' dcint ,
;

: c-long
    dup ' lit , ,
    ' dclong ,
;

: c-ptr
    dup ' lit , ,
    ' dcptr ,
;

: returns swap ' lit , , ' lit , , ;
: finish ' exit , ' eow , ;

: returns:ptr
    returns
    ' dccallptr ,
    finish
;

: returns:int
    returns
    ' dccallint ,
    finish
;

: returns:void
    returns
    ' dccallvoid ,
    finish
;

: returns:char
    returns
    ' dccallchar ,
    finish
;

: returns:short
    returns
    ' dccallshort ,
    finish
;

: returns:bool
    returns
    ' dccallbool ,
    finish
;

: returns:long
    returns
    ' dccalllong ,
    finish
;

: ;c-library
    drop
;

(
s" /usr/lib/libgif.so" c-library
  s" DGifOpenFileName" c-fn gifOpen c-ptr returns:ptr
  s" DGifCloseFile" c-fn gifClose c-ptr returns:int
;c-library
)
