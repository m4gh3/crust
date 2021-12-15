CXXFLAGS=-g

build/lib/libparsergen.so: gen/parsergen.so.gen build/rrextab.o
	cd gen; cgnalec dbg parsergen ../build/rrextab.o
	mv gen/libparsergen.so build/lib

build/rrextab.o: src/rrextab.cpp src/rrextab.hpp
	g++ -c -fPIC $(CXXFLAGS) src/rrextab.cpp -o build/rrextab.o

#build/include/mytokens.h: build/lib/libtokenames.so src/tokens.h.gen
#	cgnale -l ./build/lib/libtokenames.so -f src/tokens.h.gen > build/include/mytokens.h

clean:
	-rm build/lib/libparsergen.so
	#-rm build/include/mytokens.h
	-rm build/rrextab.o
