/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmMakefileLibraryTargetGenerator.h,v $
  Language:  C++
  Date:      $Date: 2007/08/14 18:12:08 $
  Version:   $Revision: 1.5 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmMakefileLibraryTargetGenerator_h
#define cmMakefileLibraryTargetGenerator_h

#include "cmMakefileTargetGenerator.h"

class cmMakefileLibraryTargetGenerator: 
  public cmMakefileTargetGenerator
{
public:
  cmMakefileLibraryTargetGenerator();

  /* the main entry point for this class. Writes the Makefiles associated
     with this target */
  virtual void WriteRuleFiles();  
  
protected:
  void WriteStaticLibraryRules();
  void WriteSharedLibraryRules(bool relink);
  void WriteModuleLibraryRules(bool relink);
  void WriteLibraryRules(const char *linkRule, const char *extraFlags,
                         bool relink);
  // MacOSX Framework support methods
  void WriteFrameworkRules(bool relink);
  void CreateFramework(std::string& targetName,
                       std::string& outpath);
  void CreateFrameworkLinksAndDirs(std::string& targetName,
                                   std::string& outpath,
                                   const char* version);
  void CopyFrameworkSources(std::string& targetName,
                            std::string& outpath,
                            const char* version,
                            const char* propertyName,
                            const char* subdir);
};

#endif
