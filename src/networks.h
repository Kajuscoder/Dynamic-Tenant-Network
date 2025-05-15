#ifndef NETWORKS_H
#define NETWORKS_H

#include "resource.h"
#include "database.h"

#include <cpprest/json.h>
#include <cpprest/http_client.h>

using namespace utility;               
using namespace web;     
using namespace web::http;
using namespace web::http::client; 


class Networks : public Resource {
private:
    std::string allowedQueryParam;
    bool isValidIPv4Address(const std::string& address);
    void handleQueryParams(http_request request, dbOpRet& dbRet);
    virtual std::string validateJson(json::value json);
public:
    Networks(string_t host, dbRef _db) : allowedQueryParam("vni"),
        Resource(host, "networks", _db, "tenant_id", "vni") {};
};

#endif