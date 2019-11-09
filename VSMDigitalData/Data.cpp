#include "stdafx.h"
#include "Data.h"
#include <sstream>

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

std::string getNextTok(std::string& str)
{
	const std::string pattern = ", \f\n\r\t\v";
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
		while (mCurrentLine.empty() && !mFile.eof())
		{
			std::getline(mFile, mCurrentLine);
			mCurrentLine = trim(mCurrentLine);
		}
		if (mFile.eof())
		{
			break;
		}

		// $00000000
		if (mCurrentLine.at(0) == '$')
		{
			std::string tok = getNextTok(mCurrentLine);
			sscanf(tok.c_str(), "$%x", &mData);
			gotNextOutput = true;
			break;
		}
	}
}
