# Custom Linux Shell

A custom shell implementation in C, providing a user-friendly command-line interface with support for job control, command history, directory navigation, and more. This shell mimics and extends basic Unix shell features, supporting both built-in and external commands, pipes, redirection, and background jobs.

## Features
  - `hop`: Change directories (supports `~`, `..`, `-`, and absolute/relative paths).
  - `reveal`: List directory contents (supports `-l` and `-a` flags).
  - `log`: View, purge, or execute from command history.
  - `activities`: List background jobs.
  - `ping <pid> <sig>`: Send a signal to a process.
  - `fg <job#>`: Bring a background job to the foreground.
  - `bg <job#>`: Resume a stopped job in the background.
  - `STOP`: Exit the shell.

## Architecture Overview

The shell is modular, with each major feature implemented in a separate C file and header. The main loop handles user input, tokenization, parsing, and dispatches commands to the appropriate module. Signal handling is integrated for job control (SIGINT, SIGTSTP).

### Main Flow
1. **Prompt**: Displayed using user, host, and directory info.
2. **Input**: Read and tokenize user input.
3. **Parse**: Validate syntax and identify command type.
4. **Dispatch**: Built-in commands are handled internally; others are executed as external processes.
5. **Job Control**: Background and stopped jobs are tracked and managed.
6. **Logging**: Commands are logged for history and replay.

## Module Explanations

- **main.c**: Main loop, signal handling, command dispatch.
- **prompt.c/h**: Prompt formatting and display.
- **parser.c/h**: Tokenizes input, parses for syntax and operators.
- **hop.c/h**: Implements `hop` (cd-like) command, manages previous/current directory.
- **reveal.c/h**: Implements `reveal` (ls-like) command, supports flags and error handling.
- **log.c/h**: Maintains command history, supports viewing, purging, and re-executing commands.
- **execute.c/h**: Handles execution of external commands, pipes, redirection, and background jobs.
- **jobs.c/h**: Tracks background and stopped jobs, supports job control commands.

## Example Session

```
<user@host:~> hop Documents
<user@host:~/Documents> reveal -la
drwxr-xr-x 2 user user 4096 Dec 24 10:00 .
drwxr-xr-x 5 user user 4096 Dec 24 09:00 ..
-rw-r--r-- 1 user user  123 Dec 24 09:30 notes.txt
<user@host:~/Documents> sleep 30 &
[1] 12345
<user@host:~/Documents> activities
[12345] : sleep 30 - Running
<user@host:~/Documents> fg 1
<user@host:~/Documents> STOP
logout
```

## Error Handling

- Invalid commands and syntax are detected and reported.
- Directory and file errors (e.g., missing directory) print clear messages.
- Handles signals robustly to avoid shell crashes.
- Prevents duplicate log entries and invalid log operations.

## Extensibility

- Add new built-in commands by creating a new module and updating the main dispatch logic.
- Modular headers and source files make it easy to extend or replace features.
- Logging and job control can be expanded for more advanced shell features.

## Developer Notes

- Follows C99 and POSIX standards for portability.
- Uses static arrays for simplicity; can be refactored for dynamic allocation.
- Designed for clarity and educational value, not for production use.
- Persistent history is stored in `.log_history` in the working directory.

## Troubleshooting

- If the shell does not start, ensure you have GCC and a POSIX-compliant environment.
- Use `make clean` to remove old binaries if you encounter build issues.
- Check file permissions if you cannot execute `shell.out`.

---

## Build Instructions

Requires GCC and a POSIX-compliant system (Linux recommended).

```sh
make
```
This produces `shell.out` in the project root.

To clean build artifacts:
```sh
make clean
```

## Usage
Run the shell:
```sh
./shell.out
```

### Example Commands
- `hop ~` — Go to home directory
- `hop ..` — Go up one directory
- `hop -` — Go to previous directory
- `reveal -la /etc` — List all files in `/etc` with details
- `log` — Show command history
- `log purge` — Clear command history
- `log execute 2` — Re-run the 2nd command in history
- `ls | grep txt > files.txt` — Use pipes and redirection
- `sleep 10 &` — Run in background
- `activities` — List background jobs
- `fg 1` — Bring job 1 to foreground
- `STOP` — Exit the shell

## Code Structure

- `src/main.c` — Main shell loop, signal handling, command dispatch
- `src/prompt.c` — Prompt display logic
- `src/parser.c` — Tokenization and parsing of input
- `src/hop.c` — Directory navigation (cd-like)
- `src/reveal.c` — Directory listing (ls-like)
- `src/log.c` — Command history management
- `src/execute.c` — Command execution, pipes, redirection
- `src/jobs.c` — Job control and background process management
- `include/` — Header files for each module
- `Makefile` — Build instructions

## Notes
- Handles signals (Ctrl+C, Ctrl+Z) for job control.
- Maintains previous and home directory for navigation.
- Persistent command log in `.log_history`.
- Designed for educational and experimental use.

---

*Developed by BManav00. For learning and research purposes.*
