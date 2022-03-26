#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include "const.h"

class Options
{
private:
    json data;

public:
    Options();
    ~Options();
    bool load();
    void save();
    int get(std::string name, int def);
    std::string get(std::string name, std::string def);
    void set(std::string name, int val);
    void set(std::string name, std::string val);
    static std::string ask(const std::string &q, bool hide = false);
};

#endif // __OPTIONS_H__