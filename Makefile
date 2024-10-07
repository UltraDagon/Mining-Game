all:
	g++ -I src/include -L src/lib -o main main.cpp -lmingw32 -lSDL2main -lSDL2
run:
	g++ -I src/include -L src/lib -o main main.cpp functs.cpp functs.h -lmingw32 -lSDL2main -lSDL2
	./main.exe