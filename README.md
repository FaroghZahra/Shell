# Shell
Linux Shell Code in C language. Shell is implemented version wise.
# version 1
Takes input from command line. tokenize the command line separating on spaces and pass the tokenized input to execute function that executes the command if it exists using fork and exec() calls.
# version 2
Added functionality of maintains internal commands like help, exit and cd
# version 3
Added functionality of input output redirection using dup2 call and functionality of pipe operator done using pipe call. Can maintain upto 10 pipes.
# version 4
added functionality to run multiple commands on single line separated by semi colons and able to execute commands in background if & is given at end of command.
# version 5
added functionality to maintain command history and accessing them using !n command. added internal(build-in) command of "history". history is maintained using 2D array.
# version 6
added functionality of if else block; can use basic operators for comparision. e.g -eq, -gt, -le etc. Need to provide space on both sides of equation in square bracket, as in shell. The input of if else block is stopped when given "fi" as in actual shell.
