.DEFAULT_GOAL = all
.PHONY: all clean

CC := gcc
CFLAGS := -Wall -g -pedantic -Wextra
AUX := -lm -lpthread

#Ficheros a ejecutar
EXES = ejercicio4 ejercicio4b ejercicio5a ejercicio5b ejercicio6 ejercicio8 ejercicio9 ejercicio12a ejercicio12b ejercicio13
AUX_OBJ = #En esta práctica no hay librerías auxiliares

#Targets
all : $(EXES)

$(EXES) : % : %.o $(AUX_OBJ)
	$(CC) $(CFLAGS) -o $@ $@.o $(AUX_OBJ) $(AUX)

clean :
	rm -rfv *.o core $(EXES)
