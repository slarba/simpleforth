
( syntaktista sokeria vektoreille )
: v3| immediate ;
: | immediate ;
: m| immediate ;

fvariable anglecos
fvariable anglesin

: fneg inline 0.0 fswap f- ;

: v3. -frot fswap ." v3| " f. space f. space f. ."  |" ;

: fpick ( n -- nthelement )
    fsp@ swap floatsize * + f@ ;

: m3.
    8 fpick f. space
    7 fpick f. space
    6 fpick f. cr
    5 fpick f. space
    4 fpick f. space
    3 fpick f. cr
    2 fpick f. space
    1 fpick f. space
    0 fpick f. cr

    9 times fdrop
;

: computeanglesincos inline
    fdup fsin anglesin f!
    fcos anglecos f!
;

: zrotmat ( angleinradians -- matrix )
    computeanglesincos

 m|  anglecos f@          anglesin f@   0.0  |
  |  anglesin f@ fneg     anglecos f@   0.0  |
  |  0.0                  0.0           1.0  |
;

: xrotmat ( angleinradians -- matrix )
    computeanglesincos

  m|  1.0    0.0                  0.0          |
   |  0.0    anglecos f@          anglesin f@  |
   |  0.0    anglesin f@ fneg     anglecos f@  |
;

: yrotmat ( angleinradians -- matrix )
    computeanglesincos

  m|  anglecos f@      0.0      anglesin f@ fneg  |
   |  0.0              1.0      0.0               |
   |  anglesin f@      0.0      anglecos f@       |
;

: identitymat ( -- matrix )
    m| 1.0  0.0  0.0 |
     | 0.0  1.0  0.0 |
     | 0.0  0.0  1.0 |
;

: normalize ( vector -- normalizedvector )
    fdupvec v3len^2 fsqrt v3s/
;
