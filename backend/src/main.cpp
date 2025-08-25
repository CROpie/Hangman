#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <set>
#include <cstdlib>

#include "minisocket.hpp"

#include "json.hpp"
using json = nlohmann::json;

minisocket::Server server;

void onMessage(int client, const std::string& msg) {

    std::cout << "Received from client " << client << ": " << msg << "\n";

    json state;
    state["received"] = true;
    state["message"] = "test";
    server.sendFrame(client, state.dump(2));
}

int main() {

    // std::string host = "127.0.0.1";
    const char* port = "9002";

    server.init(port, &onMessage, true);
    std::cout << "Server started on " << port << std::endl;

    server.run();
    return 0;
}