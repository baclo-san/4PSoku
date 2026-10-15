//
// The relay, run inside the game by the player who hosts.
//
// 4P netplay always goes through the relay: every player, the host included,
// is a client of it. 2v2Server.exe used to be the only way to run one, so the
// host had to start it beside the game and then connect to 127.0.0.1. Now the
// game's own Host does both (see hostThroughRelay in main.cpp): the relay
// runs on a thread of its own, and the host's game joins it like everyone
// else's.
//

#include <winsock2.h>
#include <windows.h>
#include <timeapi.h>
#include <atomic>
#include <cstring>
#include <fstream>
#include <memory>
#include <thread>
#include "../Server/Server.hpp"
#include "../Server/RelayLog.hpp"
#include "HostRelay.hpp"

#pragma comment(lib, "winmm.lib")

// 4psoku-relay.log in the game folder, started over each time the game is.
static std::ofstream relayFile;
static RelayLogBuf relayBuf{relayFile.rdbuf()};
static std::ostream relayStream{&relayBuf};

std::ostream &relayLog()
{
	return relayStream;
}

// Never destroyed: a joinable std::thread's destructor ends the process, and
// at exit the thread is already gone without having been joined.
static std::thread *relayThread = nullptr;
static std::atomic<bool> relayStopping{false};
static unsigned short relayPort = 0;

void stopHostRelay()
{
	if (!relayThread)
		return;
	relayStopping = true;
	relayThread->join();
	delete relayThread;
	relayThread = nullptr;
	relayStopping = false;
	relayLog() << "Relay on port " << relayPort << " stopped." << std::endl;
	relayPort = 0;
}

bool startHostRelay(unsigned short port)
{
	if (relayThread && relayPort == port)
		return true;
	stopHostRelay();
	if (!relayFile.is_open())
		relayFile.open("4psoku-relay.log", std::ios::trunc);
	relayLog() << "Hosting from the game: starting the relay on port " << port << "." << std::endl;
	// The relay admits only games stamped with its own game id, the host's
	// included; 4PSoku stamps it at 0x858B80 with a copy kept in main.cpp.
	if (memcmp(versionString2v2, reinterpret_cast<const void *>(0x858B80), sizeof(versionString2v2)) != 0)
		relayLog() << "WARNING: this relay's game id differs from the one 4PSoku stamped into the game. "
			<< "It will refuse every player, this one included: the two copies of versionString2v2 "
			<< "(Server.cpp, SokuMod/main.cpp) must be bumped together." << std::endl;

	std::unique_ptr<Server> server;

	try {
		server = std::make_unique<Server>(port);
	} catch (std::exception &e) {
		relayLog() << "Could not start the relay: " << e.what() << std::endl;
		return false;
	}
	relayPort = port;
	relayThread = new std::thread([s = std::move(server)]() {
		// The relay forwards traffic as it polls: a 15 ms default timer
		// would add up to that much to every packet.
		timeBeginPeriod(1);
		while (!relayStopping) {
			try {
				s->update();
			} catch (std::exception &e) {
				relayLog() << "Relay error, carrying on: " << e.what() << std::endl;
			}
			Sleep(1);
		}
		timeEndPeriod(1);
	});
	return true;
}
