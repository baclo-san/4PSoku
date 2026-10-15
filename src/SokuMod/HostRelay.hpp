//
// The relay inside the game, for the player who hosts. See HostRelay.cpp.
//

#ifndef INC_4PSOKU_HOSTRELAY_HPP
#define INC_4PSOKU_HOSTRELAY_HPP


// Starts the relay on this port, or keeps it if it already runs there; one on
// another port is stopped first. False if the port cannot be bound.
bool startHostRelay(unsigned short port);
void stopHostRelay();


#endif //INC_4PSOKU_HOSTRELAY_HPP
