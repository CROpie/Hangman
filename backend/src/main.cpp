#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <set>
#include <cstdlib>

#define ASIO_STANDALONE
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "json.hpp"
using json = nlohmann::json;

typedef websocketpp::server<websocketpp::config::asio> server;
server wsServer;

void onMessage(websocketpp::connection_hdl hdl, server::message_ptr msg) {

    std::cout << "onMessage\n";

    // if (msg->get_payload().empty()) return;

    std::cout << msg->get_payload();

    json state;
    state["received"] = true;
    state["message"] = "test";
    wsServer.send(hdl, state.dump(), websocketpp::frame::opcode::text);

}

int main() {

    wsServer.init_asio();
    wsServer.set_message_handler(&onMessage);

    std::string host = "127.0.0.1";
    int port = 9002;

    asio::ip::tcp::endpoint ep(asio::ip::address::from_string(host), port);
    wsServer.listen(ep);

    wsServer.start_accept();

    std::cout << "Server started on " << ep.address().to_string() << std::endl;
    wsServer.run();
    return 0;

}