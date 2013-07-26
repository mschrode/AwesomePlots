#include <iostream>
#include <cstdlib>

#include "Filter.h"
#include "Config.h"
#include "Event.h"
#include "GlobalParameters.h"
#include "Selection.h"
#include "Variable.h"


std::vector<Filter*> Filter::garbage_;
TString Filter::offset_ = "    ";


// ---------------------------------------------------------------
const Filter* Filter::create(const TString &expr, const std::vector<TString> &dataSetLabels, unsigned int lineNum, bool firstIteration, const TString &label) {
  if( GlobalParameters::debug() ) {
    if( firstIteration ) {
      std::cout << "\nBuilding selection '" << label << "'" << std::endl;
      std::cout << "Filter::create(): '" << expr << "'" << std::endl;
    } else {
      std::cout << "Filter::create(): '" << expr << "'" << std::endl;
    }
  }

  // Syntax checks
  if( firstIteration ) {
    checkForDanglingOperators(expr,lineNum);
    checkForMismatchingParentheses(expr,lineNum);
    checkForInvalidBooleanOperators(expr,lineNum);
  }

  const Filter* filter = 0;

  // If this selection is only to be applied to specific datasets
  // create dataset filter
  if( firstIteration && !dataSetLabels.empty() ) {
    filter = new FilterDataSet(create(expr,dataSetLabels,lineNum,false,label),dataSetLabels);
  } else {
    TString cfg = cleanExpression(expr);
    if( GlobalParameters::debug() ) std::cout << "  Cleaned expr: '" << cfg << "'" << std::endl;

    // Check if whole expression is negated
    if( isNegated(cfg) ) {
      if( GlobalParameters::debug() ) std::cout << "  Negated expression: remember NOT" << std::endl;
      filter = new FilterNOT(create(cfg,dataSetLabels,lineNum,false,""));
    } else {

      // Possibly, split expression
      TString expr1 = "";
      TString expr2 = "";
      TString op = "";
      if( isSplit(cfg,expr1,expr2,op) ) {
	const Filter* filter1 = create(expr1,dataSetLabels,lineNum,false,"");
	const Filter* filter2 = create(expr2,dataSetLabels,lineNum,false,"");
	if( op == "AND" ) {
	  filter = new FilterAND(filter1,filter2);
	} else if( op == "OR" ) {
	  filter = new FilterOR(filter1,filter2);
	}
      } else {
	// Loop over previously defined selections
	// and check whether cfg corresponds to one of their labels
	for(SelectionIt itS = Selection::begin(); itS != Selection::end(); ++itS) {
	  if( (*itS)->uid() == cfg ) { // A selection with this name exists; reuse it
	    if( GlobalParameters::debug() ) std::cout << "    Reusing selection '" << cfg << "'" << std::endl;
	    filter = (*itS)->filter();
	    break;
	  }
	}
	if( filter == 0 ) {
	  filter = Cut::create(cfg,lineNum);
	}
      }
    }
  }

  return filter;
}


// ---------------------------------------------------------------
TString Filter::cleanExpression(const TString &expr) {
  TString cfg = expr;
  
  // Remove all spaces
  cfg.ReplaceAll(" ","");

  // Remove parentheses enclosing the whole expression
  bool checkAgain = true;
  while( checkAgain ) {
    unsigned int nOpenBrackets = 0;
    unsigned int pos = 0;
    if( cfg(pos) == '(' ) {
      ++nOpenBrackets;
      ++pos;
      for(; pos < cfg.Length()-1; ++pos) {
	if( cfg(pos) == '(' ) {
	  ++nOpenBrackets;
	} else if( cfg(pos) == ')' ) {
	  --nOpenBrackets;
	}
	if( nOpenBrackets == 0 ) {
	  checkAgain = false;
	  break;
	}
      }
      if( nOpenBrackets > 0 ) {
	cfg = cfg(1,cfg.Length()-2);
      }
    } else {
      checkAgain = false;
    }
  }

  return cfg;
}


// ---------------------------------------------------------------
void Filter::checkForDanglingOperators(const TString &expr, unsigned int lineNum) {
  bool syntaxError = false;
  if( expr.BeginsWith("&") || expr.BeginsWith("|") || expr.BeginsWith(">") || expr.BeginsWith("<") || expr.BeginsWith("=") || expr.BeginsWith("+") || expr.BeginsWith("-") ) syntaxError = true;
  if( expr.EndsWith("&") || expr.EndsWith("|") || expr.EndsWith(">") || expr.EndsWith("<") || expr.EndsWith("=") || expr.EndsWith("+") || expr.EndsWith("-") ) syntaxError = true;
  if( syntaxError ) {
    std::cerr << "\n\nERROR: Wrong syntax when specifying cut in line " << lineNum << std::endl;
    std::cerr << "Invalid cut expression '" << expr << "'" << std::endl;
    std::cerr << "Dangling operator" << std::endl;
    exit(-1);
  }
}


// ---------------------------------------------------------------
void Filter::checkForMismatchingParentheses(const TString &expr, unsigned int lineNum) {
  if( expr.Contains("(") || expr.Contains(")") ) {
    unsigned int nOpenBrackets = 0;
    bool syntaxError = false;
    for(unsigned int pos = 0; pos < expr.Length(); ++pos) {
      if( expr(pos) == '(' ) {
	++nOpenBrackets;
      } else if( expr(pos) == ')' ) {
	if( nOpenBrackets > 0 ) {
	  --nOpenBrackets;
	} else {
	  syntaxError = true;
	  break;
	}
      }
    }
    if( nOpenBrackets > 0 || syntaxError ) {
      std::cerr << "\n\nERROR: Wrong syntax when specifying cut in line " << lineNum << std::endl;
      std::cerr << "Mismatching parentheses in cut expression '" << expr << "'" << std::endl;
      exit(-1);
    }
  }
}


// ---------------------------------------------------------------
void Filter::checkForInvalidBooleanOperators(const TString &expr, unsigned int lineNum) {
  bool syntaxError = false;
  for(unsigned int pos = 0; pos < expr.Length(); ++pos) {
    if( expr(pos) == '&' ) {
      if( expr(pos+1) == '&' ) {
	++pos;
      } else {
	syntaxError = true;
	break;
      }
    }
    if( expr(pos) == '|' ) {
      if( expr(pos+1) == '|' ) {
	++pos;
      } else {
	syntaxError = true;
	break;
      }
    }
  }
  if( syntaxError ) {
    std::cerr << "\n\nERROR: Wrong syntax when specifying cut in line " << lineNum << std::endl;
    std::cerr << "Invalid cut expression '" << expr << "'" << std::endl;
    std::cerr << "Contains incomplete boolean operator '&' or '|'" << std::endl;
    exit(-1);
  }
}


// ---------------------------------------------------------------
bool Filter::isNegated(TString &expr) {
  bool isNOT = false;
  if( expr(0) == '!' ) {
    if( expr.Contains("&&") || expr.Contains("||") ) { // It's not a simple cut; check if NOT refers to the whole, composite expression, i.e. if expr is !(..)
      if( expr(1) == '(' && expr(expr.Length()-1) == ')' ) {
	unsigned int nOpenBrackets = 1;
	bool bracketsClosedBeforeEndOfExpr = false;
	for(unsigned int pos = 2; pos < expr.Length()-1; ++pos) {
	  if( expr(pos) == '(' ) {
	    ++nOpenBrackets;
	  } else if( expr(pos) == ')' ) {
	    --nOpenBrackets;
	  }
	  if( nOpenBrackets == 0 ) { // !( does not span the whole expression
	    bracketsClosedBeforeEndOfExpr = true;
	    break;
	  }
	}
	if( !bracketsClosedBeforeEndOfExpr ) {
	  isNOT = true;
	  expr = expr(1,expr.Length()-1); // Remove '!'
	  expr = cleanExpression(expr); // Expression is enclosed in parentheses	    
	}
      } 
    } else {			// It's a sinlge cut, and it's negated
      isNOT = true;
      expr = expr(1,expr.Length()-1); // Remove '!'
      expr = cleanExpression(expr); // Expression might possibly be enclosed in parentheses
    }
  }

  return isNOT;
}


// ---------------------------------------------------------------
bool Filter::isSplit(const TString &expr, TString &expr1, TString &expr2, TString &op) {
  bool split = false;
  unsigned int nOpenBrackets = 0;
  for(unsigned int pos = 0; pos < expr.Length(); ++pos) {
    // Check for expressions in brackets
    if( expr(pos) == '(' ) {
      ++nOpenBrackets;
    } else if( expr(pos) == ')' ) {
      if( nOpenBrackets > 0 ) {
	--nOpenBrackets;
      }
    }
    // If no bracketed expression, check for boolean operators
    if( nOpenBrackets == 0 ) {
      if( expr(pos) == '&' ) { // Case: two expressions, AND
	if( expr(pos+1) == '&' ) {
	  split = true;
	  op = "AND";
	  expr1 = expr(0,pos);
	  pos = pos+2;
	  expr2 = expr(pos,expr.Length()-pos);
	  if( GlobalParameters::debug() ) {
	    std::cout << "  Split into: '" << expr1 << "'  &&  '" << expr2 << "'" << std::endl;
	  }
	}
	break;
      }
      if( expr(pos) == '|' ) { // Case: two expressions, OR
	if( expr(pos+1) == '|' ) {
	  split = true;
	  op = "OR";
	  expr1 = expr(0,pos);
	  pos = pos+2;
	  expr2 = expr(pos,expr.Length()-pos);
	  if( GlobalParameters::debug() ) {
	    std::cout << "  Split into: '" << expr1 << "'  ||  '" << expr2 << "'" << std::endl;
	  }
	}
	break;
      }
    }
  }

  return split;
}



// Use this method to delete all existing selections
// Reused selections are treated correctly
// ---------------------------------------------------------------
void Filter::clear() {
  for(std::vector<Filter*>::iterator it = garbage_.begin();
      it != garbage_.end(); ++it) {
    delete *it;
  }
}


// ---------------------------------------------------------------
Cut* Cut::create(const TString &expr, unsigned int lineNum) {
  if( GlobalParameters::debug() ) std::cout << "    Cut::create(): '" << expr << "'" << std::flush;

  TString cfg = cleanExpression(expr);

  Cut* cut = 0;

  std::vector<TString> compOps;
  compOps.push_back(">=");
  compOps.push_back(">");
  compOps.push_back("<=");
  compOps.push_back("<");
  compOps.push_back("==");
  compOps.push_back("!=");

  std::vector<TString> parts;
  TString op;
  // Split into VAR, VAL, and comparison operator
  for(std::vector<TString>::const_iterator oit = compOps.begin();
      oit != compOps.end(); ++oit) {
    if( cfg.Contains(*oit) ) {
      Config::split(cfg,*oit,parts);
      op = *oit;
      break;
    }
  }

  // Syntax check
  if( parts.size() == 0 ) {
    std::cerr << "\n\nERROR: Wrong syntax when specifying cut in line " << lineNum << std::endl;
    std::cerr << "Cut expression '" << expr << "' contains none" << std::endl;
    std::cerr << "of the valid comparison operators ('>','>=','<','<=','==','!=')" << std::endl;
    exit(-1);
  }
  if( parts.size() == 1 || parts.size() > 3 ) {
    std::cerr << "\n\nERROR: Wrong syntax when specifying cut in line " << lineNum << std::endl;
    std::cerr << "Invalid cut expression '" << expr << "'" << std::endl;
    exit(-1);
  }
  if( parts.size() == 3 && !( op == ">" || op == ">=" || op == "<" || op == "<=" ) ) {
    std::cerr << "\n\nERROR: Wrong syntax when specifying cut in line " << lineNum << std::endl;
    std::cerr << "Invalid cut expression '" << expr << "'" << std::endl;
    exit(-1);
  }

  if( parts.size() == 2 ) {
    TString var;
    double val;
    if( Variable::exists(parts.at(0)) && parts.at(1).IsFloat() ) {
      var = parts.at(0);
      val = parts.at(1).Atof();
    } else if( Variable::exists(parts.at(1)) && parts.at(0).IsFloat() ) {
      var = parts.at(1);
      val = parts.at(0).Atof();
      if(      op == "<"  ) op = ">";
      else if( op == "<=" ) op = ">=";
      else if( op == ">"  ) op = "<";
      else if( op == ">=" ) op = "<=";
    } else {
      std::cerr << "\n\nERROR: Wrong syntax when specifying cut in line " << lineNum << std::endl;
      std::cerr << "Invalid cut expression '" << parts.at(0) << "  " << op << "  " << parts.at(1) << "'" << std::endl;
      if( !(Variable::exists(parts.at(0)) && Variable::exists(parts.at(0))) ) {
	std::cerr << "No known variables specified" << std::endl;
      } else if( !(parts.at(0).IsFloat() && parts.at(1).IsFloat()) ) {
	std::cerr << "No valid numbers specified" << std::endl;
      }
      exit(-1);
    }
    if(      op == ">"  ) cut = new CutGreaterThan(var,val); 
    else if( op == ">=" ) cut = new CutGreaterEqualThan(var,val); 
    else if( op == "<"  ) cut = new CutLessThan(var,val); 
    else if( op == "<=" ) cut = new CutLessEqualThan(var,val); 
    else if( op == "==" ) cut = new CutEqual(var,val); 
    else if( op == "!=" ) cut = new CutNotEqual(var,val); 
  } else {
    TString var = parts.at(1);
    double val1 = parts.at(0).Atof();
    double val2 = parts.at(2).Atof();
    if( !Variable::exists(var) ) {
      std::cerr << "\n\nERROR: Wrong syntax when specifying cut in line " << lineNum << std::endl;
      std::cerr << "Invalid cut expression '" << expr << "'" << std::endl;
      std::cerr << "Variable '" << var << "' does not exist" << std::endl;
    } else if( !(parts.at(0).IsFloat() && parts.at(2).IsFloat()) ) {
      std::cerr << "\n\nERROR: Wrong syntax when specifying cut in line " << lineNum << std::endl;
      std::cerr << "Invalid cut expression '" << expr << "'" << std::endl;
      std::cerr << "'" << parts.at(0) << "' and '" << parts.at(2) << "' are no numbers" << std::endl;
    }
    if(      op == "<"  ) cut = new CutLessThanLessThan(val1,var,val2);
    else if( op == "<=" ) cut = new CutLessEqualThanLessEqualThan(val1,var,val2);
    else if( op == ">"  ) cut = new CutLessThanLessThan(val2,var,val1);
    else if( op == ">=" ) cut = new CutLessEqualThanLessEqualThan(val2,var,val1);
  }

  return cut;
}



// ---------------------------------------------------------------
CutGreaterThan::CutGreaterThan(const TString &var, double val)
  : Cut("") {
  var_ = var;
  val_ = val;
  uid_ = var_+" > ";
  uid_ += val_;

  if( GlobalParameters::debug() ) std::cout << " (CutGreaterThan)" << std::endl;
}


// ---------------------------------------------------------------
CutGreaterEqualThan::CutGreaterEqualThan(const TString &var, double val)
  : Cut("") {
  var_ = var;
  val_ = val;
  uid_ = var_+" >= ";
  uid_ += val_;

  if( GlobalParameters::debug() ) std::cout << " (CutGreaterEqualThan)" << std::endl;
}


// ---------------------------------------------------------------
CutLessThan::CutLessThan(const TString &var, double val)
  : Cut("") {
  var_ = var;
  val_ = val;
  uid_ = var_+" < ";
  uid_ += val_;

  if( GlobalParameters::debug() ) std::cout << " (CutLessThan)" << std::endl;
}


// ---------------------------------------------------------------
CutLessEqualThan::CutLessEqualThan(const TString &var, double val)
  : Cut("") {
  var_ = var;
  val_ = val;
  uid_ = var_+" <= ";
  uid_ += val_;

  if( GlobalParameters::debug() ) std::cout << " (CutLessEqualThan)" << std::endl;
}


// ---------------------------------------------------------------
CutEqual::CutEqual(const TString &var, double val)
  : Cut("") {
  var_ = var;
  val_ = val;
  uid_ = var_+" == ";
  uid_ += val_;

  if( GlobalParameters::debug() ) std::cout << " (CutEqual)" << std::endl;
}


// ---------------------------------------------------------------
CutNotEqual::CutNotEqual(const TString &var, double val)
  : Cut("") { 
  var_ = var;
  val_ = val;
  uid_ = var_+" != ";
  uid_ += val_;

  if( GlobalParameters::debug() ) std::cout << " (CutNotEqual)" << std::endl;
}


// ---------------------------------------------------------------
CutLessThanLessThan::CutLessThanLessThan(double val1, const TString &var, double val2)
  : Cut("") {
  var_ = var;
  val_ = val1;
  val2_ = val2;
  uid_ = "";
  uid_ += val_;
  uid_ += " < "+var_+" < ";
  uid_ += val2_;

  if( GlobalParameters::debug() ) std::cout << " (CutLessThanLessThan)" << std::endl;
}


// ---------------------------------------------------------------
bool CutLessThanLessThan::passes(const Event* evt, const TString &dataSetLabel) const {
  double x = evt->get(var_);
  
  return x > val_ && x < val2_;
}



// ---------------------------------------------------------------
CutLessEqualThanLessEqualThan::CutLessEqualThanLessEqualThan(double val1, const TString &var, double val2)
  : Cut("") {
  var_ = var;
  val_ = val1;
  val2_ = val2;
  uid_ = "";
  uid_ += val_;
  uid_ += " <= "+var_+" <= ";
  uid_ += val2_;

  if( GlobalParameters::debug() ) std::cout << " (CutLessEqualThanLessEqualThan)" << std::endl;
}


// ---------------------------------------------------------------
bool CutLessEqualThanLessEqualThan::passes(const Event* evt, const TString &dataSetLabel) const {
  double x = evt->get(var_);
  
  return x >= val_ && x <= val2_;
}



// ---------------------------------------------------------------
BooleanOperator::BooleanOperator(const Filter* filter1, const Filter* filter2, const TString &name)
  : Filter("["+filter1->uid()+"] "+name+" ["+filter2->uid()+"]"),
    filter1_(filter1), filter2_(filter2), name_(name) {

  if( GlobalParameters::debug() ) std::cout << "    BooleanOperator::BooleanOperator() '" << uid() << "'" << std::flush;
}


// ---------------------------------------------------------------
TString BooleanOperator::printOut() const {
  TString txt = "";
  offset_+="|";
  txt += offset_+"-- "+name_+"\n";
  txt += offset_+"    |\n";  
  offset_ += "    ";
  txt += filter1_->printOut()+"\n";
  txt += filter2_->printOut();
  offset_.Chop();
  offset_.Chop();
  offset_.Chop();
  offset_.Chop();
  offset_.Chop();

  return txt;
}


// ---------------------------------------------------------------
FilterAND::FilterAND(const Filter* filter1, const Filter* filter2)
  : BooleanOperator(filter1,filter2,"AND") {
  if( GlobalParameters::debug() ) std::cout << " (AND)" << std::endl;
}


// ---------------------------------------------------------------
bool FilterAND::passes(const Event* evt, const TString &dataSetLabel) const {
  if( filter1_->passes(evt,dataSetLabel) ) {
    if( filter2_->passes(evt,dataSetLabel) ) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}


// ---------------------------------------------------------------
FilterOR::FilterOR(const Filter* filter1, const Filter* filter2)
  : BooleanOperator(filter1,filter2,"OR") {
  if( GlobalParameters::debug() ) std::cout << " (OR)" << std::endl;
}


// ---------------------------------------------------------------
bool FilterOR::passes(const Event* evt, const TString &dataSetLabel) const {
  if( filter1_->passes(evt,dataSetLabel) ) {
    return true;
  } else if( filter2_->passes(evt,dataSetLabel) ) {
    return true;
  } else {
    return false;
  }
}


// ---------------------------------------------------------------
FilterNOT::FilterNOT(const Filter* filter)
  : Filter("NOT["+filter->uid()+"]"), filter_(filter) {
  if( GlobalParameters::debug() ) std::cout << "    FilterNOT::FilterNOT() '" << uid() << "'" << std::endl;
}


// ---------------------------------------------------------------
FilterDataSet::FilterDataSet(const Filter* filter, const std::vector<TString> &applyToDataSets)
  : Filter("FilterDataSet"), filter_(filter), applyToDataSets_(applyToDataSets) {
  if( GlobalParameters::debug() ) std::cout << "    FilterDataSet::FilterDataSet() (" << applyToDataSets_.front() << std::flush;
  for(std::vector<TString>::const_iterator it = applyToDataSets_.begin()+1;
      it != applyToDataSets_.end(); ++it) {
    std::cout << ", " << *it;
  }
  std::cout << ")" << std::endl;
}



// ---------------------------------------------------------------
TString FilterDataSet::printOut() const { 
  TString txt = offset_+"(";
  for(std::vector<TString>::const_iterator it = applyToDataSets_.begin();
      it != applyToDataSets_.end(); ++it) {
    txt += (*it)+", ";
  }
  txt.Chop();
  txt.Chop();
  txt += ")";

  return txt;
}


// ---------------------------------------------------------------
bool FilterDataSet::passes(const Event* evt, const TString &dataSetLabel) const {
  bool applyFilter = false;
  for(std::vector<TString>::const_iterator it = applyToDataSets_.begin();
      it != applyToDataSets_.end(); ++it) {
    if( *it == dataSetLabel ) {
      applyFilter = true;
      break;
    }
  }

  return applyFilter ? filter_->passes(evt,dataSetLabel) : true;
}


