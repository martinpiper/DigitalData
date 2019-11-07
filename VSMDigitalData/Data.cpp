#include "stdafx.h"
#include "Data.h"

Data::Data()
{
	init(0);
}

Data::~Data()
{
}

void Data::init(const CHAR *filename)
{
	mData = 0;
}

void Data::simulate(const ABSTIME time, const bool clockEdge)
{
	if(!clockEdge)
	{
		return;
	}
	mData++;
}
