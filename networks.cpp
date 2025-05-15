#include <regex>
#include "networks.h"

#define TENANT_ID_FIELD "tenant_id"
#define VTEP_IP_FIELD "vtep_ip"

void Networks::handleQueryParams(http_request request, dbOpRet& dbRet) {
    auto r = request.relative_uri();
    auto q = uri::split_query(r.query());
    if(q.size() == 0)
        return;
    for(auto a : q) {
        if(a.first == this->allowedQueryParam) {
            for(int idx = 0, cur = 0; idx < dbRet.retVal.size(); idx++) {
                if(std::to_string(dbRet.retVal[idx][U(allowedQueryParam)].as_integer()) == a.second) {
                    dbRet.retVal = dbRet.retVal[idx];
                    return;
                }
            }
        }
    }
    dbRet.retStatus = DB_NOTFOUND;
}

bool Networks::isValidIPv4Address(const std::string& address) {
    std::regex ipv4Regex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    return std::regex_match(address, ipv4Regex);
}

/*
 * Overriding the default behaviour to support 
 * extra validation on the JSON received in a
 * POST / PUT / PATCH request
 */ 
std::string Networks::validateJson(json::value json) {
    if(! json.has_string_field(TENANT_ID_FIELD)) {
        return "Tenant Id not present";
    }
    std::string tenantId = json[U(TENANT_ID_FIELD)].as_string();
    
    if(! json.has_string_field(VTEP_IP_FIELD)) {
        return "VTEP IP not present";
    } 
    std::string vtepIp = json[U(VTEP_IP_FIELD)].as_string();
    if(! this->isValidIPv4Address(vtepIp)) {
        return "VTEP IP not valid";
    }


    /*
    json::value val = json[U("author")];
    if(! val.is_integer()) {
        return false;
    }
    int authorId = val.as_integer();
    std::string sCurUri = AUTHOR_Resource + std::to_string(authorId);
    string_t curUri = conversions::to_string_t(sCurUri);

    http_client client(curUri);
    bool stat = false;
    client.request(methods::GET).then([&stat](http_response response) {
            if (response.status_code() == status_codes::OK) {
                stat = true;
            }
        }).wait();
    */ 
    return "";
}
