#ifndef STYLE_H
#define STYLE_H

#include "TString.h"

#include "Config.h"
#include "DataSet.h"
#include "Selection.h"


class Style {
public:
  static void init(const Config &cfg, const TString &key);

  static int markerStyle(const DataSet* dataSet) {
    return markers_.find(dataSet->label()) != markers_.end() ? markers_[dataSet->label()] : -1;
  }
  static int color(const DataSet* dataSet) {
    return colors_.find(dataSet->label()) != colors_.end() ? colors_[dataSet->label()] : -1;
  }
  static TString tlatexLabel(const DataSet* dataSet) {
    return dataSetLabels_.find(dataSet->label()) != dataSetLabels_.end() ? dataSetLabels_[dataSet->label()] : dataSet->label();
  }
  static TString tlatexType(const DataSet* dataSet);
  static TString tlatexLabel(const TString &selectionLabel) {
    return selectionLabels_.find(selectionLabel) != selectionLabels_.end() ? selectionLabels_[selectionLabel] : selectionLabel;
  }
  static bool plotYields() { return plotYields_; }
  

private:
  static std::map<TString,int> markers_;
  static std::map<TString,int> colors_;
  static std::map<TString,TString> dataSetLabels_;
  static std::map<TString,TString> selectionLabels_;
  static bool plotYields_;

  static void setGStyle();
};
#endif
