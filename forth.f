: '\n' 10 ;
: bl   32 ;
: cr '\n' emit ;
: space bl emit ;
: negate 0 swap - ;
: true 1 ;
: false 0 ;
: not 0= ;

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
    latest @
    lit call ,
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

( nyt voidaan k‰ytt‰‰ t‰t‰ ( nestattua ) notaatiota! )

20 constant kakskyta
50 value foo

kakskyta . cr

foo . cr
40 to foo
foo . cr

: testi
    30 to foo
    foo
    case
	20 of ." joops" endof
	30 of ." heips" endof
	40 of ." lkjfds" endof
	." oletuskeissi"
    endcase
;

testi
foo . cr

depth . cr

: tulosta ." heippa maailma!" cr ;

: koe
    ' tulosta
    dup execute execute
;

\ ' unnamed disasm

' koe execute

depth .
