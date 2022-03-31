# Lock-Free and Wait-Free Queue

Lock-free and wait-free Circular queue (Ring queue).
It uses atomic operation to be lock-free.

## Performance
  
Tested on i7-1165G7  
For 1 million operations,  
normal mutex lock queue took 2sec.  
lock-free queue took only 0.1sec.  

## Feature
Written in C/C++11.  
Support multi-platform.  
Support both 32-bit and 64-bit.  

## How to use
Just copy axisqueue.h and include in the project.  
Enjoy!  

## How to run the sample
cd sample  
make  
./testqueue 

# Author
- Pit Suwongs พิทย์ สุวงศ์ (admin@ornpit.com)  
