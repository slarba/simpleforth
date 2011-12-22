
( syntaktista sokeria vektoreille )
: v3| immediate ;
: | immediate ;
: m| immediate ;

variable anglecos
variable anglesin

: fneg inline 0.0 fswap f- ;

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
