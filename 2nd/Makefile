2nd: 2nd.o bitmap_manager.o
	g++ -o 2nd 2nd.o bitmap_manager.o -std=c++11
bitmap_manager.o: bitmap_manager.cpp
	g++ -c bitmap_manager.cpp -std=c++11
2nd.o: 2nd.cpp
	g++ -c 2nd.cpp -std=c++11
clean:
	rm -f *.o 2nd