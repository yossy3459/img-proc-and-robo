3rd: 3rd.o bitmap_manager.o
	g++ -o 3rd 3rd.o bitmap_manager.o -std=c++11
bitmap_manager.o: bitmap_manager.cpp
	g++ -c bitmap_manager.cpp -std=c++11
3rd.o: 3rd.cpp
	g++ -c 3rd.cpp -std=c++11
clean:
	rm -f *.o 3rd