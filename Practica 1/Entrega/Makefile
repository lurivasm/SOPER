.DEFAULT_GOAL = all
.PHONY: all clean

CC := gcc
CFLAGS := -Wall -g -pedantic -Wextra

#Ficheros a ejecutar
EXES = Ejercicio4 Ejercicio4b Ejercicio5a Ejercicio5b Ejercicio6 Ejercicio8 ejercicio9 ejercicio12a ejercicio12b ejercicio13
AUX_OBJ = #En esta práctica no hay librerías auxiliares

#Targets
all : $(EXES)

$(EXES) : % : %.o $(AUX_OBJ)
	$(CC) $(CFLAGS) -o $@ $@.o $(AUX_OBJ) -lm -lpthread

clean :
	rm -rfv *.o core $(EXES)
