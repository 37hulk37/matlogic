#include "Conditions.hpp"
#include <ranges>
#include <tuple>
#include <optional>
#include <algorithm>
#include <numeric>
#include <execution>
#include <functional>
#include <type_traits>
#include <set>
#include <utility>

using namespace bddHelper;

namespace
{
  template < class ... Trest >
  struct unique_types;

  template < class T1, class T2, class ... Trest >
  struct unique_types< T1, T2, Trest ... >
    : unique_types< T1, T2 >, unique_types< T1, Trest ... >, unique_types< T2, Trest ... > { };

  template < class T1, class T2 >
  struct unique_types< T1, T2 >
  {
    static_assert(!std::is_same< T1, T2 >::value, "Types must be unique");
  };
}

namespace std
{
  template< class T >
  bool between(T const &val, T const &lo, T const &hi)
  {
    return between( val, lo, hi, std::less_equal< T >() );
  }

  template< class T, class Compare >
  bool between(T const &val, T const &lo, T const &hi, Compare comp)
  {
    return comp(lo, val) && comp(val, hi);
  }
}

namespace
{
  config config;
  std::vector leftNeighbourXYOffset = {0, -1};
  std::vector rightNeighbourXYOffset = {-1, 0};

  bool vSkl = config.isVertSkleika();
  bool hSkl = config.isHorSkleika();

  // Don't touch it...
  bool useSkleika = vSkl || hSkl;

  template < class ... V_ts >
  void addLoopCondition(std::tuple< V_ts... > values, BDDHelper &h, BDDFormulaBuilder &builder);

  template < class V_t1, class V_t2 >
  void addNeighbours(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder);

  template < class V_t1, class V_t2 >
  void addLeftNeighbour(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder);

  template < class V_t1, class V_t2 >
  void addRightNeighbour(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder);

  // See below
  std::optional< Object > getNeighbour_(Object obj, std::vector< int > neighbourXYOffset);
  // See below
  std::optional< Object > getLeftNeighbour(Object obj);
  // See below
  std::optional< Object > getRightNeighbour(Object obj);
  // See below
  std::vector< Object > getNeighbours(Object obj);

  // See below
  bdd equal(bdd a, bdd b);
  // See below
  bdd notEqual(std::vector< bdd > v1, std::vector< bdd > v2);

  // See below
  void addFirstCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  // See below
  void addSecondCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  // See below
  void addThirdCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  // See below
  void addFourthCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  // See below
  void addUniqueCondition(BDDHelper &h, BDDFormulaBuilder &builder);
  // See below
  void addValuesUpperBoundCondition(BDDHelper &h, BDDFormulaBuilder &builder);

  template < class ... V_ts >
  void addLoopCondition(std::tuple< V_ts... > values, BDDHelper &h, BDDFormulaBuilder &builder)
  {
    // Helps to avoid controversy in conditions.
    unique_types< V_ts... > check_uniquness;
    auto resultFormulaToAdd = bdd_false();
    // Here we loop through objects and say that
    // current object must have all given values.
    for (auto i : std::views::iota(0, BDDHelper::nObjs))
    {
      auto obj = static_cast< Object >(i);
      auto formulas = bdd_true();
      std::apply([&formulas, &h, &obj](auto &&... args) {
        ((formulas &= h.getObjectVal(obj, args)), ...); //Here we say current object must have all given values.
      }, values);
      resultFormulaToAdd |= formulas;
    }
    // Add result condition
    builder.addCondition(resultFormulaToAdd);
  }

  template < class V_t1, class V_t2 >
  void addNeighbours(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder)
  {
    static_assert(traits_::IsValueType_v< V_t1 > && traits_::IsValueType_v< V_t2 >, "Value must be one of properties type");
    auto resultFormulaToAdd = bdd_false();
    for (auto objNum : std::views::iota(0, BDDHelper::nObjs))
    {
      auto obj = static_cast< Object >(objNum);
      for (auto neighbObj : getNeighbours(obj))
        resultFormulaToAdd |= (h.getObjectVal(obj, value1) & h.getObjectVal(neighbObj, value2));
    }
    builder.addCondition(resultFormulaToAdd);
  }

  template < class V_t1, class V_t2 >
  void addLeftNeighbour(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder)
  {
    static_assert(traits_::IsValueType_v< V_t1 > && traits_::IsValueType_v< V_t2 >, "Value must be one of properties type");
    auto resultFormulaToAdd = bdd_false();
    for (auto objNum : std::views::iota(0, BDDHelper::nObjs))
    {
      auto obj = static_cast< Object >(objNum);
      if (auto neighbObj = getLeftNeighbour(obj); neighbObj.has_value())
        resultFormulaToAdd |= (h.getObjectVal(obj, value1) & h.getObjectVal(*neighbObj, value2));
    }
    builder.addCondition(resultFormulaToAdd);
  }

  template < class V_t1, class V_t2 >
  void addRightNeighbour(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder)
  {
    static_assert(traits_::IsValueType_v< V_t1 > && traits_::IsValueType_v< V_t2 >, "Value must be one of properties type");
    auto resultFormulaToAdd = bdd_false();
    for (auto objNum : std::views::iota(0, BDDHelper::nObjs))
    {
      auto obj = static_cast< Object >(objNum);
      if (auto neighbObj = getRightNeighbour(obj); neighbObj.has_value())
        resultFormulaToAdd |= (h.getObjectVal(obj, value1) & h.getObjectVal(*neighbObj, value2));
    }
    builder.addCondition(resultFormulaToAdd);
  }

  std::optional< Object > getLeftNeighbour(Object obj)
  {
    return getNeighbour_(obj, leftNeighbourXYOffset);
  }

  std::optional< Object > getRightNeighbour(Object obj)
  {
    auto res = getNeighbour_(obj, rightNeighbourXYOffset);
    return res;
  }

  std::optional< Object > getNeighbour_(Object obj, std::vector< int > neighbourXYOffset)
  {
    assert(neighbourXYOffset.size() == 2);
    struct Point
    {
      int x;
      int y;
      constexpr bool operator==(const Point &rhs) const
      {
        return x == rhs.x && y == rhs.y;
      }
    };
    auto pointToObj =
      [](Point p) -> Object {
        assert(std::between(p.x, 0, 2) and std::between(p.y, 0, 2));
        return static_cast< Object >(p.x + p.y * 3);
      };
    auto normX =
      [](Point p) -> Point {
        assert(!std::between(p.x, 0, 2));
        Point res = p;
        if (p.x < 0)
          res.x = 3 + p.x % 3;
        else
          res.x = p.x % 3;
        return res;
      };
    auto normY =
      [](Point p) -> Point {
        assert(!std::between(p.y, 0, 2));
        Point res = p;
        if (p.y < 0)
          res.y = 3 + p.y % 3;
        else
          res.y = p.y % 3;
        return res;
      };
    assert((normX({ 3, 0 }) == Point{ 0, 0 }));
    assert((normY({ 0, 3 }) == Point{ 0, 0 }));
    assert((normX({ -1, 0 }) == Point{ 2, 0 }));
    assert((normY({ 0, -1 }) == Point{ 0, 2 }));
    assert((normX(normY({ -1, -1 })) == Point{ 2, 2 }));
    assert((normX(normY({ 4, 3 })) == Point{ 1, 0 }));
    auto objNum = toNum(obj); // First we convert obj to int
    Point objPos = { objNum % 3, objNum / 3 }; // next we calculate obj coordinates
    Point neighbObjPos = { objPos.x + neighbourXYOffset[0], objPos.y + neighbourXYOffset[1] }; // calcuate neighbour coords

    if (!std::between(neighbObjPos.x, 0, 2) and !std::between(neighbObjPos.y, 0, 2))
    {
      if (!vSkl or !hSkl)
        return std::nullopt;
      return pointToObj(normX(normY(neighbObjPos)));
    }
    if (!std::between(neighbObjPos.x, 0, 2))
    {
      if (!hSkl)
        return std::nullopt;
      return pointToObj(normX(neighbObjPos));
    }

    if (!std::between(neighbObjPos.y, 0, 2))
    {
      if (!vSkl)
        return std::nullopt;
      return pointToObj(normY(neighbObjPos));
    }

    return pointToObj(neighbObjPos);
  }


  std::vector< Object > getNeighbours(Object obj)
  {
    auto left = getLeftNeighbour(obj);
    auto right = getRightNeighbour(obj);
    std::vector< Object > resArr;
    if (left)
      resArr.push_back(*left);
    if (right)
      resArr.push_back(*right);
    return resArr;
  }

  bdd equal(bdd a, bdd b)
  {
    return (a & b) | ((not a) & (not b));
  }

  bdd notEqual(std::vector< bdd > a, std::vector< bdd > b)
  {
    assert(a.size() == 4 && a.size() == b.size());
    return std::inner_product(
      a.begin(),
      a.end(),
      b.begin(),
      bdd_false(),
      std::bit_or< bdd >(),
      [](bdd a, bdd b) {
      return not equal(a, b);
    });
  }


  void addUniqueCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    auto propRange = std::views::iota(0, BDDHelper::nProps);
    //We loop over properties
    std::for_each(std::execution::par, propRange.begin(), propRange.end(),
      [&](auto propNum) {
      auto prop = static_cast< Property >(propNum);
      auto obj1Range = std::views::iota(0, BDDHelper::nObjs);
      std::for_each(std::execution::par, obj1Range.begin(), obj1Range.end(),
        [&](auto objNum1) {
        auto obj1 = static_cast< Object >(objNum1);
        auto obj2Range = std::views::iota(objNum1 + 1, BDDHelper::nObjs);
        std::for_each(std::execution::par, obj2Range.begin(), obj2Range.end(),
          [&](auto objNum2) {
          auto obj2 = static_cast< Object >(objNum2);
          builder.addConditionTh(notEqual(h.getObjPropertyVars(obj1, prop), h.getObjPropertyVars(obj2, prop)));
        });
      });
    });
  }

  void addValuesUpperBoundCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    for (auto objNum : std::views::iota(0, BDDHelper::nObjs))
    {
      auto obj = static_cast< Object >(objNum);
      for (auto propNum : std::views::iota(0, BDDHelper::nProps))
      {
        auto prop = static_cast< Property >(propNum);
        for (auto valNum : std::views::iota(9, 16))
        {
          builder.addCondition(not h.numToBinUnsafe(valNum, h.getObjPropertyVars(obj, prop)));
        }
      }
    }
  }

  void addFirstCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
      for (auto fconfig: config.getFirstCondition()) {
          builder.addCondition(h.getObjectVal(std::get<0>(fconfig), std::get<1>(fconfig)));
      }
  }

  void addSecondCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
      for (auto fconfig: config.getSecondConditionWithOwns()) {
          addLoopCondition(fconfig, h, builder);
      }

      for (auto fconfig: config.getSecondConditionWithTransport()) {
          addLoopCondition(fconfig, h, builder);
      }

      for (auto fconfig: config.getSecondConditionWithColor()) {
          addLoopCondition(fconfig, h, builder);
      }
  }

  void addThirdCondition(BDDHelper &h, BDDFormulaBuilder &builder) {}

  void addFourthCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
        for (auto fconfig: config.getForthCondition()) {
            addNeighbours(std::get<0>(fconfig), std::get<1>(fconfig), h, builder);
        }
  }
}

namespace conditions
{
    void addConditionByType(ConditionTypes type, BDDHelper &h, BDDFormulaBuilder &builder) {
        switch (type) {
            case ConditionTypes::FIRST: {
                addFirstCondition(h, builder);
                break;
            }
            case ConditionTypes::SECOND: {
                addSecondCondition(h, builder);
                break;
            }
            case ConditionTypes::THIRD: {
                addThirdCondition(h, builder);
                break;
            }
            case ConditionTypes::FOURTH: {
                addFourthCondition(h, builder);
                break;
            }
            case ConditionTypes::UNIQUE: {
                addUniqueCondition(h, builder);
                break;
            }
            case ConditionTypes::UPPER_BOUND: {
                addValuesUpperBoundCondition(h, builder);
                break;
            }
        }
    }

      void addConditions(BDDHelper &h, BDDFormulaBuilder &builder, const std::set<ConditionTypes>& types)
      {
          for (auto type: types) {
              addConditionByType(type, h, builder);
          }
      }
}
