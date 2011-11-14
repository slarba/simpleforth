
variable exit-debugger
variable debugger-returnaddr

: breakpoint
    ." -- in debugger, say 'continue' to resume" cr
    0 exit-debugger !
    rsp@ @ debugger-returnaddr !
    begin
	exit-debugger @ 0=
    while
	    interpret
    repeat
    ." debugger finished" cr
;

: continue immediate
    1 exit-debugger !
;

: disasm-current
    debugger-returnaddr @ disasm
;
