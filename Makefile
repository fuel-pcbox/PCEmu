all:
	$(CXX) -g main.cpp 808x.cpp ram.cpp io.cpp io8.cpp -o pcemu -std=c++0x