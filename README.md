# miniShell
Mini Shell is a custom shell program written in C that demostrates how Unix/Linux processes are created. System call fork() is used to create these processes. It takes no arguments and returns a process ID. The purpose of fork() is to create a new process, which becomes the child process of the caller. After a new child process is created, both processes will execute the next instruction following the fork() system call.

To Compile:
```
gcc -o test miniShell.c
```

![alt tag](https://i.imgur.com/E46U8eU.jpg)
