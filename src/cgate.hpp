/** \file cgate.hpp
 *  Header for CGate class.
 *  \author Šimon Let <letsimon@fit.cvut.cz> */

#ifndef CGATE_1654654896456416546548964564
#define CGATE_1654654896456416546548964564

#include <string>
#include <map>
#include <set>
#include <iostream>
#include <stdexcept>
using namespace std;

/// Class that takes care of all input/output tasks
/** Contains all arguments from commandline.
 *  Checks syntax and validity of input, is rather benevolent.
 *  Shows messages and questions.
 */
class CGate{
public:
  /* Severity
   * 0 - x - Nothing
   * 1 - FAIL - Everything went wrong (-s)
   * 2 - ERR - Something went wrong
   * 3 - WARN - Something could go wrong (default)
   * 4 - NOTICE - Important messages
   * 5 - INFO - Normal messages (-v)
   * 6 - DEBUG - Debug info
   */
  /// Severity level 1 - used when passing messages to console   
  const static int FAIL = 1;
  /// Severity level 2 - used when passing messages to console   
  const static int ERR = 2;
  /// Severity level 3 - used when passing messages to console   
  const static int WARN = 3;
  /// Severity level 4 - used when passing messages to console   
  const static int NOTICE = 4;
  /// Severity level 5 - used when passing messages to console   
  const static int INFO = 5;
  /// Severity level 6 - used when passing messages to console   
  const static int DEBUG = 6;
  
private:
  // parsed data
  char mCmd;
  map<char,string> mPaths; 
  /// Opitons loaded from commandline
  set<char> mFlags;
  /// Parameters loaded from commandline
  map<char,int> mParams;
  
  /// Sets action to \a c
  void setAction(char c);
  /// Turns on \a c option
  void setArgOn(char c);
  /// Sets parameter \a c to \a x
  void setArgVal(char c, int x);
  /// Sets parameter \a c to values from 0 to 9.
  /** \throw exception if \a argv[\a i + 1] is not a number from 0 to 9 */
  void setArgVal(char c, char* argv[], int & i); 
  /// Sets path \a c to value of next argument on commandline 
  void loadPath(char c,  char* argv[], int & i);
  /// \return True if \a c is valid commandline option
  bool isFlag(char c);
  /// Runs interactive options input
  void interactiveOpt();
  /// Checks if commandline arguments are satisfactory
  void checkUp();
  /// Sets unset values to defaults
  void setDefaults();
  /// \return Number for console colors
  int getColor(int verb)const;
public:
  /// Parses commandline arguments
  CGate(int argc, char* argv[]);
  CGate();
  
  /// Gets action loaded from commandline
  /** \return Action loaded from commandline */
  char Cmd()const{return mCmd;}
  /// Gets directory to perform acton on
  /** \return Directory to perform acton on */
  const string & Dir()const{return mPaths.at('d');}
  /// Gets target archive to use for action
  /** \return Target archive to use for action */
  const string & Target()const{return mPaths.at('t');}
  /// Gets path to part of archive to recover
  /** \return Path to part of archive to recover */
  const string & PartialRecovery()const{return mPaths.at('p');}
  /// Checks if option \a c is set
  /** \return True if option \a c is set */
  bool Flag(char c)const{return mFlags.count(c);}
  /// Gets value of parameter \a c
  /** \return value of parameter \a c 
   *  Takes care of default parameter values
   */
  int Param(char c)const{
    try{ return mParams.at(c);
    }catch(out_of_range & e){
      if(c == 'v') return 3; // Verbose default value;
      else return 0;
    }
  }
  
  /// Shows message if verbosity of message (\a msgVerb) is lower than verbosity set by user 
  /** \return True if message was shown */
  bool ShowMsg(int msgVerb, const string & s, bool showExtra = true)const;
  /// Shows list of options if verbosity of message (\a msgVerb) is lower than verbosity set by user
  /** \return True if list was shown */
  bool ShowOptList(int msgVerb, const map<char,string> & opt, bool showExtra = true)const;
  /// Asks question with classic answer options (y/n) if verbosity of message (\a msgVerb) is lower than verbosity set by user
  /** \return Answer if obtained otherwise default value is returned */
  int AskMsgOpt(int msgVerb, const string & msg, const string & options)const;
  /// Asks question with custom options if verbosity of message (\a msgVerb) is lower than verbosity set by user
  /** \return Answer if obtained otherwise default value is returned */
  int AskMsgOpt(int msgVerb, const string & msg, const map<char,string> & opt)const;
  /// Asks question with custom options displayed in list if verbosity of message (\a msgVerb) is lower than verbosity set by user
  /** \return Answer if obtained otherwise default value is returned */
  int AskMsgOptList(int msgVerb, const string & msg, const map<char,string> & opt)const;
  /// Asks question expectiong string as answer if verbosity of message (\a msgVerb) is lower than verbosity set by user 
  /** \return Answer if obtained otherwise default value is returned */
  string AskMsgStr(int msgVerb, const string & msg)const;
  /// Asks question expectiong char as answer if verbosity of message (\a msgVerb) is lower than verbosity set by user
  /** \return Answer if obtained otherwise default value is returned */
  char AskMsgChar(int msgVerb, const string & msg)const;
  /// Shows help
  static void ShowHelp();
};

#endif //CGATE_1654654896456416546548964564
