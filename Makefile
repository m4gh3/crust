CXXFLAGS=-g

rrextab: src/rrextab.cpp build/include/mytokens.h
	g++ src/rrextab.cpp -o rrextab

build/include/mytokens.h: build/lib/libtokenames.so src/tokens.h.gen
	cgnale -l ./build/lib/libtokenames.so -f src/tokens.h.gen > build/include/mytokens.h

build/lib/libtokenames.so: gen/tokenames.so.gen
	cd gen; cgnalec rel tokenames
	mv gen/libtokenames.so build/lib