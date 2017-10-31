/** \file cbackup.hpp
 *  Header for CActions child classes - CBackup, CUpdate a CRecover.
 *  \author Šimon Let <letsimon@fit.cvut.cz> */


#ifndef CBACKUP_4549568745687456986548956547468
#define CBACKUP_4549568745687456986548956547468

#include "cactions.hpp"
//using namespace std;

/// Performes non-incremental backup
/** Uses methods of \a CActions.
 */
class CBackup : public CActions{
public:
  /// Performes non-incremental backup 
  /** Ctor - calls \a CActions ctor and uses it's methods
   *  \throw Exception if something goes wrong. 
  */
  CBackup(CGate & user);
};

/// Performes incremental backup
/** Uses methods of \a CActions.
 */
class CUpdate : public CActions{
public:
  /// Performes incremental backup
  /** Ctor - calls \a CActions ctor and uses it's methods
   *  \throw Exception if something goes wrong. 
   */
  CUpdate(CGate & user);
};

/// Performes recovery
/** Uses methods of \a CActions.
 */
class CRecover : public CActions{
public:
  /** Ctor - calls \a CActions ctor and uses it's methods
   *  \throw Exception if something goes wrong. 
   */
  CRecover(CGate & user);
};

/// Lists files from archive
/** Uses methods of \a CActions.
 */
class CList : public CActions{
public:
  /** Ctor - calls \a CActions ctor and uses it's methods
   *  \throw Exception if something goes wrong. 
   */
  CList(CGate & user);
};



#endif //CBACKUP_4549568745687456986548956547468