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

void onMessage(int client_fd, const std::string& msg) {

    std::cout << "Received from client " << client_fd << ": " << msg << "\n";

    json state;
    state["received"] = true;
    state["message"] = "test";
    state["client_fd"] = client_fd;
    server.sendFrame(client_fd, state.dump(2));
}

int main() {

    const char* port = "9002";

    server.init(port, &onMessage, true);
    std::cout << "Server started on " << port << std::endl;

    server.run();
    return 0;
}