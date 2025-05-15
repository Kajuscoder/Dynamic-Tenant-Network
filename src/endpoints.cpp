#include <regex>
#include "endpoints.h"

#define IP_FIELD "ip"
#define MAC_FIELD "mac"
#define IFC_FIELD "interface"

bool EndPoints::isValidIPv4Address(const std::string& address) {
    std::regex ipv4Regex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    return std::regex_match(address, ipv4Regex);
}

/*
 * Overriding the default behaviour to support 
 * extra validation on the JSON received in a
 * POST / PUT / PATCH request
 */ 
std::string EndPoints::validateJson(json::value json) {
    if(! json.has_string_field(IP_FIELD)) {
        return "Endpoint IP not present";
    }
    std::string ip = json[U(IP_FIELD)].as_string();
    if(! this->isValidIPv4Address(ip)) {
        return "Endpoint IP not valid";
    }
    
    if(! json.has_string_field(MAC_FIELD)) {
        return "MAC not present";
    } 
    std::string mac = json[U(MAC_FIELD)].as_string();

    if(! json.has_string_field(IFC_FIELD)) {
        return "Endpoint Interface not present";
    }
     
    return "";
}
