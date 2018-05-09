.DEFAULT_GOAL = all
.PHONY: all clean

CC := gcc
CFLAGS := -Wall -g -std=c99
AUX := -lpthread

#Ficheros a ejecutar
EXES = carrera
AUX_OBJ = semaforos.o lib.o

#Targets
all : $(EXES)

$(EXES) : % : %.o $(AUX_OBJ)
	$(CC) $(CFLAGS) -o $@ $@.o $(AUX_OBJ) $(AUX)

semaforos.o : semaforos.c semaforos.h
	$(CC) $(CFLAGS) -c $<

lib.o : lib.c lib.h
	$(CC) $(CFLAGS) -c $<

clean :
	rm -rfv *.o core $(EXES)
