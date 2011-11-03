58 consthere @ c!
0  consthere @ 1+ c!
consthere @ create
' word , ' create , ' latest , ' @ , ' hidden , ' ] , ' exit , ' eow ,
59 consthere @ c!
0  consthere @ 1+ c!
consthere @ create immediate
' lit , ' exit , ' , , ' lit , ' eow , ' , , ' latest , ' @ , ' hidden , ' [ , ' exit , ' eow ,

: '\n' 10 ;
: bl   32 ;
: cr '\n' emit ;
: space bl emit ;
: negate 0 swap - ;
: true 1 ;
: false 0 ;
: not 0= ;

: cell+ cellsize + ;
: cell- cellsize - ;

: c, here @ c! here @ 1+ here ! ;
: const, consthere @ ! consthere @ cell+ consthere ! ;
: constc, consthere @ c! consthere @ 1+ consthere ! ;

: literal immediate ' lit , , ;
: char word c@ ;
: ':' [ char : ] literal ;
: ';' [ char ; ] literal ;
: '(' [ char ( ] literal ;
: ')' [ char ) ] literal ;
: '"' [ char " ] literal ;
: 'A' [ char A ] literal ;
: '0' [ char 0 ] literal ;
: '-' [ char - ] literal ;
: '.' [ char . ] literal ;

: hide
    word find hidden ;

: if immediate
    ' 0branch ,
    here @ 
    0 ,
;

: then immediate
    dup
    here @ swap -
    swap !
;

: else immediate
    ' branch ,
    here @
    0 ,
    swap
    dup
    here @ swap -
    swap !
;

: [compile] immediate
    word find
    dup @ f_builtin and
    if
	>cfa @ ,
    else
	lit call , >cfa ,
    then
;

: recurse immediate
    ' call ,
    latest @
    >cfa ,
;

: begin immediate
    here @
;

: until immediate
    ' 0branch ,
    here @ -
    ,
;

: again immediate
    ' branch ,
    here @ -
    ,
;

: while immediate
    ' 0branch ,
    here @
    0 ,
;

: repeat immediate
    ' branch ,
    swap
    here @ - ,
    dup
    here @ swap -
    swap !
;

: unless immediate
    ' 0= ,
    [compile] if
;

: \ immediate
    begin
	key
	'\n' <>
    while
    repeat
;

\ yksiriviset kommentit toimii nyt

: ( immediate      \ moniriviset kommentit
    1
    begin
	key
	dup '(' =
	if
	    drop 1+
	else
	    ')' =
	    if
		1-
	    then
	then
    dup 0=
    until
    drop
;

: nip ( x y -- y ) swap drop ;
: tuck ( x y -- y x y ) swap over ;
: pick 1+ cellsize * dsp@ + @ ;

: spaces ( n -- )
    begin
	dup 0>
    while
	    space
	    1-
    repeat
    drop
;

: depth
    s0 @ dsp@ -
    cell-
;

: decimal 10 base ! ;
: hex 16 base ! ;

: u. ( u -- )
    base @ /mod
    ?dup if
	recurse
    then

    dup 10 < if
	'0'
    else
	10 -
	'A'
    then
    +
    emit
;

: .s ( -- )
    dsp@
    begin
	dup s0 @ <
    while
	    dup @ u.
	    space
	    cell+
    repeat
    drop
;

: uwidth ( u -- width )
    base @ /
    ?dup if
	recurse 1+
    else
	1
    then
;

: u.r ( u width -- )
    swap
    dup
    uwidth
    rot
    swap -
    spaces
    u.
;

: .r
    swap
    dup 0< if
	negate
	1
	swap
	rot
	1-
    else
	0 swap rot
    then
    swap
    dup
    uwidth
    rot
    swap -
    spaces
    swap
    if
	'-' emit
    then
    u.
;

: . 0 .r space ;
: u. u. space ;

: ? @ . ;

: within
    -rot over
    <= if
	> if true else false then
    else
	2drop false then
;

: aligned
    cellsize 1- + cellsize 1- invert and ;

: align here @ aligned here ! ;
: constalign consthere @ aligned consthere ! ;

: s" immediate
    state @ if            ( if compiling, emit a lit instruction with the starting pointer )
	consthere @          ( save string starting pos )
	begin
	    key     ( startpos key )
	    dup '"' <>  ( startpos key notadoublequote )
	while
		constc,
	repeat
	drop
	0 constc,  ( null-terminate! )
	' lit ,
	,              ( emit starting pos )
	constalign
    else
	consthere @
	begin
	    key
	    dup '"' <>
	while
		over c!
		1+
	repeat
	drop
	0 over c! drop
	consthere @
    then
;

: ." immediate
    state @ if
	[compile] s"
	' tell ,
    else
	begin
	    key
	    dup '"' = if
		drop exit
	    then
	    emit
	again
    then
;

: constant
    word create
    ' lit ,
    ,
    ' exit ,
;

: allot
    here @ swap here +!
;

: cells cellsize * ;

: variable
    1 cells allot
    word create
    ' lit ,
    ,
    ' exit ,
;

: value
    word create
    ' lit ,
    ,
    ' exit ,
;

: to immediate
    word find
    >cfa
    cell+
    state @ if
	' lit ,
	,
	' ! ,
    else
	!
    then
;

: case immediate 0 ;
: of immediate
    ' over ,
    ' = ,
    [compile] if
    ' drop ,
;
: endof immediate
    [compile] else
;
: endcase immediate
    ' drop ,
    begin ?dup while [compile] then repeat
;

: :noname
    0 create
    here @
    ] 
;

: ['] immediate
    ' lit ,
;

: next-opcode ( opcodeaddr -- opcodeaddr opcode )
    dup @ swap cell+ swap ;

: id. cell+ cell+ tell ;
: print-call-target ( jumpaddr -- )
    hdrsize - id.
;

: print-address ( addr -- )
    ." 0x" hex u. decimal 4 spaces ;

: disasm-next-instr ( xt -- nextopcodeaddr )
    dup print-address
    next-opcode
    case
	' call  of ." call <" next-opcode print-call-target ." >" endof
	' die     of ." die" endof
	' exit    of ." exit" endof
	' branch  of ." branch ( " next-opcode . ." )" endof
	' 0branch  of ." 0branch ( " next-opcode . ." )" endof
	' lit     of ." lit " next-opcode . endof
	' dup  of ." dup" endof
	' 2dup  of ." 2dup" endof
	' ?dup  of ." ?dup" endof
	' swap  of ." swap" endof
	' drop  of ." drop" endof
	' 2drop  of ." 2drop" endof
	' /mod of ." /mod" endof
	' >r  of ." >r" endof
	' r>  of ." r>" endof
	' rsp@  of ." rsp@" endof
	' rsp!  of ." rsp!" endof
	' over  of ." over" endof
	' rot  of ." rot" endof
	' -rot  of ." -rot" endof
	' find  of ." find" endof
	' create  of ." create" endof
	' word  of ." word" endof
	' key  of ." key" endof
	' emit  of ." emit" endof
	' tell  of ." tell" endof
	' latest  of ." latest" endof
	' ] of ." ]" endof
	' [ of ." [" endof
	' 1+  of ." 1+" endof
	' 1-  of ." 1-" endof
	' +!  of ." +!" endof
	' -!  of ." -!" endof
	' +  of ." +" endof
	' - of ." -" endof
	' *  of ." *" endof
	' /  of ." /" endof
	' <  of ." <" endof
	' >  of ." >" endof
	' =  of ." =" endof
	' <>  of ." <>" endof
	' <=  of ." <=" endof
	' >=  of ." >=" endof
	' 0=  of ." 0=" endof
	' 0<>  of ." 0<>" endof
	' 0>  of ." 0>" endof
	' 0<  of ." 0<" endof
	' mod  of ." mod" endof
	' invert  of ." invert" endof
	' and  of ." and" endof
	' or  of ." or" endof
	' xor  of ." xor" endof
	' lshift  of ." lshift" endof
	' rshift  of ." rshift" endof
	' >cfa  of ." >cfa" endof
	' ,  of ." ," endof
	' dsp@  of ." dsp@" endof
	' @  of ." @" endof
	' c@  of ." c@" endof
	' !  of ." !" endof
	' c!  of ." c!" endof
	' interpret  of ." interpret" endof
	' hidden  of ." hidden" endof
	' execute  of ." execute" endof
	' '  of ." '" endof
	' malloc  of ." malloc" endof
	' mfree  of ." mfree" endof
	' open-file of ." open-file" endof
	' close-file of ." close-file" endof
	' ?eof of ." ?eof" endof
    endcase
    cr
;

: disassemble ( xt -- )
    begin
	dup @
	' eow <>
    while
	    disasm-next-instr
    repeat
    drop
;

hide disasm-next-instr
hide next-opcode
hide print-call-target

: ?hidden
    @ f_hidden and ;
: ?immediate
    @ f_immediate and ;

: words
    latest @
    begin
	?dup
    while
	    dup ?hidden not if
		dup id.
		space
	    then
	    cell+ @
    repeat
    cr
;

( oliomekanismi )

: object-size ( classdef -- objsize )
    ;
: baseclass-def ( classdef -- baseclassdef )
    cell+ ;
: vtable-size ( classdef -- vtablesize )
    2 cells + ;
: vtable-ptr ( classdef -- vtableptr )
    3 cells + ;

: send ( ... object method -- ... )
    over @ vtable-ptr + @ execute ;

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

: unimplemented-method drop ;

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

    m: s" <object>" swap drop ; implements tostring
endclass

: new ( classdef -- object )
    dup object-size @ cell+ malloc   ( classdef object )
    2dup !                           ( classdef object )
    swap drop dup construct send
;

: delete ( object -- )
    dup destruct send
    mfree
;

class: myclass <base object
   1 cells var field1
   1 cells var field2

   method method1
   method method2
   method method3

m:
    ." construct" cr
    20 swap field1 !
; implements construct

m:
    ." destruct" cr drop
; implements destruct

endclass

myclass new value testiotus

( -------------------------------------------------- )
    
: strlen ( str -- len )
    dup
    begin
	dup c@ 0<>
    while
	    1+
    repeat
    swap -
;

: for-reading ( -- mode ) s" r" ;
: for-reading-and-writing ( -- mode ) s" r+" ;

variable input-stack
16 cells allot input-stack !

: push-input-stack ( fp -- )
    input-stack @ !
    input-stack @ cell+ input-stack !
;

: pop-input-stack ( -- fp )
    input-stack @ cell- input-stack !
    input-stack @ @
;

: include ( -- )
    word
    for-reading open-file            ( fp )
    dup if                           ( fp )
	<stdin> @ push-input-stack   \ save old stdin
	<stdin> !                    \ store fp as new stdin
	begin
	    <stdin> @ ?eof not       \ as long as there is something to interpret
	while
		interpret            \ ... interpret!
	repeat
	<stdin> @ close-file         \ close the file
	pop-input-stack <stdin> !    \ and restore old stdin
    else
	." no such file" cr
	drop
    then
;

: welcome
    ." MLT Forth version " version . cr
    ." Welcome!" cr
;

welcome
hide welcome
