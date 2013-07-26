#ifndef CONFIG_H
#define CONFIG_H

#include <map>
#include <string>
#include <vector>

#include "TString.h"

class Config {
public:
  // List of <name> : <value> pairs. Corresponds to one config line.
  class Attributes {
    friend class Config;

  public:
    Attributes(unsigned int lineNumber) : lineNum_(lineNumber) {};

    unsigned int lineNumber() const { return lineNum_; }
    bool hasName(const TString &name) const { return values_.find(name) != values_.end(); }
    std::vector<TString> listOfNames() const;
    std::vector<TString> listOfNames(const TString &containedStr) const;
    TString value(const TString &name) const;
    bool isBoolean(const TString &name) const;
    bool valueBoolean(const TString &name) const;
    bool isInteger(const TString &name) const;
    int valueInteger(const TString &name) const;
    bool isDouble(const TString &name) const;
    double valueDouble(const TString &name) const;
    unsigned int nValues() const { return values_.size(); }

  private:
    class Value {
    public:
      Value();
      Value(const TString &value);

      TString value() const { return value_; }
      bool isBoolean() const { return isBoolean_; }
      bool valueBoolean() const { return valueBoolean_; }
      bool isInteger() const { return isInteger_; }
      int valueInteger() const { return isInteger() ? value_.Atoi() : 999999; }
      bool isDouble() const { return isDouble_; }
      double valueDouble() const { return isDouble() ? value_.Atof() : 999999.; }
      
      
    private:
      TString value_;
      bool isBoolean_;
      bool valueBoolean_;
      bool isInteger_;
      bool isDouble_;
    };

    unsigned int lineNum_;
    std::map<TString,Value> values_;

    void add(const TString &name, const TString &value) { values_[name] = Value(value); }
  };


  // Tools to modify / parse values
  static bool split(const TString &str, const TString &delim, std::vector<TString> &parts);
  static bool split(const TString &str, const TString &delim, TString &first, TString &second);
  static TString after(const TString &str, const TString &delim);
  static bool enclosed(const TString &str, const TString &delimStart, const TString &delimEnd, TString &encl);
  static int color(const TString &cfg);


  // Each config object parses the config file
  // and stores the <key>::<attributes> pairs
  Config(const TString &fileName);

  std::vector<Attributes> operator()(const TString &key) const;
  TString fileName() const { fileName_; }


private:
  static bool split(const std::string &str, const std::string &delim, std::string &first, std::string &second);
  static void trim(std::string &str);
  static bool isComment(const std::string &line);

  const std::string keyAndAttributesDelimiter_;
  const std::string attributeNameAndValueDelimiter_;
  const TString fileName_;

  std::map< TString,std::vector<Attributes> > keyAttributesMap_;

  Attributes getAttributes(const std::string &line, unsigned int lineNum) const;
};
#endif
