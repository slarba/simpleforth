
( sanakirjat eli simppelit namespacet )
(
pläni:

in: vocabulary_name   ( becomes a parent for new vocabulies )

vocabulary: nimi
use: toinenvocab
use: kolmasvocab
...

definitions:
...

latest on aina nykyisen sanakirjan viimeisimmän sanan osoitin
sanakirjasta sanojen haku eli find toimii siten, että sanakirja on itse asiassa
lista latest-pointtereita. jos natiivi find ei löydä sanaa, otetaan latestiksi listan
seuraava latest, etsitään sieltä ja jos ei mistään löydy -> virhe.

)

variable current-vocab
variable latest-defined-vocab

0 current-vocab !
0 latest-defined-vocab !

\ vocabulary structure:
\    cell vocab-latest
\    cell name
\    cell next-vocab-entry
\    n cells used_vocab_pointers
\    null

\ accessors

: vocab-name ( vocabentry -- name ) cell+ @ ;
: vocab-next ( vocabentry -- nextvocabentry/0 ) 2 cells + @ ;
: vocab-latest ( vocab-entry -- latest ) @ ;
: set-vocab-name ( name vocabentry -- ) cell+ ! ;
: set-vocab-next ( nextentry vocabentry -- ) 2 cells + ! ;
: set-vocab-latest ( latest vocabentry -- ) ! ;
: vocab-useslist ( vocabentry -- useslist ) 3 cells + ;

: find-vocabulary ( name -- vocabulary/0 )
    latest-defined-vocab @         ( name latestvocab )
    begin
	dup 0= if                  \ is the entry zero?
	    2drop 0 exit           \ return zero
	else
	    2dup vocab-name strcmp   \ compare names
	then
    while
	    vocab-next
    repeat
    swap drop
;

: in: immediate
    word find-vocabulary
    ?dup if
	latest @ current-vocab @ set-vocab-latest   \ save latest to current vocabulary
	dup current-vocab !                         \ this is the new current vocabulary
	vocab-latest latest !                       \ get new latest from current vocabulary and save it to latest
    else
	." no such vocabulary" cr
    then
;

: vocabulary: immediate
    word            ( vocabname )
    make-const-str  ( constvocabname )
    consthere @     ( constvocabname vocabulary )
    2dup set-vocab-name   ( constvocabname vocabulary )
    swap drop             ( vocabulary )
    latest-defined-vocab @  ( vocabulary latestvocab )
    over set-vocab-next     ( vocabulary )  \ link them
    latest @
    over set-vocab-latest   ( vocabulary )  \ save latest
    dup latest-defined-vocab !  \ make it the last defined vocab
    vocab-useslist consthere !       \ advance consthere
;

: use:
    word find-vocabulary
    ?dup if
	const,
    else
	." no such vocabulary to use" cr
    then
;

: definitions:
    0 const,   \ terminate uses list
;

vocabulary: forth
latest-defined-vocab @ current-vocab !

definitions:

: find ( wordname -- word )
    dup find    \ try to find from current latest first
    ?dup if
	." -- löytyi suoraan" cr
	swap drop exit
    else
	." -- ei löytyny suoraan..." cr
	latest @                         ( latest )
	current-vocab @ vocab-useslist   ( latest useslist )
	begin
	    dup @                        ( latest useslist vocabentry/0 )
	while
		." -- kierros..." cr
		dup @ vocab-latest       ( latest useslist usedlatest )
		latest !                 ( latest useslist )
		2 pick                   ( latest useslist wordname)
		find                     ( latest useslist word/0 )
		?dup if
		    swap drop swap drop swap drop
		    exit
		else
		    cell+
		then
	repeat
	2drop 0
	." -- not found!" cr
    then
;

in: forth