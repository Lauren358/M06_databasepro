all :  
	g++ -pedantic-errors ./*.cpp -g -lsqlite3 -o a.out

clean :
	rm a.out