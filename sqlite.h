#ifndef __SQLITE_H__
#define __SQLITE_H__

#include "const.h"
#include "sqlite/cipher.h"
#include "sqlite/sqlite_modern_cpp.h"

class mydb : public sqlite::database
{
private:
    string salt;
    string password;
    Cipher cipher;

public:
    mydb(const string &file, const string& _password, const string &sql = "");
    string encrypt(const string &str);
    string decrypt(const string &str);
};

class myst : public sqlite::database_binder
{
public:
    myst(mydb db, const string &sql) : sqlite::database_binder(db << sql) {}
};

#endif // __SQLITE_H__