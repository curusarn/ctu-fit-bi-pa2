/** \file main.cpp
 *  Main for backup program.
 *  \author Šimon Let <letsimon@fit.cvut.cz> */

/** \mainpage
 *  PlaceHolder */


#include "cgate.hpp"
#include "cfile.hpp"
#include "cactions.hpp"
#include "cbackup.hpp"
using namespace std;
typedef unsigned int uint;

int main(int argc, char* argv[]){
  
  try{
    CGate user(argc,argv); //Everything related to user - command, switches, prompt 
    user.ShowMsg(CGate::DEBUG,string("Action: ") + user.Cmd());
    user.ShowMsg(CGate::DEBUG,string("Directory: ") + user.Dir());
    user.ShowMsg(CGate::DEBUG,string("Target: ") + user.Target());
    user.ShowMsg(CGate::DEBUG,string("Partial recovery: ") + user.PartialRecovery());
   
    user.ShowMsg(CGate::DEBUG,"Parsing successful");
    
    switch(user.Cmd()){
      case 'h':
	user.ShowMsg(CGate::DEBUG,"Action: Show help");
	CGate::ShowHelp();
	return 0;
      case 'b':
	if(!user.Flag('u')){
	  user.ShowMsg(CGate::DEBUG,"Action: Backup");
	  CBackup action(user);
	  return 0; //New Backup
	}
	else{
	  user.ShowMsg(CGate::DEBUG,"Action: Update");
	  CUpdate action(user);
	  return 0; //Incremental backup
	}
      case 'r':{
	//Recovery
	user.ShowMsg(CGate::DEBUG,"Action: Recover");
	CRecover action(user);
	return 0;}
      case 'l':{
	//List
	user.ShowMsg(CGate::DEBUG,"Action: List");
	CList action(user);
	return 0;}
    }
    
  }catch(int i){
    if(i != 2){
      cerr << "Press any key to show help... (kill me if you don't want help)" << endl;
      cin.ignore();
      CGate::ShowHelp();
    }
    return i;
  }
  return 0; 
}