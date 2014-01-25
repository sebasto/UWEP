#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "MS5803_beaglebone.h"

MS5803_14BA::MS5803_14BA(){
#if DEBUG
	std::cout << "Init MS5803_14BA\n";
#endif

	char dev[256] = I2C_DEV_FILE;
    int addr = MS5803_14BA_I2C_ADDRESS;
	int r;
	char buf[2];
	unsigned char command[2];
	
	fd = open(dev, O_RDWR );
	if(fd < 0)
	{
			perror("Opening i2c device node\n");
	}

	r = ioctl(fd, I2C_SLAVE, addr);
	if(r < 0)
	{
			perror("Selecting i2c device\n");
	}
	
	//init the sensor by sending reset
	command[0] = Reset;
	r = write(fd, &command, 1);
    usleep(I2C_DELAY);

	// read the calibration eeprom and initialize constants array
	for(int i = 0; i < 8; i++) {
		command[0] = PromBaseAddress + (unsigned char)(i*2);// output enable | read input i

		r = write(fd, &command, 1);
		usleep(I2C_DELAY);
		// the read is always one step behind the selected input
		r = read(fd, &buf, 2);
		if(r != 2) {
			perror("reading i2c device\n");
		}
		usleep(I2C_DELAY);
		CalConstant[i] = (buf[0] << 8) + buf[1];
#if DEBUG
		printf("0x%02x 0x%02x 0x%02x  %d\n", 0xA0 + (i*2),buf[0], buf[1],(int)CalConstant[i]);
#endif
	}
  
}

MS5803_14BA::~MS5803_14BA(){
	close(fd);
}

long MS5803_14BA::getAdcData(unsigned char command){
	unsigned char buf[3];
	int r;
	long AdcData;

	r = write(fd, &command, 1);
	usleep(I2C_DELAY);
	
	command = AdcRead;
	r = write(fd, &command, 1);
	usleep(I2C_DELAY);
	
	r = read(fd, &buf, 3);
	//printf ("got %d bytes in response : 0x%02x 0x%02x 0x%02x\n",r,buf[0],buf[1],buf[2]);
	if(r != 3) {
		perror("reading i2c device in MS5803_14BA\n");
	}
	  
	AdcData = ((long)buf[0] << 16) + ((long)buf[1] << 8) + (long)buf[2];

	return (AdcData);
}

void MS5803_14BA::getData(float* tempRes, float* presRes){
	float Temperature, Pressure, TempDifference, Offset, Sensitivity;
	long AdcTemperature = getAdcData(D2_512);
	long AdcPressure = getAdcData(D1_512);
	float T2, Off2, Sens2;  // Offsets for second-order temperature computation
	
	// Calculate the Temperature (first-order computation)
  
	TempDifference = (float)(AdcTemperature - ((long)CalConstant[5] << 8));
	Temperature = (TempDifference * (float)CalConstant[6])/ pow(2, 23);
	Temperature = Temperature + 2000;  // This is the temperature in hundredths of a degree C
  
	// Calculate the second-order offsets

	if (Temperature < 2000.0)  // Is temperature below or above 20.00 deg C ?
	{
		T2 = 3 * pow(TempDifference, 2) / pow(2, 33);
		Off2 = 1.5 * pow((Temperature - 2000.0), 2);
		Sens2 = 0.625 * pow((Temperature - 2000.0), 2);
	}
	else
	{
		T2 = (TempDifference * TempDifference) * 7 / pow(2, 37);
		Off2 = 0.0625 * pow((Temperature - 2000.0), 2); 
		Sens2 = 0.0;
	}
  
	// Print the temperature results
  
	Temperature = Temperature / 100;  // Convert to degrees C
	Temperature = Temperature -(T2 / 100); // Last compensation
	*tempRes = Temperature;
		
	// Calculate the pressure parameters

	Offset = (float)CalConstant[2] * pow(2,16);
	Offset = Offset + ((float)CalConstant[4] * TempDifference / pow(2, 7));

	Sensitivity = (float)CalConstant[1] * pow(2, 15);
	Sensitivity = Sensitivity + ((float)CalConstant[3] * TempDifference / pow(2, 8));

	// Add second-order corrections

	Offset = Offset - Off2;
	Sensitivity = Sensitivity - Sens2;

	// Calculate absolute pressure in bars

	Pressure = (float)AdcPressure * Sensitivity / pow(2, 21);
	Pressure = Pressure - Offset;
	Pressure = Pressure / pow(2, 15);
	Pressure = Pressure / 10;  // Set output to mbars = hectopascal;
	*presRes = Pressure;
}

/*
	~MS5803_14BA();
	
	getPressure();
	getTemperature();
*/
