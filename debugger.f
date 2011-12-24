
variable exit-debugger
variable debugger-returnaddr

: breakpoint
    ." BREAKPOINT, say 'continue' to resume or 'break' to reset" cr
    0 exit-debugger !
    rsp@ @ debugger-returnaddr !
    begin
	exit-debugger @ if
	    exit
	then

	format-debugger-prompt input-stream @ prompt

	begin
	    input-stream @ ?eol not
	while
		interpret
	repeat
	cr
    again
;

: continue immediate
    1 exit-debugger !
;

: break immediate
    1000 throw
;

: disasm-current
    debugger-returnaddr @ disasm
;

\ :noname 1000 throw ; debugger-vector !
' breakpoint debugger-vector !
