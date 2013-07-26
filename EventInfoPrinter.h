// $Id: EventInfoPrinter.h,v 1.7 2013/04/22 16:21:51 mschrode Exp $

#ifndef EVENT_INFO_PRINTER_H
#define EVENT_INFO_PRINTER_H

#include <list>
#include <map>
#include <set>
#include <vector>

#include "TString.h"

#include "Config.h"
#include "DataSet.h"
#include "Event.h"

class EventInfoPrinter {
public:
  EventInfoPrinter(const Config &cfg);
  ~EventInfoPrinter() {};

private:
  static TString runSortVar_;
  static bool greaterByRun(const Event* evt1, const Event* evt2) {
    return evt1->get(EventInfoPrinter::runSortVar_) < evt2->get(EventInfoPrinter::runSortVar_);	
  }

  const Config &cfg_;

  std::map< TString, unsigned int > selectionVariables_;
  std::set<TString> printedSelections_;
  std::map< TString, std::vector<const Event*> > printedEvts_;
  TString outFileName_;
  TString latexSlidesName_;

  bool init(const TString &key);
  void selectEvents();
  void print() const;
  std::list<TString> listOfPrintedVariables() const;
  bool printAllEvents() const;
  bool printSelection(const TString &uid) const;

  // To sort evts according to one of their quantities
  class EvtValPair {
  public:
    static bool valueGreaterThan(const EvtValPair *idx1, const EvtValPair *idx2);

    EvtValPair(const Event *evt, double value)
      : evt_(evt), val_(value) {}

    const Event* event() const { return evt_; }
    double value() const { return val_; }

  private:
    const Event* evt_;
    const double val_;
  };

  TString varNameNJets;
  TString varNameHT;
  TString varNameMHT;
  TString varNameRunNum;
  TString varNameLumiBlockNum;
  TString varNameEvtNum;
};
#endif
