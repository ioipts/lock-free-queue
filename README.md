# Lock-Free and Wait-Free Queue

การเก็บข้อมูลเป็นคิว (เข้าก่อนออกที่หลัง) ซึ่งสามารถใช้กับมัลติเธรดได้  
แต่ไม่ต้องมีตัวล็อคแต่อย่างใด ช่วยเพิ่มความเร็วเมื่อเทียบกับวิธีล็อคแบบปกติ  
ถึง 20 เท่า ซึ่งเธรดตัวเขียนข้อมูลลงคิว และเธรดตัวอ่านข้อมูลออกจากคิว สามารถมีได้หลายเธรดด้วย  

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
