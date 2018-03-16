.DEFAULT_GOAL = all
.PHONY: all clean

CC := gcc
CFLAGS := -Wall -g -pedantic -Wextra
AUX := -lpthread

#Ficheros a ejecutar
EXES = ejercicio2 ejercicio4
AUX_OBJ = #En esta práctica no hay librerías auxiliares

#Targets
all : $(EXES)

$(EXES) : % : %.o $(AUX_OBJ)
	$(CC) $(CFLAGS) -o $@ $@.o $(AUX_OBJ) $(AUX)

clean :
	rm -rfv *.o core $(EXES)
