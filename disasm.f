
( disassembler )

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
