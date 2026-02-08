#include "dbHeader.h"
#include "dbManager.h"

bool createFile(){
    if (DatabaseManager::instance().createDatabaseStructure() != true) return false;
    if (DatabaseManager::instance().initializeMetaData() != true) return false;
    saveDatabase();
    DatabaseManager::instance().closeAndLock();
    return true;
}


