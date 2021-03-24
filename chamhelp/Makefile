
.SILENT:

all: ChamHelp.class test

ChamHelp.class: ChamHelp.java Makefile
	echo "compiling..."
	javac ChamHelp.java

decode: ChamHelp.class chamhelp.hlp
	$(RM) chamhelp.txt
	java ChamHelp -d chamhelp.hlp chamhelp.txt
	echo "copy chamhelp.txt to chamhelp-edited.txt"

encode: ChamHelp.class chamhelp-edited.txt
	$(RM) chamhelp-edited.hlp
	java ChamHelp -e chamhelp-edited.txt chamhelp-edited.hlp

test: ChamHelp.class
	echo "testing..."
	$(RM) chamhelp.tst
	$(RM) chamhelp.tst2
	$(RM) chamhelp.hlptest

	java ChamHelp -d chamhelp.hlp chamhelp.tst
	java ChamHelp -e chamhelp.tst chamhelp.hlptest
	java ChamHelp -d chamhelp.hlptest chamhelp.tst2
	
	diff -q chamhelp.hlp chamhelp.hlptest
	diff -q chamhelp.tst chamhelp.tst2

	$(RM) chamhelp.tst
	$(RM) chamhelp.tst2
	$(RM) chamhelp.hlptest
	
clean:
	$(RM) ChamHelp.class
	$(RM) chamhelp.txt
	$(RM) chamhelp.tst
	$(RM) chamhelp.tst2
	$(RM) chamhelp-edited.hlp
