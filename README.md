FailSafeBoard14
===============

SPECIFICATIONS
--------------
Read in a PPM signal (8 channel), representing RC control signals.  Synthesize a PPM signal (8 channel), representing the failsafed condition.  Analyze the RC PPM signal to determine if there is a loss of RC communications and/or loss of RC tranciever health.

CURRENT TODO
------------

CHANGELOG
---------
12/9/13
-Add date to header section (FailSafeBoard)
-Change int flags to char flags (FailSafeBoard)
-Combined control register writes (FailSafeBoard)
-Moved sei() to end of setup() (FailSafeBoard)
-Disabled flag status logic (FailSafeBoard)