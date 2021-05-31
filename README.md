# Small-shell

This is small-shell. 
The Small shell is a lightweight Unix-like shell with customized features developed by C language. In reality, people used operating systems such as Linux, Windows, macOS to communicate with their hardware devices, and the small shell is an operating system kernel with the least features, which is suitable for low-performance devices.

The small-shell made the implementation that includes some basic shell features. These features are:  
 
  ・ pwd(print out the working directory),  
  ・ cd(move users current working directory to a different one),  
  ・ ls(Display the files in a given directory),  
  ・ mkdir(create a directly), rmdir(remove a directly), 
  ・ rm(Remove files or directories if used recursively),  
  ・ mv(Move or rename files and directories),  
  ・ cat(Dump the character data from given file to the terminal), 
    
The extra features:  
  ・ changeCmdName(modify the command name),  
  ・ lock(lock the file), needs password,  
  ・ unlock(unlock the file), needs password  

How to run:  
Run make on any linux environment with makeFile(version higher than 3.82) and gcc (version higher than 4.8.5).

$make  
$smallShell  

Run the test:  
sh testScript  
