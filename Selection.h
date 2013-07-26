#ifndef SELECTION_H
#define SELECTION_H

#include <vector>

#include "TString.h"

#include "Config.h"
#include "Event.h"
#include "Filter.h"


class Selection;

typedef std::vector<Selection*> Selections;
typedef std::vector<Selection*>::const_iterator SelectionIt;


class Selection {
public:
  static void init(const Config &cfg, const TString key);
  static const Selection* find(const TString &uid);
  static SelectionIt begin() { return selections_.begin(); }
  static SelectionIt end() { return selections_.end(); }
  static unsigned int maxLabelLength();
  static void clear();

  Selection(const TString &uid, const Filter* filter) : uid_(uid), filter_(filter) {};

  const Filter* filter() const { return filter_; }
  bool passes(const Event* evt, const TString &dataSetLabel) const { filter_->passes(evt,dataSetLabel); }
  void print() const;
  TString uid() const { return uid_; }


private:
  static Selections selections_;
  static bool isInit_;
  static bool printFilterTree_;

  const TString uid_;
  const Filter* filter_;
};
#endif
