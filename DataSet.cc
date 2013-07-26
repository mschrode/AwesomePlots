#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "DataSet.h"
#include "EventBuilder.h"
#include "GlobalParameters.h"
#include "Variable.h"


DataSetUidMap DataSet::dataSetUidMap_;
bool DataSet::isInit_ = false;


TString DataSet::uid(const TString &label, const TString &selectionUid) {
  return label+":"+selectionUid;
}

TString DataSet::uid(const TString &label, const Selection* selection) {
  return uid(label,selection->uid());
}

const DataSet* DataSet::find(const TString &label, const Selection* selection) {
  return find(uid(label,selection));
}

const DataSet* DataSet::find(const TString &uid) {
  DataSetUidIt it = dataSetUidMap_.find(uid);
  if( it == dataSetUidMap_.end() ) {
    std::cerr << "ERROR: DataSet with uid '" << uid << "' does not exist" << std::endl;
    exit(-1);
  }
  
  return it->second;
}


DataSets DataSet::findAllWithSelection(const TString &selectionUid) {
  DataSets dataSets;
  for(DataSetUidIt it = dataSetUidMap_.begin(); it != dataSetUidMap_.end(); ++it) {
    if( it->second->selectionUid() == selectionUid ) {
      dataSets.push_back(it->second);
    }
  }

  return dataSets;
}

DataSets DataSet::findAllWithLabel(const TString &label) {
  DataSets dataSets;
  for(SelectionIt its = Selection::begin(); its != Selection::end(); ++its) {
    dataSets.push_back(find(label,*its));
  }

  return dataSets;
}


bool DataSet::uidExists(const TString &uid) {
  return dataSetUidMap_.find(uid) != DataSet::dataSetUidMap_.end();
}


bool DataSet::labelExists(const TString &label) {
  bool exists = false;
  for(DataSetUidIt it = dataSetUidMap_.begin(); it != dataSetUidMap_.end(); ++it) {
    if( it->second->label() = label ) {
      exists = true;
      break;
    }
  }

  return exists;
}

DataSet::Type DataSet::toType(const TString &type) {
  DataSet::Type result;
  if( type == "Data" || type == "data" ) result = Data;
  else if( type == "MC" || type == "mc" ) result = MC;
  else if( type == "Prediction" || type == "prediction" ) result = Prediction;
  else if( type == "MCPrediction" || type == "mcprediction" ) result = MCPrediction;
  else if( type == "Signal" || type == "signal" ) result = Signal;
  else {
    std::cerr << "\n\nERROR in DataSet::toType(): type '" << type << "' does not exist" << std::endl;
    exit(-1);
  }

  return result;
}


TString DataSet::toString(Type type) {
  TString result = "none";
  if( type == Data ) result = "data";
  else if( type == MC ) result = "MC";
  else if( type == Prediction ) result = "prediction";
  else if( type == MCPrediction ) result = "MC prediction";
  else if( type == Signal ) result = "signal";

  return result;
}


void DataSet::init(const Config &cfg, const TString key) {
  if( isInit_ ) {
    std::cerr << "WARNING: Datasets already initialized. Skipping." << std::endl;
  } else {
    std::cout << "  Reading datasets and applying selections...  " << std::flush;

    std::vector<Config::Attributes> attrList = cfg(key);
    for(std::vector<Config::Attributes>::const_iterator it = attrList.begin();
	it != attrList.end(); ++it) {
      if( it->hasName("label") && it->hasName("type") && it->hasName("files") && it->hasName("tree") ) {
	TString label = it->value("label");
	TString type = it->value("type");
	std::vector<TString> files;
	Config::split(it->value("files"),",",files);
	// Prepend global path
	std::vector<TString>::iterator fileIt = files.begin();
	for(; fileIt != files.end(); ++fileIt) {
	  if( !( (*fileIt)(0) == '/' || (*fileIt)(0) == '~') ) {
	    *fileIt = GlobalParameters::inputPath()+(*fileIt);
	  }
	}
	TString tree = it->value("tree");
	TString weight = "";
	std::vector<double> scales(files.size(),1.);
	std::vector<TString> uncDn;
	std::vector<TString> uncUp;
	std::vector<TString> uncLabel;
	if( it->hasName("weight") ) {
	  weight = it->value("weight");
	  if( !(Variable::exists(weight) || weight.IsFloat()) ) {
	    std::cerr << "\n\nERROR in DataSet::createDataSets(): wrong config syntax" << std::endl;
	    std::cerr << "  when specifying the weight in line with key '" << key << "'" << std::endl;
	    std::cerr << "  Syntax is '..., weight : expr, ...', where 'expr' is either a float or an existing variable" << std::endl;
	  }
	}
	if( it->hasName("scales") ) {
	  std::vector<TString> scalesTmp;
	  Config::split(it->value("scales"),",",scalesTmp);
	  // Case 1: same scale factor for all input files
	  if( scalesTmp.size() == 1 ) {
	    for(unsigned int i = 1; i < files.size(); ++i) {
	      scalesTmp.push_back(scalesTmp.at(0));
	    }
	  }
	  // Check if number of scale factors equals number of
	  // input files
	  if( scalesTmp.size() != files.size() ) {
	    std::cerr << "\n\nERROR in DataSet::createDataSets(): wrong number of scale factors specified" << std::endl;
	    std::cerr << "  in line with key '" << key << "'" << std::endl;
	    std::cerr << "  Either specify one scale factor, valid for all input files of this DataSet, or specify one scale factor per input file of this DataSet" << std::endl;
	    std::cerr << "  Syntax is '...; scale : num; ...' or '...; scale : num1, num2, ..., num<n>', where 'num' is a float and <n> equals the number of input files" << std::endl;
	    exit(-1);
	  }
	  // Convert scale factors into floats
	  scales.clear();
	  for(std::vector<TString>::iterator scaleIt = scalesTmp.begin();
	      scaleIt != scalesTmp.end(); ++scaleIt) {
	    if( !(*scaleIt).IsFloat() ) {
	      std::cerr << "\n\nERROR in DataSet::createDataSets(): wrong config syntax" << std::endl;
	      std::cerr << "  when specifying the scale factors in line with key '" << key << "'" << std::endl;
	      std::cerr << "  Syntax is '...; scale : num; ...' or '...; scale : num1, num2, ..., num<n>', where 'num' is a float and <n> equals the number of input files" << std::endl;
	      exit(-1);
	    }
	    scales.push_back((*scaleIt).Atof());
	  }
	}
	// Optionally, parse uncertainties:
	// Get all key-value pairs that contain the key 'uncertainty'
	// The expected format is 'uncertainty <label> : ...'
	std::vector<TString> uncNames = it->listOfNames("uncertainty");
	for(std::vector<TString>::const_iterator uit = uncNames.begin();
	    uit != uncNames.end(); ++uit) {
	  // Find uncertainty label
	  TString tmp = "";	// this will just read 'uncertainty' and is ignored
	  TString ul = "";	// this will be the label, if specified
	  Config::split(*uit," ",tmp,ul);

	  // Special case: no uncertainty label specified
	  // Assume this is the total uncertainty and there are
	  // no other sources
	  if( ul == "" ) {
	    if( uncNames.size() > 1 ) {
	      std::cerr << "\n\nERROR in DataSet::createDataSets(): wrong config syntax" << std::endl;
	      std::cerr << "  when specifying the uncertainties in line with key '" << key << "'" << std::endl;
	      exit(-1);
	    }
	    ul = GlobalParameters::defaultUncertaintyLabel();
	  }
	  uncLabel.push_back(ul);

	  // Now, parse the uncertainty value for the lower and upper
	  // uncertainty variables
	  std::vector<TString> uCfgs;
	  Config::split(it->value(*uit),",",uCfgs);
	  // Case 1: one uncertainty, used as symmetric uncertainty
	  if( uCfgs.size() == 1 ) {
	    if( uCfgs.front().Contains("+") || uCfgs.front().Contains("-") ) {
	      std::cerr << "\n\nERROR in DataSet::createDataSets(): wrong config syntax" << std::endl;
	      std::cerr << "  when specifying the uncertainty" << std::flush;
	      if( uncNames.size() > 1 ) std::cerr << " with label '" << ul << "'";
	      std::cerr << std::endl;
	      std::cerr << "  Upper and lower uncertainty have to be ',' separated" << std::endl;
	      exit(-1);	      
	    }
	    uncDn.push_back(uCfgs.front());
	    uncUp.push_back(uCfgs.front());
	  }
	  // Case 2: separate lower and upper uncertainty,
	  // specified by '-' and '+', respectively
	  else if( uCfgs.size() == 2 &&
		   ( 
		    ( uCfgs.at(0).Contains("+") && uCfgs.at(1).Contains("-") )
		    ||
		    ( uCfgs.at(0).Contains("-") && uCfgs.at(1).Contains("+") )
		     )
		   ) {
	    if( uCfgs.at(0).Contains("+") && uCfgs.at(1).Contains("-") ) {
	      uncDn.push_back(Config::after(uCfgs.at(1),"-"));
	      uncUp.push_back(Config::after(uCfgs.at(0),"+"));
	    } else if( uCfgs.at(0).Contains("-") && uCfgs.at(1).Contains("+") ) {
	      uncDn.push_back(Config::after(uCfgs.at(0),"-"));
	      uncUp.push_back(Config::after(uCfgs.at(1),"+"));
	    }
	  }
	  else {
	    std::cerr << "\n\nERROR in DataSet::createDataSets(): wrong config syntax" << std::endl;
	    std::cerr << "  when specifying the uncertainties in line with key '" << key << "'" << std::endl;
	    exit(-1);
	  }
	}

	// Create basic (unselected) dataset and
	// store it in global map of datasets
	DataSet* basicDataSet = new DataSet(DataSet::toType(type),label,files,tree,weight,uncDn,uncUp,uncLabel,scales);
	dataSetUidMap_[basicDataSet->uid()] = basicDataSet;

	// Fancy output
	if( attrList.size() > 3 && it == attrList.begin()+2 ) {
	  std::cout << "  wait for it...  " << std::flush;
	}

      } else {
	std::cerr << "\n\nERROR in DataSet::createDataSets(): wrong config syntax" << std::endl;
	std::cerr << "  in line with key '" << key << "'" << std::endl;
	std::cerr << "  in config file '" << cfg.fileName() << "'." << std::endl;
	std::cerr << "  Syntax is '" << key << " : label [label], type [type], files [fileName], tree [treeName], weight <weight_expr>'" << std::endl;
	exit(-1);
      }
    }
    isInit_ = true;
    std::cout << "ok" << std::endl;
  }
}


void DataSet::clear() {
  for(std::map<TString,const DataSet*>::iterator it = dataSetUidMap_.begin();
      it != dataSetUidMap_.end(); ++it) {
    delete it->second;
  }
}


DataSet::DataSet(Type type, const TString &label, const std::vector<TString> &fileNames, const TString &treeName, const TString &weight, const std::vector<TString> &uncDn, const std::vector<TString> &uncUp, const std::vector<TString> &uncLabel, const std::vector<double> &scales)
  : hasMother_(false), type_(type), label_(label), selectionUid_("unselected") {
  if( GlobalParameters::debug() ) {
    std::cout << "DEBUG: Entering DataSet::DataSet()" << std::endl;
    std::cout << "       Creating DataSet '" << label << "'" << std::endl;
  }

  // Sanity checks
  if( uidExists(uid()) ) {
    std::cerr << "\n\nERROR in DataSet::DataSet(): a dataset with label '" << label << "' already exists.\n\n\n" << std::endl;
    exit(-1);
  }

  if( uncLabel.size() > 1 ) {
    for(std::vector<TString>::const_iterator it = uncLabel.begin();
	it != uncLabel.end(); ++it) {
      if( *it == GlobalParameters::defaultUncertaintyLabel() ) {
	std::cerr << "\n\nERROR in DataSet::DataSet(): the dataset with label '" << label << "'" << std::endl;
	std::cerr << "  has more than one sub-uncertainties and a total uncertainty defined!" << std::endl;
	std::cerr << "  \nPer dataset, you may specify *either*" << std::endl;
	std::cerr << "  - a total uncertainty via 'uncertainty: <size>'" << std::endl;
	std::cerr << "  - several sub-uncertainties via 'uncertainty <id1>: <size>; uncertainty <id2>: <size>; ...'\n\n\n" << std::endl;
	exit(-1);
      }
    }
  }

  // Read events from tree
  EventBuilder ebd;
  std::vector<TString>::const_iterator fileIt = fileNames.begin();
  std::vector<double>::const_iterator scaleIt = scales.begin();
  for(; fileIt != fileNames.end(); ++fileIt, ++scaleIt) {
    Events evts = ebd(*fileIt,treeName,weight,uncDn,uncUp,uncLabel,*scaleIt);
    evts_.insert(evts_.end(),evts.begin(),evts.end());
  }

  // Compute yield and uncertainties
  computeYield(uncLabel);

  // Create selected datasets
  for(SelectionIt selIt = Selection::begin();
      selIt != Selection::end(); ++selIt) {

    if( (*selIt)->uid() != "unselected" ) {
      // Create selected dataset and store it
      // in global map of datasets
      DataSet* selectedDataSet = new DataSet(this,(*selIt)->uid(),applySelection(*selIt));
      dataSetUidMap_[selectedDataSet->uid()] = selectedDataSet;
    }
  }


  if( GlobalParameters::debug() ) {
    std::cout << "DEBUG: Leaving DataSet::DataSet()" << std::endl;
  }
}


DataSet::DataSet(const DataSet *ds, const TString &selectionUid, const Events &evts)
  : hasMother_(true), type_(ds->type()), label_(ds->label()), selectionUid_(selectionUid) {
  if( uidExists(uid()) ) {
    std::cerr << "\n\nERROR in DataSet::DataSet(): a dataset with label '" << label_ << "' and selection '" << selectionUid_ << "' already exists." << std::endl;
    exit(-1);
  }

  // Copy uncertainty labels
  std::vector<TString> uncLabels;
  for(std::vector<TString>::const_iterator it = ds->systLabelsBegin();
      it != ds->systLabelsEnd(); ++it) {
    uncLabels.push_back(*it);
  }
  
  // Store events
  for(EventIt it = evts.begin(); it != evts.end(); ++it) {
    evts_.push_back(*it);
  }

  // Compute yield and uncertainties
  computeYield(uncLabels);
}


DataSet::~DataSet() {
  if( !hasMother_ ) {
    for(Events::iterator it = evts_.begin(); it != evts_.end(); ++it) {
      delete *it;
    }
  }
  evts_.clear();
}


// Compute yield (weighted number of events)
// and uncertainties
void DataSet::computeYield(const std::vector<TString> &uncLabel) {
  if( GlobalParameters::debug() ) {
    std::cout << "DEBUG: Entering DataSet::computeYield()" << std::endl;
    std::cout << "       Computing yields and uncertainties for '" << uid() << "'" << std::endl;
  }

  // Set all uncertainty variables to zero
  yield_  = 0.;
  stat_   = 0.;
  totSystDn_ = 0.;
  totSystUp_ = 0.;
  systDn_.clear();
  systUp_.clear();
  for(std::vector<TString>::const_iterator it = systLabelsBegin();
      it != systLabelsEnd(); ++it) {
    systDn_[*it] = 0.;
    systUp_[*it] = 0.;
  }
  systLabels_.clear();

  // Copy uncertainty labels
  for(std::vector<TString>::const_iterator it = uncLabel.begin();
      it != uncLabel.end(); ++it) {
    systLabels_.push_back(*it);
  }
  
  // Loop over events and count yield (sum of event weights)
  // for nominal and varied weights
  for(EventIt evtIt = evts_.begin(); evtIt != evts_.end(); ++evtIt) {
    yield_ += (*evtIt)->weight();
    stat_  += pow((*evtIt)->weight(),2.);
    if( (*evtIt)->hasUnc() ) {
      totSystDn_ += (*evtIt)->weight() * (1.-(*evtIt)->relTotalUncDn());
      totSystUp_ += (*evtIt)->weight() * (1.+(*evtIt)->relTotalUncUp());
      for(std::vector<TString>::const_iterator systIt = systLabelsBegin();
	  systIt != systLabelsEnd(); ++systIt) {
	systDn_[*systIt] += (*evtIt)->weight() * (1.-(*evtIt)->relUncDn(*systIt));
	systUp_[*systIt] += (*evtIt)->weight() * (1.+(*evtIt)->relUncUp(*systIt));
      }
    }
  }

  // Set systematic uncertainty
  if( evts_.size() > 0 && evts_.front()->hasUnc() ) {
    hasSyst_ = true;
    totSystDn_ = yield_-totSystDn_;
    totSystUp_ = totSystUp_-yield_;
    for(std::vector<TString>::const_iterator systIt = systLabelsBegin();
	systIt != systLabelsEnd(); ++systIt) {
      systDn_[*systIt] = yield_ - systDn_[*systIt];
      systUp_[*systIt] = systUp_[*systIt] - yield_;
    }    
  } else {
    hasSyst_ = false;
  }

  // Set statistical uncertainty, depending on dataset type
  // Several cases are distinguished depending on the type of data
  // - 'Data'        : sqrt(number of events)
  // - 'MC'          : sqrt( sum weight^2 ) = MC statistics
  // - 'MCPrediction': sqrt( sum weight^2 ) = MC statistics
  // - 'Signal'      : sqrt( sum weight^2 ) = MC statistics
  // - 'Prediction'  : sqrt( sum weight^2 ) = control sample statistics
  if( type() == Data ) {
    stat_ = sqrt(yield_);
  } else {
    stat_ = sqrt(stat_);
  }

  if( GlobalParameters::debug() ) {
    std::cout << "DEBUG: Leaving DataSet::computeYield()" << std::endl;
  }
}


// ---------------------------------------------------------------
Events DataSet::applySelection(const Selection* sel) const {
  Events passed;
  for(EventIt it = evtsBegin(); it != evtsEnd(); ++it) {
    if( sel->passes(*it,label()) ) passed.push_back(*it);
  }

  return passed;
}




double DataSet::systDn(const TString &label) const {
  double unc = 0.;
  std::map<TString,double>::const_iterator it = systDn_.find(label);
  if( it != systDn_.end() ) {
    unc = it->second;
  }

  return unc;
}

double DataSet::systUp(const TString &label) const {
  double unc = 0.;
  std::map<TString,double>::const_iterator it = systUp_.find(label);
  if( it != systUp_.end() ) {
    unc = it->second;
  }
  
  return unc;
}
