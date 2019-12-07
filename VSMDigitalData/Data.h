#pragma once
#include "StdAfx.h"
#include <vector>
#include <fstream>
#include <string>
#include <list>

#pragma warning(disable : 4251)

class DLLEXPORT Data
{
public:
	Data();
	virtual ~Data();

	void clear();

	void init(const CHAR *filename);

	void simulate(const double time, const unsigned int dInput, const unsigned int dInputPositiveEdge, const unsigned int dInputNegativeEdge);

	const unsigned int getData()
	{
		return mData;
	}

	const bool waitingForInput()
	{
		return mWaitingForMask || mWaitingForPositiveEdge || mWaitingForNegativeEdge;
	}

private:
	unsigned int mData;
	std::ifstream *mFile;
	std::string mCurrentLine;

	std::ifstream mInputData;
	std::list<int> mInputDataBits;
	int mInputDataNumBytes;

	std::list<int> mCountingBits;
	std::list<std::ifstream*> mFiles;

	unsigned int mWaitingForPositiveEdge , mWaitingForNegativeEdge;
	unsigned int mWaitingForMask, mWaitingForData;
};
