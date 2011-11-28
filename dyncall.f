
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
	' dcreset ,
    then
;

: c-bool
    ' dcbool ,
;

: c-char
    ' dcchar ,
;

: c-short
    ' dcshort ,
;

: c-int
    ' dcint ,
;

: c-long
    ' dclong ,
;

: c-ptr
    ' dcptr ,
;

: returns ' lit , , ;
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
