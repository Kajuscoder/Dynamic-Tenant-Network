#ifndef RESOURCE_H
#define RESOURCE_H

#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <locale>
#include <codecvt>

#include "database.h"

using namespace utility;               
using namespace web;                   
using namespace web::http;
using namespace web::http::experimental::listener;

/* 
 * Used to detect the id of an target entry provided 
 * in a GET / PUT / PATCH / DELETE request
 */
typedef enum _uriIdValue {
    URI_INVALID_ID = -1,
    URI_NO_ID,
    URI_VALID_ID
} uriIdValue;


class Resource {
private:
    http_listener listener;
    
    string_t uri;
    dbRef db;
    dbTableRef dbTable;
    int keyCounter;
    string_t unique_key;
    string_t auto_incr_key;

    /* 
     * Methods that can be overrode to extend the features 
     */
    virtual std::string validateJson(json::value json);
    virtual void handleQueryParams(http_request request, dbOpRet& dbRet);
    virtual void afterPostAction(json::value json);
    virtual void afterDeleteAction(json::value json);
    virtual bool allowPutMethod();

    void handleGet(http_request request);
    void handlePost(http_request request);
    void handlePut(http_request request);
    void handlePatch(http_request request);
    void handleDelete(http_request request);

public:
    Resource(string_t host, string_t resource, dbRef _db, 
             string_t _unique_key, string_t _auto_incr_key);
    void open();
};

#define resourceRef Resource*

#endif