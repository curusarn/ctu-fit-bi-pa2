/** \file cactions.cpp 
 * Impementation for CActions child classes - CBackup, CUpdate a CRecover.
 *  \author Šimon Let <letsimon@fit.cvut.cz> */

#include "cactions.hpp"

#include <list>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <sys/stat.h>

#include "cfile.hpp"

using namespace std;

// recursively adds reguler files and empty directories to map
uint CActions::getFiles(map<string,CFile> & files, const string & dirPath){
  uint fileCnt = 0, regFileCnt = 0;
  DIR * dir;
  struct dirent *file;
  dir = opendir( (string(mUser.Dir()) + dirPath).c_str() );
  if (dir)
  {  
    while ((file = readdir(dir)) != NULL)
    {
      string filePath = dirPath + "/" + file->d_name;
      switch(file->d_type){
	case DT_DIR:
	  if( string(file->d_name) == "." || string(file->d_name) == "..") break;
	  mUser.ShowMsg(CGate::DEBUG,string("Expanding directory \"") + mUser.Dir()+filePath + '"');
	  regFileCnt += getFiles(files,filePath);
	  fileCnt++;
	  break;
	case DT_REG:{
	  CFile tmpFile(filePath);
	  if(!mArchiveFound && tmpFile.isEqual(mUser.Target())){
	    mArchiveFound = true;
	    break;    
	  }
	  mUser.ShowMsg(CGate::DEBUG,string("Indexing regular file \"") + mUser.Dir()+filePath + '"');
	  files.insert(make_pair(filePath, tmpFile));
	  fileCnt++;
	  regFileCnt++;
	  break;}
	default:
	  mUser.ShowMsg(CGate::WARN,string("Unhandled filetype \"") + mUser.Dir()+filePath + '"');
      }
    }
    closedir(dir);
    if(!fileCnt){
      mUser.ShowMsg(CGate::DEBUG,string("Indexing empty directory \"") + mUser.Dir()+dirPath + '"');
      files.insert(make_pair(dirPath, CFile(dirPath)));
    }
  }
  else{
    mUser.ShowMsg(CGate::FAIL,string("Directory not found \"") + mUser.Dir()+dirPath + '"');
    throw 1;
    //files.insert(make_pair(dirPath, CFile(dirPath)));
  }
  return regFileCnt;
}

// returns filedata loaded from header in archive
CExtendedFile CActions::getNextFileHead(){
  while(true){
    uint32_t head, ctime, pathSize, dataSize;
    string path;
    streampos start;
    start = mStream.tellg();
    if(!getUINT32(head)){
      if(mStream.eof()) mStream.clear();
      mUser.ShowMsg(CGate::DEBUG,"End of archive reached");
      return CExtendedFile();
    }
    getUINT32(ctime);
    getUINT32(dataSize);
    getUINT32(pathSize);
    
    getString(path,pathSize);
    if(head == EMPTY){
      mStream.seekg(dataSize, ios::cur);
      mUser.ShowMsg(CGate::DEBUG,string("Skipping disabled file - const: ") + to_string(head) + " ctime: " + to_string(ctime) + " size: " + to_string(dataSize) + " pathLength: " + to_string(pathSize));
      continue;
    }
    mUser.ShowMsg(CGate::DEBUG,string("Getting file info - const: ") + to_string(head) + " ctime: " + to_string(ctime) + " size: " + to_string(dataSize) + " pathLength: " + to_string(pathSize));
    return CExtendedFile(head,path,ctime,dataSize,start);
  }  
}

bool CActions::getUINT32( uint32_t & data ){
  return mStream.read((char *) &data, 4 );
}
bool CActions::putUINT32( uint32_t data){
  return mStream.write((char *) &data, 4 );
}

bool CActions::getString( string & str, uint len){
  char * tmp = new char[len];
  bool ret;
  ret = mStream.read( tmp, len ); 
  str.assign(tmp, len);
  delete [] tmp;
  return ret;
}
bool CActions::putString( const string & str){
  return mStream.write((char *) str.c_str(), str.length() );  
}

// gets filedata from archive and writes it into file, creates file and path if needed 
bool CActions::getFileData(const CExtendedFile & file){
  char * buff = new char[10000];
  uint bytesLeft = file.GetSize(), cnt = 10000;
  string newPath = mUser.Dir() + file.GetPath();
  createFilePath(newPath);
  ofstream out;
  out.open(newPath, ios::binary);
  while(bytesLeft){
    if(bytesLeft < 10000) cnt = bytesLeft;
    mStream.read( buff, cnt);
    out.write( buff, cnt);
    bytesLeft -= cnt;
  }
  out.close();
  delete [] buff;
  return mStream.good() && out.good();
}

// puts data from file to archive
bool CActions::putFileData(const CFile & file){
  ifstream in;
  in.open(mUser.Dir() + file.GetPath(), ios::binary);
  mStream << in.rdbuf();
  in.close();
  return in.good();
}

bool CActions::createPath(const string & aPath){
  istringstream ss(aPath);
  string dirPath = "";
  struct stat data;
  bool ret = true;
  
  while(true){
    string tmp;
    getline(ss,tmp,'/');
    dirPath += tmp;
    if(!ss.good()) break;
    if(stat( dirPath.c_str(), &data)) ret = ret && !mkdir(dirPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    else if(!S_ISDIR(data.st_mode)) return false;   
    
    dirPath += "/";
  }
  return ret;
}

bool CActions::createFile(const string & aPath){
  int fe = fileExists(aPath);
  if(fe){
    if(fe == -1 || !mUser.Flag('f')){
      mUser.ShowMsg(CGate::WARN,string("Unable to create file \"") + aPath + "\" - file exists (maybe try -f (force) flag)"); 
      return false;
    }
    mUser.ShowMsg(CGate::NOTICE,string("Overwriting file \"") + aPath + "\" (-f (force) flag is set)"); 
    remove(aPath.c_str());
  }
  else mUser.ShowMsg(CGate::INFO,string("Creating file \"") + aPath + '\"'); 
  ofstream out(aPath);
  if(!out.is_open()){
    mUser.ShowMsg(CGate::WARN,string("Failed to create file \"") + aPath + '\"'); 
    return false;
  }
  out.close();
  if(!out.good()){
    mUser.ShowMsg(CGate::WARN,string("Failed to create file \"") + aPath + '\"'); 
    return false;    
  }
  mUser.ShowMsg(CGate::DEBUG,string("File \"") + aPath + "\" created"); 
  return true;  
}  

int CActions::fileExists (const string & str)const {
  struct stat data;
  if(stat(str.c_str(), &data) == 0){
    if(S_ISREG(data.st_mode)) return 1;
    else return -1;    
  }
  return 0;
}

// true if path belongs to partial recovery path or if -p not specified 
bool CActions::fileIsRequested(const string & path)const{
  const string & part = mUser.PartialRecovery();
  if(part == "") return true;
  if(path.length() < part.length()) return false;
  for(uint i = 0; i < part.length(); i++)
    if(path[i] != part[i]) return false;
  return true;
}

// erases partial recovery path from filepath  
void CActions::cropFilePath(CExtendedFile & file){
  const string & part = mUser.PartialRecovery();
  if(part == "") return;
  string path = file.GetPath(); 
  uint len = path.length() - part.length();
  if(len){
    file.SetPath(path.substr(part.length(),string::npos));
    mUser.ShowMsg(CGate::DEBUG,"Classic crop");
    return;    
  }
  else
    for(uint i = 1; i <= path.length() ; i++)
      if(path[path.length()-i] == '/'){
	file.SetPath(path.substr(part.length()-i,string::npos));
	mUser.ShowMsg(CGate::DEBUG,"Advanced crop");
	return;
      }
  mUser.ShowMsg(CGate::DEBUG,"Uncropped");
}

// =============== PUBLIC ==================
// =============== PUBLIC ==================
// =============== PUBLIC ==================

// returns map of files found in filesystem
map<string,CFile> CActions::FS2Map(){
  map<string,CFile> files;
  uint regCnt;
  mUser.ShowMsg(CGate::NOTICE,"Looking for files to archive"); 
  if( (regCnt = getFiles(files,"")) ) mUser.ShowMsg(CGate::INFO,string("Files found: ") + to_string(regCnt) );
  else mUser.ShowMsg(CGate::FAIL,"No files found");
  return files;
}

bool CActions::CreateFile(){
  mUser.ShowMsg(CGate::NOTICE,"Creating archive");  
  if(!createFilePath(mUser.Target())){
    mUser.ShowMsg(CGate::FAIL,"Creating archive failed");
    throw 1;
    return false;    
  }
  return true;
}

bool CActions::OpenStream(){
  mUser.ShowMsg(CGate::NOTICE,"Openning archive");  
  mStream.open(mUser.Target(), ios::in | ios::out | ios::binary);
  if(mStream.is_open()) return true;
  CActions::mUser.ShowMsg(CGate::FAIL,"Openning archive failed");  
  throw 1;
  return false;
}

// puts file into archive
void CActions::PutNextFile(const CFile & f){
  struct stat data = f.GetStat(); 
  uint32_t head;
  uint siz = data.st_size;
  if(S_ISREG(data.st_mode)) head = FILE;
  else if(S_ISDIR(data.st_mode)){
    head = DIRECTORY;
    siz = 0;
  }
  else mUser.ShowMsg(CGate::FAIL,"Unexpected unknown file in map - dev's fault");
  putUINT32(head);
  putUINT32(f.GetCtime());
  putUINT32(siz);
  putUINT32(f.GetPath().length());
  
  putString(f.GetPath());
  
  if(head == FILE) mUser.ShowMsg(CGate::INFO,string("Adding regular file \"") + mUser.Dir()+f.GetPath() + "\" to archive"); 
  else if(head == DIRECTORY) mUser.ShowMsg(CGate::DEBUG,string("Adding directory \"") + mUser.Dir()+f.GetPath() + "\" to archive");
  
  if(siz) putFileData(f.GetPath());
}

// gets file header and skips filedata
CExtendedFile CActions::GetNextFile(){
  CExtendedFile file = getNextFileHead();
  mStream.seekg(file.GetSize(), ios::cur);
  return file;
}

// rewinds to start of previous file and marks it as disabled
void CActions::DisablePrevFile(streampos start){
  streampos back = mStream.tellg();
  mStream.seekg(start);
  putUINT32(EMPTY);
  mStream.seekg(back);
}

bool CActions::CloseStream(){
  mStream.close();
  if(!mStream.good()){
    mUser.ShowMsg(CGate::FAIL,"Writing into archive failed");  
    throw 1;
  }
  mUser.ShowMsg(CGate::NOTICE,"Archive closed successfully");  
  return true;
}

// recovers next file
bool CActions::MakeNextFile(){
  CExtendedFile file = getNextFileHead();
  if(file.isEOF()) return false;
  if(!fileIsRequested(file.GetPath())){
    mUser.ShowMsg(CGate::DEBUG,string("Skipping file \"") + mUser.Dir()+file.GetPath() + "\"");  
    return true;
  }
  else cropFilePath(file);
  if(file.GetType() == FILE) return getFileData(file);
  if(file.GetType() == DIRECTORY){
    mUser.ShowMsg(CGate::DEBUG,string("Creating directory \"") + mUser.Dir()+file.GetPath() + "\"");  
    if(!createPath(mUser.Dir() + file.GetPath() + "/x")){
      mUser.ShowMsg(CGate::WARN,string("Creating directory \"") + mUser.Dir()+file.GetPath() + "\" failed");  
      return false;
    }
    return true;
  }
  mUser.ShowMsg(CGate::FAIL,string("Corrupted archive or dev's falut ") + to_string(file.GetType()));
  throw 1;   
}

CActions::CActions(CGate & user):mArchiveFound(false),mUser(user){
  CFile::SetRoot(mUser.Dir());
}
