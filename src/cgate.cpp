/** \file cgate.cpp
 *  Implementation for CGate class.
 *  \author Šimon Let <letsimon@fit.cvut.cz> */

#include <cstring>
#include "cgate.hpp"
using namespace std;

#define NO_EXTRA false

typedef unsigned int uint;

CGate::CGate(int argc, char* argv[]):mCmd(' '){
  if(argc == 1){
    ShowMsg(FAIL,"Syntax error - No arguments");
    throw 0;
  }
  for(int i = 1; i < argc; i++){
    int sl = strlen(argv[i]);
    char c;
    ShowMsg(DEBUG,"Loading next option");
    for(int j = 0; j < sl; j++){
      c = tolower(argv[i][j]); 
      ShowMsg(DEBUG,string("Loading \"") + c + "\"");
      if(!j){
	if(c == '-') continue;
	ShowMsg(FAIL,string("Syntax error - Options have to start with \"-\" ") + c + " found instead");
	throw 1; // syntax err - no '-'
      }
      
      switch(c){
	case 'h': //help
	case 'b': //backup
	case 'r': //recover
	case 'l': //list
	  setAction(c);
	  break;
	case 'u': //update - incremental backup
	case 'i': //interactive
	case 'f': //force
	//case 'a': //all files
	  setArgOn(c);
	  break;
	case 'v': //verbouse
	case 'w':
	  if(j+1 < sl && isdigit(argv[i][j+1])) setArgVal(c,argv[i][++j]-'0'); 
	  else if(j+1 == sl && i+1 != argc && argv[i+1][0] != '-') setArgVal(c,argv,i); 
	  else if(c == 'v') setArgVal(c,5); // no value - default -v value
	  else ShowMsg(ERR,string("Syntax error - No digit after -w options"));
	  break;
	case 's': //silent
	  setArgVal('v',1);
	  break;  
	case 'd': //root dir
	case 't': //target file
	case 'p': //partial recovery
	  if(j+1 != sl || i+1 == argc){
	    ShowMsg(FAIL,string("Syntax error - No path after \"") + c + "\" option");
	    throw 1; // syntax err - no path
	  }
	  loadPath(c,argv,i);
	  break;
	default:
	  if(!isalpha(c)){
	    ShowMsg(FAIL,string("Syntax error - \"") + c + "\" is not a letter");
	    throw 1; // syntax err - switch is not a letter
	  }
	  ShowMsg(WARN,string("Syntax warninig - Unknow option \"-") + c + '\"');
      }      
    }
  }
  
  int w;
  if((w = Param('w'))){
    time_t rawtime;
    tm * timeinfo;
    time(&rawtime);
    timeinfo=localtime(&rawtime);
    int weekday=timeinfo->tm_wday;
    if(!weekday) weekday = 7;
    if(weekday != w){
      ShowMsg(WARN,"Weekday doesn't match"); 
      throw 2;
    }
  }
  if(Flag('i') && Param('v')) interactiveOpt();
  checkUp();
  setDefaults();
}

void CGate::setAction(char c){
  if(mCmd != ' '){
    ShowMsg(FAIL,"Syntax error - Multiple \"action\" (-brh) options"); 
    throw 1;
  }
  mCmd = c;
}

void CGate::setArgOn(char c){
  uint s = mFlags.size();
  mFlags.insert(c);
  if(s == mFlags.size()) ShowMsg(WARN,string("Syntax warninig - Repeating \"-") + c + "\" option");
}
void CGate::setArgVal(char c, int x){
  uint s = mParams.size();
  mParams[c] = x;
  if(s == mParams.size()) ShowMsg(WARN,string("Syntax warninig - Repeating \"-") + c + "\" option");
}
void CGate::setArgVal(char c, char* argv[], int & i){ 
  i++;
  if(!isdigit(argv[i][0])) ShowMsg(WARN,string("Syntax warninig - Value for option \"-") + c + "\" is not a digit"); 
  uint s = mParams.size();
  mParams[c] = argv[i][0]-'0';
  if(s == mParams.size()) ShowMsg(WARN,string("Syntax warninig - Repeating \"-") + c + "\" option");
  if(strlen(argv[i]) > 1){
    ShowMsg(WARN,string(string("Syntax warninig - Value for option \"-") + c + "\" is not in range from 0 to 9"));
    mParams[c] = 9;
  }
  
}

void CGate::loadPath(char c,  char* argv[], int & i){
  uint s = mPaths.size();
  string & path = mPaths[c] = argv[++i];
  if(s == mPaths.size()){
    ShowMsg(FAIL,string("Syntax error - Repeating \"-") + c + "\" option");
    throw 1;
  }
  if(path[path.length()-1] == '/') path.pop_back(); // get rid of last char if equals "/"
  if(c == 'p' && path[0] != '/') path.insert(0,1,'/'); // add "/" to 0th position if not
}

bool CGate::isFlag(char c){
  switch(c){
    case 'f':
    case 'a':
      return true;
    default:
      return false;    
  }  
}


void CGate::interactiveOpt(){
  if(Cmd() == ' '){ //
    map<char,string> actions;
    actions['H'] = "Help";
    actions['b'] = "Backup (standard)";
    actions['u'] = "Backup (incremental)";
    actions['r'] = "Recovery";  
    //
    int x = AskMsgOptList(FAIL,"Action to perform?",actions);
    auto it = actions.begin();
    advance(it,x);
    char c = it->first;
    ShowMsg(WARN,string("Action was set to \"") + (char)tolower(c) + " - " + actions[c] + '\"',NO_EXTRA);
    //
    if(c == 'u'){
      c = 'b';
      mFlags.insert('u');
    } 
    mCmd = tolower(c);
  }
  if(mCmd == 'h') return;
  if(!mPaths.count('d')) //
    if(!AskMsgOpt(WARN,"Set working directory?","Yn")){
      string s = AskMsgStr(WARN,"Type in worknig directory");
      if(s != ""){
	mPaths['d'] = s;
	ShowMsg(WARN,string("Working directory set to \"") + s + '\"',NO_EXTRA);
      }
      else ShowMsg(FAIL,"Failed to set working directory - current directory will be used",NO_EXTRA);
    }
  if(!mPaths.count('t')) //
    if(!AskMsgOpt(WARN,"Set target archive file?","Yn")){
      string s = AskMsgStr(WARN,"Type in path to target file");
      if(s != ""){
	mPaths['t'] = s; 
	ShowMsg(WARN,string("Target archive file set to \"") + s + '\"',NO_EXTRA);
      }
      else ShowMsg(FAIL,"Failed to set terget archive - default will be used",NO_EXTRA);
    }
  if(mCmd == 'r' && !mPaths.count('p')) //
    if(!AskMsgOpt(WARN,"Set directory for partial recovery?","Yn")){
      string s = AskMsgStr(FAIL,"Type in directory");
      if(s != ""){
	mPaths['p'] = s;
	ShowMsg(WARN,string("Directory set to \"") + s + '\"',NO_EXTRA);
      }
      else ShowMsg(FAIL,"Failed to set directory - whole archive will be ",NO_EXTRA);
    }
  if(!mParams.count('v')) 
    if(!AskMsgOpt(WARN,"Set verbosity?","Yn")){
      map<char,string> verbOpt;
      verbOpt['0'] = "NOTHING";
      verbOpt['1'] = "FAIL (silent)";
      verbOpt['2'] = "ERROR";
      verbOpt['3'] = "WARNING (default)";
      verbOpt['4'] = "NOTICE";
      verbOpt['5'] = "INFO (verbouse)";
      verbOpt['6'] = "DEBUG";
      int as = AskMsgOptList(FAIL,"Type in verbosity level",verbOpt);
      if(as == -1) as = 3;
      mParams['v'] = as;
      char c = (char)(as+'0');
      ShowMsg(WARN,string("Verbiousness level set to \"") + c + " - " + verbOpt[c] + '\"',NO_EXTRA);
    }    
  if(!AskMsgOpt(WARN,"Set other options?","Yn")){
    ShowMsg(WARN,"Possible options are:",NO_EXTRA);
    map<char,string> posOpt;
    posOpt['f'] = "force";
    ShowOptList(FAIL,posOpt,NO_EXTRA);
    string s = AskMsgStr(FAIL,"Type in options (all at once, eg. \"fa\")");
    for(uint i = 0; i < s.length(); i++){
      if(isFlag(s[i])){
	mFlags.insert(s[i]);
	ShowMsg(WARN,string("\"-") + s[i] + "\" flag was set",NO_EXTRA);
      }
    }
  }
}

void CGate::checkUp(){
  if(mCmd == ' '){
    ShowMsg(FAIL,"No action set");
    throw 1;    
  }
  string defaultTarget;
  if(!mPaths.count('d') && mCmd != 'l'){
    defaultTarget = "backup.dat";
    ShowMsg(WARN,"Warning - root directory is not set - \".\" will be used"); 
  }
  else defaultTarget = mPaths['d'] + ".dat"; 
  if(!mPaths.count('t')) ShowMsg(WARN,string("Warning - target archive file is not set - \"") + defaultTarget + "\" will be used"); 
  
}

void CGate::setDefaults(){
  mPaths.insert(make_pair('d', "."));
  if(mPaths['d'] == ".") mPaths.insert(make_pair('t', "backup.dat")); 
  else mPaths.insert(make_pair('t', mPaths['d'] + ".dat"));
  mPaths.insert(make_pair('p', ""));
  
  //mParam.emplace('v', 3);
}


int CGate::getColor(int verb)const{
  switch(verb){
    case 1: //fail
      return 31;
    case 2: //err
      return 31;
    case 3: //warn
      return 33;
    case 4: //notice
      return 39;
    case 5: //info
      return 39;
    case 6: //debug
      return 90;
    default:
      return 39;
  }
}

// ======================================= Public =======================================
// ======================================= Public =======================================
// ======================================= Public =======================================

bool CGate::ShowMsg(int msgVerb, const string & s, bool showExtra)const{
  ostream & os = cerr;
  int setVerb = Param('v');
  if(msgVerb > setVerb) return false;
  os << "\033[" << getColor(msgVerb) << "m";
  if(setVerb > 5) os << msgVerb << ':';
  os << s;    
  if(showExtra && msgVerb == 1) os << " - Program will exit now!";
  os << "\033[39m" << endl;
  return true;
}

bool CGate::ShowOptList(int msgVerb, const map<char,string> & opt, bool showExtra)const{
  for(auto it : opt)
    if(!ShowMsg(msgVerb,string("  ") + (char) tolower(it.first) + " - " + it.second,showExtra)) return false;
  return true;
}

int CGate::AskMsgOpt(int msgVerb, const string & msg, const string & opt)const{
  string optExpand = " (", optDesription = " (";
  int defaultOpt = -1;
  for(uint i = 0; i < opt.length(); i++){
    char c = opt[i];
    if(c != tolower(c)) defaultOpt = i;
    if(i){
      optExpand += '/';
      optDesription += '/';
    }
    switch(tolower(c)){
      case 'y':
	optDesription += "yes";
	break;
      case 'n':
	optDesription += "no";
	break;
      case 'a':
	optDesription += "yes to all";
	break;
      case 'd':
	optDesription += "no, don\'t ask again";
	break;
      default:
	ShowMsg(DEBUG,string("Internal error - ") + c + " has no meaning - Dev\'s falut");
	continue;
    }
    optExpand += c;
  }
  optExpand += ")";
  optDesription += ")";
  
  if(!ShowMsg(msgVerb,msg + optExpand + optDesription,NO_EXTRA)) return defaultOpt;
  
  char c;
  string s;
  getline(cin,s);
  c = (s.length()) ? s[0] : ' ';
  if(c == ' ') return defaultOpt;
  for(uint i = 0; i < opt.length(); i++)
    if(tolower(opt[i]) == tolower(c)) return i;
  return defaultOpt;
}

int CGate::AskMsgOpt(int msgVerb, const string & msg, const map<char,string> & opt)const{
  string optExpand = " (", optDesription = " (";
  int defaultOpt = -1;
  for(auto it = opt.begin(); it != opt.end(); it++){
    char c = it->first;
    const string & s = it->second; 
    if(c != tolower(c)) defaultOpt = distance(opt.begin(),it);
    if(it != opt.begin()){
      optExpand += '/';
      optDesription += '/';
    }
    optExpand += c;
    optDesription += s;
  }
  optExpand += ")";
  optDesription += ")";
  
  if(!ShowMsg(msgVerb,msg + optExpand + optDesription,NO_EXTRA)) return defaultOpt;
  
  string s;
  char c;
  getline(cin,s);
  c = (s.length()) ? s[0] : ' ';
  if(c == ' ') return defaultOpt;
  auto it = opt.find(c);
  if(it != opt.end()) return distance(opt.begin(),it);
  return defaultOpt;
}

int CGate::AskMsgOptList(int msgVerb, const string & msg, const map<char,string> & opt)const{
  string optExpand = " (";
  int defaultOpt = -1;
  for(auto it = opt.begin(); it != opt.end(); it++){
    char c = it->first;
    if(c != tolower(c)) defaultOpt = distance(opt.begin(),it);
    if(it != opt.begin())
      optExpand += '/';
    optExpand += c;
  }
  optExpand += ")";
  
  if(!ShowMsg(msgVerb,msg + optExpand,NO_EXTRA)) return defaultOpt;
  ShowOptList(msgVerb,opt,NO_EXTRA);
  
  string s; 
  char c;
  getline(cin,s);
  c = (s.length()) ? s[0] : ' ';
  if(c == ' ') return defaultOpt;
  auto it = opt.find(c);
  if(it != opt.end()) return distance(opt.begin(),it);
  return defaultOpt;  
}

string CGate::AskMsgStr(int msgVerb, const string & msg)const{
  string s;
  if(!ShowMsg(msgVerb, msg,NO_EXTRA)) return "";
  getline(cin,s);
  return s;
}

char CGate::AskMsgChar(int msgVerb, const string & msg)const{
  string s = AskMsgStr(msgVerb, msg);
  char c = (s.length()) ? s[0] : ' ';
  return c;  
}
  
void CGate::ShowHelp(){
  ostream & os = cerr;
  string RS = "    ";
  os << "Showing help" << endl
  << endl << "BRIEF" << endl
  << RS << "Backup all the files! is software made by Šimon Let that makes backups - obviously." << endl
  << endl << "SYNOPSIS" << endl
  << RS << "letsimon ACTION [PATHS] [FLAGS]" << endl
  
  << endl << RS << "ACTIONS, PATHS and FLAGS can be mixed - order is up to you" << endl
  << RS << "Using  many arguments is fine except for PATHS - one of a kind is allowed" << endl
  
  << endl << "USAGE" << endl
  << RS << "ACTIONS" << RS << "format: -%c" << endl
  << RS << "Use one of following arguments:" << endl
  << RS << "-h Show this help" << endl
  << RS << "-b Backup" << endl
  << RS << RS << "Performes backup of files specified after option -d to archive after option -t" << endl
  << RS << RS << "Add -u for incremental backup of -f for enabling overwriting of files" << endl
  << RS << "-r Recover" << endl
  << RS << RS << "Recovers files from archive specified after option -t to directory after option -d" << endl
  << RS << RS << "Add -p %s to specify directory/file to recover from archive - more info in PATHS section below" << endl
  << RS << "-l List archive content" << endl
  << RS << RS << "Lists content of archive specified after option -t" << endl
  << RS << RS << "Useful before running recovery with option -p" << endl
  
  << endl << RS << "PATHS" << RS << "format: -%c %s" << endl
  << RS << "-d Root directory" << endl
  << RS << RS << "Path to directory considered root for all performed actions - used as prefix" << endl
  << RS << RS << "Default: \".\"" << endl
  << RS << "-t Target archive" << endl
  << RS << RS << "Path to archive" << endl
  << RS << RS << "Default: \"<root dir>.dat\" except if <root dir> == \".\" then \"backup.dat\" is used" << endl
  << RS << "-p Partial recovery path" << endl
  << RS << RS << "Path relative to archive's root specifing directory/file that user wants to recover" << endl
  << RS << RS << "Using -l action to get proper path is strongly recommended!" << endl
  << RS << RS << "Default: \"\" All files will be recovered" << endl
  
  << endl << RS << "VERBOSITY" << RS << "format: -v %d OR -v%d" << endl
  << RS << "Verbiosity levels determine which messages are printed" << endl
  << RS << "There are 7 verbiosity levels" << endl
  << RS << RS << "0 - x - No messages have this vebosity" << endl
  << RS << RS << "1 - FAIL - Everything went wrong (-s silent)" << endl
  << RS << RS << "2 - ERR - Something went wrong" << endl
  << RS << RS << "3 - WARN - Something could go wrong (default)" << endl
  << RS << RS << "4 - NOTICE - Important messages" << endl
  << RS << RS << "5 - INFO - Normal messages (-v verbouse)" << endl
  << RS << RS << "6 - DEBUG - Debug info" << endl
  << RS << "Message is printed if <verbosity of message> is less or equal <verbosity set by user>" << endl
  << RS << "You can use -s (same as -v1), -v (same as -v5) or leave out -v argument for default value: 3" << endl
  << RS << "There is an exception in messages usage when prforming -l action - file listings have WARN verbiosity because file listing is main functionality" << endl
  << RS << "Verbosity takes effect imidately - you can show/hide parsing messages by modifing verbiosity on begining and/or end of commandline arguments" << endl
  
  << endl << RS << "OTHER OPTIONS" << endl
  << RS << "-i Interactive mode" << endl
  << RS << RS << "Asks for not specified arguments - experimental" << endl
  << endl << RS << "-w %d OR -w%d Weekdays filter" << endl
  << RS << RS << "Performes action only if day of the week equals passed argument - (1 - Monday .. 7 - Sunday)" << endl
  
  << endl << "OPTIONS" << endl
  << RS << "-b Backup (ACTION)" << endl
  << RS << RS << "Performes backup of files specified after option -d to archive after option -t" << endl
  << RS << "-d Root directory (PATH)" << endl
  << RS << RS << "Path to directory considered root for all performed actions - used as prefix" << endl
  << RS << "-f Force (FLAG)" << endl
  << RS << RS << "Overwrites files if colisions occure" << endl
  << RS << "-h Show this help (ACTION)" << endl
  << RS << "-i Interactive mode (FLAG)" << endl
  << RS << RS << "Asks for not specified arguments - experimental" << endl
  << RS << "-l List archive content (ACTION)" << endl
  << RS << RS << "Lists content of archive specified after option -t" << endl
  << RS << "-p Partial recovery path (PATH)" << endl
  << RS << RS << "Path relative to archive's root specifing directory/file that user wants to recover" << endl
  << RS << "-r Recover (ACTION)" << endl
  << RS << RS << "Recovers files from archive specified after option -t to directory after option -d" << endl
  << RS << "-s Silent (VERBOSITY/FLAG)" << endl
  << RS << RS << "Prints just FAIL and ERROR messages (sets verbosity to 2 - ERR)" << endl
  << RS << "-t Target archive (PATH)" << endl
  << RS << RS << "Path to archive" << endl
  << RS << "-u Update (FLAG)" << endl
  << RS << RS << "Makes incremental backup" << endl
  << RS << "-v Verbouse (VERBOSITY/FLAG)" << endl
  << RS << RS << "Prints all messages except for DEBUG ones (sets verbosity to 5 - INFO)" << endl
  << RS << "-v %d OR -v%d Verbosity (VERBOSITY)" << endl
  << RS << RS << "Prints all messages more important than %d (sets verbosity to %d)" << endl
  << RS << "-w %d OR -w%d Weekdays filter" << endl
  << RS << RS << "Performes action only if day of the week equals passed argument - (1 - Monday .. 7 - Sunday)" << endl 
  
  << endl << "EXAMPLES" << endl
  << RS << "Cause syntax error to make program fail and display help - already did? Good job!" << endl
  << RS << "letsimon" << endl
  << RS << "make run" << endl
  
  << endl << RS << "Backup contents of directory ./examples/whatever.d to ./examples/backup1.dat" << endl
  << RS << "letsimon -bd ./examples/whatever.d -t ./examples/backup1.dat" << endl
  
  << endl << RS << "Overwrite previous ./examples/backup1.dat backup with backup of directory ./examples/dir" << endl
  << RS << "letsimon -bfd ./examples/whatever.d -t ./examples/backup1.dat" << endl
  
  << endl << RS << "Update previous ./examples/backup1.dat backup with backup of directory ./examples/dir" << endl
  << RS << "letsimon -bud ./examples/whatever.d -t ./examples/backup1.dat" << endl
  
  << endl << RS << "Recover all files from ./examples/backup1.dat backup to directory ./examples/recovery" << endl
  << RS << "letsimon -rd ./examples/recovery -t ./examples/backup1.dat" << endl
  
  << endl << RS << "List files from ./examples/backup1.dat" << endl
  << RS << "letsimon -lt ./examples/backup1.dat" << endl
  
  << endl << RS << "Recover files from path <archive>/some/directory from ./examples/backup1.dat backup to directory ./examples/recovery" << endl
  << RS << "letsimon -rd ./examples/recovery -t ./examples/backup1.dat -p /some/directory" << endl
  
  << endl << RS << "Run program in interactive mode - asks for all missing arguments - experimental" << endl
  << RS << "letsimon -i" << endl 
  
  << endl << "TROUBLESHOOTING" << endl
  << RS << "Unable to create file - file exists" << endl 
  << RS << RS << "Add -f to enable overwriting files or" << endl
  << RS << RS << "Add -u if you wish to make an incremental backup" << endl
  
  << endl << RS << "Partial recovery doesn't seem to work" << endl 
  << RS << RS << "You have to specify (-p) path relatively to ARCHIVE" << endl
  << RS << RS << "Running -l action will show you right paths to files" << endl
  
  
  << endl;
}

