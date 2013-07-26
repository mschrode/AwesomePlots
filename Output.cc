#include <iostream>
#include <sys/stat.h>


#include "Output.h"


Output::Output() {
}

void Output::addPlot(TCanvas* can, const TString &var, const TString &selection) {
  TString plotName = cleanName(GlobalParameters::analysisId()+"__"+var+"__"+selection);
  
  std::map< TString, std::vector<TString> >::iterator itVar = plotsNormedSpectra_.begin();
  if( itVar != plotsNormedSpectra_.end() ) {
    itVar->second.push_back(plotName);
  } else {
    std::vector<TString> v;
    v.push_back(plotName);
    plotsNormedSpectra_[var] = v;
  }

  storeCanvas(can,selection,plotName);
}


void Output::addPlot(TCanvas* can, const TString &var, const TString &dataSetLabel, const TString &selection) {

  TString dsLabel  = cleanName(dataSetLabel);
  TString plotName = cleanName(GlobalParameters::analysisId()+"__"+var+"__"+dsLabel+"__"+selection);

  std::map< TString, std::map< TString, std::vector<TString> > >::iterator itVar = plotsSingleSpectrum_.find(var);
  if( itVar != plotsSingleSpectrum_.end() ) {
    std::map< TString, std::vector<TString> >::iterator itDS = itVar->second.find(dsLabel);
    if( itDS != itVar->second.end() ) {
      itDS->second.push_back(plotName);
    } else {
      std::vector<TString> v;
      v.push_back(plotName);
      itVar->second[dsLabel] = v;
    }
  } else {
    std::vector<TString> v;
    v.push_back(plotName);
    std::map< TString, std::vector<TString> > m;
    m[dsLabel] = v;
    plotsSingleSpectrum_[var] = m;
  }

  storeCanvas(can,selection,plotName);
}

void Output::addPlot(TCanvas* can, const TString &var, const std::vector<TString> &dataSetLabels, const TString &selection) {
  TString label = dataSetLabels.front();
  for(std::vector<TString>::const_iterator it = dataSetLabels.begin()+1;
      it != dataSetLabels.end(); ++it) {
    label += "+"+*it;
  }
  TString dsLabel  = cleanName(label);
  TString plotName = cleanName(GlobalParameters::analysisId()+"__"+var+"__"+dsLabel+"__"+selection);

  std::map< TString, std::map< TString, std::vector<TString> > >::iterator itVar = plotsStack_.find(var);
  if( itVar != plotsStack_.end() ) {
    std::map< TString, std::vector<TString> >::iterator itDS = itVar->second.find(dsLabel);
    if( itDS != itVar->second.end() ) {
      itDS->second.push_back(plotName);
    } else {
      std::vector<TString> v;
      v.push_back(plotName);
      itVar->second[dsLabel] = v;
    }
  } else {
    std::vector<TString> v;
    v.push_back(plotName);
    std::map< TString, std::vector<TString> > m;
    m[dsLabel] = v;
    plotsStack_[var] = m;
  }

  storeCanvas(can,selection,plotName);
}


void Output::addPlot(TCanvas* can, const TString &var, const std::vector<TString> &dataSetLabels1, const std::vector<TString> &dataSetLabels2, const TString &selection) {
  TString label1 = dataSetLabels1.front();
  for(std::vector<TString>::const_iterator it = dataSetLabels1.begin()+1;
      it != dataSetLabels1.end(); ++it) {
    label1 += "+"+*it;
  }
  TString label2 = dataSetLabels2.front();
  for(std::vector<TString>::const_iterator it = dataSetLabels2.begin()+1;
      it != dataSetLabels2.end(); ++it) {
    label2 += "+"+*it;
  }
  TString dsLabel  = cleanName(label1+"_vs_"+label2);
  TString plotName = cleanName(GlobalParameters::analysisId()+"__"+var+"__"+dsLabel+"__"+selection);

  std::map< TString, std::map< TString, std::vector<TString> > >::iterator itVar = plotsStack_.find(var);
  if( itVar != plotsStack_.end() ) {
    std::map< TString, std::vector<TString> >::iterator itDS = itVar->second.find(dsLabel);
    if( itDS != itVar->second.end() ) {
      itDS->second.push_back(plotName);
    } else {
      std::vector<TString> v;
      v.push_back(plotName);
      itVar->second[dsLabel] = v;
    }
  } else {
    std::vector<TString> v;
    v.push_back(plotName);
    std::map< TString, std::vector<TString> > m;
    m[dsLabel] = v;
    plotsStack_[var] = m;
  }

  storeCanvas(can,selection,plotName);
}


void Output::storeCanvas(TCanvas* can, const TString &selection, const TString &plotName) {
  can->SetName(plotName);
  can->SetTitle(plotName);
  if( GlobalParameters::outputEPS() ) can->SaveAs(resultDir()+"/"+dir(selection)+"/"+plotName+".eps","eps");
  if( GlobalParameters::outputPDF() ) can->SaveAs(resultDir()+"/"+dir(selection)+"/"+plotName+".pdf");
  if( GlobalParameters::outputPNG() ) can->SaveAs(resultDir()+"/"+dir(selection)+"/"+plotName+".png");
}
 

void Output::createLaTeXSlide() const {
//   std::cout << "\n\nSingle Spectrum" << std::endl;
//   for(std::map< TString, std::map< TString, std::vector<TString> > >::const_iterator itVar = plotsSingleSpectrum_.begin(); itVar != plotsSingleSpectrum_.end(); ++itVar) {
//     std::cout << "  Variable " << itVar->first << std::endl;
//     for(std::map< TString, std::vector<TString> >::const_iterator itDS = itVar->second.begin(); itDS != itVar->second.end(); ++itDS) {
//       std::cout << "    Dataset " << itDS->first << std::endl;
//       for(std::vector<TString>::const_iterator itFN = itDS->second.begin(); itFN != itDS->second.end(); ++itFN) {
// 	std::cout << "      " << *itFN << std::endl;
//       }
//     }
//   }

//   std::cout << "\n\nStacks" << std::endl;
//   for(std::map< TString, std::map< TString, std::vector<TString> > >::const_iterator itVar = plotsStack_.begin(); itVar != plotsStack_.end(); ++itVar) {
//     std::cout << "  Variable " << itVar->first << std::endl;
//     for(std::map< TString, std::vector<TString> >::const_iterator itDS = itVar->second.begin(); itDS != itVar->second.end(); ++itDS) {
//       std::cout << "    Dataset " << itDS->first << std::endl;
//       for(std::vector<TString>::const_iterator itFN = itDS->second.begin(); itFN != itDS->second.end(); ++itFN) {
// 	std::cout << "      " << *itFN << std::endl;
//       }
//     }
//   }

//   std::cout << "\n\nNormalised Spectra" << std::endl;
//   for(std::map< TString, std::vector<TString> >::const_iterator itVar = plotsNormedSpectra_.begin(); itVar != plotsNormedSpectra_.end(); ++itVar) {
//     std::cout << "  Variable " << itVar->first << std::endl;
//     for(std::vector<TString>::const_iterator itFN = itVar->second.begin(); itFN != itVar->second.end(); ++itFN) {
//       std::cout << "    " << *itFN << std::endl;
//     }
//   }
}


TString Output::dir(const TString &selection) {
  std::map< TString, TString >::const_iterator it = dirs_.find(selection);
  if( it != dirs_.end() ) {
    return it->second;
  } else {
    TString dirName = cleanName(selection);
    TString path = resultDir()+"/"+dirName;
    mkdir(path.Data(),S_IRWXU);
    dirs_[selection] = dirName;
    return dirName;
  }
}


TString Output::cleanName(const TString &name) {
  TString cleanedName = name;
  cleanedName.ReplaceAll(">","gtr");
  cleanedName.ReplaceAll("<","lss");
  cleanedName.ReplaceAll(" ","_");
  cleanedName.ReplaceAll(":","-");
  cleanedName.ReplaceAll("_{","");
  cleanedName.ReplaceAll("#","");
  cleanedName.ReplaceAll("{","");
  cleanedName.ReplaceAll("}","");
  cleanedName.ReplaceAll("[","");
  cleanedName.ReplaceAll("]","");

  return cleanedName;
}


TString Output::cleanLatexName(const TString &name) {
  TString cleanedName = name;
  cleanedName.ReplaceAll("_","\\_");
  cleanedName.ReplaceAll("#","\\");

  return cleanedName;
}
