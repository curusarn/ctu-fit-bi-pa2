/** \file cbackup.cpp
 *  Impementation for CActions child classes - CBackup, CUpdate a CRecover.
 *  \author Šimon Let <letsimon@fit.cvut.cz> */

#include "cactions.hpp"
#include "cbackup.hpp"
#include "cfile.hpp"
#include "cgate.hpp"
#include <map>

using namespace std;


// ============ BACKUP ==============
// ============ BACKUP ==============
// ============ BACKUP ==============

CBackup::CBackup(CGate & user):CActions(user){
  CActions::CreateFile();
  CActions::OpenStream();
    
  map<string,CFile> files = CActions::FS2Map();
   
  for(auto f : files)
    CActions::PutNextFile(f.second);
  
  CActions::CloseStream();
}

// ============= UPDATE =============
// ============= UPDATE =============
// ============= UPDATE =============

CUpdate::CUpdate(CGate & user):CActions(user){
  if(!FileExists()){
    CActions::mUser.ShowMsg(CGate::FAIL,"Archive file does not exist");  
    throw 1;    
  }
  CActions::OpenStream();
  
  map<string,CFile> files = CActions::FS2Map(); 
    
  CActions::mUser.ShowMsg(CGate::NOTICE,"Looking for already archived files");  
  while(true){
    CExtendedFile fileS = CActions::GetNextFile();
    if(fileS.isEOF()) break;
    
    CActions::mUser.ShowMsg(CGate::DEBUG,string("Checking file ") + mUser.Dir()+fileS.GetPath());  
    
    auto it = files.find(fileS.GetPath());
    if(it == files.end()){
      CActions::mUser.ShowMsg(CGate::DEBUG,string("File \"") + mUser.Dir()+fileS.GetPath() + "\" was not found in filesystem"); 
      continue;
    }
    CFile & fileM = it->second;
    
    if(fileM.isNewerThan(fileS)){
      CActions::mUser.ShowMsg(CGate::DEBUG,string("Deleting file \"") + mUser.Dir()+fileS.GetPath() + "\" from archive - will be updated"); 
      CActions::DisablePrevFile(fileS.GetStart());
    }
    else{
      CActions::mUser.ShowMsg(CGate::DEBUG,string("File \"") + mUser.Dir()+fileS.GetPath() + "\" is already in archive"); 
      files.erase(it);
    }
  } 
  
  if(files.size()){
    CActions::mUser.ShowMsg(CGate::INFO,string("Files to update: ") + to_string(files.size()));
  
    for(auto f : files)
      CActions::PutNextFile(f.second);
  }
  else CActions::mUser.ShowMsg(CGate::WARN,"No files to update"); 
    
  CActions::CloseStream();
}

// ============= RECOVER ============
// ============= RECOVER ============
// ============= RECOVER ============

CRecover::CRecover(CGate & user):CActions(user){
  if(!FileExists()){
    CActions::mUser.ShowMsg(CGate::FAIL,"Archive file does not exist");  
    throw 1;    
  }
  CActions::OpenStream();
  
  uint cnt = 0;
  while(MakeNextFile()) cnt++;
  
  if(!cnt) CActions::mUser.ShowMsg(CGate::WARN,"Archive is empty"); 
  else CActions::mUser.ShowMsg(CGate::INFO,string("Files in archive: ") + to_string(cnt)); 
  
  CActions::CloseStream();
}

// =============  LIST  =============
// =============  LIST  =============
// =============  LIST  =============

CList::CList(CGate & user):CActions(user){
  if(!FileExists()){
    CActions::mUser.ShowMsg(CGate::FAIL,"Archive file does not exist");  
    throw 1;    
  }
  CActions::OpenStream();
  
  CActions::mUser.ShowMsg(CGate::NOTICE,"Listing archived files:");  
  uint fileCnt = 0;
  while(true){
    CExtendedFile fileS = CActions::GetNextFile();
    if(fileS.isEOF()) break;
    fileCnt++;
    CActions::mUser.ShowMsg(CGate::WARN,string("<archive>") + fileS.GetPath());  
  } 
  
  if(fileCnt){
    CActions::mUser.ShowMsg(CGate::NOTICE,string("Files found: ") + to_string(fileCnt));
  }
  else CActions::mUser.ShowMsg(CGate::WARN,"No files found"); 
  
  CActions::CloseStream();
}