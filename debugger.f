
variable exit-debugger
variable debugger-returnaddr

: breakpoint
    ." BREAKPOINT, say 'continue' to resume" cr
    0 exit-debugger !
    rsp@ @ debugger-returnaddr !
    begin
	exit-debugger @ if
	    exit
	then

	." [" depth cell / . current-vocab @ vocab-name tell
	s" ] DEBUG> " input-stream @ prompt

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

: disasm-current
    debugger-returnaddr @ disasm
;
