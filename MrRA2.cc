// $Id: MrRA2.cc,v 1.17 2013/05/21 16:35:25 mschrode Exp $

#include <cstdlib>
#include <iomanip>
#include <iostream>

#include "DataSet.h"
#include "EventInfoPrinter.h"
#include "GlobalParameters.h"
#include "MrRA2.h"
#include "EventYieldPrinter.h"
#include "Output.h"
#include "PlotBuilder.h"
#include "Selection.h"
#include "Style.h"
#include "Variable.h"



int main(int argc, char *argv[]) {
  if( argc > 1 ) {
    MrRA2* mr = new MrRA2(argv[1]);
    delete mr;
  } else {
    std::cerr << "\n\n  ERROR: Missing configuration file" << std::endl;
    std::cerr << "  Usage './run config-file-name\n" << std::endl;
  }

  return 0;
}


MrRA2::MrRA2(const TString& configFileName) {
  std::cout << "\n +------------------------------------------------+" << std::endl;
  std::cout << " |                                                |" << std::endl;
  std::cout << " |     MrRA2 - the Really Awesome plotting 2l     |" << std::endl;
  std::cout << " |                                                |" << std::endl;
  std::cout << " +------------------------------------------------+\n" << std::endl;
  if( GlobalParameters::cvsTag() == "" ) {
    std::cout << "Developer's version" << std::endl;
  } else {
    std::cout << "Version " << GlobalParameters::cvsTag() << std::endl;
  }
  std::cout << "\n" << std::endl;

  // Initialization
  std::cout << "Initializing MrRA2" << std::endl;
  Config cfg(configFileName);
  checkForLatestSyntax(cfg);
  GlobalParameters::init(cfg,"global");
  Style::init(cfg,"style");
  Variable::init(cfg,"variable");
  Selection::init(cfg,"selection");
  DataSet::init(cfg,"dataset");
  std::cout << "\n\n\n";
  

  // Print setup info
  std::cout << "The following datasets are defined:" << std::endl;
  DataSets inputDataSets = DataSet::findAllUnselected();
  for(DataSetIt itd = inputDataSets.begin(); itd != inputDataSets.end(); ++itd) {
    std::cout << "  " << (*itd)->label() << " (type '" << DataSet::toString((*itd)->type()) << "'): " << (*itd)->size() << " entries" << std::endl;
  }

  std::cout << "\nThe following selections are defined:" << std::endl;
  for(SelectionIt its = Selection::begin(); its != Selection::end(); ++its) {
    if( (*its)->uid() != "unselected" ) (*its)->print();
  }
  std::cout << "\n";


  // Print simple cut flow
  std::cout << "The following number of events (entries) are selected:" << std::endl;
  for(DataSetIt itd = inputDataSets.begin(); itd != inputDataSets.end(); ++itd) {
    std::cout << "  " << std::setw(Selection::maxLabelLength()) << (*itd)->label() << " (" << DataSet::toString((*itd)->type()) << ") : " << std::setw(15) << (*itd)->yield() << " (" << (*itd)->size() << ")" << std::endl;
    DataSets selectedDataSets = DataSet::findAllWithLabel((*itd)->label());
    for(DataSetIt itsd = selectedDataSets.begin(); itsd != selectedDataSets.end(); ++itsd) {
      std::cout << "    " << std::setw(Selection::maxLabelLength()) << (*itsd)->selectionUid() << " : " << std::setw(15) << (*itsd)->yield() << " (" << (*itsd)->size() << ")" << std::endl;
    }
  }

  // Control the output
  Output out;

  // Control plots without selection
  std::cout << "\n\n\nProcessing the output" << std::endl;
  PlotBuilder(cfg,out);
  EventInfoPrinter evtInfoPrinter(cfg);
  EventYieldPrinter evtYieldPrinter;

  std::cout << "Done.\nThank you for using MrRA2! Want to donate money? Contact M. Schroeder." << std::endl;
}

MrRA2::~MrRA2() {
  DataSet::clear();
  Selection::clear();
}


void MrRA2::checkForLatestSyntax(const Config &cfg) const {
  // 2013.05.09: New syntax for plots
  std::vector<Config::Attributes> attrList = cfg("plot");
  for(std::vector<Config::Attributes>::const_iterator it = attrList.begin();
      it != attrList.end(); ++it) {

    if( it->hasName("datasets") || ( it->hasName("dataset1") && it->hasName("dataset2") ) ) {
      std::cerr << "\n\n\nDear user (Hallo Arne...)!\n" << std::endl;
      std::cerr << "With the current version of MrRA2, the config-syntax has been changed" << std::endl;
      std::cerr << "towards greater readability and more convenience! The old syntax you" << std::endl;
      std::cerr << "are using in your config-file is not supported anymore. Please change" << std::endl;
      std::cerr << "it as follows:\n" << std::endl;
      std::cerr << "1) 'Data vs Bkg' plots" << std::endl;
      std::cerr << "   old syntax: 'plot :: variable: <var>; dataset1: <name[+name+...]>; dataset2: <name[+name+...]>; [signal: <name[,name,...]>;] histogram:...' " << std::endl;
      std::cerr << "   new syntax: 'plot :: variable: <var>; data: <name>; background: <name[+name+...]>; [signal: <name[,name,...]>;] histogram:...' " << std::endl;
      std::cerr << "                                          -^-    -^-       -^-" << std::endl;
      std::cerr << "" << std::endl;
      std::cerr << "2) 'Comparison of spectra' plots" << std::endl;
      std::cerr << "   old syntax: 'plot :: variable: <var>; datasets: <name[,name,...]>; histogram:...' " << std::endl;
      std::cerr << "   new syntax: 'plot :: variable: <var>; dataset: <name[,name,...]>; histogram:...' " << std::endl;
      std::cerr << "                                               -^-" << std::endl;
      std::cerr << "" << std::endl;
      std::cerr << "You can find an example config file in config/example.txt" << std::endl;
      std::cerr << "" << std::endl;      
      std::cerr << "" << std::endl;
      std::cerr << "To repair your config file, you can probably (no guarantee!!) simply type:" << std::endl;
      std::cerr << "> sed -i 's/dataset1/data/g' <config-file>" << std::endl;
      std::cerr << "> sed -i 's/dataset2/background/g' <config-file>" << std::endl;
      std::cerr << "> sed -i 's/datasets/dataset/g' <config-file>" << std::endl;
      std::cerr << "\n\n" << std::endl;
      exit(-1);
    }
  }


  // 2013.05.20: New syntax for variables
  attrList = cfg("event content");
  if( !attrList.empty() ) {
    std::cerr << "With the current version of MrRA2, the config-syntax has been changed." << std::endl;
    std::cerr << "The old syntax you are using is not supported anymore. Please apply the" << std::endl;
    std::cerr << "following changes:" << std::endl;
    std::cerr << "\n 'event content' --> 'variable'" << std::endl;
    std::cerr << "" << std::endl;
    std::cerr << "You can find an example config file in config/example.txt" << std::endl;
    std::cerr << "" << std::endl;      
    std::cerr << "" << std::endl;
    std::cerr << "To repair your config file, you can probably (no guarantee!!) simply type:" << std::endl;
    std::cerr << "> sed -i 's/event content/variable/g' <config-file>" << std::endl;
    std::cerr << "\n\n" << std::endl;
    exit(-1);
  }

  // 2013.05.20: New syntax for file names
  attrList = cfg("dataset");
  for(std::vector<Config::Attributes>::const_iterator it = attrList.begin();
      it != attrList.end(); ++it) {
    if( it->hasName("file") ) {
      std::cerr << "With the current version of MrRA2, the config-syntax has been changed." << std::endl;
      std::cerr << "The old syntax you are using is not supported anymore. Please apply the" << std::endl;
      std::cerr << "following changes:" << std::endl;
      std::cerr << "\n in the 'dataset' section: 'file' --> 'files'" << std::endl;
      std::cerr << "" << std::endl;
      std::cerr << "You can find an example config file in config/example.txt" << std::endl;
      std::cerr << "" << std::endl;      
      std::cerr << "" << std::endl;
      std::cerr << "To repair your config file, you can probably (no guarantee!!) simply type:" << std::endl;
      std::cerr << "> sed -i '/dataset/ s=file=files=' <config-file>" << std::endl;
      std::cerr << "\n\n" << std::endl;
      exit(-1);
    }
  }
}
