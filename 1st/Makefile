1st: 1st.o bitmap_manager.o
	g++ -o 1st 1st.o bitmap_manager.o -std=c++11
bitmap_manager.o: bitmap_manager.cpp
	g++ -c bitmap_manager.cpp -std=c++11
1st.o: 1st.cpp
	g++ -c 1st.cpp -std=c++11
clean:
	rm -f *.o 1st