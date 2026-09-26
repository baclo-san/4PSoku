//
// Created by PinkySmile on 14/08/24.
//

#include <winsock2.h>
#include <windows.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")
#include <iostream>
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