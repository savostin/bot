#include "db.h"

shared_ptr<DB> DB::db = nullptr;

DB::DB(const string &file) : sqlite::sqlcipher_database(file)
{
}

bool DB::set_key(const std::string &key)
{
    try
    {
        sqlite::sqlcipher_database::set_key(key);
        *(db) << "create table if not exists ch (id int); ";
        *(db) << "drop table ch; ";
    }
    catch (...)
    {
        return false;
    }
    return true;
}

DB &DB::o()
{
    return *(db);
}

bool DB::init(const std::string &file)
{
    if (db == nullptr)
    {
        try
        {
            DB d(file);
            DB::db = make_shared<DB>(d);
            return true;
        }
        catch (...)
        {
            DB::db = nullptr;
            return false;
        }
    }
    return true;
}
