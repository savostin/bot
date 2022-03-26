#ifndef __CRYPT_H__
#define __CRYPT_H__

#include "const.h"
#include "sqlite/cipher.h"

class Crypt : public Cipher {
private:
  std::string salt;
  std::string password;

public:
  Crypt(const std::string &password, const std::string &salt = "");
  ~Crypt();
  std::string encrypt(const std::string &str);
  std::string decrypt(const std::string &str);

  static shared_ptr<Crypt> crypt;
  static bool init(const std::string &password);
};
#endif // __CRYPT_H__