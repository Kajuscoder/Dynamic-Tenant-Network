#ifndef ENDPOINTS_H
#define ENDPOINTS_H

#include "resource.h"
#include "database.h"

#include <cpprest/json.h>
#include <cpprest/http_client.h>

using namespace utility;               
using namespace web;     
using namespace web::http;
using namespace web::http::client; 

class EndPoints : public Resource {
private:
    bool isValidIPv4Address(const std::string& address);
    virtual std::string validateJson(json::value json);
public:
    EndPoints(string_t host, dbRef _db) : 
        Resource(host, "endpoints", _db, "", "endpoint_id") {};
};

#endif