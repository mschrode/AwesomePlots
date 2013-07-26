#ifndef MR_RA2_H
#define MR_RA2_H

#include "TString.h"

#include "Config.h"

class MrRA2 {
public:
  MrRA2(const TString& configFileName);
  ~MrRA2();

private:
  void checkForLatestSyntax(const Config &cfg) const;
};
#endif
