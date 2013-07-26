#ifndef PLOT_BUILDER_H
#define PLOT_BUILDER_H

#include <map>
#include <vector>

#include "TCanvas.h"
#include "TGraphAsymmErrors.h"
#include "TH1.h"
#include "TH2.h"
#include "TLegend.h"
#include "TPaveText.h"
#include "TString.h"

#include "Config.h"
#include "DataSet.h"
#include "Output.h"


class PlotBuilder {
public:
  PlotBuilder(const Config &cfg, Output &out);
  ~PlotBuilder();

private:
  class HistParams {
  public:
    HistParams() : nBinsX_(1), xMin_(0), xMax_(1), nBinsY_(0), yMin_(0), yMax_(1), logx_(false), logy_(false), logz_(false), norm_(false), hasOverflowBin_(true) {};
    HistParams(const TString &cfg);	

    int nBinsX() const { return nBinsX_; }
    double xMin() const { return xMin_; }
    double xMax() const { return xMax_; }
    int nBinsY() const { return nBinsY_; }
    double yMin() const { return yMin_; }
    double yMax() const { return yMax_; }
    bool logx() const { return logx_; }
    bool logy() const { return logy_; }
    bool logz() const { return logz_; }
    bool norm() const { return norm_; }
    bool hasOverflowBin() const { return hasOverflowBin_; }

  private:
    int nBinsX_;
    double xMin_;
    double xMax_;
    int nBinsY_;
    double yMin_;
    double yMax_;
    bool logx_;
    bool logy_;
    bool logz_;
    bool norm_;
    bool hasOverflowBin_;
  };

  static unsigned int count_;

  const unsigned int canSize_;

  Output &out_;

  void run(const Config &cfg, const TString &key) const;
  void plotDistribution(const TString &var, const DataSet *dataSet, const HistParams &histParams) const;
  void plotDistribution2D(const TString &var1, const TString &var2, const DataSet *dataSet, const HistParams &histParams) const;
  void plotStackedDistributions(const TString &var, const DataSets &dataSets, const HistParams &histParams) const;
  void plotComparedDistributions(const TString &var, const DataSets &dataSets, const HistParams &histParams) const;
  void plotDataVsBkg(const TString &var, const DataSet *data, const DataSets &bkgs, const DataSets &signals, const HistParams &histParams) const;
  void createDistribution1D(const DataSet *dataSet, const TString &var, TH1* &h, TGraphAsymmErrors* &uncert, const HistParams &histParams) const;
  void createDistributionRatio(const DataSet *dataSet, const TString &var1, const TString &var2, TH1* &h, TGraphAsymmErrors* &uncert, const HistParams &histParams) const;
  void createDistribution2D(const DataSet *dataSet, const TString &var1, const TString &var2, TH2* &h, const HistParams &histParams) const;
  void createStack1D(const DataSets &dataSets, const TString &var, std::vector<TH1*> &hists, std::vector<TString> &legEntries, TGraphAsymmErrors* &uncert, const HistParams &histParams) const;

  void storeCanvas(TCanvas* can, const TString &var, const DataSet* dataSet) const;
  void storeCanvas(TCanvas* can, const TString &var, const TString &selection) const;
  void storeCanvas(TCanvas* can, const TString &var, const DataSets &dataSets) const;
  void storeCanvas(TCanvas* can, const TString &var, const DataSet* dataSet, const DataSets &dataSets) const;

  int color(const DataSet *dataSet) const;
  int markerStyle(const DataSet *dataSet) const;
  void setMarkerStyle(TH1* h, const DataSet *dataSet) const;
  void setGenericStyle(TH1* h, const DataSet *dataSet) const;
  void setXTitle(TH1* h, const TString &var) const;
  void setXTitle(TH1* h, const TString &var1, const TString &var2) const;
  void setYTitle(TH1* h, const TString &var) const;
  TPaveText* header(const DataSet* ds, bool showLumi, const TString &info = "") const;
  TPaveText* header(const DataSets &ds, bool isSimulation, bool showLumi, const TString &info = "") const;
  TPaveText* header(bool isSimulation, bool showLumi, const TString &info = "") const;
  TLegend* legend(unsigned int nEntries) const;
  TString lumiLabel() const;
  TString dataSetLegEntry(const TH1* h, const DataSet* ds) const;
  void setYRange(TH1* &h, double logMin = -1.) const;

  bool checkForUnderOverFlow(const TH1* h, const TString &var, const DataSet *dataSet) const;
};
#endif
