Name: Ajay Chaudhary
Roll number: 2401MC47
Assignment: MA3105 Operating System Lab
Set number: 2

Q1:
Implemented mprotect() and munprotect() as xv6 system calls.
The kernel validates alignment, positive length, process bounds, and presence of every page before changing any PTE.
The helper clears or sets PTE_W, and the system calls flush the TLB with lcr3().
The prottest program verifies readable protected memory, a killed writing child, successful unprotection, and invalid inputs.

Q2:
Added a priority field to each process and initialize every newly allocated process to priority 10.
set_priority() validates priorities from 1 through 20, updates the process table under ptable.lock, and returns the old value.
The scheduler scans runnable processes under ptable.lock and selects the smallest priority number.
The schedtest program creates children with priorities 5, 10, and 15 and verifies their completion order before printing Done.

Screenshot evidence:
Capture and save screenshots as Output/Q1_output.png and Output/Q2_output.png.
