/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmMakefileUtilityTargetGenerator.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/01 17:51:25 $
  Version:   $Revision: 1.6 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmMakefileUtilityTargetGenerator.h"

#include "cmGeneratedFileStream.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmTarget.h"

//----------------------------------------------------------------------------
cmMakefileUtilityTargetGenerator::cmMakefileUtilityTargetGenerator()
{
  this->CustomCommandDriver = OnUtility;
}

//----------------------------------------------------------------------------
void cmMakefileUtilityTargetGenerator::WriteRuleFiles()
{
  this->CreateRuleFile();

  *this->BuildFileStream
    << "# Utility rule file for " << this->Target->GetName() << ".\n\n";

  // write the custom commands for this target
  this->WriteTargetBuildRules();

  // Collect the commands and dependencies.
  std::vector<std::string> commands;
  std::vector<std::string> depends;

  // Utility targets store their rules in pre- and post-build commands.
  this->LocalGenerator->AppendCustomDepends
    (depends, this->Target->GetPreBuildCommands());

  this->LocalGenerator->AppendCustomDepends
    (depends, this->Target->GetPostBuildCommands());

  this->LocalGenerator->AppendCustomCommands
    (commands, this->Target->GetPreBuildCommands());

  // Depend on all custom command outputs for sources
  this->DriveCustomCommands(depends);

  this->LocalGenerator->AppendCustomCommands
    (commands, this->Target->GetPostBuildCommands());

  // Add dependencies on targets that must be built first.
  this->AppendTargetDepends(depends);

  // Add a dependency on the rule file itself.
  this->LocalGenerator->AppendRuleDepend(depends,
                                         this->BuildFileNameFull.c_str());

  // If the rule is empty add the special empty rule dependency needed
  // by some make tools.
  if(depends.empty() && commands.empty())
    {
    std::string hack = this->GlobalGenerator->GetEmptyRuleHackDepends();
    if(!hack.empty())
      {
      depends.push_back(hack);
      }
    }

  // Write the rule.
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      this->Target->GetName(),
                                      depends, commands, true);

  // Write the main driver rule to build everything in this target.
  this->WriteTargetDriverRule(this->Target->GetName(), false);

  // Write clean target
  this->WriteTargetCleanRules();

  // close the streams
  this->CloseFileStreams();
}

