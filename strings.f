
: strappend ( a b -- ab )
    swap 2dup     ( b a b a )
    strlen 1+     ( b a b alen+1 )
    swap strlen + ( b a blen+alen+1 )
    malloc-atomic     ( b a buffer )
    strcpy
    strcat
;

: strleft ( str amt -- cutstr )
    dup malloc-atomic   ( str amt buffer )
    swap strncpy
;

: strright ( str amt -- cutstr )
    swap dup strlen         ( amt str lenofstr )
    rot                     ( str lenofstr amt )
    - +
    dup strlen 1+ malloc-atomic
    strcpy
;

: strdup ( a -- copyofa )
    dup strlen 1+   ( a alen+1 )
    malloc-atomic   ( a newbuf )
    strcpy
;

: format ( ... fmtstring -- formattedstring )
    
;
