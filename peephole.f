( peephole optimizer )

: next-instruction ( codeaddr -- nextcodeaddr )
    dup @ swap cell+ swap
    case
	' lit of cell+ endof
	' lit+ of cell+ endof
	' lit- of cell+ endof
	' call of cell+ endof
	' jump of cell+ endof
	' branch of cell+ endof
	' 0branch of cell+ endof
	' 1branch of cell+ endof
	' var@ of cell+ endof
	' var! of cell+ endof
    endcase
;

: ?notendofword ( addr -- addr noteow? )
    dup @ ' eow <> ;

: ?isbranch ( addr -- addr branch? )
    dup @
    case
	' branch of 1 endof
	' 0branch of 1 endof
	' 1branch of 1 endof
	0 swap
    endcase
;

: fixoffset-afterborder ( border offsetaddr offset -- border offsetaddr fixedoffset )
    dup 0< if           \ backward jump? will it go over the border?
	2dup +          ( border offsetaddr offset targetaddr )
	3 pick < if       \ jump over the border?
	    cell+
	then
    \ no, forward jump. no need to do anything
    then
;

: fixoffset-beforeborder ( border offsetaddr offset -- border offsetaddr fixedoffset )
    dup 0> if
	2dup +
	3 pick > if
	    cell-
	then
	\ forward jump and not over the border
    then
;

: fixoffset ( border offsetaddr offset -- border offsetaddr fixedoffset )
    2 pick 2 pick > if
	fixoffset-beforeborder
    else
	fixoffset-afterborder
    then
;

: fixbranches ( border currinst -- )
    begin
	?notendofword
    while
	    ?isbranch if
		cell+              ( border offsetaddr )
		dup @              ( border offsetaddr offset )
		fixoffset          ( border offsetaddr fixedoffset )
		over !
		cell+
	    else
		next-instruction
	    then
    repeat
    2drop
;

: save-code ( addr code -- addr+cell )
    over ! cell+
;

variable pattern-list

: patterns immediate
    here @   \ mark pattern and replacement list start. consists of cell pairs (pattern,replacement)
;

: end-patterns immediate
    , 0 , 0 ,          \ end of list marker pair
    pattern-list !   \ save pattern and replacement list into pattern list
;

: save-bytecode
    dup           ( wordname wordname )
    find          ( wordname word/0 )
    ?dup if
	nip >cfa @
    else
	number
    then
    save-code
;

: endpatternmarker inline hex 1badedda decimal ;
: wildcardmarker inline hex 2dabafab decimal ;

: ?endofpattern endpatternmarker = ;
: save-wildcard wildcardmarker save-code ;
: terminate-list endpatternmarker save-code consthere ! ;
: wildcard? s" ?" str= ;

\ compile pattern into const space
: p{ immediate ( -- pattern )
    consthere @            ( patt_start )
    dup                    ( patt_start addr )
    begin
	word                 ( patt_start addr word )
	dup wildcard?        ( patt_start addr word iswildcard )
	if
	    drop save-wildcard
	else
	    dup s" }p" str= if
		drop terminate-list exit
	    else
		save-bytecode
	    then
	then
    again
;

\ replacement pattern
: r{ immediate ( -- replacement )
    consthere @
    dup                  ( replacement_start replacement )
    begin
	word             ( replacement_start replacement word )
	dup wildcard?
	if
	    drop save-wildcard
	else
	    dup s" }r" str=   ( replacement_start replacement word isendmarker )
	    if
		drop terminate-list exit
	    else
		save-bytecode
	    then
	then
    again
;

: -> immediate
    , ;

: advance ( addr1 addr2 -- addr1+cell addr2+cell )
    swap cell+ swap cell+ ;

: match ( codeaddr pattern -- true/false )
    begin
	over @ over @          ( codeaddr pattern bytecode patterncode )
	dup ?endofpattern if              \ if we reached pattern's end, that's a match!
	    2drop 2drop 1 exit
	else                   \ otherwise need to compare the codes
	    dup wildcardmarker = if
		\ patterncode wildcardmarker matches always
		2drop
	    else
		<> if             \ no match, exit with false
		    2drop 0 exit
		then
	    then
	    advance
	then
    again
;

: search-match ( codeaddr -- matchcodeaddr replacementaddr/0 )
    pattern-list @         ( codeaddr patternlist )
    begin
	2dup @             ( codeaddr patternlist codeaddr pattern )
	match if           ( codeaddr patternlist matched? )
	    cell+ @ exit
	else
	    cell+ cell+
	    dup @ 0= if
		drop 0 exit
	    then
	then
    again
;

: replace-pattern ( codeaddr replacementaddr -- )
    begin
	2dup @       ( codeaddr replacementaddr codeaddr replacement )
	dup ?endofpattern not while
	    dup wildcardmarker = if
		2drop   \ leave question marks intact
	    else
		swap !
	    then
	    advance
    repeat
    2drop 2drop
;

defer remove-noops

: scan-and-replace ( codeaddr -- )
    dup         ( beginaddr codeaddr )
    begin
	dup @         \ beginaddr codeaddr instruction
	' eow <>      \ beginaddr codeaddr iseow
    while             \ beginaddr codeaddr
	    search-match   \ beginaddr codeaddr match/0
	    ?dup if
		2dup replace-pattern drop          \ beginaddr codeaddr
		over remove-noops    ( remove noops now to make other patterns fire right away )
	    else
		next-instruction
	    then
    repeat
    2drop
;

: copy-instr ( dest src -- dest+cell src+cell )
    2dup @ swap !
    advance
;

: remove-noop ( startaddrofmove -- )
    dup dup cell+            ( startaddrofmove currdest nextinstraddr )
    begin
	dup @ case
	    ' lit of copy-instr endof
	    ' lit+ of copy-instr endof
	    ' lit- of copy-instr endof
	    ' call of copy-instr endof
	    ' jump of copy-instr endof
	    ' var@ of copy-instr endof
	    ' var! of copy-instr endof
	    ' branch of copy-instr endof
	    ' 0branch of copy-instr endof
	    ' 1branch of copy-instr endof
	    ' eow of copy-instr 2drop drop exit endof
	endcase
	copy-instr
    again
;

: remove-noops ( wordaddr -- )
    dup                ( wordaddr currinst )
    begin
	dup @          ( wordaddr currinst opcode )
	' eow <>       ( wordaddr currinst isnoteow )
    while
	    dup @ ' noop = if    ( wordaddr currinst isnoop )
		dup 2 pick fixbranches
		dup remove-noop
	    else
		next-instruction
	    then
    repeat
    2drop
;

variable const-expr-pattern

p{ lit ? lit ? ? }p   const-expr-pattern !

: replace-const-expr ( matchedaddr op -- )
    swap
    dup cell+ @        ( op matchedaddr a )
    over 3 cells + @   ( op matchedaddr a b )
    3 pick exec-builtin ( op matchedaddr a_op_b )
    over cell+ !       ( op matchedaddr )
    2 cells +          ( op firstnoopaddr )
    dup ' noop swap !  ( op firstnoopaddr )
    cell+              ( op 2ndnoopaddr )
    dup ' noop swap !  ( op 2ndnoopaddr )
    cell+              ( op 3rdnoopaddr )
    ' noop swap !      ( op )
    drop
;

: constant-fold-replace ( codeaddr -- nomatch )
    dup 4 cells + @ case                 ( codeaddr op )
	' + of ' + replace-const-expr 0 endof
	' - of ' - replace-const-expr 0 endof
	' / of ' / replace-const-expr 0 endof
	' * of ' * replace-const-expr 0 endof
	2drop 1 exit
    endcase
;

: constant-fold-search ( codeaddr -- )
    dup         ( wordaddr codeaddr )
    begin
	dup @                ( wordaddr codeaddr instr )
	' eow <>             ( wordaddr codeaddr noteow )
    while                          ( wordaddr codeaddr )
	    dup const-expr-pattern @ match if        ( wordaddr codeaddr isamatch? )
		dup constant-fold-replace if         ( wordaddr codeaddr notreplaced? )
		    next-instruction     \ not replaced, next instruction
		else
		    over remove-noops    \ replaced, remove noops now
		then
	    else
		next-instruction
	    then
    repeat
    2drop
;

: optimize ( -- )
    latest @ >cfa
    dup constant-fold-search
    scan-and-replace
;

: opt-word immediate ( -- )
    word find >cfa
    dup constant-fold-search
    scan-and-replace
;

patterns
  p{ lit ? + }p               -> r{ lit+ ? noop }r ,
  p{ lit ? - }p               -> r{ lit- ? noop }r ,
  p{ lit 1 + }p               -> r{ 1+ noop noop }r  ,
  p{ lit 1 - }p               -> r{ 1- noop noop }r  ,
  p{ lit 1 * }p               -> r{ noop noop noop }r ,
  p{ lit 1 / }p               -> r{ noop noop noop }r ,
  p{ lit 0 + }p               -> r{ noop noop noop }r ,
  p{ lit 0 1- }p              -> r{ lit -1 noop }r ,
  p{ lit 0 1+ }p              -> r{ lit 1 noop }r ,
  p{ lit 0 - }p               -> r{ noop noop noop }r ,
  p{ lit -1 + }p              -> r{ 1- noop noop }r ,
  p{ lit -1 - }p              -> r{ 1+ noop noop }r ,
  p{ swap over }p             -> r{ tuck noop }r   ,
  p{ swap swap }p             -> r{ noop noop }r   ,
  p{ dup swap }p              -> r{ dup noop }r    ,
  p{ drop drop }p             -> r{ 2drop noop }r  ,
  p{ lit ? @ }p               -> r{ var@ ? noop }r  ,
  p{ lit ? ! }p               -> r{ var! ? noop }r  ,
  p{ swap drop swap drop }p   -> r{ 2nip noop noop noop }r ,
  p{ swap drop }p             -> r{ nip noop }r ,
  p{ nip nip nip }p           -> r{ 2nip nip noop }r ,
  p{ nip nip }p               -> r{ 2nip noop }r ,
  p{ over over }p             -> r{ 2dup noop }r ,
  p{ dup @ }p                 -> r{ dup@ noop }r ,
  p{ 0= 0branch ? }p          -> r{ noop 1branch ? }r ,
  p{ 0<> 0branch ? }p         -> r{ noop 0branch ? }r ,
  p{ r> r> }p                 -> r{ 2r> noop }r ,
  p{ >r >r }p                 -> r{ 2>r noop }r ,
  p{ >r r> }p                 -> r{ noop noop }r ,
  p{ r> >r }p                 -> r{ noop noop }r ,
  p{ rdrop rdrop }p           -> r{ 2rdrop noop }r ,
  p{ rot -rot }p              -> r{ noop noop }r ,
  p{ -rot rot }p              -> r{ noop noop }r ,
  p{ dup drop }p              -> r{ noop noop }r ,
  p{ over drop }p             -> r{ noop noop }r ,
  p{ + + }p                   -> r{ bi+ noop }r
end-patterns

s" ;" create immediate
' lit ,
' exit ,
' , ,
' lit ,
' eow ,
' , ,
' latest ,
' @ ,
' hidden ,
' call ,
' optimize ,
' [ ,
' exit ,
' eow ,

hide next-instruction
hide ?notendofword
hide ?isbranch
hide fixoffset-afterborder
hide fixoffset-beforeborder
hide fixoffset
hide fixbranches
hide save-code
hide patterns
hide end-patterns
hide save-bytecode
hide save-wildcard
hide terminate-list
hide wildcard?
hide p{
hide r{
hide ->
hide advance
hide match
hide search-match
hide replace-pattern
hide scan-and-replace
hide copy-instr
hide remove-noop
hide remove-noops
