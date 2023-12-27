#include <set>
#include "bdd.h"
#include "BDDHelper.hpp"
#include "BDDFormulaBuilder.hpp"

using namespace bddHelper;
namespace conditions
{
  void addConditions(bddHelper::BDDHelper &h, BDDFormulaBuilder &builder, const std::set<ConditionTypes>& types, config& config);
}