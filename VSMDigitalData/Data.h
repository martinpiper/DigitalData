#pragma once
#include "StdAfx.h"
#include "vsm.hpp"
#include <vector>

#pragma warning(disable : 4251)

class DLLEXPORT Data
{
public:
	Data();
	virtual ~Data();

	void init(const CHAR *filename);

	void simulate(const ABSTIME time, const bool clockEdge);

	const int getData()
	{
		return mData;
	}

private:
	unsigned int mData;
};
