/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmSetPropertiesCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2007/06/25 16:50:29 $
  Version:   $Revision: 1.9 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmSetPropertiesCommand.h"
#include "cmSetTargetPropertiesCommand.h"
#include "cmSetTestsPropertiesCommand.h"
#include "cmSetSourceFilesPropertiesCommand.h"

// cmSetPropertiesCommand
bool cmSetPropertiesCommand::InitialPass(
  std::vector<std::string> const& args)
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
  
  cmProperty::ScopeType scope;
  const char *scopeName = 0;
  if (args[0] == "GLOBAL" && numFiles == 1)
    {
    scope = cmProperty::GLOBAL;
    }
  else if (args[0] == "DIRECTORY" && numFiles >= 1)
    {
    scope = cmProperty::DIRECTORY;
    if (numFiles == 2)
      {
      scopeName = args[1].c_str();
      }
    }
  else if (args[0] == "TARGET" && numFiles == 2)
    {
    scope = cmProperty::TARGET;
    scopeName = args[1].c_str();
    }
  else if (args[0] == "TEST" && numFiles == 2)
    {
    scope = cmProperty::TEST;
    scopeName = args[1].c_str();
    }
  else if (args[0] == "SOURCE_FILE" && numFiles == 2)
    {
    scope = cmProperty::SOURCE_FILE;
    scopeName = args[1].c_str();
    }
  else
    {
    this->SetError("called with illegal arguments.");
    return false;
    }

  switch (scope) 
    {
    case cmProperty::TARGET:
      {
      bool ret = cmSetTargetPropertiesCommand::
        SetOneTarget(scopeName,propertyPairs, this->Makefile);
      if (!ret)
        {
        std::string message = "Can not find target to add properties to: ";
        message += scopeName;
        this->SetError(message.c_str());
        return ret;
        }
      }
      break;
    case cmProperty::DIRECTORY:
      {
      // lookup the makefile from the directory name
      cmLocalGenerator *lg = this->Makefile->GetLocalGenerator();
      if (numFiles == 2)
        {
        std::string sd = scopeName;
        // make sure the start dir is a full path
        if (!cmSystemTools::FileIsFullPath(sd.c_str()))
          {
          sd = this->Makefile->GetStartDirectory();
          sd += "/";
          sd += scopeName;
          }
        
        // The local generators are associated with collapsed paths.
        sd = cmSystemTools::CollapseFullPath(sd.c_str());
        
        lg = this->Makefile->GetLocalGenerator()->GetGlobalGenerator()->
          FindLocalGenerator(sd.c_str());
        }
      if (!lg)
        {
        this->SetError
          ("DIRECTORY argument provided but requested directory not found. "
           "This could be because the directory argument was invalid or, "
           "it is valid but has not been processed yet.");
        return false;
        }
      
      for(j= propertyPairs.begin(); j != propertyPairs.end(); ++j)
        {
        const char *pn = j->c_str();
        ++j;
        lg->GetMakefile()->SetProperty(pn,j->c_str());
        }
      }
      break;
    case cmProperty::GLOBAL:
      {
      for(j= propertyPairs.begin(); j != propertyPairs.end(); ++j)
        {
        const char *pn = j->c_str();
        ++j;
        this->Makefile->GetCMakeInstance()->SetProperty(pn, j->c_str());
        }
      }
      break;
    case cmProperty::TEST:
      {
      std::string errors;
      bool ret = cmSetTestsPropertiesCommand::
        SetOneTest(scopeName,propertyPairs, this->Makefile, errors);
      if (!ret)
        {
        this->SetError(errors.c_str());
        return ret;
        }
      }
      break;
    case cmProperty::SOURCE_FILE:
      {
      std::string errors;
      bool ret = cmSetSourceFilesPropertiesCommand::
        RunCommand(this->Makefile,
                   args.begin()+1, args.begin()+2,
                   args.begin() + 2, args.end(),
                   errors);
      if (!ret)
        {
        this->SetError(errors.c_str());
        return ret;
        }
      }
      break;
    case cmProperty::VARIABLE:
    case cmProperty::CACHED_VARIABLE:
      // not handled by SetProperty
      break;
    }

  return true;
}

