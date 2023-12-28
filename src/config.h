#ifndef BDD_EXAMPLE_CONFIG_H
#define BDD_EXAMPLE_CONFIG_H

#include <set>
#include <map>
#include "BDDHelper.hpp"

using namespace bddHelper;

class config {
private:
    std::vector<std::tuple<Object, Nation>> firstCondition;
    std::vector<std::tuple<Nation, Owns>> secondConditionWithOwns;
    std::vector<std::tuple<Nation, Transport>> secondConditionWithTransport;
    std::vector<std::tuple<Nation, Hair>> secondConditionWithColor;
    std::vector<std::tuple<Nation, Nation>> thirdCondition;
    std::vector<std::tuple<Nation, Nation>> forthCondition;

    std::vector<int> leftNeighbourXYOffset;
    std::vector<int> rightNeighbourXYOffset;

    bool vertSkleika;
    bool horSkleika;

    std::map<std::string, Transport> mapP = {
            {"HELICOPTER", Transport::HELICOPTER},
            {"BUS", Transport::BUS},
            {"TRAIN", Transport::TRAIN},
            {"HELICOPTER", Transport::HELICOPTER},
            {"BUS", Transport::BUS},
            {"BOAT", Transport::BOAT},
            {"BIKE", Transport::BIKE},
            {"SCOOTER", Transport::SCOOTER},
            {"PLANE", Transport::PLANE},
            {"TROLLEYBUS", Transport::TROLLEYBUS},
    };
    std::map<std::string, Hair> mapC = {
            {"RED", Hair::RED},
            {"GREEN", Hair::GREEN},
            {"BLUE", Hair::BLUE},
            {"YELLOW", Hair::YELLOW},
            {"PURPLE", Hair::PURPLE},
            {"BROWN", Hair::BROWN},
            {"AQUA", Hair::AQUA},
            {"BEIGE", Hair::BEIGE},
            {"WHITE", Hair::WHITE},
    };
    std::map<std::string, Owns> mapO = {
            {"DRON", Owns::DRON},
            {"PLANE", Owns::PLANE},
            {"CAR", Owns::CAR},
            {"HOMYAK", Owns::HOMYAK},
            {"FISH", Owns::FISH},
            {"BALL", Owns::BALL},
            {"BIRD", Owns::BIRD},
            {"LION", Owns::LION},
            {"ELEPHANT", Owns::ELEPHANT},
    };

    void readProperties(std::string& filename);

    static std::vector<std::string> split(std::string& str, char separator);
public:
    explicit config(std::string filename = "../properties.properties");

    [[nodiscard]] const std::vector<std::tuple<Object, Nation>> &getFirstCondition() const;

    [[nodiscard]] const std::vector<std::tuple<Nation, Owns>> &getSecondConditionWithOwns() const;

    [[nodiscard]] const std::vector<std::tuple<Nation, Transport>> &getSecondConditionWithTransport() const;

    [[nodiscard]] const std::vector<std::tuple<Nation, Hair>> &getSecondConditionWithColor() const;

    [[nodiscard]] const std::vector<std::tuple<Nation, Nation>> &getThirdCondition() const;

    [[nodiscard]] const std::vector<std::tuple<Nation, Nation>> &getForthCondition() const;

    std::vector<int> & getLeftNeighbourXyOffset();

    std::vector<int> & getRightNeighbourXyOffset();

    [[nodiscard]] bool isVertSkleika() const {
        return vertSkleika;
    }

    [[nodiscard]] bool isHorSkleika() const {
        return horSkleika;
    }
};


#endif //BDD_EXAMPLE_CONFIG_H
