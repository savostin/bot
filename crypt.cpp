#include "crypt.h"

shared_ptr<Crypt> Crypt::crypt = nullptr;

Crypt::Crypt(const std::string &_password, const std::string &_salt)
    : Cipher("aes-256-cbc", "sha256", 100, false), salt(_salt),
      password(_password) {
  if (salt.empty()) {
  }
}

Crypt::~Crypt() {}

string Crypt::encrypt(const string &str) {
  try {
    return Cipher::encrypt(str, password, salt);
  } catch (...) {
    return "???";
  }
}

string Crypt::decrypt(const string &str) {
  try {
    return Cipher::decrypt(str, password, salt);
  } catch (...) {
    return "???";
  }
}

bool Crypt::init(const std::string &password) {
  if (crypt == nullptr) {
    Crypt c(password);
    Crypt::crypt = make_shared<Crypt>(c);
    return true;
  }
  return false;
}
