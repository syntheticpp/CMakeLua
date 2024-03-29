/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmSetTargetPropertiesCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2008/01/28 13:38:36 $
  Version:   $Revision: 1.9 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmSetTargetPropertiesCommand.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"

// cmSetTargetPropertiesCommand
bool cmSetTargetPropertiesCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // first collect up the list of files
  std::vector<std::string> propertyPairs;
  bool doingFiles = true;
  int numFiles = 0;
  std::vector<std::string>::const_iterator j;
  for(j= args.begin(); j != args.end();++j)
    {
    if(*j == "PROPERTIES")
      {
      doingFiles = false;
      // now loop through the rest of the arguments, new style
      ++j;
      while (j != args.end())
        {
        propertyPairs.push_back(*j);
        ++j;
        if(j == args.end())
          {
          this->SetError("called with incorrect number of arguments.");
          return false;
          }
        propertyPairs.push_back(*j);
        ++j;
        }
      // break out of the loop because j is already == end
      break;
      }
    else if (doingFiles)
      {
      numFiles++;
      }
    else
      {
      this->SetError("called with illegal arguments, maybe missing "
                     "a PROPERTIES specifier?");
      return false;
      }
    }
  if(propertyPairs.size() == 0)
    {
     this->SetError("called with illegal arguments, maybe missing "
                    "a PROPERTIES specifier?");
     return false;
    }
  
  // now loop over all the targets
  int i;
  for(i = 0; i < numFiles; ++i)
    {   
    bool ret = cmSetTargetPropertiesCommand::SetOneTarget
      (args[i].c_str(),propertyPairs,this->Makefile);
    if (!ret)
      {
      std::string message = "Can not find target to add properties to: ";
      message += args[i];
      this->SetError(message.c_str());
      return false;
      }
    }
  return true;
}

bool cmSetTargetPropertiesCommand
::SetOneTarget(const char *tname, 
               std::vector<std::string> &propertyPairs,
               cmMakefile *mf)
{
  if(cmTarget* target = mf->FindTargetToUse(tname))
    {
    // now loop through all the props and set them
    unsigned int k;
    for (k = 0; k < propertyPairs.size(); k = k + 2)
      {
      target->SetProperty(propertyPairs[k].c_str(),
                          propertyPairs[k+1].c_str());
      }
    }
  // if file is not already in the makefile, then add it
  else
    { 
    return false;
    }
  return true;
}
