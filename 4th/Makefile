4th: 4th.o bitmap_manager.o
	g++ -o 4th 4th.o bitmap_manager.o -std=c++11
bitmap_manager.o: bitmap_manager.cpp
	g++ -c bitmap_manager.cpp -std=c++11
4th.o: 4th.cpp
	g++ -c 4th.cpp -std=c++11
clean:
	rm -f *.o 4th