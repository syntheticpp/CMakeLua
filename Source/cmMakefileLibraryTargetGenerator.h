/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmMakefileLibraryTargetGenerator.h,v $
  Language:  C++
<<<<<<< cmMakefileLibraryTargetGenerator.h
  Date:      $Date: 2008/02/18 21:38:34 $
  Version:   $Revision: 1.6 $
=======
  Date:      $Date: 2008-04-08 04:06:46 $
  Version:   $Revision: 1.7 $
>>>>>>> 1.7

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
  cmMakefileLibraryTargetGenerator(cmTarget* target);

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
  void CreateFramework();

  // Store the computd framework version for OS X Frameworks.
  std::string FrameworkVersion;
};

#endif
