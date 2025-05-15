#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include "resource.h"
#include "database.h"

#include <cpprest/json.h>
#include <cpprest/http_client.h>
#include <fstream>

using namespace utility;               
using namespace web;     
using namespace web::http;
using namespace web::http::client; 


class Connections : public Resource {
private:
    std::string hostAddr;
    std::string allowedQueryParam1;
    std::string allowedQueryParam2;
    std::ofstream *commandLog;

    virtual void handleQueryParams(http_request request, dbOpRet& dbRet);
    virtual std::string validateJson(json::value json);
    virtual void afterPostAction(json::value json);
    virtual void afterDeleteAction(json::value json);
    virtual bool allowPutMethod();
    
public:
    Connections(string_t host, dbRef _db) :
        hostAddr(host), 
        allowedQueryParam1("vni"),
        allowedQueryParam2("endpoint_id"),
        Resource(host, "connections", _db, "", "conn_id") {
            commandLog = new std::ofstream("command.log");
        };
    ~Connections() {
        commandLog->close();
    }
};

#endif