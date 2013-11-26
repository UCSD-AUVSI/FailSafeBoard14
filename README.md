FailSafeBoard14
===============

SPECIFICATIONS
--------------
Read in a PPM signal (8 channel), representing RC control signals.  Synthesize a PPM signal (8 channel), representing the failsafed condition.  Analyze the RC PPM signal to determine if there is a loss of RC communications and/or loss of RC tranciever health.

CURRENT TODO
------------
- Analysis of RC PPM Health
-- Use TIMER0_OVF_vect
- Enable Serial Communication
- Determine if can pass through RC PPM