/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmGlobalVisualStudioGenerator.h,v $
  Language:  C++
  Date:      $Date: 2007/11/19 18:44:51 $
  Version:   $Revision: 1.5 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmGlobalVisualStudioGenerator_h
#define cmGlobalVisualStudioGenerator_h

#include "cmGlobalGenerator.h"

/** \class cmGlobalVisualStudioGenerator
 * \brief Base class for global Visual Studio generators.
 *
 * cmGlobalVisualStudioGenerator provides functionality common to all
 * global Visual Studio generators.
 */
class cmGlobalVisualStudioGenerator : public cmGlobalGenerator
{
public:
  cmGlobalVisualStudioGenerator();
  virtual ~cmGlobalVisualStudioGenerator();

  /**
   * Basic generate implementation for all VS generators.
   */
  virtual void Generate();

  /**
   * Configure CMake's Visual Studio macros file into the user's Visual
   * Studio macros directory.
   */
  virtual void ConfigureCMakeVisualStudioMacros();

  /**
   * Where does this version of Visual Studio look for macros for the
   * current user? Returns the empty string if this version of Visual
   * Studio does not implement support for VB macros.
   */
  virtual std::string GetUserMacrosDirectory();

  enum MacroName {MacroReload, MacroStop};

  /**
   * Call the ReloadProjects macro if necessary based on
   * GetFilesReplacedDuringGenerate results.
   */
  virtual void CallVisualStudioMacro(MacroName m,
                                     const char* vsSolutionFile = 0);

protected:
  virtual void CreateGUID(const char*) {}
  virtual void FixUtilityDepends();
  const char* GetUtilityForTarget(cmTarget& target, const char*);

private:
  void FixUtilityDependsForTarget(cmTarget& target);
  void CreateUtilityDependTarget(cmTarget& target);
  bool CheckTargetLinks(cmTarget& target, const char* name);
};

#endif
