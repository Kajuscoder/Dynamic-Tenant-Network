#ifndef DATABASE_H
#define DATABASE_H

#include <map>
#include <cpprest/json.h>

using namespace utility;
using namespace web; 

/*
 * Signifies status of any DB operation 
 */
typedef enum _dbOpStatus {
    DB_SUCCESS,
    DB_ADDED,
    DB_UPDATED,
    DB_REMOVED,
    DB_NOTFOUND,
    DB_ALREADYEXISTS,
    DB_FAILED
} dbOpStatus;

/*
 * Contains outcome of an DB Operation
 * along with the JSON output in case of 
 * a DB Read operation
 */
typedef struct _dbOpRet {
    dbOpStatus retStatus;
    json::value retVal;
    _dbOpRet(dbOpStatus os) : retStatus(os) {}
    _dbOpRet(dbOpStatus os, json::value jv) : retStatus(os), retVal(jv) {}
} dbOpRet;

#define dbTableRef string_t

class DataBase {
private:
    /* 
     * For this "Proof of Concept" project, an in-memory 
     * 3-dimensional C++ Map is being used as Database.  
     * 
     * Key of the first-level Map is actually a "string" 
     * representing a Table for each ReST resource.
     * 
     * Key of the second-level Map is a "string"
     * Key of the third-level Map is a "string"
     * 
     * Entire JSON document is being saved as value to
     * mimic the behaviours of Document-style NoSQL Databasses
     * like MongoDB or Firestore. 
     */
    std::map<dbTableRef, std::map<std::string, std::map<std::string, json::value>>> inMemDB;
public:
    DataBase();

    dbTableRef createTable(string_t uriName);

    dbOpRet getEntry(dbTableRef table);
    
    dbOpRet getEntry(dbTableRef table, std::string key1);
    dbOpRet getEntry(dbTableRef table, std::string key1, std::string key2);
    
    dbOpStatus addEntry(dbTableRef table, std::string key1, std::string key2, json::value entry);
    
    dbOpStatus updateEntry(dbTableRef table, std::string key1, std::string key2, json::value entry);
    
    dbOpStatus removeEntry(dbTableRef table, std::string key1);
    dbOpStatus removeEntry(dbTableRef table, std::string key1, std::string key2);
};

#define dbRef DataBase*

#endif