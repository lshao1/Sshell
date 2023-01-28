General approach

1.  Command runner: does individual commands

2.  User HUD

3.  Parse inputs: handle piping inputs, file redirection and background
    jobs

4.  Piped inputs runner

5.  Writing into files

6.  Opening background jobs

7.  Error handling

Command runner

The purpose of the command runner is to run individual commands. For
example, the input "echo hello world \| grep hello \| wc -l" can be
broken down into three commands: "echo hello world", "grep hello", and
"wc-l".

> \# inputs that exist in the system
>
> ..\*inputs that exist in the system can be called by the execute
> functions
>
> ..\*to choose the right execute function to call, we looked at execv
> and execvp functions and found that the execvp functions are better
> when it comes to this project.
>
> ..\*this is because the execvp function does not need an additional
> argument, which is "\\bin".
>
> \# inputs that we need to implement
>
> ..\*inputs that we need to implement are the pwd, cd, and exit.
>
> ##pwd
>
> ..\* we wrote a function for pwd, called my_pwd(). This function calls
> the function getcwd, which gives me which directory I am currently at.
>
> ##cd
>
> ..\* we wrote a function for cd called my_cd(). This function gets the
> arguments of cd, which can be commands such as "..". We used chdir
> function and passed in the cd arguments.
>
> ##exit
>
> ..\* exit function is implemented in the main process. If the shell
> encounters exit function, it will immediately exit the main process.

User HUD

> The user hud serves the purpose of showing the user "sshell ucdavis"
> and receiving the inputs from the user.
>
> ..\* this is done in main
>
> ..\* we have a while loop that runs infinitely. Every time the user
> calls a command, it will be retrieved using fget function.
>
> ..\* The hud runs infinitely until exit is called

Parsing inputs

1.  Separating pipes

> ..\* to separate pipes, I need to separate the string input based off
> of character '\|'
>
> ..\* so I made a function called command parser
>
> ..\* it uses strtok to split the input

2.  Getting rid of space and making the command

> ..\* I then separate each individual segments of the pipes based on
> spaces
>
> ..\* the first item is always command, the second and so on is going
> to be the args
>
> ..\* so I concatenate the args together
>
> ..\* the end result is an array of string that represents
> cmd,args,cmd,args...

3.  Parsing redirect input

> ..\* redirect input is read when the command has a \> in the middle
>
> ..\* the redirect portion is discarded in the arguments portion
>
> ..\* it is later specified to the pipe runner whether if there is
> redirection and the file name

4.  Parsing background input

> ..\* if there is a & at the end, then this is a background process

Piped input runner

> The piped input runner runs each individual part of the command, when
> it is given the command and argument combination as an array of string
>
> It takes the first command argument combination, run it and give the
> results to the next
>
> The piped input runner will give a string as the final result that is
> supposed to be printed
>
> I need it to be a string because it needs to be stored if the command
> is supposed to run in the background

1.  Opening up multiple processes

> ..\* there is a mother process that is responsible for forking all the
> children
>
> ..\* each child is assigned a number and that indicates which command
> it is running
>
> ..\* the child will run its command and arguments with the addition of
> a piped input from another child that serves as the stdin.
>
> ..\* the mother process then waits for every single child at the end
> and records the output of the processes. Once all of the children are
> done, the piping is considered finished.

2.  Piping results

> ..\* in the beginning, the mother process will open the appropriate
> amount of pipes
>
> ..\* the pipes are then used by each child
>
> ..\* the processnumber of each child indicates which pipes it will use
>
> ..\* the child uses its pipe to write to and read from the previous
> pipe
>
> ..\* the final result is also piped, then read into a buffer

Write into file

The parameters for the pipe runner needs 3 more inputs. One is a
redirection bit that indicates whether if we need to be redirecting. One
is a truncate bit that indicates whether if the file needs to be
truncated. The last is the file path.

..\* if there is redirecting, then the last output of the pipe needs to
put its result into the file.

..\* the file needs a new fd because I will be hooking the output of the
last process to the file

..\* this is done by checking a child to see whether if it is the last
process.

..\* the output of that process will then be hooked as a pipe to the
file that it needs to write to.

..\* to differentiate between truncate or not truncate, if the truncate
bit is set, then fd2(the file descriptor for the file) will be opened
with o_trunc, or else it will be opened with o_append.

Error handling

Missing command

..\* there can be redirection missing command, which is solved when
analyzing the \> character. If what comes before the \> character is
empty, then there is a missing command

..\* alternatively, the pipes can tell me how many commands there are.
So I made a function to find how many pipe character '\|' there are. The
number of pipe characters plus1 is the amount of commands that we are
expecting.

No output file

..\* no output file occurs if what is after the \> symbol is empty.

Cannot open output file

..\* if there are special characters in the file path, then the file
cannot be opened

Mislocated output redirection

..\* for this one, I need to find the occurrence of \> and the
occurrence of the last pipe symbol. If the \> comes before the pipe
symbol, then the output direction is mislocated.

Command does not exist

..\* if the command does not exist, then execvp will return a -1. If I
see the -1 output, I will print the error and manually exit with the
value -1.

Too many arguments

..\* too many arguments occurs when the arguments for one of the
commands exceeds 12.

..\* since my command and args is put together into an array as
cmd,args,cmd,args, I can just look at the odd number elements of the
whole command: 1, 3, 5, 7

..\* this means that as long as I can take a look at the how many spaces
exist in each args, if there are 12 spaces, that means that there are 13
arguments and therefore would fail.
