/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmFileTimeComparison.h,v $
  Language:  C++
  Date:      $Date: 2006/03/15 16:02:01 $
  Version:   $Revision: 1.2 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmFileTimeComparison_h
#define cmFileTimeComparison_h

#include "cmStandardIncludes.h"

class cmFileTimeComparisonInternal;

/** \class cmFileTimeComparison
 * \brief Helper class for performing globbing searches.
 *
 * Finds all files that match a given globbing expression.
 */
class cmFileTimeComparison
{
public:
  cmFileTimeComparison();
  ~cmFileTimeComparison();

  /**
   *  Compare file modification times.
   *  Return true for successful comparison and false for error.
   *  When true is returned, result has -1, 0, +1 for
   *  f1 older, same, or newer than f2.  
   */
  bool FileTimeCompare(const char* f1, const char* f2, int* result);

protected:
  
  cmFileTimeComparisonInternal* Internals;
};


#endif

