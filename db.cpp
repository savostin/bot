#include "db.h"

shared_ptr<DB> DB::db = nullptr;

DB::DB(const string &file) : sqlite::database(file)
{
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
            return false;
        }
    }
    return true;
}
