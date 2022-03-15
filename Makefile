# Copyright (C) 2021 m4gh3

#This file is part of m4gpiler.

#    m4gpiler is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#    m4gpiler is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#    You should have received a copy of the GNU General Public License
#    along with m4gpiler.  If not, see <http://www.gnu.org/licenses/>.

CXXFLAGS=-g

m4ghpiler: build/src/m4gpiler.cpp build/rrextab.o src/rrextab.hpp build/lazyfied_ostream.o
	g++ $(CXXFLAGS) build/src/m4gpiler.cpp build/rrextab.o -I src -o m4gpiler -g

build/src/m4gpiler.cpp: src/m4gpiler.cpp.gen build/lib/libparsergen.so
	cgnale -l build/lib/libparsergen.so -c src/m4gpiler.cpp.gen > build/src/m4gpiler.cpp

build/lib/libparsergen.so: gen/parsergen.so.gen build/rrextab.o
	cd gen; cgnalec dbg parsergen ../build/rrextab.o
	mv gen/libparsergen.so build/lib

build/rrextab.o: src/rrextab.cpp src/rrextab.hpp
	g++ -c -fPIC $(CXXFLAGS) src/rrextab.cpp -o build/rrextab.o

build/lazyfied_ostream.o: src/lazyfied_ostream.cpp src/lazyfied_ostream.hpp
	g++ -c $(CXXFLAGS) src/lazyfied_ostream.cpp -o build/lazyfied_ostream.o

clean:
	-rm build/lib/libparsergen.so
	-rm build/rrextab.o
	-rm build/src/m4gpiler.cpp
	-rm m4gpiler
	
