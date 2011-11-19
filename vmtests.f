
begin-suite

s" +"    { 1 2 +   -> 3 }
s" +"    { -2 3 +  -> 1 }
s" -"    { 1 2 -   -> -1 }
s" -"    { -3 2 -  -> -5 }
s" *"    { 2 3 *   -> 6 }
s" *"    { -2 3 *  -> -6 }
s" /"    { 6 3 /   -> 2 }
s" /"    { 6 -3 /  -> -2 }
s" 1+"   { 2 1+    -> 3 }
s" 1-"   { 2 1-    -> 1 }
s" dup"  { 2 dup   -> 2 2 }
s" swap" { 2 3 swap -> 3 2 }
s" nip"  { 2 3 nip  -> 3 }
s" 2nip" { 2 3 4 2nip -> 4 }
s" 2dup" { 2 3 2dup -> 2 3 2 3 }
s" ?dup" { 1 ?dup -> 1 1 }
s" ?dup" { 0 ?dup -> 0 }
s" swapdup" { 2 3 swapdup -> 3 2 2 }
s" over" { 4 5 over  -> 4 5 4 }
s" tuck" { 4 5 1 tuck -> 4 1 5 1 }
s" drop" { 4 5 drop -> 4 }
s" rot"  { 4 5 6 rot -> 5 6 4 }
s" -rot" { 4 5 6 -rot -> 6 4 5 }
s" 2drop" { 5 6 3 2drop -> 5 }
s" bi+" { 4 5 6 bi+ -> 15 }
s" invert" { 0 invert -> -1 }
s" invert" { -1 invert -> 0 }

test-report

\ jatka!
