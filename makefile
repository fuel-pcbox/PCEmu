all:
	$(CXX) -g -fpermissive main.cpp 808x.cpp ram.cpp io.cpp io8.cpp pic.cpp pit.cpp kbdc.cpp -o pcemu -std=c++0x