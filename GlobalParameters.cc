#include <cstdlib>
#include <iostream>
#include <sys/stat.h>

#include "GlobalParameters.h"


// CVS Information; will be substituted by cvs
TString GlobalParameters::CVSKeyWordRevision_ = "$Revision: 1.12 $";
TString GlobalParameters::CVSKeyWordName_ = "$Name:  $";

bool GlobalParameters::debug_ = false;
TString GlobalParameters::lumi_ = "";
GlobalParameters::PublicationStatus GlobalParameters::publicationStatus_ = GlobalParameters::Internal;
TString GlobalParameters::id_ = "Plot";
TString GlobalParameters::inputPath_ = "";
bool GlobalParameters::outputEPS_ = false;
bool GlobalParameters::outputPNG_ = false;
bool GlobalParameters::outputPDF_ = false;


void GlobalParameters::init(const Config &cfg, const TString &key) {
  std::cout << "  Setting global parameters...  " << std::flush;
  std::vector<Config::Attributes> attrList = cfg(key);
  for(std::vector<Config::Attributes>::const_iterator it = attrList.begin();
      it != attrList.end(); ++it) {
    if( it->hasName("debug") ) debug_ = it->isBoolean("debug") ? debug_ = it->valueBoolean("debug") : debug_ = false;
    if( it->hasName("id") ) id_ = it->value("id");
    if( it->hasName("lumi") ) lumi_ = it->value("lumi");
    if( it->hasName("input path") ) {
      inputPath_ = it->value("input path");
      if( !(inputPath_.EndsWith("/")) ) inputPath_ += "/";
    }
    if( it->hasName("output formats") ) {
      std::vector<TString> formats;
      Config::split(it->value("output formats"),",",formats);
      for(std::vector<TString>::const_iterator itf = formats.begin();
	  itf != formats.end(); ++itf) {
	TString format = *itf;
	format.ToLower();
	format.ReplaceAll(".",""); 
	if(      format == "eps" ) outputEPS_ = true;
	else if( format == "png" ) outputPNG_ = true;
	else if( format == "pdf" ) outputPDF_ = true;
	else {
	  std::cerr << "    \nWARNING: unknown or unsupported output format '" << *itf << "' defined in line " << it->lineNumber() << std::endl;
	  std::cerr << "    Will be ignored" << std::endl;
	}
      }
    }
    if( it->hasName("publication status") ) {
      TString status = it->value("publication status");
      status.ToLower();
      if( status == "preliminary" ) publicationStatus_ = Preliminary;
      else if( status == "public" ) publicationStatus_ = Public;
      else if( status == "internal" ) publicationStatus_ = Internal;
      else {
	publicationStatus_ = Internal;
	std::cerr << "    \nWARNING: unknown publication status '" << it->value("publication status") << "' defined in line " << it->lineNumber() << std::endl;
	std::cerr << "    Using default status 'Internal'" << std::endl;
      }
    }
  }

  // Check values
  if( !outputEPS() && !outputPNG() && !outputPDF() ) outputPDF_ = true;	// Make pdf default output format

  std::cout << "ok" << std::endl;

  std::cout << "  Preparing the environment...  " << std::flush;
  mkdir("results",S_IRWXU);
  mkdir(("results/"+analysisId()).Data(),S_IRWXU);

  std::cout << "ok" << std::endl;
}


TString GlobalParameters::cvsRevision() {
  TString rev = CVSKeyWordRevision_;
  rev.ReplaceAll(" ","");
  rev.ReplaceAll("$","");
  rev.ReplaceAll("Revision:","");
  
  return rev;
}


TString GlobalParameters::cvsTag() {
  TString name = CVSKeyWordName_;
  name.ReplaceAll(" ","");
  name.ReplaceAll("$","");
  name.ReplaceAll("Name:","");
  
  return name;
}




