CC = g++ -std=c++17
#CXXFLAGS = -I/usr/local/Cellar/opencv@3/3.4.9_1/include/opencv -I/usr/local/Cellar/opencv@3/3.4.9_1/include
#LDFLAGS = -L/usr/local/Cellar/opencv@3/3.4.9_1/lib
#CXXFLAGS = -I/usr/local/Cellar/opencv@3/3.4.5_6/include/opencv -I/usr/local/Cellar/opencv@3/3.4.5_6/include
#LDFLAGS = -L/usr/local/Cellar/opencv@3/3.4.5_6/lib
#CXXFLAGS = -I/usr/local/Cellar/opencv@2/2.4.13.7_5/include/opencv -I/usr/local/Cellar/opencv@2/2.4.13.7_5/include
#LDFLAGS = -L/usr/local/Cellar/opencv@2/2.4.13.7_5/lib

LIBS = -lcurl

main: main.o file_send.o
	$(CC) -o main main.o file_send.o $(LDFLAGS) $(LIBS)

main.o: main.cpp
	$(CC) -c main.cpp $(CXXFLAGS)

file_send.o: file_send.cpp
	$(CC) -c file_send.cpp $(CXXFLAGS)


clean:
