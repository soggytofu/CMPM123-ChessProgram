# Chess AI Implementation Project

## 🎯 Project Overview

Oh yeah, if your reading this, could just a reminder for a regrade for the previous assignemnt.
I TA for another class, I totally forget about regrading assignments, so I just put it here :P
Thank you very much

For this assignment, I added the fen functionality, castling, pawn promotion, and en pessant.

For fen string functionality is line 58, where you can use the function loadFromFEN and put in a 
fen string. The function for it is right below on line 67. 

For Castling, theres a check for each rook in the h file on line 57-62, Each one is labeled 
color-Rook-side, so whiteRookRight would be an example to see if the right most white rook has
moved to be allowed to castle. Also, there is a check for the white and black king to see if 
it has moved at all either. Caslting is dealt in the function bitMovedFromTo and Katt where it
starts from line 614 on the king attack. There is a check to see if both pieces for a castle
has not moved, to then allow it. Finally, on line 309, you can see if the king funcitonality to
allow the king to swap with the rook and disable castling for other pieces.

Castling is a quick check to see if a pawn in bitMovedFromTo on line 286 to see if a pawn moved 
has reached the end of the board, then create a queen in its place.

Finally, en pessant is checked first to see if a pawn has moved 2 spaces recently in bitMovedFromTo
on line 282. Then, in patt on line 530, you check to see if in its attack, and if that en pessant
piece is directly left or right, then allow for the attack. Finally, you go back to bitMovedFromTo
and delete it accordingly.

