ardutopo: MS5803_beaglebone.o ardutopo.o
	g++ -o ardutopo ardutopo.o MS5803_beaglebone.o

ardutopo.o: ardutopo.cpp
	g++ -c ardutopo.cpp

MS5803_beaglebone.o: MS5803_beaglebone.h MS5803_beaglebone.cpp
	g++ -c MS5803_beaglebone.cpp
