#ifndef FILTER_H
#define FILTER_H

#include <vector>

#include "TString.h"

#include "Config.h"
#include "Event.h"


class Filter {
public:
  static const Filter* create(const TString &expr, const std::vector<TString> &dataSetLabels, unsigned int lineNum, const TString &label) { return create(expr,dataSetLabels,lineNum,true,label); }
  static void clear();

  Filter(const TString &uid) : uid_(uid) { garbage_.push_back(this); }
  virtual ~Filter() {};

  virtual TString printOut() const = 0;
  virtual bool passes(const Event* evt, const TString &dataSetLabel) const = 0;

  TString uid() const { return uid_; }


protected:
  static TString offset_;

  static TString cleanExpression(const TString &expr);

  TString uid_;


private:
  static std::vector<Filter*> garbage_;

  static const Filter* create(const TString &expr, const std::vector<TString> &dataSetLabels, unsigned int lineNum, bool firstIteration, const TString &label);
  static void checkForDanglingOperators(const TString &expr, unsigned int lineNum);
  static void checkForMismatchingParentheses(const TString &expr, unsigned int lineNum);
  static void checkForInvalidBooleanOperators(const TString &expr, unsigned int lineNum);
  static bool isNegated(TString &expr);
  static bool isSplit(const TString &expr, TString &expr1, TString &expr2, TString &op);
};


class Cut : public Filter {
public:
  static Cut* create(const TString &expr, unsigned int lineNum);

  Cut(const TString &uid) : Filter(uid) {};
  virtual ~Cut() {};

  TString printOut() const { return offset_+"|-- "+uid(); }
  virtual bool passes(const Event* evt, const TString &dataSetLabel) const = 0;


protected:
  TString var_;
  double val_;
};


class CutGreaterThan : public Cut {
public:
  CutGreaterThan(const TString &var, double val);

  bool passes(const Event* evt, const TString &dataSetLabel) const { return evt->get(var_) > val_; }
};


class CutGreaterEqualThan : public Cut {
public:
  CutGreaterEqualThan(const TString &var, double val);

  bool passes(const Event* evt, const TString &dataSetLabel) const { return evt->get(var_) >= val_; }
};


class CutLessThan : public Cut {
public:
  CutLessThan(const TString &var, double val);

  bool passes(const Event* evt, const TString &dataSetLabel) const { return evt->get(var_) < val_; }
};


class CutLessEqualThan : public Cut {
public:
  CutLessEqualThan(const TString &var, double val);

  bool passes(const Event* evt, const TString &dataSetLabel) const { return evt->get(var_) <= val_; }
};


class CutEqual : public Cut {
public:
  CutEqual(const TString &var, double val);

  bool passes(const Event* evt, const TString &dataSetLabel) const { return evt->get(var_) == val_; }
};


class CutNotEqual : public Cut {
public:
  CutNotEqual(const TString &var, double val);

  bool passes(const Event* evt, const TString &dataSetLabel) const { return evt->get(var_) != val_; }
};


class CutLessThanLessThan : public Cut {
public:
  CutLessThanLessThan(double val1, const TString &var, double val2);

  bool passes(const Event* evt, const TString &dataSetLabel) const;

private:
  double val2_;
};


class CutLessEqualThanLessEqualThan : public Cut {
public:
  CutLessEqualThanLessEqualThan(double val1, const TString &var, double val2);

  bool passes(const Event* evt, const TString &dataSetLabel) const;

private:
  double val2_;
};


class BooleanOperator : public Filter {
public:
  BooleanOperator(const Filter* filter1, const Filter* filter2, const TString &name);

  TString printOut() const;
  virtual bool passes(const Event* evt, const TString &dataSetLabel) const = 0;
  

protected:
  const Filter* filter1_;
  const Filter* filter2_;


private:
  TString name_;
};


class FilterAND : public BooleanOperator {
public:
  FilterAND(const Filter* filter1, const Filter* filter2);

  bool passes(const Event* evt, const TString &dataSetLabel) const;
};


class FilterOR : public BooleanOperator {
public:
  FilterOR(const Filter* filter1, const Filter* filter2);

  bool passes(const Event* evt, const TString &dataSetLabel) const;
};


class FilterNOT : public Filter {
public:
  FilterNOT(const Filter* filter);

  TString printOut() const { return offset_+"|-- "+uid(); }
  bool passes(const Event* evt, const TString &dataSetLabel) const { return !(filter_->passes(evt,dataSetLabel)); }


private:
  const Filter* filter_;
};


// Lets pass all events
class FilterTRUE : public Filter {
public:
  FilterTRUE() : Filter("FilterTRUE") {};
  
  TString printOut() const { return offset_+"TRUE"; }
  bool passes(const Event* evt, const TString &dataSetLabel) const { return true; }
};


// Returns true for all but the specified datasets
// Otherwise apply cuts
class FilterDataSet : public Filter {
public:
  FilterDataSet(const Filter* filter, const std::vector<TString> &applyToDataSets);
  
  TString printOut() const;
  bool passes(const Event* evt, const TString &dataSetLabel) const;

  
private:
  const Filter* filter_;

  std::vector<TString> applyToDataSets_;
};

#endif
