/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmGlobalWatcomWMakeGenerator.cxx,v $
  Language:  C++
  Date:      $Date: 2007/10/22 16:48:39 $
  Version:   $Revision: 1.14 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmGlobalWatcomWMakeGenerator.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"

cmGlobalWatcomWMakeGenerator::cmGlobalWatcomWMakeGenerator()
{
  this->FindMakeProgramFile = "CMakeFindWMake.cmake";
  this->ForceUnixPaths = false;
  this->ToolSupportsColor = true;
  this->NeedSymbolicMark = true;
  this->EmptyRuleHackCommand = "@cd .";
}

void cmGlobalWatcomWMakeGenerator
::EnableLanguage(std::vector<std::string>const& l, 
                 cmMakefile *mf, 
                 bool optional)
{
  // pick a default 
  mf->AddDefinition("WATCOM", "1");
  mf->AddDefinition("CMAKE_QUOTE_INCLUDE_PATHS", "1");
  mf->AddDefinition("CMAKE_MANGLE_OBJECT_FILE_NAMES", "1");
  mf->AddDefinition("CMAKE_MAKE_LINE_CONTINUE", "&");
  mf->AddDefinition("CMAKE_MAKE_SYMBOLIC_RULE", ".SYMBOLIC");
  mf->AddDefinition("CMAKE_NO_QUOTED_OBJECTS", "1");
  mf->AddDefinition("CMAKE_GENERATOR_CC", "wcl386");
  mf->AddDefinition("CMAKE_GENERATOR_CXX", "wcl386");
  this->cmGlobalUnixMakefileGenerator3::EnableLanguage(l, mf, optional);
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalWatcomWMakeGenerator::CreateLocalGenerator()
{
  cmLocalUnixMakefileGenerator3* lg = new cmLocalUnixMakefileGenerator3;
  lg->SetSilentNoColon(true);
  lg->SetDefineWindowsNULL(true);
  lg->SetWindowsShell(true);
  lg->SetWatcomWMake(true);
  lg->SetMakeSilentFlag("-s -h");
  lg->SetGlobalGenerator(this);
  lg->SetIgnoreLibPrefix(true);
  lg->SetPassMakeflags(false);
  lg->SetUnixCD(false);
  lg->SetIncludeDirective("!include");
  return lg;
}

//----------------------------------------------------------------------------
void cmGlobalWatcomWMakeGenerator
::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates Watcom WMake makefiles.";
  entry.Full = "";
}
