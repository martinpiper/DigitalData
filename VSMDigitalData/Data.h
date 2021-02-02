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

	const bool anyError()
	{
		return !mError.empty();
	}

	const std::string &getError()
	{
		return mCurrentFilename + " : " + std::to_string(mCurrentLineNumber) + " : " + mError;
	}

	const std::string &getCurrentFilename()
	{
		return mCurrentFilename;
	}

	const int getCurrentLineNumber()
	{
		return mCurrentLineNumber;
	}

	const int getWaitingForData()
	{
		return mWaitingForData;
	}

private:
	void CheckFileIsOpen(std::ifstream * toCheck);

	unsigned int mData;
	std::ifstream *mFile;
	std::string mCurrentLine;
	std::string mError;
	std::string mCurrentFilename;
	int mCurrentLineNumber;

	std::ifstream mInputData;
	std::list<int> mInputDataBits;
	int mInputDataNumBytes;

	std::list<int> mCountingBits;
	std::list<std::ifstream*> mFiles;
	std::list<int> mLineNumbers;

	unsigned int mWaitingForPositiveEdge , mWaitingForNegativeEdge;
	unsigned int mWaitingForMask, mWaitingForData;
};
