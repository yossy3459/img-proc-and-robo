5th: 5th.o bitmap_manager.o
	g++ -o 5th 5th.o bitmap_manager.o -std=c++11
bitmap_manager.o: bitmap_manager.cpp
	g++ -c bitmap_manager.cpp -std=c++11
5th.o: 5th.cpp
	g++ -c 5th.cpp -std=c++11
clean:
	rm -f *.o 5th