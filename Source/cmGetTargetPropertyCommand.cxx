/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmGetTargetPropertyCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/31 16:03:52 $
  Version:   $Revision: 1.9 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmGetTargetPropertyCommand.h"

// cmSetTargetPropertyCommand
bool cmGetTargetPropertyCommand::InitialPass(
  std::vector<std::string> const& args)
{
  if(args.size() != 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::string var = args[0].c_str();
  const char* targetName = args[1].c_str();

  cmTarget *tgt = this->Makefile->GetLocalGenerator()->GetGlobalGenerator()
    ->FindTarget(0, targetName, true);
  if (tgt)
    {
    cmTarget& target = *tgt;
    const char *prop = target.GetProperty(args[2].c_str());
    if (prop)
      {
      this->Makefile->AddDefinition(var.c_str(), prop);
      return true;
      }
    }
  this->Makefile->AddDefinition(var.c_str(), (var+"-NOTFOUND").c_str());
  return true;
}

