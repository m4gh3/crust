CXXFLAGS=-g

m4ghpiler: build/src/m4gpiler.cpp build/rrextab.o src/rrextab.hpp
	g++ $(CXXFLAGS) build/src/m4gpiler.cpp build/rrextab.o -I src -o m4gpiler -g

build/src/m4gpiler.cpp: src/m4gpiler.cpp.gen build/lib/libparsergen.so
	cgnale -l build/lib/libparsergen.so -c src/m4gpiler.cpp.gen > build/src/m4gpiler.cpp

build/lib/libparsergen.so: gen/parsergen.so.gen build/rrextab.o
	cd gen; cgnalec dbg parsergen ../build/rrextab.o
	mv gen/libparsergen.so build/lib

build/rrextab.o: src/rrextab.cpp src/rrextab.hpp
	g++ -c -fPIC $(CXXFLAGS) src/rrextab.cpp -o build/rrextab.o

clean:
	-rm build/lib/libparsergen.so
	-rm build/rrextab.o
	-rm build/src/m4gpiler.cpp
	-rm m4gpiler
	
