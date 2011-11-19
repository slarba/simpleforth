
variable total-errors
variable total-tests
variable actual-depth
variable actual-results
20 cells allot actual-results !

: begin-suite
    0 total-errors !
    0 total-tests !
;

: test-report
    cr
    ." Total tests run : " total-tests @ . cr
    ." Failed tests    : " total-errors @ . cr
;

: ->
    depth cellsize / dup actual-depth !
    ?dup if
	0 do
	    actual-results @ i cells + !
	loop
    then
;

: { tell ."  ..." space ;

: }
    1 total-tests +!
    depth cellsize / actual-depth @ = if
	depth cellsize / ?dup if
	    0 do
		actual-results @ i cells + @
		<> if
		    ." incorrect result" cr
		    1 total-errors +!
		    unloop exit
		then
	    loop
	then
    else
	." wrong number of results" cr
	1 total-errors +!
	exit
    then
    ." OK" cr
;

hide actual-depth
hide actual-results
