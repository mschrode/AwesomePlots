#include <iostream>

#include "TError.h"
#include "TStyle.h"

#include "Config.h"
#include "DataSet.h"
#include "Selection.h"
#include "Style.h"


std::map<TString,int> Style::markers_;
std::map<TString,int> Style::colors_;
std::map<TString,TString> Style::dataSetLabels_;
std::map<TString,TString> Style::selectionLabels_;
bool Style::plotYields_ = true;


void Style::init(const Config &cfg, const TString &key) {
  std::cout << "  Setting style parameters...  " << std::flush;

  // ROOT style-settings
  gErrorIgnoreLevel = 1001;
  setGStyle();

  std::vector<Config::Attributes> attrList = cfg(key);
  for(std::vector<Config::Attributes>::const_iterator it = attrList.begin();
      it != attrList.end(); ++it) {

    // Style parameters related to datasets
    if( it->hasName("dataset") ) {
      TString dataSetLabel = it->value("dataset");
      if( it->hasName("marker") && it->isInteger("marker") ) {
	markers_[dataSetLabel] = it->valueInteger("marker");
      }
      if( it->hasName("color") ) {
	colors_[dataSetLabel] = cfg.color(it->value("color"));
      }
      if( it->hasName("plot label") ) {
	dataSetLabels_[dataSetLabel] = it->value("plot label");
      }
    }

    // Style parameters related to selections
    if( it->hasName("selection") ) {
      TString selectionLabel = it->value("selection");
      if( it->hasName("plot label") ) {
	selectionLabels_[selectionLabel] = it->value("plot label");
      }
    }

    // Plotting parameters
    if( it->hasName("plot yields") ) {
      if( it->isBoolean("plot yields") ) {
	plotYields_ = it->valueBoolean("plot yields");
      }
    }
  }

  std::cout << "ok" << std::endl;
}



void Style::setGStyle() {
  // Zero horizontal error bars
  gStyle->SetErrorX(0);

  //  For the canvas
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasColor(kWhite);
  gStyle->SetCanvasDefH(800); //Height of canvas
  gStyle->SetCanvasDefW(800); //Width of canvas
  gStyle->SetCanvasDefX(0);   //Position on screen
  gStyle->SetCanvasDefY(0);
  
  //  For the frame
  gStyle->SetFrameBorderMode(0);
  gStyle->SetFrameBorderSize(1);
  gStyle->SetFrameFillColor(kBlack);
  gStyle->SetFrameFillStyle(0);
  gStyle->SetFrameLineColor(kBlack);
  gStyle->SetFrameLineStyle(0);
  gStyle->SetFrameLineWidth(1);
    
  //  For the Pad
  gStyle->SetPadBorderMode(0);
  gStyle->SetPadColor(kWhite);
  gStyle->SetPadGridX(false);
  gStyle->SetPadGridY(false);
  gStyle->SetGridColor(0);
  gStyle->SetGridStyle(3);
  gStyle->SetGridWidth(1);
  
  //  Margins
  gStyle->SetPadTopMargin(0.08);
  gStyle->SetPadBottomMargin(0.19);
  gStyle->SetPadLeftMargin(0.20);
  gStyle->SetPadRightMargin(0.07);

  //  For the histo:
  gStyle->SetHistLineColor(kBlack);
  gStyle->SetHistLineStyle(0);
  gStyle->SetHistLineWidth(2);
  gStyle->SetMarkerSize(1.4);
  gStyle->SetEndErrorSize(4);
  
  //  For the statistics box:
  gStyle->SetOptStat(0);
  
  //  For the axis
  gStyle->SetAxisColor(1,"XYZ");
  gStyle->SetTickLength(0.03,"XYZ");
  gStyle->SetNdivisions(510,"XYZ");
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);
  gStyle->SetStripDecimals(kFALSE);
  
  //  For the axis labels and titles
  gStyle->SetTitleColor(1,"XYZ");
  gStyle->SetLabelColor(1,"XYZ");
  gStyle->SetLabelFont(42,"XYZ");
  gStyle->SetLabelOffset(0.007,"XYZ");
  gStyle->SetLabelSize(0.045,"XYZ");
  gStyle->SetTitleFont(42,"XYZ");
  gStyle->SetTitleSize(0.06,"XYZ");
  gStyle->SetTitleXOffset(1.2);
  gStyle->SetTitleYOffset(1.7);

  //  For the legend
  gStyle->SetLegendBorderSize(0);
}



TString Style::tlatexType(const DataSet* dataSet) {
  TString label = "";
  if( dataSet->type() == DataSet::Data )              label = "Data";
  else if( dataSet->type() == DataSet::MC )           label = "Sim.";
  else if( dataSet->type() == DataSet::Prediction )   label = "Pred.";
  else if( dataSet->type() == DataSet::MCPrediction ) label = "Pred.";
  else if( dataSet->type() == DataSet::Signal )       label = "Signal";

  return label;
}





