chordprc_o: chord.o
	g++ -std=c++14 chord.o -o chordprc_o
	chmod +x ./chord.sh

chord.o: chord.cpp
	g++ -std=c++14 -c chord.cpp

clean:
	rm *.o chordprc_o