#include "kernel/types.h" // Includes basic types like int, uint, etc.
#include "kernel/stat.h"  // Includes declarations for stat structure (though not strictly needed here).
#include "user/user.h"    // Includes user-space system call prototypes like fork, pipe, read, write, exit, printf.

int main(){ // Main function starts execution.

int ptc[2]; // Declares 'ptc' (Parent To Child) pipe array: [0] is read, [1] is write.
int ctp[2]; // Declares 'ctp' (Child To Parent) pipe array: [0] is read, [1] is write.
char buf[1]; // Buffer to hold the single byte being passed back and forth.
int i;      // Loop counter variable.

if(pipe(ptc) < 0 || pipe(ctp) < 0){ // Creates the two pipes; checks if either pipe creation failed.
fprintf(2, "pipe declaration failed\n"); // Prints error message to standard error (fd 2).
exit(1); // Exits the process with an error code.
}
int pid = fork(); // Creates a new child process; pid is 0 in child, child's PID in parent.


if(pid == 0){ // Checks if the current process is the child (pid == 0).
close(ptc[1]); // CHILD: Closes the write end of the Parent To Child pipe (unused by child).
close(ctp[0]); // CHILD: Closes the read end of the Child To Parent pipe (unused by child).

for(i=0;i<10;i++){ // Loop starts, repeats the ping-pong cycle 10 times.
read(ptc[0],buf,1); // CHILD: Reads 1 byte from Parent To Child pipe; blocks until parent writes (Synchronization).
printf("Child recieved ping (iteration %d)\n", i+1); // CHILD: Prints the receive message.
printf("child sent pong (iteration %d)\n", i+1); // CHILD: Prints the send message.
write(ctp[1],"x",1); // CHILD: Writes 1 byte to Child To Parent pipe, sending the "pong" back.
}

close(ptc[0]); // CHILD: Closes the remaining read end of the Parent To Child pipe.
close(ctp[1]); // CHILD: Closes the remaining write end of the Child To Parent pipe.
exit(0); // CHILD: Exits the process successfully.
}
else{ // If pid is not 0, this is the parent process.
close(ptc[0]); // PARENT: Closes the read end of the Parent To Child pipe (unused by parent).
close(ctp[1]); // PARENT: Closes the write end of the Child To Parent pipe (unused by parent).

for(i=0;i<10;i++){ // Loop starts, repeats the ping-pong cycle 10 times.
printf("parent sent ping (iteration %d)\n", i+1); // PARENT: Prints the send message.
write(ptc[1],"x",1); // PARENT: Writes 1 byte to Parent To Child pipe, sending the "ping" first.
read(ctp[0],buf,1); // PARENT: Reads 1 byte from Child To Parent pipe; blocks until child writes (Synchronization).
printf("parent recieved pong (iteration %d)\n", i+1); // PARENT: Prints the receive message.
}

wait(0); // PARENT: Waits for the child process (pid) to exit before continuing.
close(ptc[1]); // PARENT: Closes the remaining write end of the Parent To Child pipe.
close(ctp[0]); // PARENT: Closes the remaining read end of the Child To Parent pipe.
exit(0); // PARENT: Exits the process successfully.
}
}
