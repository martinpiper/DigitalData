#include "stdafx.h"
#include "Data.h"
#include <sstream>
#include "../../C64/Common/ParamToNum.h"

// https://gist.github.com/dedeexe/9080526
std::string trim_left(const std::string& str)
{
	const std::string pattern = " \f\n\r\t\v";
	return str.substr(str.find_first_not_of(pattern));
}

std::string trim_right(const std::string& str)
{
	const std::string pattern = " \f\n\r\t\v";
	return str.substr(0, str.find_last_not_of(pattern) + 1);
}

std::string trim(const std::string& str)
{
	if (str.empty())
	{
		return "";
	}
	return trim_left(trim_right(str));
}

std::string getNextTok(std::string& str , const std::string pattern = ", \f\n\r\t\v")
{
	size_t pos = str.find_first_of(pattern);
	std::string ret = str.substr(0, pos);
	if (std::string::npos != pos)
	{
		pos = str.find_first_not_of(pattern, pos);
		str = str.substr(pos);
		str = trim(str);
	}
	else
	{
		str.clear();
	}
	return ret;
}

Data::Data()
{
}

Data::~Data()
{
}

void Data::init(const CHAR *filename)
{
	mData = 0;
	mFile.open(filename);
}

void Data::simulate(const double time, const unsigned int dInput, const unsigned int dInputPositiveEdge, const unsigned int dInputNegativeEdge)
{
	if (mFile.eof())
	{
		return;
	}

	bool gotNextOutput = false;
	while (!gotNextOutput)
	{
		if (mInputData.is_open() && (mInputData.eof() || mInputDataNumBytes == 0))
		{
			mInputData.close();
		}

		char theByte = 0;
		int numBitsRemaining = 0;
		if (mInputData.is_open() && !mInputData.eof() && mInputDataNumBytes > 0)
		{
			mInputData.get(theByte);
			if (!mInputData.eof())
			{
				numBitsRemaining = 8;
			}
		}

		if (mInputData.is_open() && mInputData.eof())
		{
			mInputData.close();
		}

		if (mInputData.is_open() && numBitsRemaining > 0)
		{
			mInputDataNumBytes--;
			for (int theBit : mInputDataBits)
			{
				if (numBitsRemaining <= 0 && mInputData.is_open() && mInputDataNumBytes > 0)
				{
					mInputData.get(theByte);
					mInputDataNumBytes--;
					numBitsRemaining = 8;
				}

				if (mInputData.is_open() && (mInputData.eof() || mInputDataNumBytes == 0))
				{
					mInputData.close();
				}


				mData = mData & ~(1 << theBit);
				mData = mData | ((theByte & 1) << theBit);
				theByte >>= 1;
				numBitsRemaining--;
			}
			gotNextOutput = true;
			break;
		}


		while (mCurrentLine.empty() && !mFile.eof())
		{
			std::getline(mFile, mCurrentLine);
			mCurrentLine = trim(mCurrentLine);
		}
		if (mFile.eof())
		{
			break;
		}

		if (mCurrentLine.at(0) == ';')
		{
			mCurrentLine.clear();
			continue;
		}

		// d$00000000
		if (mCurrentLine.at(0) == 'd')
		{
			mCurrentLine = mCurrentLine.substr(1);
			std::string tok = getNextTok(mCurrentLine);
			mData = ParamToUNum(tok.c_str());
			gotNextOutput = true;
			break;
		}

		// <0,1,2,3,4,5,6,7,@$123,>$123,<..\foo\bar.bin
		if (mCurrentLine.at(0) == '<')
		{
			mCurrentLine = mCurrentLine.substr(1);
			unsigned int startPos = 0;
			mInputDataNumBytes = INT_MAX;
			mInputDataBits.clear();
			
			while (mCurrentLine.at(0) != '<')
			{
				std::string tok = getNextTok(mCurrentLine);
				if (tok.at(0) == '@')
				{
					tok = tok.substr(1);
					startPos = ParamToUNum(tok.c_str());
					continue;
				}
				else if (tok.at(0) == '>')
				{
					tok = tok.substr(1);
					mInputDataNumBytes = ParamToUNum(tok.c_str());
					continue;
				}
				else
				{
					mInputDataBits.push_back(ParamToUNum(tok.c_str()));
				}
			}

			if (mInputData.is_open())
			{
				mInputData.close();
			}
			mInputData.clear();
			mCurrentLine = mCurrentLine.substr(1);
			mInputData.open(mCurrentLine, std::ios_base::in | std::ios_base::binary);
			mInputData.seekg((size_t)startPos , std::ios::beg);
			mCurrentLine.clear();
			continue;
		}

		printf("Unkown comand: %s\n", mCurrentLine.c_str());
		break;
	}
}
