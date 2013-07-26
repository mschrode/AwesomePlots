#ifndef EVENT_BUILDER_H
#define EVENT_BUILDER_H

#include "TString.h"

#include "Event.h"

class EventBuilder {
public:
  Events operator()(const TString &fileName, const TString &treeName, const TString &weight, const std::vector<TString> &uncDn, const std::vector<TString> &uncUp, const std::vector<TString> &uncLabel, double scale) const;
};
#endif
