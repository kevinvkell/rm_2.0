kevin kell

to build the files simply run make
to run rm just do ./rm and any options desired
don't forget to set the trash environment variable

Rm works by first determining if it is recursive or not
if it is recursive it will use the POSIX nftw function which cycles
through directories and runs a callback on each directory and its files
using this I copy and delete all files on another partition
rename works if it isn't on another partition

if it isn't recursive it is simple to use system calls to move the files to the trash

I didn't have time to finish dv, but dump works well. It is straight forward and uses 
the nftw to delete all files and directries in the trash
