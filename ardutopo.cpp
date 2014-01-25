#include <stdio.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "MS5803_beaglebone.h"

int main(){
	MS5803_14BA profsensor;
	float temp,pres;
	
	profsensor.getData(&temp,&pres);
	
	std::cout << "Temperature = " << temp << "\n";
	std::cout << "Pressure in bars : " << pres << "\n";
}