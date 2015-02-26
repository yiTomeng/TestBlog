#sound_test:sound_test.o sound_lib.o
#	gcc sound_test.o sound_lib.o -lasound -o sound_test

#sound_test.o:sound_lib.h sound_lib_err.h
3	gcc  -g -c sound_test.c

sound_lib.o:sound_lib.h sound_lib_err.h
	gcc  -g -c sound_lib.c

.phony:clean

clean:
	-rm sound_test sound_*.o sound_*.BAK
