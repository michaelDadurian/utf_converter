Command line program written in C program designed to convert UTF-16 encoded text from UTF-16LE to UTF-16BE, and vice-versa. Uses the surrogate pair algorithm to extract UTF-8 code points. The program reads the input file one byte at a time, but the conversion occurs one glyph at a time. A glyph is a struct that holds the multi-byte encoding of each character. The conversion occurs by swapping the bytes within a glyph.


## Features:

  Converts UTF-16 encoded text from UTF-16LE to UTF-16BE and vice-versa as well as UTF-8 encoded text to UTF-16LE or UTF-16BE.

  # Portability:
  
  -The program runs on Ubuntu as well as Sparky, a Unix based system provided by Stony Brook University that runs SunOS sparc. Sparky is a    big endian system while the Linux VM used to develop the software is a little endian system. 
  
  # Output Redirection via positional argument:
  
   -Converted text is outputed to another file, not just stdout.
  
  
  # Verbose output:
  
  -Level 1 verbosity displays the size of the input file, the absolute path of the input file, the input encoding, the output encoding,   and the name of the host machine and its operating system.
    
  -Level 2 verbosity displays the time the program takes to read, convert, and write to the output file. It also displays the percentage of ASCII characters, the percentage of surrogate pairs, as well as how many glyphs were created during the transcoding.
    
    
 ## USAGE STATEMENT
 
    $ ./utf --help 
    
    ./utf [-h|--help] [-v|-vv] -u OUT_ENC | --UTF=OUT_ENC IN_FILE [OUT_FILE]
   
    Option arguments: 
      -h, --help Displays this usage. 
      -v, -vv Toggles the verbosity of the program to level 1 or 2.
      
    Mandatory argument: -u OUT_ENC, --UTF=OUT_ENC Sets the output encoding. Valid values for OUT_ENC: 16LE, 16BE
    
    Positional Arguments: IN_FILE The ﬁle to convert.
                          [OUT_FILE] Output ﬁle name. If not present, defaults to stdout. 

