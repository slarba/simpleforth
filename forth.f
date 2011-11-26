58 here @ c!
0  here @ 1+ c!
here @ dup cellsize + here !
create ]
word create
latest @ hidden
]
exit
eow 
[

59 here @ c!
0  here @ 1+ c!
here @ dup cellsize + here ! create immediate
' lit ,
' exit ,
' , ,
' lit ,
' eow ,
' , ,
' latest ,
' @ ,
' hidden ,
' [ ,
' exit ,
' eow ,

: make-inline
    latest @ dup
    @ f_inline xor
    swap !
;

: inline immediate make-inline ;

: allot here @ swap here +! ;

: variable
    cellsize allot
    word create make-inline
    ' lit ,
    ,
    ' exit ,
    ' eow ,
;

variable consthere
variable consthere0

4096 cellsize * allot
dup consthere !
consthere0 !

variable datahere
variable datahere0
4096 cellsize * allot
dup datahere !
datahere0 !

1024 cellsize * allot
dup t0 !
tsp!

: cell inline cellsize ;
: cells inline cellsize * ;

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

: [compile] immediate
    word find
    dup @ f_builtin and
    if
	>cfa @ ,
    else
	' call , >cfa ,
    then
;

: unless immediate
    ' 0= ,
    [compile] if
;

: case immediate 0 ;
: of immediate
    ' over ,
    ' = ,
    [compile] if
    ' drop ,
;
: (of) immediate
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

: '\n' inline 10 ;
: cr inline 10 emit ;
: literal immediate ' lit , , ;
: char word c@ ;
: ':' inline [ char : ] literal ;
: ';' inline [ char ; ] literal ;
: '(' inline [ char ( ] literal ;
: ')' inline [ char ) ] literal ;
: '"' inline [ char " ] literal ;
: 'A' inline [ char A ] literal ;
: '0' inline [ char 0 ] literal ;
: '-' inline [ char - ] literal ;
: '.' inline [ char . ] literal ;

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

( moniriviset
  kommentit toimii nyt )

: cell+ inline cellsize + ;
: cell- inline cellsize - ;

: str= inline strcmp 0= ;
: str< inline strcmp 0< ;
: str> inline strcmp 0> ;
: str<> inline strcmp ;

: bl inline  32 ;
: space inline bl emit ;
: negate inline 0 swap - ;
: true inline 1 ;
: false inline 0 ;
: not inline 0= ;

: aligned
    cellsize 1- + cellsize 1- invert and ;

: align here @ aligned here ! ;
: constalign consthere @ aligned consthere ! ;

: c, here @ c! here @ 1+ here ! ;
: const, consthere @ ! consthere @ cell+ consthere ! ;
: constc, consthere @ c! consthere @ 1+ consthere ! ;

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

: pick 1+ cellsize * dsp@ + @ ;

: make-const-str ( str -- conststr )
    dup consthere @
    strcpy
    consthere @ swap
    strlen 1+ consthere +!
    constalign
;

( sanakirjat )
variable current-vocab
variable latest-defined-vocab

0 current-vocab !
0 latest-defined-vocab !

: vocab-name ( vocabentry -- name ) cell+ @ ;
: vocab-next ( vocabentry -- nextvocabentry/0 ) 2 cells + @ ;
: vocab-latest ( vocab-entry -- latest ) @ ;
: set-vocab-name ( name vocabentry -- ) cell+ ! ;
: set-vocab-next ( nextentry vocabentry -- ) 2 cells + ! ;
: set-vocab-latest ( latest vocabentry -- ) ! ;
: vocab-useslist ( vocabentry -- useslist ) 3 cells + ;

: find-vocabulary ( name -- vocabulary/0 )
    latest-defined-vocab @         ( name latestvocab )
    begin
	dup 0= if                  \ is the entry zero?
	    2drop 0 exit           \ return zero
	else
	    2dup vocab-name str<>   \ compare names
	then
    while
	    vocab-next
    repeat
    nip
;

: in: immediate
    word find-vocabulary
    ?dup if
	latest @ current-vocab @ set-vocab-latest   \ save latest to current vocabulary
	dup current-vocab !                         \ this is the new current vocabulary
	vocab-latest latest !                       \ get new latest from current vocabulary and save it to latest
    else
	." no such vocabulary" cr
    then
;

: vocabulary immediate
    word            ( vocabname )
    make-const-str  ( constvocabname )
    consthere @     ( constvocabname vocabulary )
    2dup set-vocab-name   ( constvocabname vocabulary )
    nip                   ( vocabulary )

    current-vocab @       ( vocabulary currentvocab )
    ?dup if
	latest @ swap set-vocab-latest
    then
    latest-defined-vocab @  ( vocabulary latestvocab )
    over set-vocab-next     ( vocabulary )  \ link them
    latest @                ( vocabulary currlatest )
    over set-vocab-latest   ( vocabulary )  \ save latest
    dup latest-defined-vocab !  \ make it the last defined vocab
    dup current-vocab !         \ it also becomes the current vocab like with in:
    vocab-useslist consthere !       \ advance consthere
;

: use immediate
    word find-vocabulary
    ?dup if
	const,
    else
	." no such vocabulary to use" cr
    then
;

: definitions immediate
    0 const,   \ terminate uses list
;

: vocabularies ( -- )
    latest-defined-vocab @
    begin
	dup
    while
	    dup vocab-name tell space
	    vocab-next
    repeat
    drop
;

: spaces ( n -- )
    begin
	dup 0>
    while
	    space
	    1-
    repeat
    drop
;

: decimal 10 base ! ;
: hex 16 base ! ;

: u. ( u -- )
    base @ u/mod
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

( vocabulary-aware new version of find )
: find ( wordname -- word )
    dup find            ( wordname dictentry ) \ try to find from current latest first
    ?dup if
	nip exit
    else
	latest @                         ( wordname latest )
	current-vocab @ vocab-useslist   ( wordname latest useslist )
	begin
	    dup @                        ( wordname latest useslist vocabentry/0 )
	while
		dup @ vocab-latest       ( wordname latest useslist usedlatest )
		latest !                 ( wordname latest useslist )
		2 pick                   ( wordname latest useslist wordname)
		find                     ( wordname latest useslist word/0 )
		?dup if
		    nip over latest !
		    2nip
		    exit
		else
		    cell+
		then
	repeat
	drop latest ! drop 0
    then
;

: ?hidden @ f_hidden and ;
: ?immediate @ f_immediate and ;
: ?builtin @ f_builtin and ;
: ?inline @ f_inline and ;

: ' immediate  ( better version of tick )
    word find
    dup 0= if
	." no such word" cr drop
	exit
    then
    dup ?builtin if
	>cfa @
    else
	>cfa
    then
    state @ if
	' lit , ,
    then
;

: [compile] immediate
    word find
    dup @ f_builtin and
    if
	>cfa @ ,
    else
	' call , >cfa ,
    then
;

vocabulary forth
definitions

: hide word find hidden ;

\ hide current-vocab
hide latest-defined-vocab
\ hide vocab-name
hide vocab-next
hide vocab-latest
hide set-vocab-name
hide set-vocab-next
hide set-vocab-latest
hide vocab-useslist
hide find-vocabulary

: copytohere ( addr -- addr+cellsize )
    dup @ , cell+
;

: perform-inline ( codetoinline -- )
    begin
	dup @ ' eow <>
    while
	    dup @ case
		' lit of copytohere endof
		' lit+ of copytohere endof
		' lit- of copytohere endof
		' call of copytohere endof
		' jump of copytohere endof
		' branch of copytohere endof
		' 0branch of copytohere endof
		' 1branch of copytohere endof
		' var@ of copytohere endof
		' var! of copytohere endof
	    endcase
	    copytohere
    repeat
    here @ cell- here !
    drop
;

: interpret
    iword
    dup 0= if
	drop exit
    then
    dup find
    ?dup if
	nip
	dup ?immediate if
	    iexecute
	else
	    state @ if
		dup ?builtin if
		    >cfa @ ,
		else
		    dup ?inline if
			>cfa perform-inline
		    else
			' call , >cfa ,
		    then
		then
	    else
		iexecute
	    then
	then
    else
	number
	state @ if
	    ' lit ,
	    ,
	then
    then
;

\ variable quit-level
\ 0 quit-level !

: quit
\    1 quit-level +!
\    quit-level @ 2 > if
\	." use print-stack-trace to view what happened" cr
\    then
    begin
	input-stream @ ?eof not
    while
	interpret
    repeat
\    die
;

quit

( redefine to inline )
: cell inline cellsize ;
: cells inline cellsize * ;

: defer immediate
    word create
    latest @ @ f_deferred xor latest @ !
    ' jump ,
    0 ,
    ' exit ,
    ' eow ,
;

: is immediate
    word find
    ?dup if
	>cfa cell+ !
    else
	." no such word" cr
    then
;

: create ( wordname )
    dup find       ( wordname previousdef )
    ?dup if    \ if previous definition was found
	dup @ f_deferred and if  \ and it was deferred
	    over create    \ create new word  ( wordname dictentry )
	    >cfa cell+     ( wordname callptr )
	    latest @ >cfa  ( wordname callptr newwordimpl )
	    swap !
	    drop      \ wordname
	    exit
	then
	drop
    then    
    create
;

\ new version of colon to support deferred words-aware create
: :
    word       ( wordname )
    create
    latest
    @
    hidden
    ]
;

hide copytohere
hide perform-inline

: cell+ inline cellsize + ;
: cell- inline cellsize - ;

: do immediate
    ' >r , ' >r ,
    [compile] begin ;

: loop immediate
    ' r> , ' r> ,
    ' 1+ ,     \ add loop var
    ' 2dup , ' >r , ' >r ,
    ' = ,
    [compile] until
    ' rdrop , ' rdrop ,
;

: unloop immediate
    ' 2rdrop ,
;

: i inline ( -- loopvar ) rsp@ cell+ @ ;

: depth
    s0 @ dsp@ -
    cell-
;

: ? @ . ;

: within
    -rot over
    <= if
	> if true else false then
    else
	2drop false then
;

: constant
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
    ' eow ,
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

: :noname
    0 create
    here @
    ] 
;

: ['] immediate
    ' lit ,
;

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

: for-reading ( -- mode ) s" r" ; inline
: for-reading-and-writing ( -- mode ) s" r+" ; inline

variable input-stack
16 cells allot input-stack !

: push-input-stack inline ( fp -- )
    input-stack @ !
    cell input-stack +!
;

: pop-input-stack inline ( -- fp )
    cell input-stack -!
    input-stack @ @
;

: include ( -- )
    word
    for-reading open-file            ( fp )
    ?dup if                           ( fp )
	input-stream @ push-input-stack   \ save old stdin
	input-stream !                    \ store fp as new stdin
	begin
	    input-stream @ ?eof not       \ as long as there is something to interpret
	while
		interpret            \ ... interpret!
	repeat
	input-stream @ close-file         \ close the file
	pop-input-stack input-stream !    \ and restore old stdin
    else
	." no such file" cr
    then
;

hide input-stack
hide push-input-stack
hide pop-input-stack

include peephole.f
opt-word include

variable compiling-lambda
0 compiling-lambda !

: :lambda immediate
    state @ if     \ if compiling
	' lit ,
	datahere @ ,
	here @     \ save old here ptr
	datahere @ here !    \ save new here for compilation
    else
	0 create
	here @
	1 compiling-lambda !
	]
    then
;

: ;; immediate
    state @ if
	' exit , ' eow ,
	compiling-lambda @ 0= if
	    here @ datahere !   \ advance consthere
	    here !    \ restore old here pointer
	else
	    [compile] [
	then
	0 compiling-lambda !
    then
;

: exception-marker
    rdrop 0
;

: catch
    dsp@ cell+ >r
    ' exception-marker >r
    execute
;

: throw ( n -- )
    ?dup if
	rsp@                         ( n rsp )
	begin
	    dup r0 @ cell- u<
	while
		dup @
		' exception-marker = if
		    cell+
		    rsp!
		    dup dup dup
		    r>
		    cell-
		    swap over
		    !
		    dsp! exit
		then
		cell+
	repeat

	drop
	case
	    -1 of ." aborted" cr endof
	    
	    ." uncaught throw " dup . cr
	endcase
	quit
    then
;

: lookup-word-from-ip
    latest @              ( codeaddr latest )
    begin
	?dup
    while
	    2dup swap     ( codeaddr latest latest codeaddr )
	    u< if
		nip
		exit
	    then
	    cell+ @
    repeat
    drop 0
;

: print-stack-trace
    rsp@
    begin
	dup r0 @ cell- <>
    while
	    dup @
	    case
		' exception-marker of ." catch ( dsp=" cell+ dup @ u. ." ) " cr endof

		dup
		lookup-word-from-ip
		
		id. cr
	    endcase
	    cell+
    repeat
    drop
    cr
;

include disasm.f
include debugger.f
include classes.f
include unittest.f

: bytes-used
      here @ here0 - ;

: const-bytes-used
      consthere @ consthere0 - ;

: welcome
    ." MLT Forth version " version . cr 
    ." Bytes used: " bytes-used . cr
    ." Const used: " const-bytes-used . cr
    ." Welcome!" cr
;

: quit
    <stdin> input-stream !
    begin
	." [" depth cell / . current-vocab @ vocab-name tell
	s" ]> " input-stream @ prompt
	input-stream @ ?eof not
    while
	    begin
		input-stream @ ?eol not
	    while
		    interpret
	    repeat
	    cr
    repeat
    die
;

welcome
hide welcome
quit
