#include "crypt.h"

shared_ptr<Crypt> Crypt::crypt = nullptr;

Crypt::Crypt(const std::string &_salt) : Cipher("aes-256-cbc", "sha256", 100, false), salt(_salt), password("")
{
}

Crypt::~Crypt()
{
}

string Crypt::encrypt(const string &str, string p)
{
    try
    {
        return Cipher::encrypt(str, p.empty() ? password : p, salt);
    }
    catch (...)
    {
        return "???";
    }
}

string Crypt::decrypt(const string &str, string p)
{
    try
    {
        return Cipher::decrypt(str, p.empty() ? password : p, salt);
    }
    catch (...)
    {
        return "???";
    }
}

void Crypt::setPassword(const std::string &_password)
{
    password = _password;
}

void Crypt::init(const std::string &salt, const std::string &password)
{
    if (crypt == nullptr)
    {
        Crypt c(salt);
        crypt = make_shared<Crypt>(c);
        crypt->setPassword(password);
    }
}
