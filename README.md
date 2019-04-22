## Welcome To Skipper
Skipper is a self acting interpreter mainly for file management. Compare, move, rename (and more) multiple files by regular expression rules. Program has simple syntax which you have to follow to execute commands correctly.
#### Key features:
- [x] Built-in basic commands
- [x] Compatible with external commands (running batch, exe etc.)
- [x] Input and output redirection
- [x] Concurrent or singe-threaded execution structure
- [ ] Control of optimisation level (nodes and its "order", currently without user decision)

## Simple Syntax - {}, [], # and !
**Regular scope** followed by ```{}``` - everything inside this scope will be executed line by line. The restriction is that regular scope cannot be next to the concurrent scope. Parent can be either a regular scope or the concurrent scope (no limit here). Every execution starts with global scope as a regular scope itself.<br><br>
**Concurrent scope** followed by ```[]``` brackets - run on the another thread, its insides are executed line by line. Regular scope can be the only parent for concurrent scope. Concurrent scope next to regular scope is not permitted.<br><br>
**Comments** followed by ```#``` - everything after is treated as comment i.e. skipped by interpreter.<br><br>
**Commands** followed by ```!``` - to distinguish regular text commands start with ```!<command name> <command args> ...``` Note that some command arguments can be mixed so instead of calling ```!command -a -b -c``` use shorter version ```!command -abc```<br>

## Examples
```
  # file.txt
  # Global Scope
  { # Regular Scope 0
    [ # Concurrent Scope 0.0
      !list -r .*\.php$ >> phps.log # Search recursively for all .php files and redirect output to phps.log
      !remove -r -v << phps.log # Remove all files listed in phps.log and print the result to the console
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
    !run a.bat
    !include foo.log # Paste the code from foo.log
  ]
  [ # Concurrent Scope B
    # Do work B
    !run b.bat
    !include foo.log # Paste the code from foo.log
  ]
```

## Commands (in alphabetical order)
- **compare**
  - [-h --help]
  - [-d --directory \<directory name\>]
  - [-f --file \<test file name\> \<pattern file name\>]
  - [-r --regex \<test regular expression\> \<pattern regular expression\>]<br>
- **copy**
	- [-h --help]
	- [-d --directory \<directory name\>]
	- [-r --recursive]
	- [\<regular expression\> \<destination directory name\>]<br>
- **help**
	- [-h --help]<br>
- **include**
	- [-h --help]
	- [-d --directory \<directory name\>]
	- [-f --file \<file name\>]<br>
- **list**
	- [-h --help]
	- [-d --directory \<directory name\>]
	- [-r --recursive]
	- [\<regular expression\>]<br>
- **move**
	- [-h --help]
	- [-d --directory \<directory name\>]
	- [-r --recursive]
	- [\<regular expression\> \<destination directory name\>]<br>
- **regex**
	-[-h --help]
	- [-m --mode]
	- [-l --list]
	- [-s --set \<new mode\>]<br>
- **remove**
	- [-h --help]
	- [-d --directory \<directory name\>]
	- [-v --verbose]
	- [-r --recursive]
	- [\<regular expression\>]<br>
- **rename**
	- [-h --help]
	- [-d --directory \<directory name\>]
	- [-r --recursive]
	- [\<regular expression\> \<new name\>]<br>
- **wait**
	- [-h --help]
	- [-d --duration]
	- [\<time duration\> \<type of duration\>]<br>
- **wipe**
	- [-h --help]
	- [-d --directory \<directory name\>]
	- [-m --mode] [-s --set \<new mode\>]
	- [-r --range \<first line\> \<last line\>]
	- [-l --line \<line\>]
	- [-k --keyword \<keyword\>]
	- [\<regular expression\>]

## Project Structure
Project is make with Visual Studio 2017. Interpreter is partly managed by Bison & Flex. Code uses the newest C++17 standard (filesystem) as well as C++14 and C++11 (regex and its many other features). FTP file managing system is upon the Boost.Asio library.
