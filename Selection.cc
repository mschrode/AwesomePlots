#include <iostream>
#include <cstdlib>

#include "Config.h"
#include "Event.h"
#include "Filter.h"
#include "Selection.h"


std::vector<Selection*> Selection::selections_; // Collection of selections to be returned
bool Selection::isInit_ = false;
bool Selection::printFilterTree_ = false;


// Create different selections as specified in a config file
// Expect format <key> :: label: <label>; cuts: [previously defined label]+[cuts]+...
// Cuts can optionally be restricted to certain datasets by specifying
// the names of those datasets in brackets directly after the cut:
// ...; cuts: [cut1] ([dataset1],[dataset2],...) + [cut2] + ...
// ---------------------------------------------------------------
void Selection::init(const Config &cfg, const TString key) {
  if( isInit_ ) {
    std::cerr << "WARNING: Selections already initialized. Skipping." << std::endl;
  } else {
    std::cout << "  Preparing selections...  " << std::flush;

    std::vector<Config::Attributes> attrList = cfg(key);
    if( attrList.size() == 0 ) { // No selections specified
      selections_.push_back(new Selection("unselected",new FilterTRUE()));
    }
    for(std::vector<Config::Attributes>::const_iterator it = attrList.begin();
	it != attrList.end(); ++it) {
      if( it->hasName("label") && it->hasName("cuts") ) {
	// Optionally, apply selection only to these datasets
	std::vector<TString> dataSetLabels;
	if( it->hasName("apply to") ) {
	  Config::split(it->value("apply to"),",",dataSetLabels);
	}

	// Setup a filter tree
	const Filter* filter = Filter::create(it->value("cuts"),dataSetLabels,it->lineNumber(),it->value("label"));	
	
	// Add this selection to the list of full selections
	selections_.push_back(new Selection(it->value("label"),filter));

      } else if( it->hasName("print") ) {
	if( it->isBoolean("print") ) {
	  printFilterTree_ = it->valueBoolean("print");
	} else {
	  std::cerr << "\n\nWARNING: Undefined print-out status in line " << it->lineNumber() << std::endl;
	  std::cerr << "  Expect 'print: true' or 'print: false'" << std::endl;
	  std::cerr << "  Using default 'false'" << std::endl;
	}
      } else {
	std::cerr << "\n\nERROR: Wrong syntax when defining selection in line " << it->lineNumber() << std::endl;
	std::cerr << "  Expect selections to be defined as" << std::endl;
	std::cerr << "  [key] :: label: [label]; cuts: ..." << std::endl;
	exit(-1);
      }
    }
    isInit_ = true;
    std::cout << "ok" << std::endl;
  }
}


// Use this method to delete all existing selections
// Reused filters are treated correctly
// ---------------------------------------------------------------
void Selection::clear() {
  for(std::vector<Selection*>::iterator it = selections_.begin();
      it != selections_.end(); ++it) {
    delete *it;
  }
  Filter::clear();
}


// ---------------------------------------------------------------
unsigned int Selection::maxLabelLength() {
  unsigned int s = 0;
  for(SelectionIt its = Selection::begin(); its != Selection::end(); ++its) {
    if( (*its)->uid().Length() > s ) s = (*its)->uid().Length();
  }
  
  return s;
}


// ---------------------------------------------------------------
void Selection::print() const {
  if( printFilterTree_ ) std::cout << std::endl;
  std::cout << "  Selection '" << uid() << "'" << std::endl;
  if( printFilterTree_ ) std::cout << filter_->printOut() << std::endl;
}
