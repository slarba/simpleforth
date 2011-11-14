
vocabulary lists
use classes

definitions

class: builder <base object
  method builder-append
  method builder-finish
endclass

: ,, ( builder element -- builderwithnewelement )
    swap builder-append send
;

: } ( builder lastelement -- builtobject )
    ,,
    builder-finish send
;

class: iteratorbase <base object
  method next-element ( -- nextelement )
  method next-exists? ( -- hasnext? )

  m: false ; implements next-exists?
  m: 0 ; implements next-element
endclass

( data structure object, common methods for all )
class: dsobject <base object
  method count    \ number of elements in this structure
  method print    \ print object
  method iterator  \ construct iterator
endclass

defer listnode-next
  
class: listiterator <base iteratorbase
  cell var current-element

  m: self current-element ! ; implements construct
  m: self current-element @ listnode-next @
      self current-element !
  ; implements next-element
  m: self current-element @ ; implements next-exists?
  m: s" <listiterator>" ; implements tostring
endclass
  
class: listnode <base dsobject
  cell var listnode-next
  cell var listnode-data

  m:
      0 self listnode-next !
       self listnode-data !  
  ; implements construct

  m:
      s" <listnode>"
  ; implements tostring

  m: self listiterator new ; implements iterator
  
  m:
      0 self
      begin
	  dup
      while
	      swap 1+ swap
	      listnode-next @
      repeat
      drop
  ; implements count
  
  m:
      ." [ "
      self
      begin
	  dup
      while
	      dup listnode-data @ .
	      listnode-next @
      repeat
      drop
      ." ]"
  ; implements print
endclass

class: listbuilder <base builder
  cell var first-list-node
  cell var latest-list-node

  m: ( itemtoadd -- builder )
      \ ." listbuilder append" cr
      listnode new               ( newnode )
      dup                        ( newnode newnode )
      self latest-list-node @    ( newnode newnode latestnode )
      ?dup if
	  \ ." adding..." cr
	  listnode-next !        ( newnode )
	  self latest-list-node !
      else                       ( newnode newnode )
	  \ ." adding first" cr
	  self first-list-node !
	  self latest-list-node !
      then
      self
  ; implements builder-append
  
  m: self first-list-node @
      \ ." listbuilder finish" cr
  ; implements builder-finish

  m: 0 self first-list-node !
      0 self latest-list-node !
      \ ." listbuilder construct" cr
  ; implements construct

  m: \ ." listbuilder destruct" cr
  ; implements destruct
  
  m: s" <listbuilder>" ; implements tostring
endclass

: l{ ( -- listbuilder )
    listbuilder new
;

: each ( iterable xt -- )
    swap iterator send         ( xt iterator )
    begin
	dup current-element @ ( xt iterator thereismore )
    while
	    dup current-element @ listnode-data @ ( xt iterator element )
	    swap >r                ( xt element )
	    swap dup >r            ( element xt )
	    execute
	    r> r>
	    dup next-element send
    repeat
    2drop
;

: lisaalistaan ( builder xt element -- )
    over execute       ( builder xt computedelement )
    swap               ( builder computedelement xt )
    >r                 ( builder computedelement )
    ,,                 ( builder )
    r>                 ( builder xt )
;

: map ( iterable xt -- )
    >r >r
    l{                 ( builder )
      r> r> swap       ( builder xt iterable )
      ' lisaalistaan each    ( builder xt iterable lisaalistaanxt )
      drop   \ xt
    builder-finish send
;

: listsum ( list -- )
    l{ 1 ,, 2 ,, 3 ,, 4 }
    :lambda
      dup 3 = if
	1+
      then
    ;; map print send
;

: pituus count send . cr ;

: listlens
    l{
      l{ 1 ,, 2 ,, 3 } ,,
      l{ 1 } ,,
      l{ 3 ,, 4 }
    } ' pituus each
;

: listatest
    l{ 1 ,, 2 ,, 3 ,, 4 }
    dup print send cr
    dup count send . cr
    iterator send tostring send tell cr
;

in: forth
