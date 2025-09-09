# Shell Implementation

## Features

-   **Command Execution**
    -   Supports execution of system commands from `/bin`, `/usr/bin`,
        etc.
    -   Background execution using `&`.
-   **Built-in Commands**
    -   `cd` : Change directory (supports `~` and `-`).
    -   `pwd` : Print current working directory.
    -   `ls [-a/-l/-al] [dir]` : List directory contents with optional
        flags.
    -   `echo [args...]` : Print arguments to stdout.
    -   `exit` : Exit the shell.
    -   `history [n]` : Show last `n` commands (default 10).
    -   `pinfo <pid>` : Show process info (status, memory, executable
        path).
    -   `search <filename>` : Search for a file in the current
        directory.
-   **I/O Redirection**
    -   Input: `< file`
    -   Output: `> file`
    -   Append: `>> file`
-   **Piping**
    -   Supports multiple pipes (e.g., `ls -l | grep txt | wc -l`).
-   **Autocompletion**
    -   Tab-completion for system commands.
    -   Filename autocompletion inside arguments.
-   **Signal Handling**
    -   Handles `Ctrl+C` and `Ctrl+Z` for foreground processes.
-   **History Management**
    -   Command history stored in `.shell_his` file (limited to 20
        entries).

------------------------------------------------------------------------

## Compilation

``` bash
g++ shell.cpp -lreadline -o myshell

------------------------------------------------------------------------

## Usage

Run the shell with:

``` bash
./myshell


Example commands:

ls -al
cd ..
pwd
echo Hello World
ls | grep cpp | wc -l
pinfo 1234
search main.cpp
history 5

------------------------------------------------------------------------

## File Structure

- `shell.cpp` : Main source file
- `.shell_his` : Stores command history



