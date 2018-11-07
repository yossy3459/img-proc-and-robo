3rd_canny: 3rd_canny.o bitmap_manager.o
	g++ -o 3rd_canny 3rd_canny.o bitmap_manager.o -std=c++11
bitmap_manager.o: bitmap_manager.cpp
	g++ -c bitmap_manager.cpp -std=c++11
3rd_canny.o: 3rd_canny.cpp
	g++ -c 3rd_canny.cpp -std=c++11
clean:
	rm -f *.o 3rd_canny