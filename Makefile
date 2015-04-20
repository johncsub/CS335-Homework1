# cs335 hw1
# to compile your project, type make and press enter

all: hw1

hw1: hw1.cpp
#	g++ lab1.cpp -Wall -o lab1 -lX11 -lGL -lGLU -lm
	g++ hw1.cpp -Wall -o hw1 -lX11 -lGL -lm

clean:
	rm -f hw1
	rm -f *.o

