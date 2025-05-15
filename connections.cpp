#include <regex>
#include "connections.h"

#define VNI_FIELD "vni"
#define ENDPOINT_FIELD "endpoint_id"

#define NETWORKS_RESOURCE "networks"
#define ENDPOINTS_RESOURCE "endpoints"

bool Connections::allowPutMethod() {
    return false;
}

void Connections::afterPostAction(json::value json) {
    int vni = json[U(allowedQueryParam1)].as_integer();
    int endpoint_id = json[U(allowedQueryParam2)].as_integer();

    std::string sCurUri = hostAddr + "/" + ENDPOINTS_RESOURCE "/" + std::to_string(endpoint_id);
    string_t curUri = conversions::to_string_t(sCurUri);

    http_client client1(curUri);
    json::value endpoint;
    http_response res1;
    client1.request(methods::GET)
           .then([&res1](http_response response) {
                res1 = response; 
            }).wait();
    res1.extract_json()
            .then([&endpoint] (json::value _json) { 
                endpoint = _json;
            })
            .wait();

    sCurUri = hostAddr + "/" + NETWORKS_RESOURCE "?" 
                          + VNI_FIELD + "=" + std::to_string(vni);
    curUri = conversions::to_string_t(sCurUri);

    http_client client2(curUri);
    json::value network;
    http_response res2;
    client2.request(methods::GET)
           .then([&res2](http_response response) {
                res2 = response; 
            }).wait();
    res2.extract_json()
            .then([&network] (json::value _json) { 
                network = _json;
            })
            .wait();

    
    std::string endpoint_mac = endpoint[U("mac")].as_string();
    std::string endpoint_interface = endpoint[U("interface")].as_string();
    std::string endpoint_ip = endpoint[U("ip")].as_string();
    std::string vtep_ip = network[U("vtep_ip")].as_string();
    
    std::string write_into_log = 
        "# Connecting Endpoint " + std::to_string(endpoint_id) + " to VNI " + std::to_string(vni) + "\n" +
        "ip link add vxlan0 type vxlan id " + std::to_string(vni) + 
        " dev " + endpoint_interface + " remote " + vtep_ip + " dstport 4789\n" +
        "ip link set vxlan0 up\n" + 
        "ip link add name br0 type bridge\n" +
        "ip link set dev br0 up\n" +
        "ip link set dev vxlan0 master br0\n" + 
        "bridge fdb append " + endpoint_mac + " dev vxlan0 dst " + endpoint_ip + "\n\n\n";

    *commandLog << write_into_log << std::endl;
      
}

void Connections::afterDeleteAction(json::value json) {
    int vni = json[U(allowedQueryParam1)].as_integer();
    int endpoint_id = json[U(allowedQueryParam2)].as_integer();

    std::string sCurUri = hostAddr + "/" + ENDPOINTS_RESOURCE "/" + std::to_string(endpoint_id);
    string_t curUri = conversions::to_string_t(sCurUri);

    http_client client1(curUri);
    json::value endpoint;
    http_response res1;
    client1.request(methods::GET)
           .then([&res1](http_response response) {
                res1 = response; 
            }).wait();
    res1.extract_json()
            .then([&endpoint] (json::value _json) { 
                endpoint = _json;
            })
            .wait();

    sCurUri = hostAddr + "/" + NETWORKS_RESOURCE "?" 
                          + VNI_FIELD + "=" + std::to_string(vni);
    curUri = conversions::to_string_t(sCurUri);

    http_client client2(curUri);
    json::value network;
    http_response res2;
    client2.request(methods::GET)
           .then([&res2](http_response response) {
                res2 = response; 
            }).wait();
    res2.extract_json()
            .then([&network] (json::value _json) { 
                network = _json;
            })
            .wait();

    
    std::string endpoint_mac = endpoint[U("mac")].as_string();
    std::string endpoint_interface = endpoint[U("interface")].as_string();
    std::string endpoint_ip = endpoint[U("ip")].as_string();
    std::string vtep_ip = network[U("vtep_ip")].as_string();
    
    std::string write_into_log = 
        "# Disconnecting Endpoint " + std::to_string(endpoint_id) + " to VNI " + std::to_string(vni) + "\n" +
        "bridge fdb delete " + endpoint_mac + " dev vxlan0 dst " + endpoint_ip + "\n" +
        "ip link set vxlan0 down\n" +
        "ip link delete vxlan0\n\n";

    *commandLog << write_into_log << std::endl;
      
}

void Connections::handleQueryParams(http_request request, dbOpRet& dbRet) {
    auto r = request.relative_uri();
    auto q = uri::split_query(r.query());
    if(q.size() == 0 || q.size() > 1)
        return;
    for(auto a : q) {
        if(a.first == this->allowedQueryParam1) {
            auto responseJson = json::value::array();
            for(int idx = 0, cur = 0; idx < dbRet.retVal.size(); idx++) {
                if(std::to_string(dbRet.retVal[idx][U(allowedQueryParam1)].as_integer()) == a.second) {
                    responseJson[cur++] = dbRet.retVal[idx];
                }
            }
            dbRet.retVal = responseJson;
            return;
        }
        if(a.first == this->allowedQueryParam2) {
            auto responseJson = json::value::array();
            for(int idx = 0, cur = 0; idx < dbRet.retVal.size(); idx++) {
                if(std::to_string(dbRet.retVal[idx][U(allowedQueryParam2)].as_integer()) == a.second) {
                    responseJson[cur++] = dbRet.retVal[idx];
                }
            }
            dbRet.retVal = responseJson;
            return;
        }
    }
    dbRet.retStatus = DB_NOTFOUND;
}

/*
 * Overriding the default behaviour to support 
 * extra validation on the JSON received in a
 * POST / PUT / PATCH request
 */ 
std::string Connections::validateJson(json::value json) {
    if(! json.has_integer_field(VNI_FIELD)) {
        return "VNI invalid";
    }
    
    if(! json.has_integer_field(ENDPOINT_FIELD)) {
        return "Endpoint ID invalid";
    } 

    json::value val = json[U(VNI_FIELD)];
    int vni = val.as_integer();
    
    val = json[U(ENDPOINT_FIELD)];
    int endpoint_id = val.as_integer();

    
    std::string sCurUri = hostAddr + "/" + ENDPOINTS_RESOURCE "/" + std::to_string(endpoint_id);
    string_t curUri = conversions::to_string_t(sCurUri);

    http_client client1(curUri);
    std::string stat = "";
    client1.request(methods::GET).then([&stat](http_response response) {
            if (response.status_code() != status_codes::OK) {
                stat = "Endpoint does not exist";
            }
        }).wait();

    if(stat != "")
        return stat;

    sCurUri = hostAddr + "/" + NETWORKS_RESOURCE "?" 
                          + VNI_FIELD + "=" + std::to_string(vni);
    curUri = conversions::to_string_t(sCurUri);

    http_client client2(curUri);
    client2.request(methods::GET).then([&stat](http_response response) {
            if (response.status_code() != status_codes::OK) {
                stat = "VNI does not exist";
            }
        }).wait();
    return stat;
}
