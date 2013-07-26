#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "TChain.h"
#include "TFile.h"

#include "EventBuilder.h"
#include "Variable.h"


Events EventBuilder::operator()(const TString &fileName, const TString &treeName, const TString &weight, const std::vector<TString> &uncDn, const std::vector<TString> &uncUp, const std::vector<TString> &uncLabel, double scale) const {
  assert( uncDn.size() == uncUp.size() );
  assert( uncDn.size() == uncLabel.size() );

  // Get tree from file
  TChain* chain = new TChain(treeName,treeName);
  chain->Add(fileName);

  // Setup vector with variables to be read from chain
  std::vector<Double_t> varsDouble_t(Variable::nVars(),0.);
  std::vector<Float_t> varsFloat_t(Variable::nVars(),0.);
  std::vector<Int_t> varsInt_t(Variable::nVars(),0);
  std::vector<UInt_t> varsUInt_t(Variable::nVars(),0);
  std::vector<UShort_t> varsUShort_t(Variable::nVars(),0);
  std::vector<UChar_t> varsUChar_t(Variable::nVars(),0);
  unsigned int idxDouble_t = 0;
  unsigned int idxFloat_t = 0;
  unsigned int idxInt_t = 0;
  unsigned int idxUInt_t = 0;
  unsigned int idxUShort_t = 0;
  unsigned int idxUChar_t = 0;

  // Parse weight variable
  Float_t varWeight = 1.;
  if( weight.IsFloat() ) varWeight = weight.Atof();

  // Parse uncertainty source variables
  std::vector<Float_t> varsUncDn(uncDn.size(),0.);
  std::vector<Float_t> varsUncUp(uncUp.size(),0.);
  std::vector<bool> symUnc(uncDn.size(),false);
  for(unsigned int i = 0; i < uncDn.size(); ++i) {
    if( uncDn.at(i).IsFloat() ) varsUncDn.at(i) = uncDn.at(i).Atof();
    if( uncUp.at(i).IsFloat() ) varsUncUp.at(i) = uncUp.at(i).Atof();
    if( uncDn.at(i) == uncUp.at(i) ) symUnc.at(i) = true;
  }

  // Catch to-be-fixed case if weight variable is also used in uncertainty
  if( !weight.IsFloat() ) {
    for(unsigned int i = 0; i < uncDn.size(); ++i) {
      if( (!uncDn.at(i).IsFloat() && uncDn.at(i) == weight) ||
	  (!uncUp.at(i).IsFloat() && uncUp.at(i) == weight)    ) {
	std::cerr << "\n\nERROR: The same tree-variable is used as weight and as uncertainty." << std::endl;
	std::cerr << "       Case needs to be implemented.\n" << std::endl;
	exit(-1);
      }
    }
  }

  // Setup branches
  for(std::vector<TString>::const_iterator it = Variable::begin(); it != Variable::end(); ++it) {
    bool treeHasVar = true;
    if( chain->GetListOfBranches()->FindObject(*it) == 0 ) {
      treeHasVar = false;
      std::cerr << "\nWARNING in EventBuilder" << std::endl;
      std::cerr << "  - TTree '" << treeName << "' in file '" << chain->GetFile()->GetName() << "' has no variable named '" << *it << "'" << std::endl;
      std::cerr << "  - Using default value 0 instead" << std::endl;
    }
    if( Variable::type(*it) == "Float_t" ) {
      if( treeHasVar ) chain->SetBranchAddress(*it,&varsFloat_t.at(idxFloat_t));
      ++idxFloat_t;
      if( *it == weight && treeHasVar ) {
	chain->SetBranchAddress(*it,&varWeight);
      }
      for(unsigned int i = 0; i < uncDn.size(); ++i) {
	if( *it == uncDn.at(i) && treeHasVar ) {
	  chain->SetBranchAddress(*it,&varsUncDn.at(i));
	}
	if( *it == uncUp.at(i) && treeHasVar && !symUnc.at(i) ) {
	  chain->SetBranchAddress(*it,&varsUncUp.at(i));
	}
      }
    } else if( Variable::type(*it) == "Double_t" ) {
      if( treeHasVar ) chain->SetBranchAddress(*it,&varsDouble_t.at(idxDouble_t));
      ++idxDouble_t;
    } else if( Variable::type(*it) == "Int_t" ) {
      if( treeHasVar ) chain->SetBranchAddress(*it,&varsInt_t.at(idxInt_t));
      ++idxInt_t;
    } else if( Variable::type(*it) == "UInt_t" ) {
      if( treeHasVar ) chain->SetBranchAddress(*it,&varsUInt_t.at(idxUInt_t));
      ++idxUInt_t;
    } else if( Variable::type(*it) == "UShort_t" ) {
      if( treeHasVar ) chain->SetBranchAddress(*it,&varsUShort_t.at(idxUShort_t));
      ++idxUShort_t;
    } else if( Variable::type(*it) == "UChar_t" ) {
      if( treeHasVar ) chain->SetBranchAddress(*it,&varsUChar_t.at(idxUChar_t));
      ++idxUChar_t;
    }
  }  

  // Loop over tree and build events
  Events evts;
  for(int i = 0; i < chain->GetEntries(); ++i) {

    // Read variables of this entry
    chain->GetEntry(i);

    // Create new event and fill variables
    Event* evt = new Event(varWeight*scale);
    idxDouble_t = 0;
    idxFloat_t = 0;
    idxInt_t = 0;
    idxUInt_t = 0;
    idxUShort_t = 0;
    idxUChar_t = 0;
    for(std::vector<TString>::const_iterator it = Variable::begin(); it != Variable::end(); ++it) {
      if( Variable::type(*it) == "Double_t" ) {
	evt->set(*it,varsDouble_t.at(idxDouble_t));
	++idxDouble_t;
      } else if( Variable::type(*it) == "Float_t" ) {
	evt->set(*it,varsFloat_t.at(idxFloat_t));
	++idxFloat_t;
      } else if( Variable::type(*it) == "Int_t" ) {
	evt->set(*it,varsInt_t.at(idxInt_t));
	++idxInt_t;
      } else if( Variable::type(*it) == "UInt_t" ) {
	evt->set(*it,varsUInt_t.at(idxUInt_t));
	++idxUInt_t;
      } else if( Variable::type(*it) == "UShort_t" ) {
	evt->set(*it,varsUShort_t.at(idxUShort_t));
	++idxUShort_t;
      } else if( Variable::type(*it) == "UChar_t" ) {
	evt->set(*it,varsUChar_t.at(idxUChar_t));
	++idxUChar_t;
      }
    }

    for(unsigned int i = 0; i < uncDn.size(); ++i) {
      double udn = varsUncDn.at(i);
      double uup = varsUncUp.at(i);
      if( symUnc.at(i) ) uup = udn;
      if( !uncDn.at(i).IsFloat() ) {
	if( varWeight ) {
	  udn = std::abs(varWeight - udn) / varWeight;
	} else {
	  udn = 0.;
	}
      }
      if( !uncUp.at(i).IsFloat() ) {
	if( varWeight ) {
	  uup = std::abs(uup - varWeight) / varWeight;
	} else {
	  uup = 0.;
	}
      }
      evt->addRelUnc(udn,uup,uncLabel.at(i));
    }

    evts.push_back(evt);
  }

  delete chain;

  return evts;
}
