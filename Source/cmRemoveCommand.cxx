/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmRemoveCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2008/01/23 15:27:59 $
  Version:   $Revision: 1.7 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmRemoveCommand.h"

// cmRemoveCommand
bool cmRemoveCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1)
    {
    return true;
    }

  const char* variable = args[0].c_str(); // VAR is always first
  // get the old value
  const char* cacheValue
    = this->Makefile->GetDefinition(variable);

  // if there is no old value then return
  if (!cacheValue)
    {
    return true;
    }
  
  // expand the variable
  std::vector<std::string> varArgsExpanded;
  cmSystemTools::ExpandListArgument(cacheValue, varArgsExpanded);
  
  // expand the args
  // check for REMOVE(VAR v1 v2 ... vn) 
  std::vector<std::string> argsExpanded;
  std::vector<std::string> temp;
  for(unsigned int j = 1; j < args.size(); ++j)
    {
    temp.push_back(args[j]);
    }
  cmSystemTools::ExpandList(temp, argsExpanded);
  
  // now create the new value
  std::string value;
  for(unsigned int j = 0; j < varArgsExpanded.size(); ++j)
    {
    int found = 0;
    for(unsigned int k = 0; k < argsExpanded.size(); ++k)
      {
      if (varArgsExpanded[j] == argsExpanded[k])
        {
        found = 1;
        break;
        }
      }
    if (!found)
      {
      if (value.size())
        {
        value += ";";
        }
      value += varArgsExpanded[j];
      }
    }
  
  // add the definition
  this->Makefile->AddDefinition(variable, value.c_str());

  return true;
}

