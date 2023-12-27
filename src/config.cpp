#include <fstream>
#include <sstream>
#include "config.h"
#include "magic_enum.h"



config::config(std::string filename) {
    readProperties(filename);
}

void config::readProperties(std::string &filename) {
    std::ifstream properties(filename);
    std::string str;
    while ( !properties.eof()) {
        properties >> str;

        auto v1 = split(str,  '=');
        auto v2 = split(str, '.');

        auto arr = split(v2[v2.size()-1], '=');

        if (str.contains("first")) {
            firstCondition.emplace_back(magic_enum::enum_cast<Object>(arr[0]).value(), magic_enum::enum_cast<Nation>(arr[1]).value());
        } else if (str.contains("second")) {
            if (str.contains("owns")) {
                secondConditionWithOwns.emplace_back(magic_enum::enum_cast<Nation>(arr[0]).value(), mapO[arr[1]]);
            } else if (str.contains("transport")) {
                secondConditionWithTransport.emplace_back(magic_enum::enum_cast<Nation>(arr[0]).value(), mapP[arr[1]]);
            } else if (str.contains("hair")) {
                secondConditionWithColor.emplace_back(magic_enum::enum_cast<Nation>(arr[0]).value(), mapC[arr[1]]);
            }
        } else if (str.contains("third")) {
            forthCondition.emplace_back(magic_enum::enum_cast<Nation>(arr[0]).value(), magic_enum::enum_cast<Nation>(arr[1]).value());
        } else if (str.contains("forth")) {
            forthCondition.emplace_back(magic_enum::enum_cast<Nation>(arr[0]).value(), magic_enum::enum_cast<Nation>(arr[1]).value());
        } else if (str.contains("neigh")) {
            if (str.contains("left")) {
                leftNeighbourXYOffset.emplace_back(stoi(arr[arr.size()-1]));
            } else {
                rightNeighbourXYOffset.emplace_back(stoi(arr[arr.size()-1]));
            }
        } else if (str.contains("vertSkleika")) {
            vertSkleika = arr[1] == "1";
        } else {
            horSkleika = arr[1] == "1";
        }
    }
}

std::vector<std::string> config::split(std::string &str, char separator) {
    std::stringstream ss(str);
    std::vector<std::string> tokens;
    std::string token;
    while (std::getline(ss, token, separator)) {
        tokens.push_back(token);
    }

    return tokens;
}

const std::vector<std::tuple<Object, Nation>> &config::getFirstCondition() const {
    return firstCondition;
}

const std::vector<std::tuple<Nation, Owns>> &config::getSecondConditionWithOwns() const {
    return secondConditionWithOwns;
}

const std::vector<std::tuple<Nation, Transport>> &config::getSecondConditionWithTransport() const {
    return secondConditionWithTransport;
}

const std::vector<std::tuple<Nation, Hair>> &config::getSecondConditionWithColor() const {
    return secondConditionWithColor;
}

const std::vector<std::tuple<Nation, Nation>> &config::getThirdCondition() const {
    return thirdCondition;
}

const std::vector<std::tuple<Nation, Nation>> &config::getForthCondition() const {
    return forthCondition;
}

std::vector<int> & config::getLeftNeighbourXyOffset() {
    return leftNeighbourXYOffset;
}

std::vector<int> & config::getRightNeighbourXyOffset() {
    return rightNeighbourXYOffset;
}