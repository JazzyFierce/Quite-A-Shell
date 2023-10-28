quash: quash.c
	gcc -g quash.c -o quash
	
FIRST_NAME=SatyaAshok
LAST_NAME=Dowluri
KUID=3125041
LAB=Quash_Shell
TAR_BASENAME=$(LAB)_$(FIRST_NAME)_$(LAST_NAME)_$(KUID)
DELIVERABLES=quash.c

tar: clean
#	create temp dir
	mkdir $(TAR_BASENAME)
#	copy the necessary files into the temp dir
	cp $(DELIVERABLES) Makefile $(TAR_BASENAME)
#	create the submission tar.gz
	tar cvzf $(TAR_BASENAME).tar.gz $(TAR_BASENAME)
#	remove the temp dir
	rm -rf $(TAR_BASENAME)

.PHONY: clean tar
