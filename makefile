#
# XENLY - high-level and general-purpose programming language
# created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
# 
# It is initially written in C programming language.
#
xenly:
	cc src/uxenly.c -o xenly -lm

clean:
	rm -f xenly
