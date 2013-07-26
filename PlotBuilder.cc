#include <cmath>
#include <cstdlib>
#include <iostream>

#include "TH1D.h"
#include "TH2D.h"
#include "TPad.h"
#include "TStyle.h"

#include "GlobalParameters.h"
#include "PlotBuilder.h"
#include "Selection.h"
#include "Style.h"
#include "Variable.h"


unsigned int PlotBuilder::count_ = 0;


PlotBuilder::PlotBuilder(const Config &cfg, Output &out)
  : canSize_(500), out_(out) {
  run(cfg,"plot");
}


PlotBuilder::~PlotBuilder() {
  
}


void PlotBuilder::run(const Config &cfg, const TString &key) const {
  std::cout << "  - Creating control plots" << std::endl;

  //// Loop over the config lines
  std::vector<Config::Attributes> attrList = cfg(key);
  for(std::vector<Config::Attributes>::const_iterator it = attrList.begin();
      it != attrList.end(); ++it) {

    if( it->hasName("variable") && it->hasName("histogram") ) {
      //// Possible plot types
      TString plotDim  = "1D";
      TString plotType = "SingleDistribution";


      //// Parse variables
      std::vector<TString> variables;
      if( it->value("variable").Contains(" vs ") ) {
	Config::split(it->value("variable")," vs ",variables);
	plotDim = "2D";
      } else {
	variables.push_back(it->value("variable"));
      }
      for(std::vector<TString>::const_iterator itv = variables.begin();
	  itv != variables.end(); ++itv) {
	if( !Variable::exists(*itv) ) {
	  std::cerr << "\n\nERROR in PlotBuilder::run(): variable '" << *itv << "' does not exist" << std::endl;
	  exit(-1);
	}
      }
      if( variables.size() > 2 ) {
	std::cerr << "\n\nERROR in PlotBuilder::run(): no plot type supports more than two variables" << std::endl;
	exit(-1);
      }


      //// Parse histogram style
      HistParams histParams(it->value("histogram"));


      //// Make plots
      // Specified datasets; defines the type of plot
      if( it->hasName("dataset") ) {
	if( it->value("dataset").Contains(",") )      plotType = "ComparedDistributions";
	else if( it->value("dataset").Contains("+") ) plotType = "StackedDistributions";
	else                                          plotType = "SingleDistribution";

	// Read dataset(s)
	std::vector<TString> dataSetLabels;
	if( plotType == "ComparedDistributions" ) {
	  Config::split(it->value("dataset"),",",dataSetLabels);
	} else if( plotType == "StackedDistributions" ) {
	  Config::split(it->value("dataset"),"+",dataSetLabels);
	} else {
	  dataSetLabels.push_back(it->value("dataset"));
	}

	// Check whether datasets exist
	for(std::vector<TString>::const_iterator itd = dataSetLabels.begin();
	    itd != dataSetLabels.end(); ++itd) {
 	  if( !DataSet::labelExists(*itd) ) {
 	    std::cerr << "\n\nERROR in PlotBuilder::run(): dataset '" << *itd << "' does not exist" << std::endl;
 	    exit(-1);
 	  }
	}

	// For each selection, get the datasets and make the plots
	for(SelectionIt its = Selection::begin(); its != Selection::end(); ++its) {
	  DataSets dataSets;
	  for(std::vector<TString>::const_iterator itd = dataSetLabels.begin();
	      itd != dataSetLabels.end(); ++itd) {
	    dataSets.push_back(DataSet::find(*itd,*its));
	  }
	  if( plotDim == "1D" ) {
	    if( plotType == "SingleDistribution" ) {
	      plotDistribution(variables.front(),dataSets.front(),histParams);
	    } else if( plotType == "StackedDistributions" ) {
	      plotStackedDistributions(variables.front(),dataSets,histParams);
	    } else if( plotType == "ComparedDistributions" ) {
	      plotComparedDistributions(variables.front(),dataSets,histParams);
	    }
	  } else if( plotDim == "2D" ) {
	    plotDistribution2D(variables.at(1),variables.at(0),dataSets.front(),histParams);
	  }
	}


      } else if( it->hasName("data") && it->hasName("background") ) {
	plotType = "DataVsBackground";
      
	// Read dataset names
	TString dataLabel = it->value("data");
	std::vector<TString> bkgLabels;
	std::vector<TString> signalLabels;
	Config::split(it->value("background"),"+",bkgLabels);
	if( it->hasName("signals") ) {
	  Config::split(it->value("signals"),",",signalLabels);
	}

	// Check whether datasets exist
	if( !DataSet::labelExists(dataLabel) ) {
	  std::cerr << "\n\nERROR in PlotBuilder::run(): dataset '" << dataLabel << "' does not exist" << std::endl;
	  exit(-1);
	}
	for(std::vector<TString>::const_iterator itd = bkgLabels.begin();
	    itd != bkgLabels.end(); ++itd) {
 	  if( !DataSet::labelExists(*itd) ) {
 	    std::cerr << "\n\nERROR in PlotBuilder::run(): dataset '" << *itd << "' does not exist" << std::endl;
 	    exit(-1);
 	  }
	}
	for(std::vector<TString>::const_iterator itd = signalLabels.begin();
	    itd != signalLabels.end(); ++itd) {
 	  if( !DataSet::labelExists(*itd) ) {
 	    std::cerr << "\n\nERROR in PlotBuilder::run(): dataset '" << *itd << "' does not exist" << std::endl;
 	    exit(-1);
 	  }
	}
	
	// For each selection, plot data vs bkg
	for(SelectionIt its = Selection::begin(); its != Selection::end(); ++its) {
	  const DataSet* data = DataSet::find(dataLabel,*its);
	  DataSets bkgs;
	  for(std::vector<TString>::const_iterator itd = bkgLabels.begin();
	      itd != bkgLabels.end(); ++itd) {
	    bkgs.push_back(DataSet::find(*itd,*its));
	  }
	  DataSets signals;
	  for(std::vector<TString>::const_iterator itd = signalLabels.begin();
	      itd != signalLabels.end(); ++itd) {
	    signals.push_back(DataSet::find(*itd,*its));
	  }
	  plotDataVsBkg(variables.front(),data,bkgs,signals,histParams);
	}
      }
    } else {
      // no 'plot' or 'histogram' definitions
      std::cerr << "\n\nERROR in PlotBuilder::run(): wrong syntax" << std::endl;
      std::cerr << "  in line with key '" << key << "'" << std::endl;
      std::cerr << "  in config file '" << cfg.fileName() << "'" << std::endl;
      exit(-1);
    }
  } // End of loop over config lines
}


void PlotBuilder::plotDistribution(const TString &var, const DataSet *dataSet, const HistParams &histParams) const {
  TH1* h = 0;
  TGraphAsymmErrors* u = 0;
  createDistribution1D(dataSet,var,h,u,histParams);

  // Under-/Overflow warning
  checkForUnderOverFlow(h,var,dataSet);

  TCanvas *can = new TCanvas("can","",canSize_,canSize_);
  //can->SetWindowSize(canSize_+(canSize_-can->GetWw()),canSize_+(canSize_-can->GetWh()));
  if( dataSet->type() == DataSet::Data ) {
    h->Draw("PE");
    if( u ) {
      u->Draw("E2same");
      h->Draw("PEsame");
    }
  } else if( dataSet->type() == DataSet::Signal ) {
    h->Draw("HIST");
    if( u ) {
      u->Draw("E2same");
    }
  } else {
    h->SetLineColor(kBlack);
    h->SetLineStyle(1);
    h->SetLineWidth(1);
    h->Draw("HISTE");
    if( u ) {
      u->Draw("E2same");
    }
  }
  TPaveText* title = header(dataSet,true,Style::tlatexLabel(dataSet)+",  "+Style::tlatexLabel(dataSet->selectionUid()));
  title->Draw("same");
  if( histParams.logx() ) can->SetLogx();
  if( histParams.logy() ) can->SetLogy();
  gPad->RedrawAxis();
  storeCanvas(can,var,dataSet);

  delete title;
  delete h;
  if( u ) delete u;
  delete can;
}


void PlotBuilder::plotDistribution2D(const TString &var1, const TString &var2, const DataSet *dataSet, const HistParams &histParams) const {
  TH2* h = 0;
  createDistribution2D(dataSet,var1,var2,h,histParams);

  TCanvas *can = new TCanvas("can","",canSize_,canSize_);
  can->SetRightMargin(can->GetRightMargin()+0.06);
  can->SetBottomMargin(can->GetBottomMargin()+0.03);
  can->SetTopMargin(can->GetTopMargin()+0.03);
  //can->SetWindowSize(canSize_+(canSize_-can->GetWw()),canSize_+(canSize_-can->GetWh()));
  h->Draw("COLZ");
  TPaveText* title = header(dataSet,true,Style::tlatexLabel(dataSet)+",  "+Style::tlatexLabel(dataSet->selectionUid()));
  title->Draw("same");
  if( histParams.logx() ) can->SetLogx();
  if( histParams.logy() ) can->SetLogy();
  if( histParams.logz() ) can->SetLogz();
  gPad->RedrawAxis();
  storeCanvas(can,var1+"vs"+var2,dataSet);

  delete title;
  delete h;
  delete can;
}


void PlotBuilder::plotComparedDistributions(const TString &var, const DataSets &dataSets, const HistParams &histParams) const {
  std::vector<TH1*> hists;
  TLegend* leg = legend(dataSets.size());
  // Loop over datasets and create distributions
  for(DataSetIt itd = dataSets.begin();
      itd != dataSets.end(); ++itd) {
    TH1* h = 0;
    TGraphAsymmErrors* u = 0;
    createDistribution1D(*itd,var,h,u,histParams);
    if( u ) delete u;
    if( histParams.norm() && h->Integral() ) {
      h->Scale(1./h->Integral("width"));
      h->GetYaxis()->SetTitle("Probability Density");
    }
    if( (*itd)->type() != DataSet::Data ) {
      h->SetLineColor(h->GetFillColor());
      h->SetLineWidth(3);
      h->SetFillStyle(0);
      leg->AddEntry(h," "+Style::tlatexLabel(*itd),"L");
    } else {
      leg->AddEntry(h," "+Style::tlatexLabel(*itd),"P");
    }
    if( histParams.norm() ) setYRange(h,histParams.logy()?3E-6:-1.);
    else setYRange(h,histParams.logy()?3E-1:-1.);
    hists.push_back(h);
  }

  TCanvas *can = new TCanvas("can","",canSize_,canSize_);
  if( dataSets.back()->type() == DataSet::Data ) {
    hists.back()->Draw("PE");
  } else {
    hists.back()->Draw("HIST");
  }
  for(int i = hists.size()-2; i >= 0; --i) {
    if( dataSets.at(i)->type() == DataSet::Data ) {
      hists.at(i)->Draw("PEsame");
    } else {
      hists.at(i)->Draw("HISTsame");
    }
  }
  TPaveText* title = header(dataSets,false,Style::tlatexLabel(dataSets.front()->selectionUid()));
  title->Draw("same");
  leg->Draw("same");
  gPad->RedrawAxis();
  if( histParams.logx() ) can->SetLogx();
  if( histParams.logy() ) can->SetLogy();
  storeCanvas(can,var,dataSets.front()->selectionUid());
  
  delete title;
  delete leg;
  for(std::vector<TH1*>::iterator it = hists.begin();
      it != hists.end(); ++it) {
    delete *it;
  }
  delete can;
}


// Plot stacked datasets
void PlotBuilder::plotStackedDistributions(const TString &var, const DataSets &dataSets, const HistParams &histParams) const {
  std::vector<TH1*> hists;
  std::vector<TString> legEntries;
  TGraphAsymmErrors* unc = 0;
  createStack1D(dataSets,var,hists,legEntries,unc,histParams);

  TLegend* leg = legend(hists.size());
  std::vector<TString>::const_iterator itL = legEntries.begin();
  for(std::vector<TH1*>::iterator itH = hists.begin();
      itH != hists.end(); ++itH,++itL) {
    leg->AddEntry(*itH,*itL,"F");
  }
  bool onlySimulatedDataSets = true;
  TPaveText* title = header(dataSets,true,Style::tlatexLabel(dataSets.front()->selectionUid()));

  // Draw
  TCanvas* can = new TCanvas("can","",canSize_,canSize_);
  can->cd();
  setYRange(hists.front(),histParams.logy()?3E-1:-1.);
  hists.front()->Draw("HISTE");
  for(std::vector<TH1*>::iterator it = hists.begin()+1;
      it != hists.end(); ++it) {
    (*it)->Draw("HISTsame");
  }
  // Plot uncertainty band
  if( unc ) unc->Draw("E2same");
  title->Draw("same");
  leg->Draw("same");
  gPad->RedrawAxis();
  if( histParams.logx() ) can->SetLogx();
  if( histParams.logy() ) can->SetLogy();
  storeCanvas(can,var,dataSets);

  for(std::vector<TH1*>::iterator it = hists.begin();
      it != hists.end(); ++it) {
    delete *it;
  }
  if( unc ) delete unc;
  delete leg;
  delete title;
  delete can;
}



void PlotBuilder::plotDataVsBkg(const TString &var, const DataSet *data, const DataSets &bkgs, const DataSets &signals, const HistParams &histParams) const {
  
  //// Create the data distribution

  // Fill distribution
  TH1* hData = 0;
  TGraphAsymmErrors* u = 0;
  createDistribution1D(data,var,hData,u,histParams);
  if( u ) delete u;
  TString legEntryData = dataSetLegEntry(hData,data);

  // Under-/Overflow warning
  checkForUnderOverFlow(hData,var,data);

  // We want a data-vs-bkg like plot. Therefore, make
  // sure that markers are set, even if data is of
  // type DataSet::MC or DataSet::MCPrediction, e.g. for 
  // closure tests
  setMarkerStyle(hData,data);

  bool simulationOnlyPlot = data->isSimulated();


  //// Create the background stack
  std::vector<TH1*> stack;
  std::vector<TString> legEntriesBkgs;
  TGraphAsymmErrors* uncBkg = 0;
  createStack1D(bkgs,var,stack,legEntriesBkgs,uncBkg,histParams);
  if( simulationOnlyPlot ) {
    for(DataSetIt itd = bkgs.begin(); itd != bkgs.end(); ++itd) {
      if( !( (*itd)->isSimulated() ) ) {
	simulationOnlyPlot = false;
	break;
      }
    }
  }


  //// Create the signal distributions
  std::vector<TH1*> hSigs;
  std::vector<TString> legEntriesSigs;
  for(DataSetIt itd = signals.begin();
      itd != signals.end(); ++itd) {
    TH1* h = 0;
    TGraphAsymmErrors* u = 0;
    createDistribution1D(*itd,var,h,u,histParams);
    hSigs.push_back(h);
    if( u ) delete u;
    legEntriesSigs.push_back(dataSetLegEntry(h,*itd));
    if( simulationOnlyPlot ) {
      if( !( (*itd)->isSimulated() ) ) {
	simulationOnlyPlot = false;
      }
    }
  }


  //// Create the ratio plot (data/bkg)
  // ratio frame
  TH1 *hRatioFrame = static_cast<TH1*>(hData->Clone("RatioFrame"));
  for(int xBin = 1; xBin <= hRatioFrame->GetNbinsX(); ++xBin) {
    hRatioFrame->SetBinContent(xBin,1.);
  }
  hRatioFrame->GetXaxis()->SetTitle(hData->GetXaxis()->GetTitle());
  hRatioFrame->GetYaxis()->SetRangeUser(0.55,1.95);
  hRatioFrame->SetLineStyle(2);
  hRatioFrame->SetLineColor(kBlack);
  hRatioFrame->SetLineWidth(2);
  hRatioFrame->SetFillColor(0);
  hRatioFrame->GetYaxis()->SetNdivisions(205);
  hRatioFrame->GetYaxis()->SetTickLength(gStyle->GetTickLength("Y")/0.2);
  hRatioFrame->GetXaxis()->SetLabelSize(gStyle->GetLabelSize("X"));
  hRatioFrame->GetYaxis()->CenterTitle();
  hRatioFrame->GetYaxis()->SetTitleOffset(0.9*hRatioFrame->GetYaxis()->GetTitleOffset());

  // Adapt the top histograms
  hData->GetXaxis()->SetLabelSize(0);
  hData->GetXaxis()->SetTitle("");
  hData->GetYaxis()->SetTickLength(gStyle->GetTickLength("Y")/0.8);
  for(std::vector<TH1*>::iterator it = stack.begin();
      it != stack.end(); ++it) {
    (*it)->GetXaxis()->SetLabelSize(0);
    (*it)->GetXaxis()->SetTitle("");
    (*it)->GetYaxis()->SetTickLength(gStyle->GetTickLength("Y")/0.8);
  }
  for(std::vector<TH1*>::iterator it = hSigs.begin();
      it != hSigs.end(); ++it) {
    (*it)->GetXaxis()->SetLabelSize(0);
    (*it)->GetXaxis()->SetTitle("");
    (*it)->GetYaxis()->SetTickLength(gStyle->GetTickLength("Y")/0.8);
  }

  // Fill the ratio plot
  TH1 *hRatio = static_cast<TH1*>(hData->Clone("Ratio"));
  hRatio->Divide(stack.front());
  hRatioFrame->GetYaxis()->SetTitle("#frac{"+Style::tlatexType(data)+"}{"+Style::tlatexType(bkgs.front())+"}");

  // Create the error band
  TGraphAsymmErrors* uncRatio = 0;
  if( uncBkg ) {
    uncRatio = static_cast<TGraphAsymmErrors*>(uncBkg->Clone());
    for(unsigned int i = 0; i < uncRatio->GetN(); ++i) {
      double denom  = uncRatio->GetY()[i];
      if( denom > 0. ) {
	uncRatio->GetY()[i] = 1.;
	uncRatio->GetEYlow()[i] = uncRatio->GetEYlow()[i]/denom;
	uncRatio->GetEYhigh()[i] = uncRatio->GetEYhigh()[i]/denom;
      }
    }
  }


  //// Legend and labels
  TLegend* leg = legend(1+stack.size()+hSigs.size());
  TPaveText* title = header(simulationOnlyPlot,(data->type()==DataSet::Data ? true : false),Style::tlatexLabel(data->selectionUid()));	// show lumi only if data plot is of type DataSet::Data
  leg->AddEntry(hData,legEntryData,"P");
  std::vector<TString>::const_iterator itL = legEntriesBkgs.begin();
  for(std::vector<TH1*>::const_iterator itH = stack.begin();
      itH != stack.end(); ++itH,++itL) {
    leg->AddEntry(*itH,*itL,"F");
  }
  itL = legEntriesSigs.begin();
  for(std::vector<TH1*>::const_iterator itH = hSigs.begin();
      itH != hSigs.end(); ++itH,++itL) {
    leg->AddEntry(*itH,*itL,"L");
  }


  //// Canvases and pads
  TCanvas* can = new TCanvas("can","",canSize_,canSize_);
  can->SetBottomMargin(0.2 + 0.8*can->GetBottomMargin()-0.2*can->GetTopMargin());
  can->cd();
  TPad *ratioPad = new TPad("bpad","",0,0,1,1);
  ratioPad->SetTopMargin(0.8 - 0.8*ratioPad->GetBottomMargin()+0.2*ratioPad->GetTopMargin());
  ratioPad->SetFillStyle(0);
  ratioPad->SetFrameFillColor(10);
  ratioPad->SetFrameBorderMode(0);


  //// Draw everything
  can->cd();
  setYRange(stack.front(),histParams.logy()?3E-1:-1.);
  stack.front()->Draw("HISTE");
  for(std::vector<TH1*>::iterator it = stack.begin()+1;
      it != stack.end(); ++it) {
    (*it)->Draw("HISTsame");
  }
  // Plot bkg uncertainty band
  if( uncBkg ) uncBkg->Draw("E2same");
  // Plot signals
  for(std::vector<TH1*>::iterator it = hSigs.begin();
      it != hSigs.end(); ++it) {
    (*it)->Draw("HISTsame");
  }
  // Plot data
  hData->Draw("PEsame");
  gPad->RedrawAxis();
  // Plot ratio plot
  ratioPad->Draw();
  ratioPad->cd();
  hRatioFrame->Draw("HIST");
  if( uncRatio ) {
    uncRatio->Draw("E2same");
  }
  hRatio->Draw("PEsame");
  title->Draw("same");
  leg->Draw("same");
  gPad->RedrawAxis();
  if( histParams.logx() ) can->SetLogx();
  if( histParams.logy() ) can->SetLogy();
  storeCanvas(can,var,data,bkgs);


  //// Clean up
  delete hData;
  for(std::vector<TH1*>::iterator it = stack.begin();
      it != stack.end(); ++it) {
    delete *it;
  }
  for(std::vector<TH1*>::iterator it = hSigs.begin();
      it != hSigs.end(); ++it) {
    delete *it;
  }
  if( uncBkg ) delete uncBkg;
  if( uncRatio ) delete uncRatio;
  delete leg;
  delete title;
  delete can;
  delete hRatio;
  delete hRatioFrame;
}


// Fill distribution of variable 'var' for dataSet with label 'dataSetLabel'
// and an uncertainty band 'uncert'. 
// ----------------------------------------------------------------------------
void PlotBuilder::createDistribution1D(const DataSet *dataSet, const TString &var, TH1* &h, TGraphAsymmErrors* &uncert, const HistParams &histParams) const {
  ++PlotBuilder::count_;
  
  // Create histogram  
  TString name = "plot";
  name += count_;
  h = new TH1D(name,"",histParams.nBinsX(),histParams.xMin(),histParams.xMax());
  h->Sumw2();
  if( histParams.xMax() > 1000. ) {
    h->GetXaxis()->SetNdivisions(505);
  }
  setXTitle(h,var);
  setYTitle(h,var);
  setGenericStyle(h,dataSet);

  // Create temporary histograms to store uncertainties
  TH1* hDn = static_cast<TH1*>(h->Clone(name+"Dn"));
  TH1* hUp = static_cast<TH1*>(h->Clone(name+"Up"));

  // Fill distributions
  for(EventIt itd = dataSet->evtsBegin(); itd != dataSet->evtsEnd(); ++itd) {
    double v = (*itd)->get(var);
    h->Fill(v,(*itd)->weight());
    if( (*itd)->hasUnc() ) {
      hDn->Fill(v,(*itd)->weightUncDn());
      hUp->Fill(v,(*itd)->weightUncUp());
    }
  }

  // Fill overflow bin
  if( histParams.hasOverflowBin() ) {
    double val = h->GetBinContent(h->GetNbinsX()) + h->GetBinContent(h->GetNbinsX()+1);
    double err = sqrt( h->GetBinError(h->GetNbinsX())*h->GetBinError(h->GetNbinsX()) + h->GetBinError(h->GetNbinsX()+1)*h->GetBinError(h->GetNbinsX()+1) );
    h->SetBinContent(h->GetNbinsX(),val);
    h->SetBinError(h->GetNbinsX(),err);

    if( hDn->GetEntries() ) {
      val = hDn->GetBinContent(hDn->GetNbinsX()) + hDn->GetBinContent(hDn->GetNbinsX()+1);
      err = sqrt( hDn->GetBinError(hDn->GetNbinsX())*hDn->GetBinError(hDn->GetNbinsX()) + hDn->GetBinError(hDn->GetNbinsX()+1)*hDn->GetBinError(hDn->GetNbinsX()+1) );
      hDn->SetBinContent(hDn->GetNbinsX(),val);
      hDn->SetBinError(hDn->GetNbinsX(),err);
    }
    if( hUp->GetEntries() ) {
      val = hUp->GetBinContent(hUp->GetNbinsX()) + hUp->GetBinContent(hUp->GetNbinsX()+1);
      err = sqrt( hUp->GetBinError(hUp->GetNbinsX())*hUp->GetBinError(hUp->GetNbinsX()) + hUp->GetBinError(hUp->GetNbinsX()+1)*hUp->GetBinError(hUp->GetNbinsX()+1) );
      hUp->SetBinContent(hUp->GetNbinsX(),val);
      hUp->SetBinError(hUp->GetNbinsX(),err);
    }
  }

  // Create uncertainty band
  if( hDn->GetEntries() && hUp->GetEntries() ) {
    std::vector<double> x(h->GetNbinsX());
    std::vector<double> xe(h->GetNbinsX());
    std::vector<double> y(h->GetNbinsX());
    std::vector<double> yed(h->GetNbinsX());
    std::vector<double> yeu(h->GetNbinsX());
    for(unsigned int i = 0; i < x.size(); ++i) {
      int bin = i+1;
      x.at(i) = h->GetBinCenter(bin);
      xe.at(i) = h->GetBinWidth(bin)/2.;
      y.at(i) = h->GetBinContent(bin);
      yed.at(i) = std::abs(h->GetBinContent(bin)-hDn->GetBinContent(bin));
      yeu.at(i) = std::abs(h->GetBinContent(bin)-hUp->GetBinContent(bin));
    }
    uncert = new TGraphAsymmErrors(x.size(),&(x.front()),&(y.front()),&(xe.front()),&(xe.front()),&(yed.front()),&(yeu.front()));
    uncert->SetMarkerStyle(1);
    uncert->SetMarkerColor(kBlue+2);
    uncert->SetFillColor(uncert->GetMarkerColor());
    uncert->SetLineColor(uncert->GetMarkerColor());
    uncert->SetFillStyle(3004);
  }

  // Delete temporary hists
  delete hDn;
  delete hUp;
}


// Fill distribution of variable 'var2' vs 'var1' for dataSet with label
// 'dataSetLabel'.
// ----------------------------------------------------------------------------
void PlotBuilder::createDistribution2D(const DataSet *dataSet, const TString &var1, const TString &var2, TH2* &h, const HistParams &histParams) const {
  ++PlotBuilder::count_;
  
  // Create histogram  
  TString name = "plot";
  name += count_;
  h = new TH2D(name,"",histParams.nBinsX(),histParams.xMin(),histParams.xMax(),histParams.nBinsY(),histParams.yMin(),histParams.yMax());
  h->Sumw2();
  if( histParams.xMax() > 1000. ) {
    h->GetXaxis()->SetNdivisions(505);
  }
  if( histParams.yMax() > 1000. ) {
    h->GetYaxis()->SetNdivisions(505);
  }
  setXTitle(h,var1);
  setYTitle(h,var2);

  // Fill distribution
  for(EventIt itd = dataSet->evtsBegin(); itd != dataSet->evtsEnd(); ++itd) {
    h->Fill((*itd)->get(var1),(*itd)->get(var2),(*itd)->weight());
  }
}


// Fill distribution of 'var1'/'var2' for dataSet with label 'dataSetLabel'
// and an uncertainty band 'uncert'. 
// ----------------------------------------------------------------------------
void PlotBuilder::createDistributionRatio(const DataSet *dataSet, const TString &var1, const TString &var2, TH1* &h, TGraphAsymmErrors* &uncert, const HistParams &histParams) const {
  ++PlotBuilder::count_;
  
  // Create histogram  
  TString name = "plot";
  name += count_;
  h = new TH1D(name,"",histParams.nBinsX(),histParams.xMin(),histParams.xMax());
  h->Sumw2();
  if( histParams.xMax() > 1000. ) {
    h->GetXaxis()->SetNdivisions(505);
  }
  setXTitle(h,var1,var2);
  setYTitle(h,"");
  setGenericStyle(h,dataSet);

  // Create temporary histograms to store uncertainties
  TH1* hDn = static_cast<TH1*>(h->Clone(name+"Dn"));
  TH1* hUp = static_cast<TH1*>(h->Clone(name+"Up"));

  // Fill distributions
  for(EventIt itd = dataSet->evtsBegin(); itd != dataSet->evtsEnd(); ++itd) {
    double v1 = (*itd)->get(var1);
    double v2 = (*itd)->get(var2);
    if( v2 > 0. ) v1 /= v2;
    h->Fill(v1,(*itd)->weight());
    if( (*itd)->hasUnc() ) {
      hDn->Fill(v1,(*itd)->weightUncDn());
      hUp->Fill(v1,(*itd)->weightUncUp());
    }
  }

  // Fill overflow bin
  if( histParams.hasOverflowBin() ) {
    double val = h->GetBinContent(h->GetNbinsX()) + h->GetBinContent(h->GetNbinsX()+1);
    double err = sqrt( h->GetBinError(h->GetNbinsX())*h->GetBinError(h->GetNbinsX()) + h->GetBinError(h->GetNbinsX()+1)*h->GetBinError(h->GetNbinsX()+1) );
    h->SetBinContent(h->GetNbinsX(),val);
    h->SetBinError(h->GetNbinsX(),err);

    if( hDn->GetEntries() ) {
      val = hDn->GetBinContent(hDn->GetNbinsX()) + hDn->GetBinContent(hDn->GetNbinsX()+1);
      err = sqrt( hDn->GetBinError(hDn->GetNbinsX())*hDn->GetBinError(hDn->GetNbinsX()) + hDn->GetBinError(hDn->GetNbinsX()+1)*hDn->GetBinError(hDn->GetNbinsX()+1) );
      hDn->SetBinContent(hDn->GetNbinsX(),val);
      hDn->SetBinError(hDn->GetNbinsX(),err);
    }
    if( hUp->GetEntries() ) {
      val = hUp->GetBinContent(hUp->GetNbinsX()) + hUp->GetBinContent(hUp->GetNbinsX()+1);
      err = sqrt( hUp->GetBinError(hUp->GetNbinsX())*hUp->GetBinError(hUp->GetNbinsX()) + hUp->GetBinError(hUp->GetNbinsX()+1)*hUp->GetBinError(hUp->GetNbinsX()+1) );
      hUp->SetBinContent(hUp->GetNbinsX(),val);
      hUp->SetBinError(hUp->GetNbinsX(),err);
    }
  }

  // Create uncertainty band
  if( hDn->GetEntries() && hUp->GetEntries() ) {
    std::vector<double> x(h->GetNbinsX());
    std::vector<double> xe(h->GetNbinsX());
    std::vector<double> y(h->GetNbinsX());
    std::vector<double> yed(h->GetNbinsX());
    std::vector<double> yeu(h->GetNbinsX());
    for(unsigned int i = 0; i < x.size(); ++i) {
      int bin = i+1;
      x.at(i) = h->GetBinCenter(bin);
      xe.at(i) = h->GetBinWidth(bin)/2.;
      y.at(i) = h->GetBinContent(bin);
      yed.at(i) = std::abs(h->GetBinContent(bin)-hDn->GetBinContent(bin));
      yeu.at(i) = std::abs(h->GetBinContent(bin)-hUp->GetBinContent(bin));
    }
    uncert = new TGraphAsymmErrors(x.size(),&(x.front()),&(y.front()),&(xe.front()),&(xe.front()),&(yed.front()),&(yeu.front()));
    uncert->SetMarkerStyle(1);
    uncert->SetMarkerColor(kBlue+2);
    uncert->SetFillColor(uncert->GetMarkerColor());
    uncert->SetLineColor(uncert->GetMarkerColor());
    uncert->SetFillStyle(3004);
  }

  // Delete temporary hists
  delete hDn;
  delete hUp;
}



void PlotBuilder::createStack1D(const DataSets &dataSets, const TString &var, std::vector<TH1*> &hists, std::vector<TString> &legEntries, TGraphAsymmErrors* &uncert, const HistParams &histParams) const {
  uncert = 0;
  for(DataSetRIt itd = dataSets.rbegin();
      itd != dataSets.rend(); ++itd) {
    TH1* h = 0;
    TGraphAsymmErrors* u = 0;
    createDistribution1D(*itd,var,h,u,histParams);
    if( u ) {
      if( uncert ) {		// Add uncertainties in quadrature
	for(unsigned int i = 0; i < uncert->GetN(); ++i) {
	  double y1  = uncert->GetY()[i];
	  double ed1 = uncert->GetEYlow()[i];
	  double eu1 = uncert->GetEYhigh()[i];
	  double y2  = u->GetY()[i];
	  double ed2 = u->GetEYlow()[i];
	  double eu2 = u->GetEYhigh()[i];
	  uncert->GetY()[i] = y1+y2;
 	  uncert->SetPointEYlow(i,sqrt(ed1*ed1+ed2*ed2));
 	  uncert->SetPointEYhigh(i,sqrt(eu1*eu1+eu2*eu2));
	}
      } else {
	uncert = u;
      }
    }
    setGenericStyle(h,*itd);
    legEntries.push_back(dataSetLegEntry(h,*itd));
    
    // Add histogram to all previous histograms
    for(std::vector<TH1*>::iterator itH = hists.begin();
	itH != hists.end(); ++itH) {
      (*itH)->Add(h);
    }
    // Add this histogram as new entry
    hists.push_back(h);
  }
}


void PlotBuilder::storeCanvas(TCanvas* can, const TString &var, const DataSet* dataSet) const {
    out_.addPlot(can,var,dataSet->label(),dataSet->selectionUid());
  }

void PlotBuilder::storeCanvas(TCanvas* can, const TString &var, const TString &selection) const {
    out_.addPlot(can,var,selection);
  }

void PlotBuilder::storeCanvas(TCanvas* can, const TString &var, const DataSets &dataSets) const {
    std::vector<TString> dataSetLabels;
    for(DataSetIt itd = dataSets.begin(); itd != dataSets.end(); ++itd) {
      dataSetLabels.push_back((*itd)->label());
    }
    out_.addPlot(can,var,dataSetLabels,dataSets.front()->selectionUid());
  }

void PlotBuilder::storeCanvas(TCanvas* can, const TString &var, const DataSet *dataSet, const DataSets &dataSets) const {
    std::vector<TString> dataSetLabels1;
    dataSetLabels1.push_back(dataSet->label());
    std::vector<TString> dataSetLabels2;
    for(DataSetIt itd = dataSets.begin(); itd != dataSets.end(); ++itd) {
      dataSetLabels2.push_back((*itd)->label());
    }
    out_.addPlot(can,var,dataSetLabels1,dataSetLabels2,dataSet->selectionUid());
  }


void PlotBuilder::setXTitle(TH1* h, const TString &var) const {
  TString xTitle = Variable::label(var);
  if( Variable::unit(var) != "" ) {
    xTitle += " ["+Variable::unit(var)+"]";
  }
  h->GetXaxis()->SetTitle(xTitle);
}

void PlotBuilder::setXTitle(TH1* h, const TString &var1, const TString &var2) const {
  TString xTitle = Variable::label(var1) + " / " + Variable::label(var2);
  if( Variable::unit(var1) != "" || Variable::unit(var2) != "" ) {
    if( Variable::unit(var1) != "" ) {
      xTitle += " ["+Variable::unit(var1)+"]";
    } else {
      xTitle += " 1";
    }
    if( Variable::unit(var2) != "" ) {
      xTitle += " / ["+Variable::unit(var2)+"]";
    }
  }
  h->GetXaxis()->SetTitle(xTitle);
}

void PlotBuilder::setYTitle(TH1* h, const TString &var) const {
  TString yTitle = "Events";
  TString className = h->ClassName();
  if( className.Contains("TH2") ) {
    yTitle = Variable::label(var);
    if( Variable::unit(var) != "" ) {
      yTitle += " ["+Variable::unit(var)+"]";
    }
  } else {
    if( h->GetBinWidth(1) != 1. || Variable::unit(var) != "" ) {
      yTitle += " / ";
      if( h->GetBinWidth(1) != 1. ) {
	TString bw = "";
	bw += h->GetBinWidth(1);
	if( bw.Contains(".") && bw.Length() > 5 ) bw = bw(0,5);
	yTitle += bw;
      }
      if( Variable::unit(var) != "" ) {
	yTitle += " "+Variable::unit(var);
      }
    }
  }
  h->GetYaxis()->SetTitle(yTitle);
}


int PlotBuilder::color(const DataSet *dataSet) const {
  // Userdefined color
  int color = Style::color(dataSet);
  if( color < 0 ) {    // Default colors
    if( dataSet->type() == DataSet::Data ) {
      color = kBlack;
    } else {
      color = kGray;
    }
  }

  return color;
}


int PlotBuilder::markerStyle(const DataSet *dataSet) const {
  // Userdefined style
  int style = Style::markerStyle(dataSet);
  if( style < 0 ) {
    // Default styles
    if( dataSet->type() == DataSet::Data ) {
      style = 21;
    }
  }

  return style;
}


void PlotBuilder::setMarkerStyle(TH1* h, const DataSet *dataSet) const {
  h->SetMarkerStyle(markerStyle(dataSet));
  h->SetMarkerColor(color(dataSet));
  h->SetLineColor(h->GetMarkerColor());
}


void PlotBuilder::setGenericStyle(TH1* h, const DataSet *dataSet) const {
  if( dataSet->type() == DataSet::Data ) {
    setMarkerStyle(h,dataSet);
  } else if( dataSet->type() == DataSet::MC || dataSet->type() == DataSet::Prediction || dataSet->type() == DataSet::MCPrediction ) {
    int c = color(dataSet);
    h->SetMarkerStyle(0);
    h->SetMarkerColor(c);
    h->SetLineColor(c);
    h->SetFillColor(c);
  } else if( dataSet->type() == DataSet::Signal ) {
    int c = color(dataSet);
    h->SetMarkerStyle(0);
    h->SetMarkerColor(c);
    h->SetLineColor(c);
    h->SetLineWidth(3);
  }
}


TPaveText* PlotBuilder::header(const DataSet* ds, bool showLumi, const TString &info) const {
  return header(ds->isSimulated(),showLumi,info);
}

TPaveText* PlotBuilder::header(const DataSets &ds, bool isSimulation, bool showLumi, const TString &info) const {
  bool simulationOnly = true;
  for(DataSetIt itd = ds.begin(); itd != ds.end(); ++itd) {
    if( !( (*itd)->isSimulated() ) ) {
      simulationOnly = false;
      break;
    }
  }
  
  return header(simulationOnly,showLumi,info);
}

TPaveText* PlotBuilder::header(bool isSimulation, bool showLumi, const TString &info) const {
  TString line = "";
  double txtSize = 0.05;
  if( GlobalParameters::publicationStatus() == GlobalParameters::Preliminary ) {
    if( isSimulation ) {
      line = "CMS Simulation";
    } else {
      line = "CMS Preliminary,  "+lumiLabel();
    }
    txtSize = 0.038;
  } else if( GlobalParameters::publicationStatus() == GlobalParameters::Public ) {
    if( isSimulation ) {
      line = "CMS Simulation";
    } else {
      line = "CMS,  "+lumiLabel();
    }
    txtSize = 0.04;
  } else {
    if( isSimulation ) {
      line = "Simulation";
    }
    else if( showLumi ) {
      if( line.Length() ) line += ",  ";
      line += lumiLabel();
    }
  }
  if( line.Length() ) line += ",  ";
  line += "8 TeV";
  if( info.Length() ) line += ",  "+info;

  double x0 = gStyle->GetPadLeftMargin();
  double x1 = 1.-gStyle->GetPadRightMargin();
  double y0 = 1.-gStyle->GetPadTopMargin();
  double y1 = 1.;
  TPaveText *txt = new TPaveText(x0,y0,x1,y1,"NDC");
  txt->SetBorderSize(0);
  txt->SetFillColor(0);
  txt->SetTextFont(42);
  txt->SetTextAlign(12);
  txt->SetTextSize(txtSize);
  txt->SetMargin(0.);
  txt->AddText(line);
  
  return txt;
}


TLegend* PlotBuilder::legend(unsigned int nEntries) const {
  double lineHeight = 0.06;
  double margin = 0.02;
  double x0 = 0.6;
  if( Style::plotYields() ) {
    x0 = 0.45;
  }
  double x1 = 1.-gStyle->GetPadRightMargin()-margin;
  double y1 = 1.-gStyle->GetPadTopMargin()-margin;
  double y0 = y1-nEntries*lineHeight;

  TLegend* leg = new TLegend(x0,y0,x1,y1);
  leg->SetBorderSize(0);
  leg->SetFillColor(0);
  leg->SetFillStyle(0);
  leg->SetTextFont(42);
  leg->SetTextSize(0.04);

  return leg;
}


TString PlotBuilder::lumiLabel() const {
  return GlobalParameters::lumi()+" fb^{-1}";
}


TString PlotBuilder::dataSetLegEntry(const TH1* h, const DataSet* ds) const {
  char entry[100];
  TString label = Style::tlatexLabel(ds).Data();
  if( Style::plotYields() ) {
    double entries = h->Integral(1,h->GetNbinsX());
    if( ds->type() == DataSet::Data ) {
      sprintf(entry," %s (%.0lf)",label.Data(),entries);
    } else {
      sprintf(entry," %s (%.1lf)",label.Data(),entries);
    }
  } else {
    sprintf(entry," %s",label.Data());
  }

  return entry;
}


void PlotBuilder::setYRange(TH1* &h, double logMin) const {
  bool log = logMin > 0.;
  double max = h->GetBinContent(h->GetMaximumBin());
  double min = 0.;
  if( log ) min = logMin;

  double relHistHeight = 1. - (gStyle->GetPadTopMargin() + gStyle->GetPadBottomMargin() );
  if( log ) {
    max = min * std::pow(max/min,0.9/relHistHeight);
  } else {
    relHistHeight -= 0.3;
    max = (max-min)/relHistHeight + min;
  }
  h->GetYaxis()->SetRangeUser(min,max);
}


bool PlotBuilder::checkForUnderOverFlow(const TH1* h, const TString &var, const DataSet *dataSet) const {
  bool underOverFlow = true;
  if( h->GetBinContent(0) > 0 ) {
    std::cerr << "      WARNING: Underflow in " << var << " distribution in dataset " << dataSet->label() << std::endl;
  } else if( h->GetBinContent(h->GetNbinsX()+1) > 0 ) {
    std::cout << "      NOTE: Overflow in " << var << " distribution in dataset " << dataSet->label() << " (last plotted bin contains overflows)" << std::endl;
  } else {
    underOverFlow = false;
  }

  return underOverFlow;
}



PlotBuilder::HistParams::HistParams(const TString &cfg)
  : nBinsX_(1), xMin_(0), xMax_(1), nBinsY_(0), yMin_(0), yMax_(1), logx_(false), logy_(false), logz_(false), norm_(false), hasOverflowBin_(true) {

  // Parse to overwrite defaults
  std::vector<TString> cfgs;
  if( Config::split(cfg,",",cfgs) ) {
    // First, find number of binning commands
    unsigned int nBinCfgs = 0;
    for(; nBinCfgs < cfgs.size(); ++nBinCfgs) { 
      if( !cfgs.at(nBinCfgs).IsFloat() ) {
	nBinCfgs - 1;
	break;
      }
    }
    // Then, parse binning
    for(unsigned int i = 0; i < nBinCfgs; ++i) {
      if( i == 0 ) nBinsX_ = cfgs.at(i).Atoi();
      else if( i == 1 ) xMin_ = cfgs.at(i).Atof();
      else if( i == 2 ) xMax_ = cfgs.at(i).Atof();
      if( nBinCfgs == 5 ) {
	if( i == 3 ) yMin_ = cfgs.at(i).Atoi();
	else if( i == 4 ) yMax_ = cfgs.at(i).Atof();
      } else if( nBinCfgs > 5 ) {
	if( i == 3 ) nBinsY_ = cfgs.at(i).Atoi();
	else if( i == 4 ) yMin_ = cfgs.at(i).Atof();
	else if( i == 5 ) yMax_ = cfgs.at(i).Atof();
      }
    }
    // Then, parse style commands
    for(unsigned int i = nBinCfgs; i < cfgs.size(); ++i) {
      if( cfgs.at(i) == "logx" ) logx_ = true;
      else if( cfgs.at(i) == "logy" ) logy_ = true;
      else if( cfgs.at(i) == "logz" ) logz_ = true;
      else if( cfgs.at(i) == "norm" ) norm_ = true;
      else if( cfgs.at(i) == "log" ) {
	if( nBinCfgs > 5 ) logz_ = true;
	else logy_ = true;
      }
    }
  }

  // Overflow bin
  double binWidth = (xMax_-xMin_)/nBinsX_;
  xMax_ += binWidth;
  nBinsX_ += 1;
}
