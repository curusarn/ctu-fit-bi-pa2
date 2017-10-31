/** \file cfile.hpp
 *  Header for CFile and CExtendedFile classes.
 *  \author Šimon Let <letsimon@fit.cvut.cz> */

#ifndef CFILE_36869486848896543543654568644
#define CFILE_36869486848896543543654568644

#include <string>
#include <sys/stat.h>


using namespace std;
/// I hate how long "unsigned int" is
typedef unsigned int uint;

/// Basic class that represents file
/** Used for storing info about all found files */
class CFile{
protected:
  /// Path prefix to use when accessing files 
  static string sRootPath;
  /// Path to file
  string mPath;
  /// Change time of file
  uint32_t mCtime;
public:
  /// Simple ctor - sets the attributes 
  CFile(uint ctime = 0, const string & path = ""):mPath(path),mCtime(ctime){}
  /// Regular ctor - gets change time from stat()
  CFile(const string & path);

  /// Gets path prefix
  /** \return \a sRootPath */
  static string & GetRoot(){ return sRootPath; }
  /// Sets directory to perform action on
  /** Called once by \a CAction to set path prefix obtained from \a CGate object (from user) */
  static void SetRoot(const string & path){ sRootPath = path; }
  
  /// \return Path to file
  const string & GetPath()const{ return mPath; }
  /// \return Last change time
  uint32_t GetCtime()const{ return mCtime; }
  /// \return Result of stat() call on file
  struct stat GetStat()const;
  /// \return Filesize obtained from stat() call
  uint32_t GetSize()const;
    
  /// Returns true when \a CFile represents EOF
  bool isEOF()const{ return !mCtime; }
  
  /// Compares change time attributes of two \a CFile instances
  bool isNewerThan(const CFile & f);
  /// Compares inodes of \a CFile and file on given path
  bool isEqual(const string & path);
};

/// Extended class that represents files
/** Used for stroing info about file loaded from archive */
class CExtendedFile : public CFile{
  const uint32_t mSize;
  const uint32_t mType;
  const streampos mStart;
public:
  /// Ctor for creating file that represents EOF
  CExtendedFile():mSize(0),mType(0xFFFFFFFF){}
  /// Basic ctor - sets the attributes
  CExtendedFile(uint32_t type, const string & path, uint32_t ctime, uint32_t size, streampos & start):CFile(ctime,path),mSize(size),mType(type),mStart(start){}  
  /// Gets type of file
  /** \return Number that determines filetype */
  uint32_t GetType()const{ return mType; }
  /// Gets filesize
  /** \return Saved filesize unlike in \a CFile where stat is called  */
  uint32_t GetSize()const{ return mSize; }
  /// Gets stream position of file in target archive
  /** \return Saved stream position of file in target archive */
  streampos GetStart()const{ return mStart; }
  /// Sets filepath to \a path
  void SetPath(const string & path){ CFile::mPath = path; }
};

#endif //CFILE_36869486848896543543654568644