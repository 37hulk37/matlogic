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

namespace // Dont read this...
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
  // True if low >= val <= hi;
  // Other words, true if value between low and hi
  template< class T >
  bool between(T const &val, T const &lo, T const &hi)
  {
    return between( val, lo, hi, std::less_equal< T >() );
  }

  // Just ignore that
  template< class T, class Compare >
  bool between(T const &val, T const &lo, T const &hi, Compare comp)
  {
    return comp(lo, val) && comp(val, hi);
  }
}

namespace
{
  /**
   * Use these to set neighbours.
   * For example, {0, -1} means neighbour
   *    N * N
   *    N O N
   *    N N N
   *
   * {1, 1} means neighbour
   *    N N N
   *    N O N
   *    N N *
   *
   * X is horizontal
   * Y is vertical.
   *    X
   *    0 1 2
   * Y 0
   *   1
   *   2
   */
  std::vector leftNeighbourXYOffset = { 0, -1 };
  std::vector rightNeighbourXYOffset = { -1, 0 };
  // Use to enable disable any skleika
  // Read about this at 30 page.
  constexpr bool vertSkleika = false;
  constexpr bool horSkleika = false;

  // Don't touch it...
  constexpr bool useSkleika = vertSkleika || horSkleika;

  // See below
  template < class ... V_ts >
  void addLoopCondition(std::tuple< V_ts... > values, BDDHelper &h, BDDFormulaBuilder &builder);

  // See below
  template < class V_t1, class V_t2 >
  void addNeighbours(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder);

  // See below
  template < class V_t1, class V_t2 >
  void addLeftNeighbour(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder);

  // See below
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

  /**
   * Sets second type condition.
   *
   * Examples:
   * ```
   * addLoopCondition({Nation::TATARIN, Owns::DRON}, ...);
   * ```
   * Says ONE of objects must have both TATARIN with DRON.
   *
   * ```
   * addLoopCondition({Nation::GERMAN, Hair::YELLOW}, ...);
   * ```
   * Says ONE of objects must have both GERMAN with YELLOW color house.
   *
   * ```
   * addLoopCondition({Nation::GERMAN, Nation::ARGENTINIAN}, ...);
   * ```
   * Says ONE of objects must have both GERMAN and ARGENTINIAN.
   * This is impossible condition, so called controversy, so result
   * function will always return false.
   */
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
      // Here we say that there must be
      // first object with all values or
      // second object with all values or
      // third object with all values or...
      resultFormulaToAdd |= formulas;
    }
    // Add result condition
    builder.addCondition(resultFormulaToAdd);
  }

  // This function says that there must be any neighbours
  // so that first must have value1 and
  //          second must have value2
  // No matter who is left and who is right.
  // Things that relate to skleika and some other shit
  // handled in getNeighbours function
  template < class V_t1, class V_t2 >
  void addNeighbours(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder)
  {
    static_assert(traits_::IsValueType_v< V_t1 > && traits_::IsValueType_v< V_t2 >, "Value must be one of properties type");
    auto resultFormulaToAdd = bdd_false();
    // So we loop though the objects and say that
    // for ANY object...
    for (auto objNum : std::views::iota(0, BDDHelper::nObjs))
    {
      auto obj = static_cast< Object >(objNum);
      // ...current object must have value1 and
      // current object's any neighbour must have value2
      for (auto neighbObj : getNeighbours(obj)) // According to skleika we may have or not neighbours of current object
        resultFormulaToAdd |= (h.getObjectVal(obj, value1) & h.getObjectVal(neighbObj, value2));
    }
    // Add result condition to formula
    builder.addCondition(resultFormulaToAdd);
  }

  // This function says that there must be any LEFT neighbours
  // so that first must have value1 and
  //          second must have value2
  // Here we say, that second IS LEFT neighbour of first
  // Things that relate to skleika and some other shit
  // handled in getLeftNeighbour function
  template < class V_t1, class V_t2 >
  void addLeftNeighbour(V_t1 value1, V_t2 value2, BDDHelper &h, BDDFormulaBuilder &builder)
  {
    static_assert(traits_::IsValueType_v< V_t1 > && traits_::IsValueType_v< V_t2 >, "Value must be one of properties type");
    auto resultFormulaToAdd = bdd_false();
    // So we loop though the objects and say that
    // for ANY object...
    for (auto objNum : std::views::iota(0, BDDHelper::nObjs))
    {
      auto obj = static_cast< Object >(objNum);
      // ...current object must have value1 and
      // current object's LEFT neighbour must have value2
      if (auto neighbObj = getLeftNeighbour(obj); neighbObj.has_value())
        resultFormulaToAdd |= (h.getObjectVal(obj, value1) & h.getObjectVal(*neighbObj, value2));
    }
    builder.addCondition(resultFormulaToAdd);
  }

  // Read about left neighbour if need
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

  // This function searches neighbour for obj
  // If neighbour exists - return neighbour object.
  // Else return None
  // Neighbour may not exist if skleika not enabled
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
    /**
     * Execute horizontal skleika.
     * For example
     * Point p{-1, 0} will be converted to {2, 0}
     * Point p{-2, 0} will be converted to {1, 0}
     * Point p{3, 0} will be converted to {0, 0}
     * Point p{4, 0} will be converted to {1, 0}
     */
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
    /**
     * Execute vertical skleika.
     * For example
     * Point p{0, -1} will be converted to {0, 2}
     * Point p{0, -2} will be converted to {0, 1}
     * Point p{0, 3} will be converted to {0, 0}
     * Point p{0, 4} will be converted to {0, 1}
     */
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
    // Now we have 4 situations.
    // First is when both horizontal and vertical limits are exceeded
    // It means we need both vertical and horizontal skleika
    if (!std::between(neighbObjPos.x, 0, 2) and !std::between(neighbObjPos.y, 0, 2))
    {
      if (!vertSkleika or !horSkleika)
        return std::nullopt;
      return pointToObj(normX(normY(neighbObjPos)));
    }
    // Second is when only horizontal limit is exceeded
    // It means we need horizontal skleika
    if (!std::between(neighbObjPos.x, 0, 2))
    {
      if (!horSkleika)
        return std::nullopt;
      return pointToObj(normX(neighbObjPos));
    }
    // Third is when only vertical limit is exceeded
    // It means we need vertical skleika
    if (!std::between(neighbObjPos.y, 0, 2))
    {
      if (!vertSkleika)
        return std::nullopt;
      return pointToObj(normY(neighbObjPos));
    }
    // Fourth is when no limits are exceeded
    // No skleika is needed
    return pointToObj(neighbObjPos);
  }

  // Just return left and right neighbours.
  // May return empty array or only one neighbour.
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

  // Speaks for itself
  bdd equal(bdd a, bdd b)
  {
    return (a & b) | ((not a) & (not b));
  }

  // a and b each contain 4 bdd variables
  // We say return condition
  // a[0] != b[0] or a[1] != b[1] or ... a[3] != b[3]
  // It's like comparing two binary numbers. Actually that is it.
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

  // Here we say, that each property value must be used exactly one time
  // For example
  // Object::FIRST have Hair::RED. That means SECOND, THIRD... can not have Hair::RED
  void addUniqueCondition(BDDHelper &h, BDDFormulaBuilder &builder, config& config)
  {
    auto propRange = std::views::iota(0, BDDHelper::nProps);
    //We loop over properties
    std::for_each(std::execution::par, propRange.begin(), propRange.end(),
      [&](auto propNum) {
      auto prop = static_cast< Property >(propNum);
      auto obj1Range = std::views::iota(0, BDDHelper::nObjs);
      // Loop over objects
      std::for_each(std::execution::par, obj1Range.begin(), obj1Range.end(),
        [&](auto objNum1) {
        auto obj1 = static_cast< Object >(objNum1);
        auto obj2Range = std::views::iota(objNum1 + 1, BDDHelper::nObjs);
        // Loop over objects start with obj1+1
        std::for_each(std::execution::par, obj2Range.begin(), obj2Range.end(),
          [&](auto objNum2) {
          auto obj2 = static_cast< Object >(objNum2);
          // We say that for current property
          // obj1 property value must be not equal to obj2 property value
          builder.addConditionTh(notEqual(h.getObjPropertyVars(obj1, prop), h.getObjPropertyVars(obj2, prop)));
        });
      });
    });
  }

  // Here we simply state that each object's properties values must be less than 9
  // In other words, each object's properties values must be NOT 9 NOT 10 NOT 11... NOT 15
  void addValuesUpperBoundCondition(BDDHelper &h, BDDFormulaBuilder &builder)
  {
    // Loop over objects
    for (auto objNum : std::views::iota(0, BDDHelper::nObjs))
    {
      auto obj = static_cast< Object >(objNum);
      // Loop over properties
      for (auto propNum : std::views::iota(0, BDDHelper::nProps))
      {
        auto prop = static_cast< Property >(propNum);
        // Loop over possible property value numbers
        for (auto valNum : std::views::iota(9, 16))
        {
          // Say that for object obj property prop can not have value equal to valNum
          builder.addCondition(not h.numToBinUnsafe(valNum, h.getObjPropertyVars(obj, prop)));
        }
      }
    }
  }

  /**
   * Here we add conditions of type 1
   * For example
   * builder.addCondition(h.getObjectVal(Object::FIRST, Nation::TATARIN));
   * This means that First object MUST have Property Nation with value Nation::TATARIN
   */
  void addFirstCondition(BDDHelper &h, BDDFormulaBuilder &builder, config& config)
  {
      for (auto fconfig: config.getFirstCondition()) {
          builder.addCondition(h.getObjectVal(std::get<0>(fconfig), std::get<1>(fconfig)));
      }
  }

  /**
   * Here we add conditions of type 2
   * For example
   * addLoopCondition(std::make_tuple(Nation::TATARIN, Owns::DRON), h, builder);
   * This says next.
   * There MUST exist object that has BOTH Nation::TATARIN and Owns::DRON.
   * No matter if it Object::FIRST or Object::SECOND or...
   */
  void addSecondCondition(BDDHelper &h, BDDFormulaBuilder &builder, config& config)
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

  /**
   * Here we add conditions of type 3
   * These are addLeftNeighbour and addRightNeighbour
   * For example
   * addLeftNeighbour(Hair::RED, Hair::GREEN, h, builder);
   * This says next.
   * There MUST exist object that are Neighbours and
   * one have Hair::RED and
   * HIS LEFT NEIGHBOUR have Hair::GREEN.
   */
  void addThirdCondition(BDDHelper &h, BDDFormulaBuilder &builder, config& config)
  {
    /**
     * I didn't add any conditions, because i preferred
     * to use 4th type condition.
     */

    // addLeftNeighbour(Hair::RED, Hair::GREEN, h, builder);
  }

  /**
   * Here we add conditions of type 4
   * For example
   * addNeighbours(Nation::CHINESE, Nation::AUSTRALIAN, h, builder);
   * This says next.
   * There MUST exist object that are Neighbours and
   * one have Nation::CHINESE and
   * second have Nation::AUSTRALIAN.
   * 
   * Actually it uses addLeftNeighbour and addLeftNeighbour
   */
  void addFourthCondition(BDDHelper &h, BDDFormulaBuilder &builder, config& config)
  {
        for (auto fconfig: config.getForthCondition()) {
            addNeighbours(std::get<0>(fconfig), std::get<1>(fconfig), h, builder);
        }
  }
}

namespace conditions
{
    void addConditionByType(ConditionTypes type, BDDHelper &h, BDDFormulaBuilder &builder, config& config) {
        switch (type) {
            case ConditionTypes::FIRST: {
                addFirstCondition(h, builder, config);
                break;
            }
            case ConditionTypes::SECOND: {
                addSecondCondition(h, builder, config);
                break;
            }
            case ConditionTypes::THIRD: {
                addThirdCondition(h, builder, config);
                break;
            }
            case ConditionTypes::FOURTH: {
                addFourthCondition(h, builder, config);
                break;
            }
            case ConditionTypes::UNIQUE: {
                addUniqueCondition(h, builder, config);
                break;
            }
            case ConditionTypes::UPPER_BOUND: {
                addValuesUpperBoundCondition(h, builder);
                break;
            }
        }
    }

      void addConditions(BDDHelper &h, BDDFormulaBuilder &builder, const std::set<ConditionTypes>& types, config& config)
      {
          for (auto type: types) {
              addConditionByType(type, h, builder, config);
          }
      }
}
