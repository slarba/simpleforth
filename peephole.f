( peephole optimizer )

\ TODO: forward branches in remove-noop before moved block!

: save-code ( addr code -- addr+cell )
    over ! cell+
;

variable pattern-list

: patterns immediate
    here @   \ mark pattern and replacement list start. consists of cell pairs (pattern,replacement)
    ." patterns start at " dup . cr
;

: end-patterns immediate
    0 , 0 ,          \ end of list marker pair
    ." patterns end at " here @ . cr
    pattern-list !   \ save pattern and replacement list into pattern list
;

\ compile pattern into const space
: p{ immediate ( -- pattern )
    consthere @            ( patt_start )
    dup                    ( patt_start addr )
    begin
	word                 ( patt_start addr word )
	dup s" ?" strcmp 0=  ( patt_start addr word iswildcard )
	if
	    drop
	    -1 save-code
	else
	    dup s" }p" strcmp 0= if
		drop
		0 save-code
		consthere !
		exit
	    else
		find >cfa @ save-code
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
	dup s" ?" strcmp 0=
	if
	    drop
	    -1 save-code
	else
	    dup s" }r" strcmp 0=   ( replacement_start replacement word isendmarker )
	    if
		drop
		0 save-code
		consthere !
		exit
	    else
		find >cfa @ save-code
	    then
	then
    again
;

: -> immediate
    , ;

: next-instruction ( codeaddr -- nextcodeaddr )
    dup @ swap cell+ swap
    case
	' lit of cell+ endof
	' call of cell+ endof
	' branch of cell+ endof
	' 0branch of cell+ endof
	' var@ of cell+ endof
    endcase
;

: match ( codeaddr pattern -- true/false )
    begin
	over @ over @          ( codeaddr pattern bytecode patterncode )
	dup 0= if              \ if we reached pattern's end, that's a match!
	    2drop 2drop 1 exit
	else                   \ otherwise need to compare the codes
	    dup -1 = if
		\ patterncode -1 matches always
		2drop
	    else
		<> if             \ no match, exit with false
		    2drop 0 exit
		then
	    then
	    swap cell+ swap cell+    \ increment addresses
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
	dup while
	    dup -1 = if
		2drop   \ leave question marks intact
	    else
		swap !
	    then
	    swap cell+ swap cell+
    repeat
    2drop 2drop
;

: scan-and-replace ( codeaddr -- )
    begin
	dup @         \ codeaddr instruction
	' eow <>      \ codeaddr iseow
    while             \ codeaddr
	    search-match   \ codeaddr match/0
	    ?dup if
		2dup replace-pattern drop          \ codeaddr
	    else
		next-instruction
	    then
    repeat
    drop
;

: copy-instr ( dest src -- dest+cell src+cell )
    2dup @ swap !
    swap cell+ swap cell+
;

: recalc-branch-offset ( branchaddress origaddress -- branchaddress )
    swap        ( origaddress branchaddress )
    dup dup @       ( origaddress branchaddress branchaddress branchamount )
    +               ( origaddress branchaddress branchtarget )
    2 pick <= if
	\ if branch goes outside the moving block, must change the branch amount
	dup cell swap +!
    then
    swap drop   \ leave branchaddress on stack
;

: remove-noop ( startaddrofmove -- )
    dup dup cell+            ( startaddrofmove currdest nextinstraddr )
    begin
	dup @ case
	    ' lit of copy-instr endof
	    ' call of copy-instr endof
	    ' var@ of copy-instr endof
	    ' branch of copy-instr 2 pick recalc-branch-offset endof
	    ' 0branch of copy-instr 2 pick recalc-branch-offset endof
	    ' eow of 2drop drop exit endof
	endcase
	copy-instr
    again
;

: remove-noops ( codeaddr -- )
    begin
	dup @          ( codeaddr opcode )
	' eow <>       ( codeaddr isnoteow )
    while
	    dup @ ' noop = if    ( codeaddr isnoop )
		dup remove-noop
	    else
		next-instruction
	    then
    repeat
    drop
;

: optimize ( -- )
    latest @ >cfa
    dup scan-and-replace
    remove-noops
;

: opt-word immediate ( -- )
    word find >cfa
    dup scan-and-replace
    remove-noops
;

patterns
  p{ swap swap }p             -> r{ noop noop }r   ->
  p{ dup swap }p              -> r{ dup noop }r    ->
  p{ drop drop }p             -> r{ 2drop noop }r  ->
  p{ lit ? @ }p               -> r{ var@ ? noop }r  ->

end-patterns

include disasm.f

: test drop swap swap drop drop dup swap drop ;
: test2 dup swap drop  ;
: test3 dup dup drop ;

: koe drop test
    begin
	pattern-list @
	swap swap
	test2 test3
	begin
	    emit
	    drop drop
	again
    again
    swap drop
;

' koe disasm
." ------------" cr
optimize
' koe disasm

