#include "stdafx.h"
#include "Data.h"
#include <sstream>
#include "../../C64/Common/ParamToNum.h"
#include <map>
#include <set>
#include <string>
#include <algorithm>


static std::map<std::string, std::string> labelValue;
static std::set<FILE*> recordingFiles;


// https://gist.github.com/dedeexe/9080526
std::string trim_left(const std::string& str)
{
	if (str.empty())
	{
		return "";
	}

	const std::string pattern = " \f\n\r\t\v";
	return str.substr(str.find_first_not_of(pattern));
}

std::string trim_right(const std::string& str)
{
	if (str.empty())
	{
		return "";
	}

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

static std::string ReplaceAll(std::string str, const std::string& from, const std::string& to)
{
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
	return str;
}

std::string getNextTok(std::string& str , std::string pattern = ", \f\n\r\t\v")
{
	// If '=' is used then do not tokenise on whitespace
	if (str.at(0) == '=')
	{
		pattern.erase(std::remove(pattern.begin() , pattern.end(), ' '));
		pattern.erase(std::remove(pattern.begin(), pattern.end(), '\t'));
	}
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

	// Look for any label replacements
	bool replaced = false;
	do
	{
		replaced = false;
		// Reverse iterate to get the longer strings first
		auto st = labelValue.rbegin();
		while (st != labelValue.rend())
		{
			const std::string &toCompare = st->first;

			std::string newRet = ReplaceAll(ret , toCompare, st->second);

			// If the substitution resulted in a change then use it and flag the change to cause further checks
			if (newRet.compare(ret) != 0)
			{
				ret = newRet;
				replaced = true;
			}

			st++;
		}
	} while (replaced);

	return ret;
}

Data::Data() : mFile(0) , mWaitingForTime(-1.0f)
{
}

Data::~Data()
{
	clear();
}

void Data::clear()
{
	delete mFile;
	mFile = 0;
	while (!mFiles.empty())
	{
		std::ifstream *file = mFiles.back();
		delete file;
		mFiles.pop_back();
		mFileNames.pop_back();
		mLineNumbers.pop_back();
	}
	mError.clear();
	mFiles.clear();
	mFileNames.clear();
	mLineNumbers.clear();
}

void Data::CheckFileIsOpen(std::ifstream *toCheck)
{
	if (!toCheck)
	{
		return;
	}
	if (toCheck->is_open())
	{
		return;
	}
	mError = "Could not open file: " + mCurrentLine;
}

void Data::init(const CHAR *filename)
{
	clear();
	mData = 0;
	mFile = new std::ifstream();
	mCurrentFilename = filename;
	mCurrentLineNumber = 0;
	mCurrentLine = filename;
	mFile->open(filename);
	CheckFileIsOpen(mFile);
	mCurrentLine.clear();

	mWaitingForPositiveEdge = 0;
	mWaitingForNegativeEdge = 0;
	mWaitingForMask = 0;
	mWaitingForData = 0;
	mWaitingForTime = -1.0f;
}

void Data::simulate(const double time, const unsigned int dInput, const unsigned int dInputPositiveEdge, const unsigned int dInputNegativeEdge)
{
	if (!mError.empty())
	{
		return;
	}
	if (!mFile)
	{
		return;
	}
	bool wasWaiting = waitingForInput();

	if ((mWaitingForTime >= 0.0f) && (time < mWaitingForTime))
	{
		return;
	}
	mWaitingForTime = -1.0f;

	if (mWaitingForMask && ((dInput & mWaitingForMask) != (mWaitingForData & mWaitingForMask)))
	{
		return;
	}
	mWaitingForMask = 0;
	mWaitingForData = 0;

	if (mWaitingForPositiveEdge && !(dInputPositiveEdge & mWaitingForPositiveEdge))
	{
		return;
	}
	mWaitingForPositiveEdge = 0;

	if (mWaitingForNegativeEdge && !(dInputNegativeEdge & mWaitingForNegativeEdge))
	{
		return;
	}
	mWaitingForNegativeEdge = 0;

	if (wasWaiting)
	{
		// To avoid data being fetched straight after the wait
		return;
	}

	if (mFile->eof())
	{
		return;
	}

	if (!mCountingBits.empty())
	{
		bool carry = true;
		for (int theBit : mCountingBits)
		{
			if (mData & (1 << theBit) && carry)
			{
				mData = mData & ~(1 << theBit);
				continue;
			}

			if (!(mData & (1 << theBit)) && carry)
			{
				mData = mData | (1 << theBit);
				carry = false;
				continue;
			}
		}
	}

	bool gotNextOutput = false;
	while (!gotNextOutput)
	{
		if (!mError.empty())
		{
			return;
		}
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


		while (mCurrentLine.empty() && !mFile->eof())
		{
			std::getline(*mFile, mCurrentLine);
			mCurrentLineNumber++;
			size_t pos = mCurrentLine.find(';');
			if (std::string::npos != pos)
			{
				mCurrentLine.erase(pos);
			}
			mCurrentLine = trim(mCurrentLine);
		}
		if (mFile->eof())
		{
			delete mFile;
			mFile = 0;
			if (!mFiles.empty())
			{
				mFile = mFiles.back();
				mFiles.pop_back();

				mCurrentFilename = mFileNames.back();
				mFileNames.pop_back();

				mCurrentLineNumber = mLineNumbers.back();
				mLineNumbers.pop_back();
				continue;
			}
			break;
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

		// s$00000000
		if (mCurrentLine.at(0) == 's')
		{
			mCurrentLine = mCurrentLine.substr(1);
			std::string tok = getNextTok(mCurrentLine);
			mData = ParamToUNum(tok.c_str());
			continue;
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
			if (mCurrentLine.length() > 1)
			{
				mInputData.open(mCurrentLine, std::ios_base::in | std::ios_base::binary);
				CheckFileIsOpen(&mInputData);
				mInputData.seekg((size_t)startPos, std::ios::beg);
			}
			mCurrentLine.clear();
			continue;
		}

		// +8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,+
		if (mCurrentLine.at(0) == '+')
		{
			mCurrentLine = mCurrentLine.substr(1);
			mCountingBits.clear();

			while (mCurrentLine.at(0) != '+')
			{
				std::string tok = getNextTok(mCurrentLine);
				mCountingBits.push_back(ParamToUNum(tok.c_str()));
			}
			std::string tok = getNextTok(mCurrentLine);
			continue;
		}

		// b$73
		if (mCurrentLine.at(0) == 'b')
		{
			mCurrentLine = mCurrentLine.substr(1);
			std::string tok = getNextTok(mCurrentLine);
			unsigned int theData = ParamToUNum(tok.c_str());

			for (int theBit : mInputDataBits)
			{
				mData = mData & ~(1 << theBit);
				mData = mData | ((theData & 1) << theBit);
				theData >>= 1;
			}
			gotNextOutput = true;
			break;
		}

		// ^+$xx
		// ^-$xx
		if (mCurrentLine.at(0) == '^')
		{
			char edge = mCurrentLine.at(1);

			mCurrentLine = mCurrentLine.substr(2);
			std::string tok = getNextTok(mCurrentLine);
			unsigned int theData = ParamToUNum(tok.c_str());

			if (edge == '+')
			{
				mWaitingForPositiveEdge = theData;
			}
			else
			{
				mWaitingForNegativeEdge = theData;
			}
			gotNextOutput = true;
			break;
		}

		// w$ffffffff,$1234
		if (mCurrentLine.at(0) == 'w')
		{
			mCurrentLine = mCurrentLine.substr(1);
			std::string tok = getNextTok(mCurrentLine);
			mWaitingForMask = ParamToUNum(tok.c_str());
			tok = getNextTok(mCurrentLine);
			mWaitingForData = ParamToUNum(tok.c_str());

			gotNextOutput = true;
			break;
		}

		// @time:0.735000
		if (mCurrentLine.compare(0 , 6 , "@time:") == 0)
		{
			mCurrentLine = mCurrentLine.substr(6);
			std::string tok = getNextTok(mCurrentLine);
			mWaitingForTime = atof(tok.c_str());
			if (time >= mWaitingForTime)
			{
				mWaitingForTime = -1.0f;
				continue;
			}

			gotNextOutput = true;
			break;
		}

		// !Test2.txt
		if (mCurrentLine.at(0) == '!')
		{
			mCurrentLine = mCurrentLine.substr(1);
			mFiles.push_back(mFile);
			mFileNames.push_back(mCurrentFilename);
			mLineNumbers.push_back(mCurrentLineNumber);

			mFile = new std::ifstream();
			mFile->open(mCurrentLine);
			mCurrentFilename = mCurrentLine;
			mCurrentLineNumber = 0;
			CheckFileIsOpen(mFile);

			mCurrentLine.clear();
			continue;
		}

		// .label1 = $000300
		if (mCurrentLine.at(0) == '.')
		{
			mCurrentLine = mCurrentLine.substr(1);

			std::string label = getNextTok(mCurrentLine , " \t=");
			std::string value = trim(mCurrentLine);

			labelValue[label] = value;

			mCurrentLine.clear();
			continue;
		}

		if (mCurrentLine.at(0) == ':')
		{
			mCurrentLine = mCurrentLine.substr(1);
			if (mCurrentLine.rfind("emit", 0) == 0)
			{
				mCurrentLine = mCurrentLine.substr(4);
				mCurrentLine = trim(mCurrentLine);

				if (!mCurrentLine.empty())
				{
					auto st = recordingFiles.rbegin();
					while (st != recordingFiles.rend())
					{
						FILE *fp = *st;

						fprintf(fp, "\n; %s\n", mCurrentLine.c_str());

						st++;
					}
				}

				mCurrentLine.clear();
				continue;
			}
		}

		printf("Unknown comand: %s\n", mCurrentLine.c_str());
		break;
	}
}

void Data::addRecordingFile(FILE* fp)
{
	if (0 == fp)
	{
		return;
	}

	recordingFiles.insert(fp);
}

void Data::removeRecordingFile(FILE* fp)
{
	if (0 == fp)
	{
		return;
	}

	recordingFiles.erase(fp);
}
