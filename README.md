## Welcome To Skipper
Skipper is a self acting interpreter mainly for file management. Compare, move, rename (and more) multiple files by regular expression rules. Program has simple syntax which you have to follow to execute commands correctly.
#### Key features:
- [x] Built-in basic commands
- [x] Compatible with external commands (.bat, .cmd, .exe and more!)
- [x] Input and output redirection (e.g. "<< args.txt" or ">> output.txt")
- [x] Concurrent or singe-threaded execution structure
- [ ] Control of optimisation level (nodes and its "order", currently without user decision)

## Simple Syntax - {}, [], # and !
**Regular scope** followed by ```{}``` - everything inside this scope will be executed line by line. The restriction is that regular scope cannot be next to the concurrent scope. Parent can be either a regular scope or the concurrent scope (no limit here). Every execution starts with global scope as a regular scope itself.<br><br>
**Concurrent scope** followed by ```[]``` brackets - run on the another thread, its insides are executed line by line. Regular scope can be the only parent for concurrent scope. Concurrent scope next to regular scope is not permitted.<br><br>
**Comments** followed by ```#``` - everything after is treated as comment i.e. skipped by interpreter.<br><br>
**Commands** followed by ```!``` - to distinguish regular text commands start with ```!<command name> <command args> ...``` Note that some command arguments can be mixed so instead of calling ```!command -a -b -c``` use shorter version ```!command -abc```<br>

## Examples
```
  # args.txt
  # This file contains regular expression pattern (argument).
  [a-zA-Z0-9]+\.log
  
  # file.txt
  # Example of !list command and !remove command.
  # Example of concurrent and regular scope.
  
  # Global Scope
  { # Regular Scope 0
    [ # Concurrent Scope 0.0
      !list -r .*\.php$ >> phps.log # Search recursively for all .php files and redirect output to the phps.log
      !remove -r -v << args.txt # Search recursively (with verbose option) and remove all files that matches regular expression pattern defined in args.txt.
    ]
    [ # Concurrent Scope 0.1
      # Do other things
      # [] # <- syntax error, concurrent scope cannot be a child of concurrent scope
    ]
    # Scopes 0.0 and 0.1 will run simultaneously (on different threads).
    # {} # <- syntax error, regular scope next to concurrent scope is not allowed
  }
  
  { # Regular Scope 1
    [ # Concurrent Scope 1.0
      { # Regular Scope 1.0.0
        [ # Regular Scope 1.0.0.0
          {} # Regular Scope 1.0.0.0.0
          {} # Regular Scope 1.0.0.0.1
          # [] # <- syntax error, concurrent scope next to regular scope is not allowed
        ]
      }
    ]
    
    [ # Concurrent Scope 1.1
    ]
    # Scopes 1.0 (or 1.0.0.0) and 1.1 will run simultaneously (on different threads).
  }
```
```
  # foo.log
  # ... some work
```
```
  # main.log
  [ # Concurrent Scope A
    # Do work A
    !run -d "directory" --file a.bat >> out.txt # Execute a.bat batch file from "directory" directory and redirect (append) output to the out.txt file.
    !include foo.log # Paste the code from foo.log
  ]
  [ # Concurrent Scope B
    # Do work B
    !run -r -R [a-z].bat # Search recursively for files that matches regular expression and execute all of them.
    !include foo.log # Paste the code from foo.log
  ]
```
## Command's [list](https://github.com/devmichalek/Skipper/blob/master/COMMANDS.md)
To get more information about particular command call ```!<command name> --help```. To redirect command to the file use the following syntax: ```!<command name> <command args> >> <file name>```. **>>** means **append to file**, **>** is **overwrite file**. To insert additional arguments to the command from file use syntax: ```!<command name> <command args> << <file name>```. **<<** means **append arguments from file**, **<** is **overwrite existing arguments on the left with the ones from file**.
## Project Structure
Project was made with Visual Studio 2017. Interpreter is partly managed by Bison & Flex (parsing of the command to be more specific). Code uses the newest C++17 standard (filesystem) as well as previous versions (regex and many other features such as multithreading).
#### Quick Download
- [Windows x64](https://drive.google.com/file/d/1ins6giqYEQGGA4nCfc9X2YU9ZcCRhYm6/view?usp=sharing)<br>
- [Windows x86](https://drive.google.com/file/d/1Wy1yFj3-iLisy3W2FG2WjtOzdRk0BVq4/view?usp=sharing)
