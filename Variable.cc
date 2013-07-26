#include <cstdlib>
#include <iostream>

#include "Variable.h"

bool Variable::isInit_ = false;
std::set<TString> Variable::validTypes_;
std::vector<TString> Variable::names_;
std::map<TString,TString> Variable::types_;
std::map<TString,TString> Variable::labels_;
std::map<TString,TString> Variable::units_;


void Variable::checkIfIsInit() {
  if( !isInit_ ) {
    std::cerr << "\n\nERROR: Variables not initialized.\nCall 'Variable::init()' before using static methods of Variable." << std::endl;
    exit(-1);
  }
}


void Variable::init(const Config &cfg, const TString &key) {
  if( !isInit_ ) {
    std::cout << "  Initializing variables...  " << std::flush;

    validTypes_.insert("Double_t");
    validTypes_.insert("Float_t");
    validTypes_.insert("UShort_t");
    validTypes_.insert("Int_t");
    validTypes_.insert("UInt_t");
    validTypes_.insert("UChar_t");

    std::vector<Config::Attributes> attrList = cfg(key);
    for(std::vector<Config::Attributes>::const_iterator it = attrList.begin();
	it != attrList.end(); ++it) {
      if( it->nValues() >= 2 ) {
	TString name = it->value("name");
	TString type = it->value("type");
	TString label = it->value("label");
	TString unit = it->value("unit");
	if( !exists(name) ) {
	  if( validType(type) ) {
	    names_.push_back(name);
	    types_[name] = type;
	    labels_[name] = label;
	    units_[name] = unit;
	  } else {
	    std::cerr << "\n\nERROR in Variable::init(): undefined type '" << type << "' of variable '" << name << "'" << std::endl;
	    std::cerr << "  in line " << it->lineNumber() << " of config file '" << cfg.fileName() << "'" << std::endl;
	    std::cerr << "  Valid types are ' " << std::flush;
	    for(std::set<TString>::const_iterator itVT = validTypes_.begin();
		itVT != validTypes_.end(); ++itVT) {
	      std::cerr << *itVT << "  " << std::flush;
	    }
	    std::cout << "'" << std::endl;
	    exit(-1);
	  }
	} else {
	  std::cerr << "\n\nWARNING in Variable::init(): multiple definition of variable '" << name << "'" << std::endl;
	  std::cerr << "  in config file '" << cfg.fileName() << "'" << std::endl;
	  std::cerr << "  using first definition and ignoring all later ones" << std::endl;
	}
      } else {
	std::cerr << "\n\nERROR in Variable::init(): wrong config syntax" << std::endl;
	std::cerr << "  in line with key '" << key << "'" << std::endl;
	std::cerr << "  in config file '" << cfg.fileName() << "'" << std::endl;
	std::cerr << "  Syntax is '[key] : treename [treename], type [type], label <label>, unit <unit>" << std::endl;
	exit(-1);
      }
    }
    isInit_ = true;
    std::cout << "ok" << std::endl;    
  }
}


bool Variable::validType(const TString &type) {
  return validTypes_.find(type) != validTypes_.end();
}


TString Variable::type(const TString &name) {
  TString type = "";
  std::map<TString,TString>::const_iterator it = types_.find(name);
  if( it != types_.end() ) {
    type = it->second;
  } else {
    std::cerr << "\n\nERROR in Variable::type: Variable '" << name << "' not specified." << std::endl;
    exit(-1);
  }
    
  return type;
}


bool Variable::exists(const TString &name) {
  std::map<TString,TString>::const_iterator it = types_.find(name);
  return it != types_.end();
}


TString Variable::label(const TString &name) {
  TString label = name;
  std::map<TString,TString>::const_iterator it = labels_.find(name);
  if( it != labels_.end() ) {
    label = it->second;
  }
    
  return label;
}


TString Variable::unit(const TString &name) {
  TString unit = name;
  std::map<TString,TString>::const_iterator it = units_.find(name);
  if( it != units_.end() ) {
    unit = it->second;
  }
    
  return unit;
}
