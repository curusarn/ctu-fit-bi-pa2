/** \file cactions.hpp
 *  Header for CActions class.
 *  \author Šimon Let <letsimon@fit.cvut.cz> */

#ifndef CACTIONS_546876510674565132
#define CACTIONS_546876510674565132

#include <map>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include "cgate.hpp"
#include "cfile.hpp"

/// I hate how long "unsigned int" is
typedef unsigned int uint;

/// Class that contains methods for work with \a CFile, \a CExtendedFile and archive file.
/** \a CBackup, \a CUpdate and \a CRecover inherits from this class and use it's methods
 *  to perform backup. 
 */
class CActions{
private:
  const static uint32_t FILE      = 0xFFFFFFFF;
  const static uint32_t EMPTY     = 0xEEEEEEEE;
  const static uint32_t DIRECTORY = 0xDDDDDDDD;
  bool mArchiveFound;
  
  uint getFiles(map<string,CFile> & files, const string & path);
  CExtendedFile getNextFileHead();
  bool getUINT32( uint32_t & data);
  bool putUINT32( uint32_t data);
  bool getString( string & str, uint len);
  bool putString( const string & str);
  
  bool getFileData(const CExtendedFile & file);
  bool putFileData(const CFile & file);
  
  bool createFilePath(const string & path){ return createPath(path) && createFile(path); }
  bool createPath(const string & aPath);
  bool createFile(const string & path);
  
  int fileExists (const string & str)const;
  bool fileIsRequested(const string & path)const;
  
  void cropFilePath(CExtendedFile & file);
protected:
  /// Object for comunication with user
  CGate & mUser;
  /// Stream to be opened on target archive
  fstream mStream;
  //CActions(){};
  //FS
  /// Scans for files to archive
  /** \return map with \a CFile objects found 
   */
  map<string,CFile> FS2Map();
  
  //Stream
  /// Checks if target archive FileExists
  /** \return 1 when regular file exists, 0 when deosn't, -1 otherwise */
  inline int FileExists(){ return fileExists(mUser.Target()); }
  /// Creates target archive
  /** \return True on success */
  bool CreateFile();
  /// Opens stream on target archive
  /** \return True on succes */
  bool OpenStream();
  /// Puts next file \a f to target archive
  void PutNextFile(const CFile & f);
  /// Gets next file from target archive
  /** Skipping all disabled files */
  CExtendedFile GetNextFile();
  /// Marks file to be skipped in target archive in future
  /** Goes to \a start of prvious file in \a mStream,
   *  changes it's "title" so that it gets skipped in future,
   *  goes back to previous stream position. 
   */
  void DisablePrevFile(streampos start);
  /// Closes stream on target archive
  /** \return True on succes */
  bool CloseStream();
  
  //Recovery
  /// Creates next file from target archive
  bool MakeNextFile();
  
public:
  /// Ctor - sets \a CGate attribute
  CActions(CGate & user);
  
};







#endif //CACTIONS_546876510674565132