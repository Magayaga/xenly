#
# XENLY - high-level and general-purpose programming language
# created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
# 
# It is initially written in C programming language.
#
xenly: src/xenly.c
	cc src/xenly.c src/print_info.c -o xenly -lm

test:
    # Add your test commands here
    # For example:
    # ./xenly <input_file> >output_file

clean:
	rm -f xenly
