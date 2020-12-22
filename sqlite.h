#ifndef __SQLITE_H__
#define __SQLITE_H__

#include "const.h"
#include "sqlite/cipher.h"
#include "sqlite/sqlite_modern_cpp.h"

class mydb : public sqlite::database
{
private:
    std::string salt;
    std::string password;
    Cipher cipher;

public:
    mydb(const std::string &file, const std::string& _password, const std::string &sql = "");
    std::string encrypt(const std::string &str);
    std::string decrypt(const std::string &str);
};

class myst : public sqlite::database_binder
{
public:
    myst(mydb db, const std::string &sql) : sqlite::database_binder(db << sql) {}
};

#endif // __SQLITE_H__