#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <iostream>
#include <string>

#include "database.h"
#include "resource.h"

class WebServer {
private:
    string_t hostAddr;
    resourceRef networksEP;
    resourceRef endpointsEP;
    resourceRef connectionsEP;
    dbRef db;
public:
    WebServer(string_t _hostAddr) : hostAddr(_hostAddr) {};
    void initServer();
    void startServer();
    void stopServer();
};

#endif