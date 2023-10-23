# READ ME

### This merge includes two files
#### alive2script.py and instList.txt

a. Run the script using the command python alive2script.py test.ll, where test.ll is the test file.

b. This script can automatically identify and distinguish the tools and options to be tested. Please write the tool and options after ; RUN:.

c. Some custom options are already listed in instList.txt. To test specific options, add them to this file. Each option should occupy one line. When an option may have values like =true or =false, you only need to add the part before the = sign to the file. For example, -force-customized-pipeline=<true|false> should be written as -force-customized-pipeline= in the file.

d. This script can recognize multiple RUN markers in a single file.

e.Optional debug mode, add the '-debug' option after the input file to output detailed information about the script execution. (for example : python alive2script.py xxx.ll -debug)