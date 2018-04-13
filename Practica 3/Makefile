.DEFAULT_GOAL = all
.PHONY: all clean

CC := gcc
CFLAGS := -Wall -g
AUX := -lpthread

#Ficheros a ejecutar
EXES = ejercicio2 ejercicio2_solved ejercicio3 ejercicio4
AUX_OBJ = semaforos.o

#Targets
all : $(EXES)

$(EXES) : % : %.o $(AUX_OBJ)
	$(CC) $(CFLAGS) -o $@ $@.o $(AUX_OBJ) $(AUX)

semaforos.o : semaforos.c semaforos.h
	$(CC) $(CFLAGS) -c $<

clean :
	rm -rfv *.o core $(EXES)
	rm *.txt
