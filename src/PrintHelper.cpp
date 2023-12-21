#include "PrintHelper.hpp"

std::string to_string(bddHelper::Object obj)
{
  using namespace bddHelper;
  switch (obj)
  {
    case Object::FIRST:   return "Object #1";
    case Object::SECOND:  return "Object #2";
    case Object::THIRD:   return "Object #3";
    case Object::FOURTH:  return "Object #4";
    case Object::FIFTH:   return "Object #5";
    case Object::SIXTH:   return "Object #6";
    case Object::SEVENTH: return "Object #7";
    case Object::EIGTH:   return "Object #8";
    case Object::NINETH:  return "Object #9";
  }
  assert(("Bad enum value", false));
  //std::unreachable();
}

std::string to_string(bddHelper::Property prop)
{
  using namespace bddHelper;
  switch (prop)
  {
    case Property::OWNS:   return "Owns";
    case Property::NATION:   return "Nation";
    case Property::COLOR:    return "Color";
    case Property::TRANSPORT:    return "Transport";
  }
  assert(("Bad enum value", false));
  //std::unreachable();
}

std::string to_string(bddHelper::Color col)
{
  using namespace bddHelper;
  switch (col)
  {
    case Color::AQUA: return "AQUA";
    case Color::BEIGE: return "BEIGE";
    case Color::BLUE: return "BLUE";
    case Color::BROWN: return "BROWN";
    case Color::GREEN: return "GREEN";
    case Color::PURPLE: return "PURPLE";
    case Color::RED: return "RED";
    case Color::WHITE: return "WHITE";
    case Color::YELLOW: return "YELLOW";
  }
  assert(("Bad enum value", false));
  //std::unreachable();
}

std::string to_string(bddHelper::Nation nat)
{
  using namespace bddHelper;
  switch (nat)
  {
    case Nation::AUSTRALIAN: return "AUSTRALIAN";
    case Nation::BRAZILIAN: return "BRAZILIAN";
    case Nation::ARABIC: return "ARABIC";
    case Nation::CHINESE: return "CHINESE";
    case Nation::ARGENTINIAN: return "ARGENTINIAN";
    case Nation::GERMAN: return "GERMAN";
    case Nation::KAZAH: return "KAZAH";
    case Nation::RUSSIAN: return "RUSSIAN";
    case Nation::TATARIN: return "TATARIN";
  }
  assert(("Bad enum value", false));
  //std::unreachable();
}

std::string to_string(bddHelper::Transport TRANSPORT)
{
  using namespace bddHelper;
  switch (TRANSPORT)
  {
    case Transport::SCOOTER: return "SCOOTER";
    case Transport::HELICOPTER: return "HELICOPTER";
    case Transport::BIKE: return "BIKE";
    case Transport::BUS: return "BUS";
    case Transport::PLANE: return "PLANE";
    case Transport::CAR: return "CAR";
    case Transport::TROLLEYBUS: return "TROLLEYBUS";
    case Transport::BOAT: return "BOAT";
    case Transport::TRAIN: return "TRAIN";
  }
  assert(("Bad enum value", false));
  //std::unreachable();
}

std::string to_string(bddHelper::Owns animal)
{
  using namespace bddHelper;
  switch (animal)
  {
    case Owns::BIRD: return "BIRD";
    case Owns::PLANE: return "PLANE";
    case Owns::DRON: return "DRON";
    case Owns::ELEPHANT: return "ELEPHANT";
    case Owns::FISH: return "FISH";
    case Owns::HOMYAK: return "HOMYAK";
    case Owns::BALL: return "BALL";
    case Owns::LION: return "LION";
    case Owns::CAR: return "CAR";
  }
  assert(("Bad enum value", false));
  //std::unreachable();
}
