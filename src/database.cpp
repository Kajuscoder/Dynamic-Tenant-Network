#include "database.h"

DataBase::DataBase() {
}

/*
 * A tableRef is returned which is just a string 
 * representing the name of the resource. 
 */
dbTableRef DataBase::createTable(string_t resourceName) {
    return resourceName;
}

/*
 * To fetch all the entries of the table or the resource
 */
dbOpRet DataBase::getEntry(dbTableRef table) {
    auto responseJson = json::value::array();
            
    int index = 0;
    for (const auto& pair : inMemDB[table]) {
        for(const auto& pair2 : pair.second)
        responseJson[index++] = pair2.second; 
    }

    return dbOpRet(DB_SUCCESS, responseJson);
}

/*
 * To fetch a specific entry of the table or the resource
 */
dbOpRet DataBase::getEntry(dbTableRef table, std::string key) {
    auto responseJson = json::value::array();
        
    auto it = inMemDB[table].find(key);
    if(it == inMemDB[table].end()) {
        return dbOpRet(DB_NOTFOUND);
    } else {
        auto responseJson = json::value::array();
            
        auto itr = it->second;
        int index = 0;
        for (auto i = itr.begin(); i != itr.end(); i++) {
            responseJson[index++] = i->second; 
        }

        return dbOpRet(DB_SUCCESS, responseJson);
    }
}

dbOpRet DataBase::getEntry(dbTableRef table, std::string key1, std::string key2) {
    auto responseJson = json::value::array();
        
    auto it = inMemDB[table].find(key1);
    if(it == inMemDB[table].end()) {
        return dbOpRet(DB_NOTFOUND);
    } else {
        auto responseJson = json::value::array();
            
        auto m = it->second;
        auto it = m.find(key2);
        if(it == m.end()) {
            return dbOpRet(DB_NOTFOUND);
        } else {
            return dbOpRet(DB_SUCCESS, it->second);
        }
    }
}

/*
 * To add a specific entry into the table or the resource
 */
dbOpStatus DataBase::addEntry(dbTableRef table, std::string key1, std::string key2, json::value entry) {
    inMemDB[table][key1][key2] = entry;
    return DB_ADDED;
}

/*
 * To update a specific entry into the table or the resource
 */
dbOpStatus DataBase::updateEntry(dbTableRef table, std::string key1, std::string key2, json::value entry) {
    if(inMemDB[table].find(key1) == inMemDB[table].end()) {
        return DB_NOTFOUND;
    } else {
        if(inMemDB[table][key1].find(key2) == inMemDB[table][key1].end())
            return DB_NOTFOUND;
        inMemDB[table][key1][key2] = entry;
        return DB_UPDATED;
    }       
}

/*
 * To remove a specific entry into the table or the resource
 */
dbOpStatus DataBase::removeEntry(dbTableRef table, std::string key) {
    if(inMemDB[table].find(key) == inMemDB[table].end()) {
        return DB_NOTFOUND;
    } else {
        inMemDB[table].erase(key);
        return DB_REMOVED;
    }       
}

dbOpStatus DataBase::removeEntry(dbTableRef table, std::string key1, std::string key2) {
    if(inMemDB[table].find(key1) == inMemDB[table].end()) {
        return DB_NOTFOUND;
    } else {
        if(inMemDB[table][key1].find(key2) == inMemDB[table][key1].end())
            return DB_NOTFOUND;
        inMemDB[table][key1].erase(key2);
        return DB_REMOVED;
    }       
}

#define dbRef DataBase*