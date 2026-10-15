//
// Where the relay writes its log. The relay runs either as 2v2Server.exe or
// inside 4PSoku.dll when a player hosts from the game, and the two log to
// different places: the exe to its console and a file next to it, the DLL to
// a file in the game folder only (the game's console, when it has one, is the
// mod's). Each defines relayLog(); Server.cpp only ever writes to it.
//

#ifndef INC_4PSOKU_RELAYLOG_HPP
#define INC_4PSOKU_RELAYLOG_HPP


#include <windows.h>
#include <cstdio>
#include <ostream>
#include <streambuf>

std::ostream &relayLog();

// Stamps each line with the local time and copies it to one or two streams.
class RelayLogBuf : public std::streambuf {
public:
	RelayLogBuf(std::streambuf *first, std::streambuf *second = nullptr) : _first(first), _second(second) {}

protected:
	int overflow(int c) override
	{
		if (c == EOF)
			return 0;
		if (this->_lineStart) {
			SYSTEMTIME t;
			char stamp[32];

			GetLocalTime(&t);
			int n = snprintf(stamp, sizeof(stamp), "%02d:%02d:%02d.%03d ", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
			this->_put(stamp, n);
			this->_lineStart = false;
		}
		char ch = static_cast<char>(c);

		this->_put(&ch, 1);
		if (c == '\n')
			this->_lineStart = true;
		return c;
	}

	int sync() override
	{
		this->_first->pubsync();
		if (this->_second)
			this->_second->pubsync();
		return 0;
	}

private:
	std::streambuf *_first;
	std::streambuf *_second;
	bool _lineStart = true;

	void _put(const char *s, std::streamsize n)
	{
		this->_first->sputn(s, n);
		if (this->_second)
			this->_second->sputn(s, n);
	}
};


#endif //INC_4PSOKU_RELAYLOG_HPP
