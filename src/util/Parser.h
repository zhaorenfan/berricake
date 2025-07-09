//
// Created by zhou on 25-6-19.
//

#ifndef PARSER_H
#define PARSER_H
#include <json.hpp>

using json = nlohmann::json;

namespace util {

class Parser {
public:
    static json file_to_json(const std::string &file_path);

};

} // util

#endif //PARSER_H
