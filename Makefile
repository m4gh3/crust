CXXFLAGS=-g

build/rrextab.o: src/rrextab.cpp src/rrextab.hpp
	g++ -c $(CXXFLAGS) src/rrextab.cpp -o build/rrextab.o

build/include/mytokens.h: build/lib/libtokenames.so src/tokens.h.gen
	cgnale -l ./build/lib/libtokenames.so -f src/tokens.h.gen > build/include/mytokens.h

build/lib/libtokenames.so: gen/tokenames.so.gen
	cd gen; cgnalec rel tokenames
	mv gen/libtokenames.so build/lib

clean:
	-rm build/lib/libtokenames.so
	-rm build/include/mytokens.h
	-rm build/rrextab.o
