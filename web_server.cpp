#include <iostream>
#include <string>

#include "web_server.h"
#include "networks.h"
#include "endpoints.h"
#include "connections.h"

/*  
 * Initialize the server with requirred Database and resources
 */
void WebServer::initServer(){
    dbRef db = new DataBase();
    networksEP = new Networks(hostAddr, db);
    endpointsEP = new EndPoints(hostAddr, db);
    connectionsEP = new Connections(hostAddr, db);
}

/*
 * Start all the Resources configured before
 */
void WebServer::startServer() {
    networksEP->open();
    endpointsEP->open();
    connectionsEP->open();
}

/*
 * Stop all the Resources configured before
 */
void WebServer::stopServer() {
    delete networksEP;
    delete endpointsEP;
    delete connectionsEP;
}
