
With this tool you can convert the Chameleon helpfile to an editable UTF-8 text
file and convert the text file back to a help file.

--------------------------------------------------------------------------------
Prerequisites:
--------------------------------------------------------------------------------

- A Java JRE or JDK needs to be installed.
- Please use the chamhelp.hlp file from the latest Chameleon update

--------------------------------------------------------------------------------
Howto
--------------------------------------------------------------------------------

First you'll have to convert the helpfile into editable text. You can do this
using the following command:

java ChamHelp -d chamhelp.hlp chamhelp.seq

Now you can make your changes. When you are done you'll have to convert the text
file back to a binary helpfile. You can do this manually using the following
command:

java ChamHelp -e chamhelp.seq chamhelp-edited.hlp

--------------------------------------------------------------------------------
File format:
--------------------------------------------------------------------------------

The editable file is in UTF-8 format, and does have Unix style line delimiters.

At the start of new page:
-------------------------

<page nnnn>     Start new page with number nnnn
<a nnnn>        Add hotkey 'a' to page number nnnn
<$xx nnnn>      Add hotkey with charvalue 'xx' in hex to page number nnnn

Inside page:
------------

<c$xx>          Insert special character with hex value 'xx'

Special characters:
-------------------

~ is checkerboard pattern (or pi in alternate set)
— is horizontal line (151 / $97)
| is vertical line
† is a line cross (134 / $86)

Colors:

$01 Text
$02 Grey text
$03 Monitor command char
$04 Link
$05 White
$06 Title monitor commands
$07 Line color

In links:

$85 = F1
$86 = F3

--------------------------------------------------------------------------------
Credits:
--------------------------------------------------------------------------------

original ChamHelp program written by Geert De Prins
UTF8->PETSCII translation hacked in by Tobias Korbmacher / Individual Computers

original Chameleon helpfile was created by Peter Wendrich
adapted to new menu system by Tobias Korbmacher / Individual Computers
Geert De Prins filled some gaps and completed missing bits
