#include "resource.h"

#define DEFAULT_UNIQUE_ID "unique"

/*
 * This method can be used to validate the entries in
 * the JSON provided in a POST / PUT / PATCH method 
 * before pushing the JSON into the database. 
 * 
 * By default, it is allowing all JSONs; however, the 
 * behaviour can be overrode in a subclass
 */
std::string Resource::validateJson(json::value json) {
    return "";
}

/* 
 * This method is called when POST method is adding 
 * a new entry into Database
 */
void Resource::afterPostAction(json::value json) {
    return;
}

/* 
 * This method is called when DELETE method is deleting 
 * an entry from Database
 */
void Resource::afterDeleteAction(json::value json) {
    return;
}

/*
 * This method can be used to restrict PUT method
 * i.e. Update on a resource
 */
bool Resource::allowPutMethod() {
    return true;
}


/* 
 * This method is called to handle Query  
 * parameters
 */
void Resource::handleQueryParams(http_request request, dbOpRet& dbRet) {
    return;
}

/* 
 * This method is invoked in case of a GET request and
 * returns an appropriate JSON as the response
 */
void Resource::handleGet(http_request request) {
    std::vector<std::string> path = uri::split_path(request.relative_uri().path());

    json::value responseJson;
    dbOpRet dbOp(DB_SUCCESS);
    /* 
     * When no particular entry is targetted, returns all
     * the entries as a JSON array
     */
    if(path.size() == 0) {
        dbOp = db->getEntry(dbTable);
        handleQueryParams(request, dbOp);
    /*
     * Otherwise returns the exact target entry
     */
    } else {
        if(unique_key == "") {
            std::vector<std::string> newPath;
            newPath.push_back(DEFAULT_UNIQUE_ID);
            for(auto& a : path)
                newPath.push_back(a);
            path = newPath;
        }

        if(path.size() == 1) {
            dbOp = db->getEntry(dbTable, path[0]);
        } else if(path.size() == 2) {
            dbOp = db->getEntry(dbTable, path[0], path[1]);
        } else {
            request.reply(status_codes::BadRequest, U("Invalid URI"));
            return;
        }
    }

    if(dbOp.retStatus == DB_NOTFOUND) {
        request.reply(status_codes::NotFound);
        return;
    } 
    responseJson = dbOp.retVal;
    request.reply(status_codes::OK, responseJson); 
}

/* 
 * This method is invoked in case of a POST request and
 * returns the newly added JSON as the response
 */
void Resource::handlePost(http_request request) {
    std::vector<std::string> path = uri::split_path(request.relative_uri().path());
    dbOpStatus stat;
    json::value responseJson;

    /*
     * POST request will only be entertained when the 
     * absolute URI is used e.g. localhost:8080/resource/ 
     */
    if(path.size() == 0) {
        request.extract_json().then([this, request, &stat, &responseJson] (json::value requestJson) {
            
            std::string vj = validateJson(requestJson);
            if(vj != "") {
                request.reply(status_codes::BadRequest, U(vj));
                return;
            }

            /* 
             * An unique Id is given to each new entry by 
             * incrementing a simple integer counter. 
             * 
             * This unique id is used as 2nd-level key in the 
             * database and to target an entry during a 
             * GET / PUT / PATCH / DELETE request
             */
            int id = ++keyCounter;
            std::string uk = (unique_key != "") ? 
                              requestJson[U(unique_key)].as_string() :
                              DEFAULT_UNIQUE_ID;
            requestJson[U(auto_incr_key)] = json::value::number(id);
            stat = db->addEntry(dbTable, uk, std::to_string(id), requestJson);
            responseJson = requestJson;
        }).wait();

        if(stat == DB_ADDED) {
            this->afterPostAction(responseJson);
            request.reply(status_codes::Created, responseJson);
        } else {
            request.reply(status_codes::InternalError);
        }
    /* 
     * Request will be ignored if any particular entry 
     * is targetted e.g. localhost:8080/resource/1/2
     */
    } else {
        request.reply(status_codes::BadRequest, U("Invalid URI"));
    } 
}

/* 
 * This method is invoked in case of a PUT request and
 * returns the newly added / updated JSON as the response
 */
void Resource::handlePut(http_request request) {
    if(! allowPutMethod()) {
        request.reply(status_codes::Forbidden);
    }

    std::vector<std::string> path = uri::split_path(request.relative_uri().path());
    dbOpStatus stat;
    json::value responseJson;

    /*
     * Unlike POST request, only entertain if the URI is
     * targetting a specific entry in the resource
     * e.g. localhost:8080/resource/1/2
     */
    if(unique_key == "") {
        std::vector<std::string> newPath;
        newPath.push_back(DEFAULT_UNIQUE_ID);
        for(auto& a : path)
            newPath.push_back(a);
        path = newPath;
    }
    if(path.size() == 2) {
        request.extract_json().then([this, request, &path, &stat, &responseJson] (json::value requestJson) {           
            std::string vj = validateJson(requestJson);
            if(vj != "") {
                request.reply(status_codes::BadRequest, U(vj));
                return;
            }


            /*
             * The unique_key and auto_incr_key of an existing entry 
             * will not be modified ever
             */
            int id = std::stoi(path[1]);
            if(unique_key != "")
                requestJson[U(unique_key)] = json::value::string(path[0]);
            requestJson[U(auto_incr_key)] = json::value::number(id);
            stat = db->updateEntry(dbTable, path[0], path[1], requestJson);
            responseJson = requestJson;
        }).wait();

        switch(stat) {
            case DB_UPDATED: {
                request.reply(status_codes::OK, responseJson);
            } 
            case DB_NOTFOUND: {
                request.reply(status_codes::NotFound);
            }
            default: {
                request.reply(status_codes::InternalError);
            } 
        }
    /*
     * Ignore the PUT request, if no valid "id" has been provided
     * in the URI e.g. localhost:8080/resource/
     */
    } else {
        request.reply(status_codes::BadRequest, U("Invalid URI"));
    }
} 

/* 
 * This method is invoked in case of a PATCH request and
 * returns the updated JSON as the response
 */
void Resource::handlePatch(http_request request) {
    std::vector<std::string> path = uri::split_path(request.relative_uri().path());
    dbOpStatus stat;
    json::value responseJson;

    /*
     * Like PUT request, only entertain if the URI is
     * targetting a specific entry in the resource
     * e.g. localhost:8080/resource/1/1
     */
    if(unique_key == "") {
        std::vector<std::string> newPath;
        newPath.push_back(DEFAULT_UNIQUE_ID);
        for(auto& a : path)
            newPath.push_back(a);
        path = newPath;
    }
    if(path.size() == 2) {
        request.extract_json().then([this, request, &path, &stat, &responseJson] (json::value requestJson) {           
            int id = std::stoi(path[1]);
            dbOpRet ret = db->getEntry(dbTable, path[0], path[1]);
            if(ret.retStatus == DB_NOTFOUND) {
                request.reply(status_codes::NotFound);
                return;
            } 
            responseJson = ret.retVal;
            
            /*
             * Copy JSON key-value pairs from the request
             * into the existing JSON of the database 
             */
            if (requestJson.is_object()) {
                for (const auto& item : requestJson.as_object()) {
                    responseJson[item.first] = item.second;
                }
            }

            /*
             * The keys of an existing entry will not be modified ever
             */
            if(unique_key != "")
                requestJson[U(unique_key)] = json::value::string(path[0]);
            requestJson[U(auto_incr_key)] = json::value::number(id);

            std::string vj = validateJson(requestJson);
            if(vj != "") {
                request.reply(status_codes::BadRequest, U(vj));
                return;
            }


            stat = db->updateEntry(dbTable, path[0], path[1], responseJson);
        }).wait();

        switch(stat) {
            case DB_UPDATED: {
                request.reply(status_codes::OK, responseJson);
            } 
            case DB_NOTFOUND: {
                request.reply(status_codes::NotFound);
            }
            default: {
                request.reply(status_codes::InternalError);
            } 
        }
    /*
     * Ignore the PATCH request, if no valid "id" has been provided
     * in the URI e.g. localhost:8080/resource/
     */
    } else {
        request.reply(status_codes::BadRequest, U("Invalid URI"));
    }
} 

/* 
 * This method is invoked in case of a DELETE request
 */
void Resource::handleDelete(http_request request) {
    std::vector<std::string> path = uri::split_path(request.relative_uri().path());

    /*
     * Like PUT / PATCH request, only entertain if the URI is
     * targetting a specific entry in the resource
     * e.g. localhost:8080/resource/1/2
     */
    if(unique_key == "") {
        std::vector<std::string> newPath;
        newPath.push_back(DEFAULT_UNIQUE_ID);
        for(auto& a : path)
            newPath.push_back(a);
        path = newPath;
    }
    if(path.size() == 2) {
        int id = std::stoi(path[1]);
        dbOpRet dbOp = db->getEntry(dbTable, path[0], path[1]);
        if(dbOp.retStatus == DB_SUCCESS)
            afterDeleteAction(dbOp.retVal);
        dbOpStatus opStat = db->removeEntry(dbTable, path[0], path[1]);

        switch(opStat) {
            case DB_REMOVED: {
                request.reply(status_codes::OK);
            } 
            case DB_NOTFOUND: {
                request.reply(status_codes::NotFound);
            }
            default: {
                request.reply(status_codes::InternalError);
            } 
        }
    /*
     * Ignore the DELETE request, if no valid "id" has been provided
     * in the URI e.g. localhost:8080/resource/
     */
    } else {
        request.reply(status_codes::BadRequest, U("Invalid URI"));
    }
}

/*
 * Constructor of the class takes host_address 
 * (e.g. https://localhost:8080), resource_name, 
 * a reference to the database etc as input  
 */
Resource::Resource(string_t host, string_t resource, dbRef _db, 
                   string_t _unique_key, string_t _auto_incr_key) : 
                listener(U(host + "/" + resource)), 
                db(_db), 
                keyCounter(0),
                unique_key(_unique_key),
                auto_incr_key (_auto_incr_key) {
    uri = host + "/" + resource;
    /*
     * Create a table in the database for the current resource
     */
    dbTable = db->createTable(resource);

    /*
     * Map relevant functions for each of the HTTP methods 
     */
    listener.support(methods::GET, std::bind(&Resource::handleGet, this, std::placeholders::_1));
    listener.support(methods::POST, std::bind(&Resource::handlePost, this, std::placeholders::_1));
    listener.support(methods::DEL, std::bind(&Resource::handleDelete, this, std::placeholders::_1));
    listener.support(methods::PUT, std::bind(&Resource::handlePut, this, std::placeholders::_1));
    listener.support(methods::PATCH, std::bind(&Resource::handlePatch, this, std::placeholders::_1));
}

/*
 * This method starts the Resource 
 */
void Resource::open() {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring ws = converter.from_bytes(uri);
    listener
        .open()
        .then([ws]() { std::wcout << L"Listening at: " << ws << std::endl; })
        .wait();
}