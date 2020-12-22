#include "sqlite.h"

mydb::mydb(const string &file, const string& _password, const string &sql) : sqlite::database(file), salt("EanJ08Zt"), password(_password), cipher("aes-256-cbc", "sha256", 1, false)
{
    if (!sql.empty())
    {
        *this << sql;
    }
}

string mydb::encrypt(const string &str)
{
    return cipher.encrypt(str, password, salt);
}

string mydb::decrypt(const string &str)
{
    return cipher.decrypt(str, password, salt);
}