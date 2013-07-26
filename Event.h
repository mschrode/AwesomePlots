#ifndef EVENT_H
#define EVENT_H

#include <map>
#include <vector>

#include "TString.h"

class Event {
  friend class EventBuilder;

public:
  ~Event() {};

  double get(const TString &var) const { return vars_.at(varIdx_.find(var)->second); }
  double weight() const { return weight_; }
  bool hasUnc() const { return relUncDn_.size() > 0; }
  double weightUncDn() const { return weight()*(1.-relTotalUncDn()); };
  double weightUncUp() const { return weight()*(1.+relTotalUncUp()); };
  double relTotalUncDn() const { return relTotalUncDn_; };
  double relTotalUncUp() const { return relTotalUncUp_; };
  double relUncDn(const TString &label) const;
  double relUncUp(const TString &label) const;
  
private:
  static std::map<TString,unsigned int> varIdx_;

  const double weight_;

  std::vector<double> vars_; // Needs double precision for correct display of runnumber!!!
  double relTotalUncDn_;
  double relTotalUncUp_;
  std::map<TString,double> relUncDn_;
  std::map<TString,double> relUncUp_;

  Event();
  Event(double weight);
  void init();
  void set(const TString &var, double val);
  void addRelUnc(double dn, double up, const TString &label = "label");
};

typedef std::vector<Event*> Events;
typedef std::vector<Event*>::const_iterator EventIt;
#endif
