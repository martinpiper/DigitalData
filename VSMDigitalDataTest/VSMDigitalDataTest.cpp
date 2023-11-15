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
	assert(data.waitingForInput() == false);
	value = data.getData();
	assert(value == 0x0);

	//
	data.simulate(0, 0, 0x00, 0x00);
	assert(data.waitingForInput() == false);
	value = data.getData();
	assert(value == 0x1);

	data.simulate(0, 0, 0x00, 0x00);
	assert(data.waitingForInput() == false);
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

	// 71 75 69 63 6b
	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xffffffff);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xffffff71);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xffffff75);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xffffff69);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xffffff63);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xffffff6b);

	// @$2,>1
	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xffffff69);

	// @$2,>$10
	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xffffff69);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xffffff63);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xffffff6b);

	// @$3
	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xffffff63);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xffffff6b);

	// >10
	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xffffff71);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xffffff75);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xffffff69);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xffffff63);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xffffff6b);

	//
	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xff00ff00);

	// +8,9,10,11,+
	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x16c);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x237);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x311);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x4aa);

	// <0,1,2,3,4,5,6,7,<t.bin
	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x571);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x675);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x769);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x863);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x96b);

	// mWaitingForPositiveEdge/mWaitingForNegativeEdge does not cause new data to be output, the address counter should also stop
	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xa6b);

	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0xa6b);

	data.simulate(0, 0, 0x01, 0x00);
	// After a successful wait, it returns waiting for the next clock cycle to trigger the data fetch
	value = data.getData();
	assert(value == 0xa6b);

	data.simulate(0, 0, 0x01, 0x00);
	value = data.getData();
	assert(value == 0x1);
	data.simulate(0, 0, 0x00, 0x00);

	data.simulate(0, 0, 0x01, 0x00);
	// After a successful wait, it returns waiting for the next clock cycle to trigger the data fetch
	value = data.getData();
	assert(value == 0x101);

	data.simulate(0, 0, 0x01, 0x00);
	value = data.getData();
	assert(value == 0x2);
	data.simulate(0, 0, 0x00, 0x00);

	data.simulate(0, 0, 0x00, 0x01);
	value = data.getData();
	assert(value == 0x102);

	data.simulate(0, 0, 0x00, 0x01);
	value = data.getData();
	assert(value == 0x3);
	data.simulate(0, 0, 0x00, 0x00);

	data.simulate(0, 0, 0x00, 0x01);
	assert(data.waitingForInput() == false);
	value = data.getData();
	assert(value == 0x103);

	data.simulate(0, 0, 0x00, 0x01);
	value = data.getData();
	assert(value == 0x4);
	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x100);	// byte 0 but with address increment
	data.simulate(0, 0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x321);

	// w$ffffffff,$1234
	data.simulate(0, 0, 0x00, 0x00);
	assert(data.waitingForInput() == true);
	value = data.getData();
	assert(value == 0x421);
	data.simulate(0, 0, 0x00, 0x00);
	assert(data.waitingForInput() == true);
	value = data.getData();
	assert(value == 0x421);
	data.simulate(0, 0x12, 0x00, 0x00);
	assert(data.waitingForInput() == true);
	value = data.getData();
	assert(value == 0x421);
	data.simulate(0, 0x1234, 0x00, 0x00);
	assert(data.waitingForInput() == false);
	data.simulate(0, 0x1234, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x501);	// byte 1 but with address increment
	data.simulate(0, 0x1234, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x0);

	// @time tests
	data.simulate(0, 0x0, 0x00, 0x00);
	assert(data.waitingForInput() == true);
	data.simulate(0.735f, 0x0, 0x00, 0x00);
	assert(data.waitingForInput() == false);
	data.simulate(0.735f, 0x0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x12345678);

	data.simulate(0.735f, 0x0, 0x00, 0x00);
	assert(data.waitingForInput() == true);

	data.simulate(1.0f, 0x0, 0x00, 0x00);
	data.simulate(1.0f, 0x0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x87654321);

	data.simulate(0, 0x0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x00);

	// Evaluation of expression
	data.simulate(0, 0x0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x377);

	// Last d0
	data.simulate(0, 0x0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x0);

	// Simulate off the end of the file, to check the correct handling of the file handle stack
	data.simulate(0, 0x0, 0x00, 0x00);
	value = data.getData();
	assert(value == 0x100);

	data.clear();
}
