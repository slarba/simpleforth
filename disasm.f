
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

: find-orig-builtin ( wordname -- dictentry )
    find
    begin
	dup ?builtin not
    while
	    cell+ @
    repeat
;

: '' immediate
    word find-orig-builtin
    >cfa @
    ' lit , ,
;

: disasm-next-instr ( xt -- nextopcodeaddr )
    dup print-address
    next-opcode
    case
	'' call  of ." call <" next-opcode print-call-target ." >" endof
	'' jump of ." jump <" next-opcode print-call-target ." >" endof
	'' die     of ." die" endof
	'' exit    of ." exit" endof
	'' branch  of ." branch ( " next-opcode . ." )" endof
	'' 0branch  of ." 0branch ( " next-opcode . ." )" endof
	'' 1branch  of ." 1branch ( " next-opcode . ." )" endof
	'' lit     of ." lit " next-opcode . endof
	'' lit+    of ." lit+ " next-opcode . endof
	'' lit-    of ." lit- " next-opcode . endof
	'' var@    of ." var@ " next-opcode . endof
	'' var!    of ." var! " next-opcode . endof
	'' dup  of ." dup" endof
	'' dup@ of ." dup@" endof
	'' 2dup  of ." 2dup" endof
	'' nip   of ." nip" endof
	'' 2nip  of ." 2nip" endof
	'' ?dup  of ." ?dup" endof
	'' swap  of ." swap" endof
	'' swapdup  of ." swapdup" endof
	'' drop  of ." drop" endof
	'' 2drop  of ." 2drop" endof
	'' /mod of ." /mod" endof
	'' 2>r of ." 2>r" endof
	'' 2r> of ." 2r>" endof
	'' >r  of ." >r" endof
	'' r>  of ." t>" endof
	'' >t  of ." >t" endof
	'' t>  of ." r>" endof
	'' tuck of ." tuck" endof
	'' rdrop of ." rdrop" endof
	'' 2rdrop of ." 2rdrop" endof
	'' rsp@  of ." rsp@" endof
	'' rsp!  of ." rsp!" endof
	'' tsp@  of ." tsp@" endof
	'' tsp!  of ." tsp!" endof
	'' over  of ." over" endof
	'' rot  of ." rot" endof
	'' -rot  of ." -rot" endof
	'' find  of ." find" endof
	'' number of ." number" endof
	'' create  of ." create" endof
	'' iword of ." iword" endof
	'' iexecute of ." iexecute" endof
	'' exec-builtin of ." exec-builtin" endof
	'' word  of ." word" endof
	'' key  of ." key" endof
	'' emit  of ." emit" endof
	'' tell  of ." tell" endof
	'' latest  of ." latest" endof
	'' ] of ." ]" endof
	'' [ of ." [" endof
	'' 1+  of ." 1+" endof
	'' 1-  of ." 1-" endof
	'' +!  of ." +!" endof
	'' -!  of ." -!" endof
	'' +  of ." +" endof
	'' bi+ of ." bi+" endof
	'' - of ." -" endof
	'' *  of ." *" endof
	'' /  of ." /" endof
	'' <  of ." <" endof
	'' >  of ." >" endof
	'' =  of ." =" endof
	'' <>  of ." <>" endof
	'' <=  of ." <=" endof
	'' >=  of ." >=" endof
	'' 0=  of ." 0=" endof
	'' 0<>  of ." 0<>" endof
	'' 0>  of ." 0>" endof
	'' 0<  of ." 0<" endof
	'' mod  of ." mod" endof
	'' invert  of ." invert" endof
	'' and  of ." and" endof
	'' or  of ." or" endof
	'' xor  of ." xor" endof
	'' lshift  of ." lshift" endof
	'' rshift  of ." rshift" endof
	'' >cfa  of ." >cfa" endof
	'' ,  of ." ," endof
	'' dsp@  of ." dsp@" endof
	'' dsp!  of ." dsp!" endof
	'' @  of ." @" endof
	'' c@  of ." c@" endof
	'' !  of ." !" endof
	'' c!  of ." c!" endof
	'' interpret  of ." interpret" endof
	'' hidden  of ." hidden" endof
	'' execute  of ." execute" endof
	'' '  of ." '" endof
	'' noop of ." noop" endof
	'' malloc  of ." malloc" endof
	'' mrealloc of ." mrealloc" endof
	'' mfree  of ." mfree" endof
	'' open-file of ." open-file" endof
	'' close-file of ." close-file" endof
	'' ?eof of ." ?eof" endof
	'' ?eol of ." ?eol" endof
	'' ccopy of ." ccopy" endof
	'' cmove of ." cmove" endof
	'' c@c! of ." c@c!" endof
	'' strcpy of ." strcpy" endof
	'' strlen of ." strlen" endof
	'' strcmp of ." strcmp" endof
	'' prompt of ." prompt" endof
	'' dcnew of ." dcnew" endof
	'' dcfree of ." dcfree" endof
	'' dcreset of ." dcreset" endof
	'' dcbool of ." dcbool" endof
	'' dcchar of ." dcchar" endof
	'' dcshort of ." dcshort" endof
	'' dcint of ." dcint" endof
	'' dclong of ." dclong" endof
	'' dcptr of ." dcptr" endof
	'' dccallvoid of ." dccallvoid" endof
	'' dccallbool of ." dccallbool" endof
	'' dccallchar of ." dccallchar" endof
	'' dccallshort of ." dccallshort" endof
	'' dccallint of ." dccallint" endof
	'' dccalllong of ." dccalllong" endof
	'' dccallptr of ." dccallptr" endof
	'' dcloadlib of ." dcloadlib" endof
	'' dcfreelib of ." dcfreelib" endof
	'' dcsymbol of ." dcsymbol" endof
	." ???unknown???"
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
hide ''
hide find-orig-builtin

(
in: forth
)
