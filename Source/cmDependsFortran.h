/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmDependsFortran.h,v $
  Language:  C++
  Date:      $Date: 2006/03/15 16:02:00 $
  Version:   $Revision: 1.7 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmDependsFortran_h
#define cmDependsFortran_h

#include "cmDepends.h"

/** \class cmDependsFortran
 * \brief Dependency scanner for Fortran object files.
 */
class cmDependsFortran: public cmDepends
{
public:
  /** Checking instances need to know the build directory name and the
      relative path from the build directory to the target file.  */
  cmDependsFortran();

  /** Scanning need to know the build directory name, the relative
      path from the build directory to the target file, the source
      file from which to start scanning, and the include file search
      path.  */
  cmDependsFortran(std::vector<std::string> const& includes);

  /** Virtual destructor to cleanup subclasses properly.  */
  virtual ~cmDependsFortran();

  /** Callback from build system after a .mod file has been generated
      by a Fortran90 compiler to copy the .mod file to the
      corresponding stamp file.  */
  static bool CopyModule(const std::vector<std::string>& args);

  /** Method to find an included file in the include path.  Fortran
      always searches the directory containing the including source
      first.  */
  bool FindIncludeFile(const char* dir, const char* includeName,
                       std::string& fileName);

protected:
  // Implement writing/checking methods required by superclass.
  virtual bool WriteDependencies(
    const char *src, const char *file,
    std::ostream& makeDepends, std::ostream& internalDepends);

  // The source file from which to start scanning.
  std::string SourceFile;

  // The include file search path.
  std::vector<std::string> const* IncludePath;

private:
  cmDependsFortran(cmDependsFortran const&); // Purposely not implemented.
  void operator=(cmDependsFortran const&); // Purposely not implemented.
};

#endif
