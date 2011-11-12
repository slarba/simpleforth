
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

class: listnode <base object
  cell var listnode-next
  cell var listnode-data

  method printtaa

  m:
      0 self listnode-next !
       self listnode-data !  
  ; implements construct

  m:
      s" <listnode>"
  ; implements tostring

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
  ; implements printtaa
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

: listatest
    l{ 1 ,, 2 ,, 3 ,, 4 } printtaa send
;

\ in: forth
