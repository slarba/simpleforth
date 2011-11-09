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

: save-bytecode find >cfa @ save-code ;
: save-wildcard -1 save-code ;
: terminate-list 0 save-code consthere ! ;
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

: advance ( addr1 addr2 -- addr1+cell addr2+cell )
    swap cell+ swap cell+ ;

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
	dup while
	    dup -1 = if
		2drop   \ leave question marks intact
	    else
		swap !
	    then
	    advance
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
    advance
;

: notendofword? ( codeaddr -- true/false ) @ ' eow <> ;
: isbranch? ( codeaddr -- true/false ) dup @ ' branch = @ ' 0branch = + ;

: update-branch-offsets ( divider startingaddr -- )
    dup           ( divider startingaddr currentaddr )
    begin
	dup notendofword?
    while
	    dup isbranch? if
		dup cell+ @   ( divider startingaddr currentaddr branchoffset )
		over +        ( divider startingaddr currentaddr branchtarget )
		dup 4 pick    ( divider startingaddr currentaddr )
	    else
	    then
    repeat
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

