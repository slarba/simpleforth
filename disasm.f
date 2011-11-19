
( disassembler )

(
vocabulary disassembler

definitions
)

: next-opcode ( opcodeaddr -- opcodeaddr opcode )
    dup @ swap cell+ swap ;

: print-call-target ( jumpaddr -- )
    hdrsize - id.
;

: print-address ( addr -- )
    ." 0x" hex u. decimal 4 spaces ;

variable firstbuiltin

: find-first-builtin ( -- )
    latest @
    begin
	dup ?builtin not
    while	    
	    cell+ @
    repeat
    firstbuiltin !
;

find-first-builtin

: find-bytecode ( bytecode -- dicthdr )
    firstbuiltin @     ( bytecode dictentry )
    begin
	2dup >cfa @ <>   ( bytecode dictentry issame? )
    while
	    cell+ @
    repeat
    nip
;

: ?hasarg ( dict-entry -- true/false )
    @ f_hasarg and ;

: ?iscall ( dict-entry -- true/false )
    >cfa @ ' call = ;

: disasm-next-instr ( xt -- nextopcodeaddr )
    dup print-address
    next-opcode
    find-bytecode dup cell+ cell+ tell
    dup ?iscall if
	drop
	space next-opcode print-call-target
    else
	?hasarg if
	    space next-opcode .
	then
    then
;

: disasm ( xt -- )
    begin
	dup @
	' eow <>
    while
	    disasm-next-instr cr
    repeat
    drop
;

hide disasm-next-instr
hide next-opcode
hide print-call-target
hide print-address

(
in: forth
)
