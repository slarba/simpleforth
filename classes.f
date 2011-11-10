
vocabulary classes
definitions

: object-size ( classdef -- objsize )
    ;
: baseclass-def ( classdef -- baseclassdef )
    cell+ ;
: vtable-size ( classdef -- vtablesize )
    2 cells + ;
: vtable-ptr ( classdef -- vtableptr )
    3 cells + ;

: send ( ... object method -- ... )
    swap dup >r     \ save self        ( method object )
    @ vtable-ptr +  \ locate vtable entry ( vtableptr )
    @ execute       \ execute method
    rdrop        \ remove self from return stack
;

: self ( -- self )
    rsp@ cell+ cell+ @ ;    \ second topmost element of the return stack is self in method calls

variable curr-defined-class
variable curr-defined-class-size
variable curr-defined-class-vtblsize

: class: ( -- )
    word create
    ' lit ,
    consthere @ ,
    ' exit ,
    ' eow ,
    consthere @ curr-defined-class !      ( store classdef ptr )
    consthere @ vtable-ptr consthere !    ( advance constpool ptr to beginning of vtable )
    0 curr-defined-class-size !
    0 curr-defined-class-vtblsize !
;

: <base ( -- )
    word find
    >cfa execute                ( baseclassdef )
    dup curr-defined-class @ baseclass-def !                 \ store baseclass pointer to current definition
    dup object-size @        curr-defined-class-size !       \ new class is at least of baseclass's size
    dup vtable-size @        curr-defined-class-vtblsize !   \ and vtable is copied
    ( now copy the vtable )
    dup vtable-size @          ( baseclassdef vtablesize )
    swap vtable-ptr swap       ( baseclassvtableptr vtablesize )
    begin
	dup 0>
    while
	    swap dup @ const, cell+ swap
	    1-
    repeat
    2drop
;

: var ( fieldsize -- )
    word create
    ' lit ,
    curr-defined-class-size @ cell+ ,
    ' + ,
    ' exit ,
    ' eow ,
    curr-defined-class-size +!
;

: unimplemented-method ;

: method ( -- )
    word create
    ' lit ,
    curr-defined-class-vtblsize @ cellsize * ,
    ' exit ,
    ' eow ,
    ' unimplemented-method const,
    1 curr-defined-class-vtblsize +!
;

: endclass ( -- )
    curr-defined-class-size @ curr-defined-class @ object-size !
    curr-defined-class-vtblsize @ curr-defined-class @ vtable-size !
;

: m: 0 create here @ ] ;

: implements ( fn -- )
    word find >cfa execute
    curr-defined-class @ vtable-ptr + ! ;

( -------------------------------------------------- )
class: object
    method construct
    method destruct
    method tostring

    m: s" <object>" ; implements tostring
endclass

: new ( classdef -- object )
    dup object-size @ cell+ malloc   ( classdef object )
    2dup !                           ( classdef object )
    nip dup construct send
;

: delete ( object -- )
    dup destruct send
    mfree
;

hide unimplemented-method
hide curr-defined-class
hide curr-defined-class-size
hide curr-defined-class-vtblsize
hide vtable-ptr
hide vtable-size
hide baseclass-def
hide object-size

in: forth
