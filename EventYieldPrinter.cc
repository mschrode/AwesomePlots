// $Id: EventYieldPrinter.cc,v 1.18 2013/05/22 14:50:22 mschrode Exp $

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <vector>

#include "DataSet.h"
#include "EventYieldPrinter.h"
#include "Output.h"
#include "Selection.h"
#include "Style.h"



EventYieldPrinter::EventYieldPrinter() 
  : inputDataSets_(DataSet::findAllUnselected()) {
  
  const TString outFileNamePrefix = Output::resultDir()+"/"+Output::id();
  prepareSummaryTable();

  std::cout << "  - Writing event-yield information to '" << outFileNamePrefix << "_EventYields.tex'" << std::endl;
  printToLaTeX(outFileNamePrefix+"_EventYields.tex");

  std::cout << "  - Writing data card to '" << outFileNamePrefix << "_DataCard.txt'" << std::endl;
  printDataCard(outFileNamePrefix+"_DataCard.txt");

  printToScreen();
}


void EventYieldPrinter::prepareSummaryTable() {

  // Print DataSets of different type together
  std::vector<DataSet::Type> printedTypes;
  printedTypes.push_back(DataSet::Data);
  printedTypes.push_back(DataSet::MC);
  printedTypes.push_back(DataSet::Prediction);
  printedTypes.push_back(DataSet::MCPrediction);
  printedTypes.push_back(DataSet::Signal);
  std::vector<bool> printTotalYield(printedTypes.size(),false);

  // Header row
  std::vector<TString> tableRow;
  std::vector<TString> tableRowLatex;
  tableRow.push_back(" Selection ");
  tableRowLatex.push_back(" Selection ");
  for(unsigned int typeIdx = 0; typeIdx < printedTypes.size(); ++typeIdx) {
    unsigned int nDataSetsOfThisType = 0;
    for(DataSetIt itd = inputDataSets_.begin(); itd != inputDataSets_.end(); ++itd) {
      if( (*itd)->type() == printedTypes.at(typeIdx) ) {
	tableRow.push_back((" "+(*itd)->label()+" "));
	tableRowLatex.push_back((" "+Style::tlatexLabel(*itd)+" "));
	++nDataSetsOfThisType;
      }
    }
    if( nDataSetsOfThisType > 1 ) {
      printTotalYield.at(typeIdx) = true;
      tableRow.push_back((" Total "+DataSet::toString(printedTypes.at(typeIdx))+" "));
      tableRowLatex.push_back((" Total "+DataSet::toString(printedTypes.at(typeIdx))+" "));
    }
  }
  summaryTable_.push_back(tableRow);
  summaryTableLatex_.push_back(tableRowLatex);

  // One row per selection
  for(SelectionIt its = Selection::begin(); its != Selection::end(); ++its) {
    tableRow.clear();
    tableRow.push_back((" "+(*its)->uid()+" "));
    tableRowLatex.clear();
    tableRowLatex.push_back((" "+Style::tlatexLabel((*its)->uid())+" "));
    DataSets selectedDataSets = DataSet::findAllWithSelection((*its)->uid());
    char yield[50];
    char stat[50];
    char systDn[50];
    char systUp[50];
    for(unsigned int typeIdx = 0; typeIdx < printedTypes.size(); ++typeIdx) {
      double totYield = 0.;
      double totStat = 0.;
      double totSystDn = 0.;
      double totSystUp = 0.;
      for(DataSetIt itsd = selectedDataSets.begin(); itsd != selectedDataSets.end(); ++itsd) {
	if( (*itsd)->type() == printedTypes.at(typeIdx) ) {
	  TString tableCell = " ";
	  if( printedTypes.at(typeIdx) == DataSet::Data ) {
	    sprintf(yield,"%.0lf",(*itsd)->yield());
	    totYield += (*itsd)->yield();
	    tableCell += yield;
	  } else {
	    sprintf(yield,"%.1lf",(*itsd)->yield());
	    totYield += (*itsd)->yield();
	    sprintf(stat,"%.1lf",(*itsd)->stat());
	    totStat = std::sqrt( (*itsd)->stat()*(*itsd)->stat() + totStat*totStat );
	    tableCell += yield;
	    tableCell += " +/- ";
	    tableCell += stat;
	    if( (*itsd)->hasSyst() ) {
	      sprintf(systDn,"%.1lf",(*itsd)->totSystDn());
	      totSystDn = std::sqrt( (*itsd)->totSystDn()*(*itsd)->totSystDn() + totSystDn*totSystDn );
	      sprintf(systUp,"%.1lf",(*itsd)->totSystUp());
	      totSystUp = std::sqrt( (*itsd)->totSystUp()*(*itsd)->totSystUp() + totSystUp*totSystUp );
	      tableCell += " +";
	      tableCell += systUp;
	      tableCell += " -";
	      tableCell += systDn;
	    }
	  }
	  tableCell += " ";
	  tableRow.push_back(tableCell);
	  tableRowLatex.push_back(tableCell);
	}
      }
      if( printTotalYield.at(typeIdx) ) {
	TString tableCell = " ";
	if( printedTypes.at(typeIdx) == DataSet::Data ) {
	  sprintf(yield,"%.1lf",totYield);
	  tableCell += yield;
	} else {
	  sprintf(yield,"%.1lf",totYield);
	  sprintf(stat,"%.1lf",totStat);
	  sprintf(systDn,"%.1lf",totSystDn);
	  sprintf(systUp,"%.1lf",totSystUp);
	  tableCell += yield;
	  tableCell += " +/- ";
	  tableCell += stat;
	  if( totSystDn > 0. ) {
	    tableCell += " +";
	    tableCell += systUp;
	    tableCell += " -";
	    tableCell += systDn;
	  }
	}
	tableCell += " ";
	tableRow.push_back(tableCell);
	tableRowLatex.push_back(tableCell);
      }
    }
    summaryTable_.push_back(tableRow);
    summaryTableLatex_.push_back(tableRowLatex);
  }

  // Sanity check
  const unsigned int nCols = summaryTable_.front().size();
  for(std::vector< std::vector<TString> >::const_iterator itr = summaryTable_.begin();
      itr != summaryTable_.end(); ++itr) { // Loop over rows
    if( itr->size() != nCols ) {
      std::cerr << "ERROR in EventYieldPrinter when building summary table" << std::endl;
      exit(-1);
    }
  }
  for(std::vector< std::vector<TString> >::const_iterator itr = summaryTableLatex_.begin();
      itr != summaryTableLatex_.end(); ++itr) { // Loop over rows
    if( itr->size() != nCols ) {
      std::cerr << "ERROR in EventYieldPrinter when building Latex summary table" << std::endl;
      exit(-1);
    }
  }

  // Find column width
  summaryTableColw_ = std::vector<unsigned int>(summaryTable_.front().size(),0);
  for(std::vector< std::vector<TString> >::const_iterator itr = summaryTable_.begin();
      itr != summaryTable_.end(); ++itr) { // Loop over rows
    unsigned int colIdx = 0;
    for(std::vector<TString>::const_iterator itc = itr->begin();
	itc != itr->end(); ++itc, ++colIdx) { // Loop over columns
      if( itc->Length() > summaryTableColw_.at(colIdx) ) summaryTableColw_.at(colIdx) = itc->Length();
    }
  }
  summaryTableLatexColw_ = std::vector<unsigned int>(summaryTableLatex_.front().size(),0);
  for(std::vector< std::vector<TString> >::const_iterator itr = summaryTableLatex_.begin();
      itr != summaryTableLatex_.end(); ++itr) { // Loop over rows
    unsigned int colIdx = 0;
    for(std::vector<TString>::const_iterator itc = itr->begin();
	itc != itr->end(); ++itc, ++colIdx) { // Loop over columns
      if( itc->Length() > summaryTableLatexColw_.at(colIdx) ) summaryTableLatexColw_.at(colIdx) = itc->Length();
    }
  }
}


void EventYieldPrinter::printToScreen() const {
  // Print header
  std::cout << "\n\n\n";
  for(unsigned int colIdx = 0; colIdx < summaryTable_.front().size(); ++colIdx) {
    std::cout << std::setw(summaryTableColw_.at(colIdx)) << summaryTable_.front().at(colIdx) << "|";
  }
  std::cout << "\n";
  for(unsigned int colIdx = 0; colIdx < summaryTable_.front().size(); ++colIdx) {
    for(unsigned int i = 0; i < summaryTableColw_.at(colIdx); ++i) {
      std::cout << "-";
    }
    std::cout << "+";
  }
  std::cout << "\n";
  // Print body
  for(std::vector< std::vector<TString> >::const_iterator itr = summaryTable_.begin()+1;
      itr != summaryTable_.end(); ++itr) { // Loop over rows
    for(unsigned int colIdx = 0; colIdx < itr->size(); ++colIdx) { // Loop over columns
      std::cout << std::setw(summaryTableColw_.at(colIdx)) << itr->at(colIdx) << "|";
    }
  std::cout << "\n";
  }
  std::cout << "\n";
}


void EventYieldPrinter::printToLaTeX(const TString &outFileName) const {
  ofstream file(outFileName);

  // Print yields and total uncertainties
  // per datasets (rows) and selections (columns)
  file << "\n\n\n\n";
  file << "%===========================================================================" << std::endl;
  file << "% Datasets vs Selections: yields and total uncertainties" << std::endl;
  file << "%===========================================================================" << std::endl;
  file << "\n\\begin{tabular}{l";
  for(unsigned int i = 1; i < summaryTableLatex_.front().size(); ++i) {
    file << "r";
  }
  file << "}\n";
  file << "\\toprule\n";
  for(unsigned int colIdx = 0; colIdx < summaryTableLatex_.front().size(); ++colIdx) {
    file << std::setw(summaryTableColw_.at(colIdx)+8) << Output::cleanLatexName(summaryTableLatex_.front().at(colIdx));
    file << (colIdx < summaryTableLatex_.front().size()-1 ? " & " : " \\\\ \n\\midrule\n");
  }
  for(std::vector< std::vector<TString> >::const_iterator itr = summaryTableLatex_.begin()+1;
      itr != summaryTableLatex_.end(); ++itr) { // Loop over rows
    for(unsigned int colIdx = 0; colIdx < itr->size(); ++colIdx) { // Loop over columns
      TString cell = Output::cleanLatexName(itr->at(colIdx));
      if( colIdx > 0 ) {
	cell.ReplaceAll(" +/- ","\\pm");
	if( cell.Contains("+") ) {
	  cell.ReplaceAll(" +","{}^{+");
	  cell.ReplaceAll(" -","}^{-");
	  cell += "}";
	}
	cell.ReplaceAll(" ","");
	cell = "$" + cell + "$";	  
      }
      file << std::setw(summaryTableLatexColw_.at(colIdx)+8) << cell;
      file << (colIdx < itr->size()-1 ? "& " : " \\\\ \n");
    }
  }
  file << "\\bottomrule \n\\end{tabular}\n\n\n";



  // Print yields, total uncertainties, and individual syst uncertainties
  // (columns) per selection (rows) for all datasets
  DataSets inputDataSets_ = DataSet::findAllUnselected();
  unsigned int width = Selection::maxLabelLength() + 4;
  file << "\n\n\n\n";
  file << "%===========================================================================" << std::endl;
  file << "% Detailed Yields and Uncertainties" << std::endl;
  file << "%===========================================================================" << std::endl;

  for(DataSetIt itd = inputDataSets_.begin(); itd != inputDataSets_.end(); ++itd) {
    file << "\n\n%---------------------------------------------------------------------------" << std::endl;
    file << "% Dataset: " << Output::cleanLatexName((*itd)->label()) << std::endl;
    file << "%---------------------------------------------------------------------------" << std::endl;
    file << "\n\\begin{tabular}{l|rrr";
    for(unsigned int i = 0; i < (*itd)->nSyst(); ++i) {
      if( i == 0 ) file << "|";
      file << "r";
    }
    file << "}\n";
    file << "\\toprule\n";
    file << std::setw(width) << "Selection";
    file << " & yield & stat & syst tot ";
    if( (*itd)->nSyst() > 1 ) {
      for(std::vector<TString>::const_iterator systIt = (*itd)->systLabelsBegin();
	  systIt != (*itd)->systLabelsEnd(); ++systIt) {
	file << " & " << *systIt << "[\\%]";
      }
    }
    file << "  \\\\ \n\\midrule\n";

    for(SelectionIt its = Selection::begin(); its != Selection::end(); ++its) {
      file << std::setw(width) << Output::cleanLatexName((*its)->uid());      
      const DataSet* selectedDataSet = DataSet::find((*itd)->label(),*its);
      char yield[50];
      char stat[50];
      char systDn[50];
      char systUp[50];
      sprintf(yield,"%.1lf",selectedDataSet->yield());
      sprintf(stat,"%.1lf",selectedDataSet->stat());
      sprintf(systDn,"%.1lf",selectedDataSet->totSystDn());
      sprintf(systUp,"%.1lf",selectedDataSet->totSystUp());
      file << " & " << yield << " & " << stat;
      file << " & ${}^{+" << systUp << "}_{-" << systDn << "}$";
      if( (*itd)->nSyst() > 1 ) {
	for(std::vector<TString>::const_iterator systIt = (*itd)->systLabelsBegin();
	    systIt != (*itd)->systLabelsEnd(); ++systIt) {
	  if( selectedDataSet->yield() > 0. ) {
	    sprintf(systDn,"%.1lf",100.*selectedDataSet->systDn(*systIt)/selectedDataSet->yield());
	    sprintf(systUp,"%.1lf",100.*selectedDataSet->systUp(*systIt)/selectedDataSet->yield());
	    file << " & ${}^{+" << systUp << "}_{-" << systDn << "}$";	  
	  }
	}
      }
      file << " \\\\" << std::endl;
    }
    file << "\\bottomrule\n\\end{tabular}" << std::endl;
  }

  file.close();
}


void EventYieldPrinter::printDataCard(const TString &outFileName) const {
  ofstream file(outFileName);

  DataSets inputDataSets_ = DataSet::findAllUnselected();
  unsigned int width = 8;

  // Determine bins
  unsigned int nBins = 0;
  TString commentOnBinning = "# ";
  TString binning = "channel = ";
  for(SelectionIt its = Selection::begin(); its != Selection::end(); ++its) {
    ++nBins;
    binning += "bin";
    binning += nBins;
    binning += "; ";
    commentOnBinning += "bin";
    commentOnBinning += nBins;
    commentOnBinning += " is for " + (*its)->uid() + ", ";
  }

  
  // Loop over input datasets
  for(DataSetIt itd = inputDataSets_.begin(); itd != inputDataSets_.end(); ++itd) {
    // General informatin
    file << "# General information:" << std::endl;
    file << "luminosity = " << 1000.*(GlobalParameters::lumi()).Atof() << " # given in pb-1" << std::endl;
    file << "channels   = " << nBins << " # total number of channels / bins. Counting ordering, MHT, HT and nJets." << std::endl;
    file << "sample     = " << (*itd)->label() << " # name of the sample" << std::endl;
    if( (*itd)->type() == DataSet::Prediction ) {
      file << "nuisances = " << (*itd)->nSyst() + 1 << " # number of nuisance/uncertainties" << std::endl;
    }

    // Binning information
    file << "\n" << commentOnBinning << std::endl;
    file << binning << std::endl;

    // Yields
    file << (*itd)->label() << "_events = ";
    
    for(SelectionIt its = Selection::begin(); its != Selection::end(); ++its) {
      const DataSet* selectedDataSet = DataSet::find((*itd)->label(),*its);
      file << std::setw(width) << selectedDataSet->yield();
    }
    file << std::endl;

    if( (*itd)->type() != DataSet::Data ) {
      // define number of uncertainties
      file << "# Uncertainties --> at least stat. and syst." << std::endl;
      file << "# In absolute numbers" << std::endl;
      file << "nuisance = stat. uncert.; ";
      if( (*itd)->hasSyst() ) {
	file << GlobalParameters::defaultUncertaintyLabel();
      }
      file << std::endl;
      // define uncertainty distributions
      file << (*itd)->label() << "_uncertaintyDistribution_1 = lnN" << std::endl;
      if( (*itd)->hasSyst() ) {
	file << (*itd)->label() << "_uncertaintyDistribution_2 = lnN" << std::endl;
      }
      // print statistical uncertainties in each bin
      file << (*itd)->label() << "_uncertainty_1 = ";
      for(SelectionIt its = Selection::begin(); its != Selection::end(); ++its) {
	const DataSet* selectedDataSet = DataSet::find((*itd)->label(),*its);
	file << std::setw(width) << selectedDataSet->stat();
      }
      file << std::endl;
      // print total systematic uncertainty in each bin
      if( (*itd)->hasSyst() ) {
	file << (*itd)->label() << "_uncertaintyDN_2 = ";
	for(SelectionIt its = Selection::begin(); its != Selection::end(); ++its) {
	  const DataSet* selectedDataSet = DataSet::find((*itd)->label(),*its);
	  file << std::setw(width) << selectedDataSet->totSystDn() << " ";
	}
	file << std::endl;
	file << (*itd)->label() << "_uncertaintyUP_2 = ";
	for(SelectionIt its = Selection::begin(); its != Selection::end(); ++its) {
	  const DataSet* selectedDataSet = DataSet::find((*itd)->label(),*its);
	  file << std::setw(width) << selectedDataSet->totSystUp() << " ";
	}
	file << std::endl;
      }
    }


    // Optionally, print individual uncertainties
    if( (*itd)->type() != DataSet::Data && (*itd)->nSyst() > 1 ) {
      // General informatin
      file << "\n\n";
      file << "# General information:" << std::endl;
      file << "luminosity = " << 1000.*(GlobalParameters::lumi()).Atof() << " # given in pb-1" << std::endl;
      file << "channels   = " << nBins << " # total number of channels / bins. Counting ordering, MHT, HT and nJets." << std::endl;
      file << "sample     = " << (*itd)->label() << " # name of the sample" << std::endl;
      if( (*itd)->type() == DataSet::Prediction ) {
	file << "nuisances = " << (*itd)->nSyst() + 1 << " # number of nuisance/uncertainties" << std::endl;
      }
      
      // Binning information
      file << "\n" << commentOnBinning << std::endl;
      file << binning << std::endl;
      
      // Yields
      file << (*itd)->label() << "_events = ";
      
      for(SelectionIt its = Selection::begin(); its != Selection::end(); ++its) {
	const DataSet* selectedDataSet = DataSet::find((*itd)->label(),*its);
	file << std::setw(width) << selectedDataSet->yield();
      }
      file << std::endl;
      
      if( (*itd)->type() != DataSet::Data ) {
	// define number of uncertainties
	file << "# Uncertainties --> at least stat. and syst." << std::endl;
	file << "# In absolute numbers" << std::endl;
	file << "nuisance = stat. uncert.; ";
	for(std::vector<TString>::const_iterator itu = (*itd)->systLabelsBegin();
	    itu != (*itd)->systLabelsEnd(); ++itu) {
	  file << *itu << "; ";
	}
	file << std::endl;
	// define uncertainty distributions
	for(unsigned int i = 1; i <= (*itd)->nSyst() + 1; ++i) {
	  file << (*itd)->label() << "_uncertaintyDistribution_" << i << " = lnN" << std::endl;
	}
	// print statistical uncertainties in each bin
	file << (*itd)->label() << "_uncertainty_1 = ";
	for(SelectionIt its = Selection::begin(); its != Selection::end(); ++its) {
	  const DataSet* selectedDataSet = DataSet::find((*itd)->label(),*its);
	  file << std::setw(width) << selectedDataSet->stat();
	}
	file << std::endl;
	// print further uncertainties in each bin
	unsigned int nUncert = 2;
	for(std::vector<TString>::const_iterator itu = (*itd)->systLabelsBegin();
	    itu != (*itd)->systLabelsEnd(); ++itu, ++nUncert) {
	  file << (*itd)->label() << "_uncertaintyDN_" << nUncert << " = ";
	  for(SelectionIt its = Selection::begin(); its != Selection::end(); ++its) {
	    const DataSet* selectedDataSet = DataSet::find((*itd)->label(),*its);
	    file << std::setw(width) << selectedDataSet->systDn(*itu) << " ";
	  }
	  file << std::endl;
	  file << (*itd)->label() << "_uncertaintyUP_" << nUncert << " = ";
	  for(SelectionIt its = Selection::begin(); its != Selection::end(); ++its) {
	    const DataSet* selectedDataSet = DataSet::find((*itd)->label(),*its);
	    file << std::setw(width) << selectedDataSet->systUp(*itu) << " ";
	  }
	  file << std::endl;
	}
      }
    }
    file << "\n\n\n\n";
  }
}
