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

: cell cellsize ;
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

: ?hidden
    @ f_hidden and ;
: ?immediate
    @ f_immediate and ;

: id. cell+ cell+ tell ;

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

: make-const-str ( str -- conststr )
    dup consthere @
    strcpy
    consthere @ swap
    strlen 1+ consthere +!
    constalign
;

: for-reading ( -- mode ) s" r" ;
: for-reading-and-writing ( -- mode ) s" r+" ;

variable input-stack
16 cells allot input-stack !

: push-input-stack ( fp -- )
    input-stack @ !
    cell input-stack +!
;

: pop-input-stack ( -- fp )
    cell input-stack -!
    input-stack @ @
;

: include ( -- )
    word
    for-reading open-file            ( fp )
    ?dup if                           ( fp )
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
    then
;

hide input-stack
hide push-input-stack
hide pop-input-stack

include classes.f

class: myotherclass <base object
   cell var kentta

   m: ." myotherclass constructor!" cr
       100 self kentta !
   ; implements construct
   
   m: ." myotherclass destructor!" cr
   ; implements destruct
endclass

class: myclass <base object
   cell var field1
   cell var field2

   method method1
   method method2
   method method3

   m: ." construct" cr
      myotherclass new self field1 !
   ; implements construct

   m:
    ." another method!" cr
    ." kentta=" self field1 @ kentta @ . cr
   ; implements method2

   m:
    ." destruct" cr
    self field1 @ delete
   ; implements destruct

endclass

myclass new value testiotus

: welcome
    ." MLT Forth version " version . cr
    ." Welcome!" cr
;

: quit
    begin
	interpret
	<stdin> @ ?eof
    until
    die
;

welcome
hide welcome
quit
