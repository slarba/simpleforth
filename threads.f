
1024 constant default-ds-size
512 constant default-rs-size
512 constant default-ts-size

: create-thread ( xt -- )
    default-ts-size default-rs-size default-ds-size new-thread
;

: ?killed inline
    @ ;

: join ( thread -- )
    begin
	dup ?killed not while
	    pause
    repeat
    drop
;

: pause-forever
    begin
	pause
    again
;

: test1
    20 9 do
	." hello " i . cr
	pause
    loop
    kill-thread
;

: test2
    10 4 do
	." world " i . cr
	pause
    loop
    kill-thread
;

: run-tests
    ' test1 create-thread
    ' test2 create-thread
    join
    join
;
