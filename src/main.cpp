#include "web_server.h"

/*
 * The URL where the Resources will be running
 */
#define HOST_ADDRESS "http://localhost:8080"

int main() {
    WebServer ws(HOST_ADDRESS);
    ws.initServer();
    ws.startServer();

    std::wcout << L"Press Enter to Stop " << std::endl;
    std::string line;
    std::getline(std::cin, line);  // Keeps the program running to listen to requests

    ws.stopServer();

    return 0;
}