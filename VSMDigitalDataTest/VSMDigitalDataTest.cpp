// VSMDigitalDataTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <assert.h>
#include "DsimModel.h"
#include "Data.h"

int main()
{
	Data data;
	data.init("Test1.txt");

	unsigned int value;

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x0);

	//
	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x1);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x123);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x456);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x321);

	//
	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x1);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x123);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x456);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x321);
}
