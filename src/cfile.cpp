/** \file cfile.cpp
 *  Implementation for CFile class.
 *  \author Šimon Let <letsimon@fit.cvut.cz> */

#include "cfile.hpp"
#include <iostream>
#include <fstream>
#include <sys/stat.h>

string CFile::sRootPath = "";

CFile::CFile(const string & path):mPath(path){
  struct stat data;
  stat( (sRootPath + mPath).c_str(), &data);
  mCtime = data.st_mtim.tv_sec;
}

struct stat CFile::GetStat()const{
  struct stat data;
  stat( (sRootPath + mPath).c_str(), &data);
  return data;
}

uint32_t CFile::GetSize()const{
  struct stat data;
  stat( (sRootPath + mPath).c_str(), &data);
  return data.st_size; 
}

bool CFile::isNewerThan(const CFile & f){
  return mCtime > f.GetCtime();  
}

bool CFile::isEqual(const string & path){
  struct stat f1, f2;
  stat( (sRootPath + mPath).c_str(), &f1);
  stat( (path).c_str(), &f2);
  return f1.st_ino == f2.st_ino;
}