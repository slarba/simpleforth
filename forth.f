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
	' pushxt  of ." pushxt" endof

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

: welcome
    ." MLT Forth version " version . cr
    ." Welcome!" cr
;

welcome
hide welcome
