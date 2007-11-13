# Makefile to generate Documentation
# This file is part of the ezRobi project (http://ezrobi.berlios.de)
# It is licensed under the GPL!

# the source files
SRC = platform--m32-rob.t2t

# thats what generated...
OUT = $(SRC:%.t2t=%.html) $(SRC:%.t2t=%.txt) $(SRC:%.t2t=%.dokuw)


# the rules...
.SUFFIXES: .t2t .html .dokuw .txt

all:	$(OUT)

.t2t.html:
	txt2tags --target html -o $@ $<

.t2t.dokuw:
	txt2tags --target doku -o $@ $<

.html.txt:
	lynx --dump $< > $@
