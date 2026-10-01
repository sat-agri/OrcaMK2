#pragma once

namespace pixhawk {
// Call after Serial1 and motor startup; starts the communication timeout.
void begin();
// Sends heartbeat, checks the link, then handles incoming MAVLink messages.
void update();
}
