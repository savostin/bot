#ifndef __SQLITE_H__
#define __SQLITE_H__

#include "const.h"
#include "sqlite/sqlite_modern_cpp.h"

class DB : public sqlite::database
{
private:
    static shared_ptr<DB> db;

public:
    DB(const std::string &file);
    static DB &o();
    static bool init(const std::string &file);
};

#endif // __SQLITE_H__