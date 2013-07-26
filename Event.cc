#include "Event.h"

#include <cmath>

#include "Variable.h"


std::map<TString,unsigned int> Event::varIdx_;


Event::Event()
  : weight_(1.), relTotalUncDn_(0.), relTotalUncUp_(0.) {
  init();
}

Event::Event(double weight)
  : weight_(weight), relTotalUncDn_(0.), relTotalUncUp_(0.) {
  init();
}

void Event::init() {
  Variable::checkIfIsInit();
  if( varIdx_.size() == 0 ) {
    for(std::vector<TString>::const_iterator it = Variable::begin();
	it != Variable::end(); ++it) {
      varIdx_[*it] = varIdx_.size()-1;
    }
  }
  vars_ = std::vector<double>(varIdx_.size());
}


void Event::set(const TString &var, double val) {
  vars_.at(varIdx_.find(var)->second) = val;
}


double Event::relUncDn(const TString &label) const {
  double unc = 0.;
  std::map<TString,double>::const_iterator it = relUncDn_.find(label);
  if( it != relUncDn_.end() ) {
    unc = it->second;
  }

  return unc;
}


double Event::relUncUp(const TString &label) const {
  double unc = 0.;
  std::map<TString,double>::const_iterator it = relUncUp_.find(label);
  if( it != relUncUp_.end() ) {
    unc = it->second;
  }

  return unc;
}


void Event::addRelUnc(double dn, double up, const TString &label) {
  if( relUncDn_.find(label) == relUncDn_.end() &&
      relUncUp_.find(label) == relUncUp_.end()    ) { 
    // Add to list of uncertainties
    relUncDn_[label] = dn;
    relUncUp_[label] = up;
    // Recompute total uncertainty
    // (quadratic sum of all uncertainties)
    relTotalUncDn_ = sqrt( relTotalUncDn_*relTotalUncDn_ + dn*dn );
    relTotalUncUp_ = sqrt( relTotalUncUp_*relTotalUncUp_ + up*up );
  }
}

