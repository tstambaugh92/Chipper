CXX=g++
CXXFLAGS=-lmingw32 -lSDL2main -lSDL2

LIB=-LC:/SDL2-2.0.14/x86_64-w64-mingw32/lib
INC=-IC:/SDL2-2.0.14/x86_64-w64-mingw32/include

testmake: ./src/game.cpp
	$(CXX) -o ./bin/test.exe ./src/*.cpp $(INC) $(LIB) $(CXXFLAGS)

clean:
	rm ./bin/*.o ./bin/*.exe