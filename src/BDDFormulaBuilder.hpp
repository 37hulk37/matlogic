#ifndef BDD_FORMULA_BUILDER_HPP
#define BDD_FORMULA_BUILDER_HPP

#include "bdd.h"
#include <mutex>
#include "config.h"

class BDDFormulaBuilder
{
public:
  BDDFormulaBuilder();

  void addCondition(bdd formula);

  void addConditionTh(bdd formula);

  bdd result();

private:
  bdd formula_;
  std::mutex mut_;
};

#endif
