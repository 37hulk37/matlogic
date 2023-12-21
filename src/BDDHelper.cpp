#include "BDDHelper.hpp"
//#include <expected>
#include <utility>
#include <algorithm>
#include <ranges>

namespace bddHelper
{
  BDDHelper::BDDHelper(vect< vect< vect< bdd > > > structedVars) :
    structVars_(std::move(structedVars))
  {
    values_ = vect< vect< vect< bdd > > >(nObjs);
    for (auto objNum : std::views::iota(0, nObjs))
    {
      values_[objNum] = vect< vect< bdd > >(nProps);
      for (auto propNum : std::views::iota(0, nProps))
      {
        values_[objNum][propNum] = vect< bdd > (nVals);
        for (auto valNum : std::views::iota(0, nVals))
        {
          values_[objNum][propNum][valNum] = numToBin(valNum, structVars_[objNum][propNum]);
        }
      }
    }
  }

  std::vector< bdd > BDDHelper::getObjPropertyVars(Object obj, Property prop)
  {
    auto objNum = toNum(obj);
    auto propNum = toNum(prop);
    return structVars_[objNum][propNum];
  }

  // See BDDHelper::numToBinUnsafe - right the next
  bdd BDDHelper::numToBin(int num, vect< bdd > vars)
  {
    assert(vars.size() == 4);
    assert(num >= 0 and num <= 8);
    return numToBinUnsafe(num, vars);
  }

  bdd BDDHelper::numToBinUnsafe(int num, vect< bdd > vars)
  {
    assert(vars.size() == 4);
    assert(num >= 0 and num <= 15);
    auto resFormula = bdd_true();
    auto currentNum = num;
    for (auto var : std::views::reverse(vars))
    {
      auto bit = currentNum % 2;
      currentNum /= 2;
      if (bit == 1)
        resFormula &= var;
      else
        resFormula &= not var;
    }
    return resFormula;
  }

  // See BDDHelper.hpp
  int toNum(Property value)
  {
    auto val = static_cast< int >(value);
    assert(("Bad value", val >= 0 and val <= 3));
    return val;
  }
}
