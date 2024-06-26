#ifndef BDD_HELPER_HPP
#define BDD_HELPER_HPP

#include <vector>
#include <type_traits>
#include <cassert>
#include <utility>
#include <cmath>
#include "bdd.h"

namespace bddHelper
{
  enum class ConditionTypes {
    FIRST,
    SECOND,
    THIRD,
    FOURTH,
    UNIQUE,
    UPPER_BOUND
  };

  enum class Object
  {
    FIRST,
    SECOND,
    THIRD,
    FOURTH,
    FIFTH,
    SIXTH,
    SEVENTH,
    EIGTH,
    NINETH
  };


  enum class Property
  {
    HAIR,
    NATION,
    TRANSPORT,
    OWNS
  };

  enum class Hair
  {
    RED,
    GREEN,
    BLUE,
    YELLOW,
    WHITE,
    PURPLE,
    BROWN,
    AQUA,
    BEIGE
  };
  enum class Nation
  {
    TATARIN,
    BRAZILIAN,
    ARGENTINIAN,
    GERMAN,
    CHINESE,
    RUSSIAN,
    ARABIC,
    AUSTRALIAN,
    KAZAH
  };
  enum class Transport
  {
    CAR,
    HELICOPTER,
    PLANE,
    BUS,
    TRAIN,
    BOAT,
    BIKE,
    SCOOTER,
    TROLLEYBUS
  };
  enum class Owns
  {
    DRON,
    PLANE,
    CAR,
    HOMYAK,
    FISH,
    BALL,
    BIRD,
    LION,
    ELEPHANT
  };

  template< class Enum_Val_t >
  int toNum(Enum_Val_t value);

  int toNum(Property value);

  // Just ignore that. Purely C++ shit
  namespace traits_
  {
    template
    < class V_t, class = std::enable_if_t<
      std::is_same_v< V_t, Hair > ||
      std::is_same_v< V_t, Nation > ||
      std::is_same_v< V_t, Transport > ||
      std::is_same_v< V_t, Owns > > >
    struct IsValueType: std::true_type
    { };

    template < class V_t >
    struct IsValueType< V_t, int >: std::false_type
    { };

    template < class V_t, class = void >
    struct PropertyFromValueEnum
    {
    };

    template < class V_t >
    struct PropertyFromValueEnum< V_t, std::enable_if_t< std::is_same_v< V_t, Hair > > >
    {
      static constexpr Property value = Property::HAIR;
    };

    template < class V_t >
    struct PropertyFromValueEnum< V_t, std::enable_if_t< std::is_same_v< V_t, Nation > > >
    {
      static constexpr Property value = Property::NATION;
    };

    template < class V_t >
    struct PropertyFromValueEnum< V_t, std::enable_if_t< std::is_same_v< V_t, Transport > > >
    {
      static constexpr Property value = Property::TRANSPORT;
    };

    template < class V_t >
    struct PropertyFromValueEnum< V_t, std::enable_if_t< std::is_same_v< V_t, Owns > > >
    {
      static constexpr Property value = Property::OWNS;
    };

    template < class V_t >
    constexpr Property PropertyFromValueEnum_v = PropertyFromValueEnum< V_t >::value;

    template < class V_t >
    constexpr bool IsValueType_v = IsValueType< V_t >::type::value;
  }

  class BDDHelper
  {
  public:
    template < class T > using vect = std::vector< T >;
    /// See all these in \b main.cpp

    static constexpr int nObjs = 9;
    static constexpr int nProps = 4;
    static constexpr int nVals = 9;
    static constexpr int nValueBits = 4;
    static constexpr int nValuesVars = nObjs * nProps * nValueBits;
    static constexpr int nTotalVars = nValuesVars;

    // See BDDHelper.cpp file
    BDDHelper(vect< vect< vect< bdd > > > structedVars);

    // See below
    template< class V_t >
    bdd getObjectVal(Object obj, V_t value);

    // See BDDHelper.cpp file
    std::vector< bdd > getObjPropertyVars(Object obj, Property prop);

    // See BDDHelper.cpp file
    bdd numToBin(int num, vect< bdd > vars);

    // See BDDHelper.cpp file
    bdd numToBinUnsafe(int num, vect< bdd > vars);

  private:
  #ifdef GTEST_TESTING // ignore
    friend class ::VarsSetupFixture_BDDHelperbasic_Test;
    friend class ::VarsSetupFixture;
    BDDHelper();
  #endif
    // See constructor
    vect< vect< vect< bdd > > > structVars_;
    vect< vect< vect< bdd > > > values_;
  };


  template < class V_t >
  inline bdd BDDHelper::getObjectVal(Object obj, V_t value)
  {
    static_assert(traits_::IsValueType_v< V_t >, "Value must be one of properties type");
    auto objNum = toNum(obj);
    auto propNum = toNum(traits_::PropertyFromValueEnum_v< V_t >);
    auto valNum = toNum(value);
    return values_[objNum][propNum][valNum];
  }

  template < class Enum_Val_t >
  int toNum(Enum_Val_t value)
  {
    static_assert(std::is_enum_v< Enum_Val_t >, "Value must be enum");
    int num = static_cast< int >(value);
    assert(("Invalid enum value found", num >= 0 and num <= 8));
    return num;
  }
}
#endif
