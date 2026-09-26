//
// Created by PinkySmile on 14/08/24.
//

#include <winsock2.h>
#include <windows.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")
#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <cstdio>

// Everything written to std::cout also goes to a file next to the exe, and
// every line gets a wall-clock timestamp in both.
//
// The console alone lost the evidence twice: once to its own flood, and once
// to a freeze that happened while a slot-ack loop was scrolling it away. A
// file keeps the whole session, and the timestamps turn "it froze" into
// "nothing went to that player for 1.8 seconds after 23:10:41.200".
class TeeBuf : public std::streambuf {
public:
	TeeBuf(std::streambuf *console, std::streambuf *file) : _console(console), _file(file) {}

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
			this->_console->sputn(stamp, n);
			this->_file->sputn(stamp, n);
			this->_lineStart = false;
		}
		this->_console->sputc((char)c);
		this->_file->sputc((char)c);
		if (c == '\n')
			this->_lineStart = true;
		return c;
	}

	// std::endl lands here, so each finished line reaches the disk: a relay
	// that is killed or crashes still leaves everything up to its last line.
	int sync() override
	{
		this->_console->pubsync();
		this->_file->pubsync();
		return 0;
	}

private:
	std::streambuf *_console;
	std::streambuf *_file;
	bool _lineStart = true;
};

static std::string logPathNextToExe()
{
	char exe[MAX_PATH];
	SYSTEMTIME t;
	char name[64];
	std::string dir;

	GetModuleFileNameA(nullptr, exe, sizeof(exe));
	dir = exe;
	dir = dir.substr(0, dir.find_last_of("\\/") + 1);
	GetLocalTime(&t);
	snprintf(name, sizeof(name), "relay-%04d%02d%02d-%02d%02d%02d.log", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
	return dir + name;
}
#include <cstdint>
#include <thread>
#include "Server.hpp"

namespace SokuLib {
	std::vector<std::string> charactersName{
		"Reimu Hakurei",
		"Marisa Kirisame",
		"Sakuya Izayoi",
		"Alice Margatroid",
		"Patchouli Knowledge",
		"Youmu Konpaku",
		"Remilia Scarlet",
		"Yuyuko Saigyouji",
		"Yukari Yakumo",
		"Suika Ibuki",
		"Reisen Udongein Inaba",
		"Aya Shameimaru",
		"Komachi Onozuka",
		"Iku Nagae",
		"Tenshi Hinanawi",
		"Sanae Kochiya",
		"Cirno",
		"Hong Meiling",
		"Utsuho Reiuji",
		"Suwako Moriya",
		"Random Select",
		"Namazu",
		//Soku2 Characters
		"Momiji Inubashiri",
		"Clownpiece",
		"Flandre Scarlet",
		"Rin Kaenbyou",
		"Yuuka Kazami",
		"Kaguya Houraisan",
		"Fujiwara no Mokou",
		"Mima",
		"Shou Tormaru",
		"Minamitsu Murasa",
		"Sekibanki",
		"Satori Komeiji",
		"Ran Yakumo",
		"Tewi Inaba"
	};
}

int main(int argc, char **argv)
{
	if (argc < 2 || argc > 4) {
		std::cerr << "Usage: " << argv[0] << " <port> [poll_interval_us=1000] [chrselect_delay_frames=10]" << std::endl;
		return EXIT_FAILURE;
	}

	// Was 5000 us, but that number was never what happened: Windows sleeps in
	// whole timer ticks, 15.6 ms by default, so every relayed packet could sit
	// unread for a full tick. In character select's lockstep that is added to
	// every input. timeBeginPeriod(1) below makes 1 ms mean 1 ms.
	static std::ofstream logFile{logPathNextToExe()};
	static TeeBuf tee{std::cout.rdbuf(), logFile.rdbuf()};

	if (logFile)
		std::cout.rdbuf(&tee);
	std::cout << "Logging to " << (logFile ? logPathNextToExe() : std::string("NOWHERE -- could not open a log file next to the exe")) << std::endl;

	uint64_t sleepTime = 1000;

#ifndef _DEBUG
	try {
#endif
		if (argc >= 3)
			sleepTime = std::stoull(argv[2]);
		if (argc >= 4)
			characterInputDelay = std::stoul(argv[3]);
		if (characterInputDelay < 2 || characterInputDelay > 30)
			throw std::invalid_argument("chrselect_delay_frames must be between 2 and 30");
		timeBeginPeriod(1);
		std::cout << "Polling every " << sleepTime << " us; character select input delay "
			<< characterInputDelay << " frames (~" << characterInputDelay * 1000 / 60
			<< " ms). If character select stutters for someone far away, raise the delay." << std::endl;
		if (sleepTime > 1000000 / 60)
			std::cerr << "Warning: Sleep time value is higher than 1/60s. This may induce some lag during games." << std::endl;

		unsigned port = std::stoul(argv[1]);

		if (port > UINT16_MAX)
			throw std::invalid_argument("Invalid port");

		Server server{static_cast<unsigned short>(port)};

		while (true) {
			server.update();
			std::this_thread::sleep_for(std::chrono::microseconds(sleepTime));
		}
#ifndef _DEBUG
	} catch (std::exception &e) {
		std::cerr << "Fatal error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
#endif
	return EXIT_SUCCESS;
}