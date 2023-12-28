#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include <set>
#include "bdd.h"
#include "BDDHelper.hpp"
#include "BDDFormulaBuilder.hpp"
#include "Conditions.hpp"
#include "PrintHelper.hpp"
#include "config.h"

using namespace bddHelper;

template < class T > using vect = std::vector< T >;

// We have nObjs objects
constexpr int nObjs = bddHelper::BDDHelper::nObjs;

constexpr int nProps = bddHelper::BDDHelper::nProps;

constexpr int nVals = bddHelper::BDDHelper::nVals;


constexpr int nValueBits = bddHelper::BDDHelper::nValueBits;

constexpr int nValuesVars = bddHelper::BDDHelper::nValuesVars;

constexpr int nTotalVars = bddHelper::BDDHelper::nTotalVars;

std::once_flag once;
std::string varset;

void extractSet(char *varset_, int size)
{
  std::call_once(once, [&]() {
    varset = std::string(varset_, size);
  });
}

void printProp(Property prop, int valNum)
{
      switch (prop)
      {
        case Property::OWNS:
          std::cout << to_string(static_cast< Owns >(valNum)) << '\n';
          break;

        case Property::HAIR:
          std::cout << to_string(static_cast< Hair >(valNum)) << '\n';
          break;

        case Property::NATION:
          std::cout << to_string(static_cast< Nation >(valNum)) << '\n';
          break;

        case Property::TRANSPORT:
          std::cout << to_string(static_cast< Transport >(valNum)) << '\n';
          break;
      }
    }

    void printObjects()
    {
      if (varset.empty())
      {
        std::cout << "No suitable object property value combination was found.\n";
        return;
      }
      if (varset.size() != nTotalVars)
      {
            std::cout << "Array varset must contain 144 values.\
                          Otherwise there is an error in calculations.\n";
            return;
      }
      for (auto objNum : std::views::iota(0, nObjs))
      {
        auto obj = static_cast< Object >(objNum);
        std::cout << to_string(obj) << " {\n";
        for (auto propNum : std::views::iota(0, nProps))
        {
          auto prop = static_cast< Property >(propNum);
          std::cout << '\t' << to_string(prop) << ": ";
          auto baseIndex = objNum * nProps * nValueBits + propNum * nValueBits;
          int valNum = (varset.at(baseIndex) << 3) +
                       (varset.at(baseIndex + 1) << 2) +
                       (varset.at(baseIndex + 2) << 1) +
                       (varset.at(baseIndex + 3) << 0);
          printProp(prop, valNum);
        }
        std::cout << "}\n";
      }
    }

    int main() {
      bdd_init(3000000, 100000);
      bdd_setvarnum(BDDHelper::nTotalVars);
      std::vector< bdd > vars(nTotalVars);
      { // Here we just put all these variables in array.
        int i = 0;
        std::generate(std::begin(vars), std::end(vars),
          [&i]() {
          return bdd_ithvar(i++);
        });
      }
      auto structedVars = vect< vect< vect< bdd > > >(nObjs);
      for (auto objNum : std::views::iota(0, nObjs))
      {
        structedVars[objNum] = vect< vect< bdd > >(nProps);
        for (auto propNum : std::views::iota(0, nProps))
        {
          auto baseIndex = objNum * nProps * nValueBits + propNum * nValueBits;
          structedVars[objNum][propNum] = vect< bdd >{
            bdd_ithvar(baseIndex + 0),
            bdd_ithvar(baseIndex + 1),
            bdd_ithvar(baseIndex + 2),
            bdd_ithvar(baseIndex + 3) };
        }
      }

    std::set<ConditionTypes> types = {
          ConditionTypes::FIRST,
          ConditionTypes::SECOND,
          ConditionTypes::FOURTH,
          ConditionTypes::UNIQUE,
          ConditionTypes::UPPER_BOUND
    };

    config config;

    // Let's explore what is BDDHelper
    bddHelper::BDDHelper h(structedVars);
    // Simpliest class in the world. Just contains result formula.
    BDDFormulaBuilder builder;
    conditions::addConditions(h, builder, types);
    std::cout << "Bdd formula created. Starting counting sets...\n";
    std::cout << "Count of true variables values combinations: " << bdd_satcount(builder.result()) << '\n';
    std::cout << "Objects are...\n";
    // Iterate over true combinations and extract one of them in varset variable.
    bdd_allsat(builder.result(), extractSet);
    // Print one of suitable objects properties combinations
    printObjects();
    bdd_done();
    return 0;
}
