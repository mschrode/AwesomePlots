// $Id: EventInfoPrinter.cc,v 1.11 2013/05/09 20:14:55 mschrode Exp $

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "EventInfoPrinter.h"
#include "Output.h"
#include "Selection.h"
#include "Variable.h"


TString EventInfoPrinter::runSortVar_ = "";


EventInfoPrinter::EventInfoPrinter(const Config &cfg)
  : cfg_(cfg) {

  // Init and run
  if( init("print event info") ) {
    // Print setup
    std::cout << "  - Writing event-provenance information to " << outFileName_ << std::endl;
    std::cout << "     - Printing " << std::flush;
    if( printAllEvents() ) {
      std::cout << "all events " << std::endl;
    } else {
      for(std::map<TString,unsigned int>::const_iterator itSV = selectionVariables_.begin();
	  itSV != selectionVariables_.end(); ++itSV) {
	std::cout << "\n         the " << itSV->second << " events with highest " << itSV->first << std::flush;
      }
      std::cout << std::endl;
    }
    if( printedSelections_.size() > 0 ) {
      std::cout << "       for selections" << std::flush;
      for(std::set<TString>::const_iterator it = printedSelections_.begin(); it != printedSelections_.end(); ++it) {
	std::cout << "\n         " << *it << std::flush;
      }
      std::cout << std::endl;
    }
    std::cout << "  - Creating LaTeX slides for event displays in " << latexSlidesName_ << std::endl;

    selectEvents();
    print();
  }
}


bool EventInfoPrinter::init(const TString &key) {
  // Get parameters
  std::vector<Config::Attributes> attrList = cfg_(key);

  // Required event-provenance information
  varNameRunNum = "";
  varNameLumiBlockNum = "";
  varNameEvtNum = "";
  bool provVarsDefined = false;
  for(std::vector<Config::Attributes>::const_iterator it = attrList.begin();
      it != attrList.end(); ++it) {
    if( it->hasName("provenance variables") ) {
      std::vector<TString> vars;
      if( Config::split(it->value("provenance variables"),"+",vars) ) {
	if( vars.size() == 3 ) {
	  varNameRunNum = vars.at(0);
	  varNameLumiBlockNum = vars.at(1);
	  varNameEvtNum = vars.at(2);
	  if( Variable::exists(varNameRunNum) && 
	      Variable::exists(varNameLumiBlockNum) &&
	      Variable::exists(varNameEvtNum) ) {
	    provVarsDefined = true;
	    EventInfoPrinter::runSortVar_ = varNameRunNum;
	    break;
	  }
	} else {
	  std::cerr << "\n\nERROR in EventInfoPrinter::init(): wrong definition of event-provenance variables" << std::endl;
	  std::cerr << "  Please use the following syntax in the config file:" << std::endl;
	  std::cerr << "  'print event info :: provenance variables: <VarNameRunNum> + <VarNameLumiBlockNum> + <VarNameEvtNum>'" << std::endl;
	  exit(-1);
	}
      }
    }
  }


  // Optionally, read selections to be printed
  for(std::vector<Config::Attributes>::const_iterator it = attrList.begin();
      it != attrList.end(); ++it) {
    if( it->hasName("selections") ) {
      std::vector<TString> sels;
      if( Config::split(it->value("selections"),",",sels) ) {
	for(std::vector<TString>::const_iterator its = sels.begin();
	    its != sels.end(); ++its) {
	  printedSelections_.insert(*its);
	}
      }
    } else {
      std::cerr << "\n\nERROR in EventInfoPrinter::init(): wrong syntax" << std::endl;
      std::cerr << "  in line with key '" << key << "'" << std::endl;
      std::cerr << "  in config file '" << cfg_.fileName() << "'" << std::endl;
      exit(-1);
    }
  }


  // Optionally, print num events with highest value of a variable
  for(std::vector<Config::Attributes>::const_iterator it = attrList.begin();
      it != attrList.end(); ++it) {
    if( it->hasName("highest") ) {
      std::vector<TString> varNumPair;
      if( Config::split(it->value("highest"),",",varNumPair) ) {
	bool correctSyntax = false;
	if( varNumPair.size() == 2 ) {
	  TString variable = varNumPair.at(0);
	  int number = varNumPair.at(1).Atoi();
	  if( Variable::exists(variable) ) {
	    selectionVariables_[variable] = number;
	    correctSyntax = true;
	  }
	}
	if( !correctSyntax ) {
	  std::cerr << "\n\nERROR in EventInfoPrinter::init(): wrong syntax" << std::endl;
	  std::cerr << "  in line with key '" << key << "'" << std::endl;
	  std::cerr << "  in config file '" << cfg_.fileName() << "'" << std::endl;
	  exit(-1);
	}
      }
    }
  }

  // Define names of special variables
  // This should be configurable
  varNameNJets = ( Variable::exists("NJets") ? "NJets" : "" );
  varNameHT = ( Variable::exists("HT") ? "HT" : "" );
  varNameMHT = ( Variable::exists("MHT") ? "MHT" : "" );

  // Output file
  outFileName_ = Output::resultDir()+"/"+Output::id()+"_EventInfo.txt";
  latexSlidesName_ = Output::resultDir()+"/"+Output::id()+"_EventDisplays.tex";

  return provVarsDefined && attrList.size() > 1;
}


void EventInfoPrinter::selectEvents() {
  // Loop over all global selections
  // !!!!!!!!!!!!!This should be treated more carefully: one set of selectionVariables_
  // per global selection
  for(SelectionIt its = Selection::begin(); its != Selection::end(); ++its) {
    if( printSelection((*its)->uid()) ) {
      // Loop over all datasets with this global selection
      DataSets selectedDataSets = DataSet::findAllWithSelection((*its)->uid());
      for(DataSetIt itsd = selectedDataSets.begin(); itsd != selectedDataSets.end(); ++itsd) {
	// Select events accoridng to specification
	std::vector<const Event*> selectedEvts;
	if( printAllEvents() ) {	// select all events to print info
	  for(EventIt itEvt = (*itsd)->evtsBegin(); itEvt != (*itsd)->evtsEnd(); ++itEvt) {
	    selectedEvts.push_back(*itEvt);
	  }
	} else {			// select n (as specified) events with highest value of selection variable
	  for(std::map<TString,unsigned int>::const_iterator itSV = selectionVariables_.begin();
	      itSV != selectionVariables_.end(); ++itSV) {
	
	    std::vector<EvtValPair*> evtValPairs;
	    for(EventIt itEvt = (*itsd)->evtsBegin(); itEvt != (*itsd)->evtsEnd(); ++itEvt) {
	      evtValPairs.push_back(new EvtValPair(*itEvt,(*itEvt)->get(itSV->first)));
	    }
	    // sort by size of variable's values
	    std::sort(evtValPairs.begin(),evtValPairs.end(),EvtValPair::valueGreaterThan);
	    // store n (as specified) events with largest value
	    for(unsigned int n = 0; 
		n < std::min(itSV->second,static_cast<unsigned int>(evtValPairs.size())); ++n) {
	      bool isNewEvt = true;
	      for(std::vector<const Event*>::const_iterator it = selectedEvts.begin();
		  it != selectedEvts.end(); ++it) {
		if( (*it) == evtValPairs.at(n)->event() ) {
		  isNewEvt = false;
		  break;
		}
	      }
	      if( isNewEvt ) selectedEvts.push_back(evtValPairs.at(n)->event());
	    }
	    for(std::vector<EvtValPair*>::iterator it = evtValPairs.begin();
		it != evtValPairs.end(); ++it) {
	      delete *it;
	    }
	  }
	}
	// Sort by run
	std::sort(selectedEvts.begin(),selectedEvts.end(),EventInfoPrinter::greaterByRun);
	// Store list of events
	printedEvts_[(*itsd)->uid()] = selectedEvts;
      }	// End of loop over datasets
    }
  } // End of loop over selections
}


void EventInfoPrinter::print() const {
  const unsigned int width = 18;
  std::list<TString> vars = listOfPrintedVariables();
  TString separator1 = "";
  TString separator2 = "";
  for(unsigned int i = 0; i < vars.size(); ++i) {
    for(unsigned int j = 0; j < width+3; ++j) {
      separator1 += "=";
      separator2 += "-";
    }
  }

  // Print file with detailed event info and some formatting
  ofstream file(outFileName_);
  // txt-style table
  for(std::map< TString, std::vector<const Event*> >::const_iterator it = printedEvts_.begin(); it != printedEvts_.end(); ++it) {
    file << separator1 << std::endl;
    file << "Dataset: '" << it->first << "'" << std::endl;
    file << separator2 << std::endl;
    for(std::list<TString>::const_iterator itVar = vars.begin(); itVar != vars.end(); ++itVar) {
      file << std::setw(width) << std::fixed << *itVar;
      if( itVar != --vars.end() ) file << " : ";
      else file << std::endl;
    }
    file << separator2 << std::endl;
    
    for(std::vector<const Event*>::const_iterator itEvt = it->second.begin();
	itEvt != it->second.end(); ++itEvt) {
      for(std::list<TString>::const_iterator itVar = vars.begin(); itVar != vars.end(); ++itVar) {
	if( Variable::type(*itVar) == "UShort_t" ||
	    Variable::type(*itVar) == "Int_t" ||
	    Variable::type(*itVar) == "UInt_t" ||
	    Variable::type(*itVar) == "UChar_t"     ) {
	  file << std::setprecision(0);
	} else {
	  file << std::setprecision(3);
	}
	file << std::setw(width) << (*itEvt)->get(*itVar);
	if( itVar != --vars.end() ) file << " : ";
	else file << std::endl;
      }
    }
    file << separator1 << std::endl;
    file << "\n\n\n";
  }
  // LaTeX-style table
  for(std::map< TString, std::vector<const Event*> >::const_iterator it = printedEvts_.begin(); it != printedEvts_.end(); ++it) {
    file << "\n\n\n%" << separator1 << std::endl;
    file << "% Dataset: '" << it->first << "'" << std::endl;
    file << "%" << separator2 << std::endl;
    file << "\n\\begin{tabular}{";
    for(unsigned int i = 0; i < vars.size(); ++i) {
      file << "r";
    }
    file << "}\n";
    file << "\\toprule\n";
    for(std::list<TString>::const_iterator itVar = vars.begin(); itVar != vars.end(); ++itVar) {
      file << std::setw(width) << std::fixed << "\\multicolumn{1}{c}{" << *itVar << "}";
      if( itVar != --vars.end() ) file << " & ";
      else file << " \\\\ \n";
    }
    file << "\\midrule\n";
    
    for(std::vector<const Event*>::const_iterator itEvt = it->second.begin();
	itEvt != it->second.end(); ++itEvt) {
      for(std::list<TString>::const_iterator itVar = vars.begin(); itVar != vars.end(); ++itVar) {
	if( Variable::type(*itVar) == "UShort_t" ||
	    Variable::type(*itVar) == "Int_t" ||
	    Variable::type(*itVar) == "UInt_t" ||
	    Variable::type(*itVar) == "UChar_t"     ) {
	  file << std::setprecision(0);
	} else {
	  file << std::setprecision(2);
	}
	if( (*itEvt)->get(*itVar) == 9999 ) {
	  file << std::setw(width) << "---";
	} else {
	  file << std::setw(width) << (*itEvt)->get(*itVar);
	}
	if( itVar != --vars.end() ) file << " & ";
	else file << " \\\\ \n";
      }
    }
    file << "\\bottomrule \n\\end{tabular}\n\n\n";
  }

  file.close();

  // Print CMSSW-like run lists
  for(std::map< TString, std::vector<const Event*> >::const_iterator it = printedEvts_.begin(); it != printedEvts_.end(); ++it) {
    ofstream file(Output::resultDir()+"/"+Output::id()+"_EventInfo__"+Output::cleanName(it->first)+"__Runlist.txt");
    for(std::vector<const Event*>::const_iterator itEvt = it->second.begin();
	itEvt != it->second.end(); ++itEvt) {
      file << std::setw(width) << (*itEvt)->get(varNameRunNum);
      file << ":";
      file << std::setw(width) << (*itEvt)->get(varNameLumiBlockNum);
      file << ":";
      file << std::setw(width) << std::setprecision(15) << (*itEvt)->get(varNameEvtNum);
      file << std::endl;
    }
    file.close();
  }


  // LaTeX slides for event displays
  ofstream texFile(latexSlidesName_);
  for(std::map< TString, std::vector<const Event*> >::const_iterator it = printedEvts_.begin();
      it != printedEvts_.end(); ++it) {
    for(std::vector<const Event*>::const_iterator itEvt = it->second.begin();
	itEvt != it->second.end(); ++itEvt) {
      texFile << "% --------------------------------------------------" << std::endl;
      texFile << "\\begin{frame}" << std::endl;
      texFile << "\\frametitle{Event " << (*itEvt)->get(varNameRunNum) << ":" << (*itEvt)->get(varNameLumiBlockNum) << ":" << std::setprecision(15) << (*itEvt)->get(varNameEvtNum) << " (" << it->first << ")}" << std::endl; // (Name of dataset and selection)

      texFile << "  \\begin{columns}" << std::endl;
      texFile << "    \\begin{column}{0.55\\textwidth}" << std::endl;
      texFile << "      \\centering" << std::endl;
      texFile << "      \\includegraphics[width=\\textwidth]{figures/ID-" << (*itEvt)->get(varNameRunNum) << "_" << std::setprecision(15) << (*itEvt)->get(varNameEvtNum) << "_" << (*itEvt)->get(varNameLumiBlockNum) << "_RhoPhi.png}" << std::endl;
      texFile << "    \\end{column}" << std::endl;
      texFile << "    \\begin{column}{0.45\\textwidth}" << std::endl;
      texFile << "      \\includegraphics[width=\\textwidth]{figures/ID-" << (*itEvt)->get(varNameRunNum) << "_" << std::setprecision(15) << (*itEvt)->get(varNameEvtNum) << "_" << (*itEvt)->get(varNameLumiBlockNum) << "_Lego.png}\\\\" << std::endl;
      texFile << "      \\includegraphics[width=\\textwidth]{figures/ID-" << (*itEvt)->get(varNameRunNum) << "_" << std::setprecision(15) << (*itEvt)->get(varNameEvtNum) << "_" << (*itEvt)->get(varNameLumiBlockNum) << "_RhoZ.png}" << std::endl;
      texFile << "    \\end{column}  " << std::endl;
      texFile << "  \\end{columns}" << std::endl;
      texFile << "\\end{frame}\n\n\n" << std::endl;
    }
  }
  texFile.close();
}


std::list<TString> EventInfoPrinter::listOfPrintedVariables() const {
  // List of variables
  std::list<TString> list;
  for(std::vector<TString>::const_iterator itV = Variable::begin();
      itV != Variable::end(); ++itV) {
    list.push_back(*itV);
  }

  // Order list such that the following variabels
  // are at front
  std::vector<TString> varsInOrder;
  varsInOrder.push_back(varNameRunNum);
  varsInOrder.push_back(varNameLumiBlockNum);
  varsInOrder.push_back(varNameEvtNum);
  if( varNameHT != "" ) varsInOrder.push_back(varNameHT);
  if( varNameMHT != "" ) varsInOrder.push_back(varNameMHT);
  if( varNameNJets != "" ) varsInOrder.push_back(varNameNJets);

  // The resulting list
  std::list<TString>::iterator endOfOrderedPart = list.begin();    
  ++endOfOrderedPart;
  for(std::vector<TString>::const_iterator itOV = varsInOrder.begin();
      itOV != varsInOrder.end(); ++itOV) {
    for(std::list<TString>::iterator it = list.begin(); it != list.end(); ++it) {
      if( *it == *itOV ) {
	TString tmp = *it;
	list.erase(it);
	list.insert(endOfOrderedPart,tmp);
	++endOfOrderedPart;
	break;
      }
    }
  }
  
  return list;
}


bool EventInfoPrinter::printAllEvents() const {
  bool printAll = false;
  if( selectionVariables_.size() == 0 ) {
    printAll = true;
  } else {
    for(std::map<TString,unsigned int>::const_iterator itSV = selectionVariables_.begin(); itSV != selectionVariables_.end(); ++itSV) {
      if( itSV->second == -1 ) {
	printAll = true;
	break;
      }
    }
  }

  return printAll;
}


// Check if selection 'uid' is to be printed
// If no selections are specified in the config, print all
bool EventInfoPrinter::printSelection(const TString &uid) const {
  return printedSelections_.size() == 0 ? true : printedSelections_.find(uid) != printedSelections_.end();
}


bool EventInfoPrinter::EvtValPair::valueGreaterThan(const EvtValPair *idx1, const EvtValPair *idx2) {
  if(idx1 == 0) {
    return idx2 != 0;
  } else if(idx2 == 0) {
    return false;
  } else {
    return idx1->value() > idx2->value();
  }
}



