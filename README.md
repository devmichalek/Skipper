## Skipper
Skipper is a self acting interpreter for file managing. Compare, move, rename multiple files by regular expression rules. Program has simple 
syntax which you have to follow if you want to execute commands correctly.

## Syntax - {}, [], # and !
Regular Scope followed by **{}** brackets,<br>
Concurrent Scope followed by **[]** brackets,<br>
Comments followed by **#** sign,<br>
Commands followed by **!** sign.<br>

## Examples
```
  # file.txt
  # Global Scope
  { # Regular Scope 0
    [ # Concurrent Scope 0.0
      !list -r .*\.php$ >> phps.log # Search recursively for all .php files and redirect output to phps.log
      !remove -r -v -l << phps.log # Remove all files listed in phps.log and print the result to the console
    ]
    [ # Concurrent Scope 0.1
      # Do other things
    ]
  }
  
  { # Regular Scope 1
    [ # Concurrent Scope 1.0
      { # Regular Scope 1.0.0
        [ # Regular Scope 1.0.0.0
          {} # Regular Scope 1.0.0.0.0
          {} # Regular Scope 1.0.0.0.1
        ]
      }
    ]
    
    [ # Concurrent Scope 1.1
    
    ]
  }
  
  # Scopes 0.0 and 0.1 will run simultaneously.
```
