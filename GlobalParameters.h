#ifndef GLOBAL_PARAMETERS_H
#define GLOBAL_PARAMETERS_H

#include "TString.h"

#include "Config.h"


class GlobalParameters {
public:
  enum PublicationStatus { Internal, Preliminary, Public };

  static void init(const Config &cfg, const TString &key);

  static bool debug() { return debug_; }
  static PublicationStatus publicationStatus() { return publicationStatus_; }
  static TString lumi() { return lumi_; }
  static TString analysisId() { return id_; }
  static TString defaultUncertaintyLabel() { return "syst. uncert."; }
  static TString inputPath() { return inputPath_; }
  static bool outputEPS() { return outputEPS_; }
  static bool outputPNG() { return outputPNG_; }
  static bool outputPDF() { return outputPDF_; }

  static TString cvsRevision();
  static TString cvsTag();

private:
  static TString CVSKeyWordRevision_;
  static TString CVSKeyWordName_;
  static bool debug_;
  static TString lumi_;
  static PublicationStatus publicationStatus_;
  static TString id_;
  static TString inputPath_;
  static bool outputEPS_;
  static bool outputPNG_;
  static bool outputPDF_;
};
#endif
