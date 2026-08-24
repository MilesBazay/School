I have completed the implementations of diskinfo, disklist, diskget and diskput

The diskinfo, disklist and diskput functions should be working fully but I had some trouble with diskget.

The diskinfo upon using ./diskinfo test.img will output exactly the same output as the provided test case.

Disklist should follow the same format as the output as well and if the specified directory is not found it will print:
"Requested directory <directory> not found."

Diskput should be working properly and disklist will display the updated file information after diskput is used. 
As an example after "./diskput test.img readme.txt /readme.txt" is used and "./disklist test.img /" is used the output will be something like:

D          0                              . 2025/10/31 17:35:05
F          0                     readme.txt 2025/11/28 20:07:49
F          7                     readme.txt 2025/11/28 22:13:24
F       2660                     readme.txt 2025/11/28 22:13:52

Diskget will provide the correct output on "./diskget test.img /sub_dir/foo2.txt foo.txt" where foo2.txt is not found in sub_dir.
Using "./diskget test.img readme.txt readme-copy.txt", will create a file but will not copy the file contents and address.

The makefile compiles all four files correctly on "make" and "make clean" will remove the compiled files. 