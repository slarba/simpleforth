
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
