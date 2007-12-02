/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmGetPropertyCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2007/10/24 18:43:10 $
  Version:   $Revision: 1.3 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmGetPropertyCommand.h"

#include "cmake.h"
#include "cmTest.h"
#include "cmPropertyDefinition.h"

// cmGetPropertyCommand
bool cmGetPropertyCommand::InitialPass(
  std::vector<std::string> const& args)
{
  if(args.size() < 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // the last argument in the property to get
  const char *property = args[args.size()-1].c_str();
  bool get_brief = false;
  if (!strcmp(property,"BRIEF_DOCS"))
    {
    get_brief = true;
    property = args[args.size()-2].c_str();
    }
  bool get_full = false;
  if (!strcmp(property,"FULL_DOCS"))
    {
    get_full = true;
    property = args[args.size()-2].c_str();
    }

  std::string output = "NOTFOUND";

  cmProperty::ScopeType scope;
  const char *scopeName = 0;
  if (args[1] == "GLOBAL")
    {
    scope = cmProperty::GLOBAL;
    }
  else if (args[1] == "VARIABLE")
    {
    scope = cmProperty::VARIABLE;
    }
  else if (args[1] == "DIRECTORY" && args.size() >= 3)
    {
    scope = cmProperty::DIRECTORY;
    if ((args.size() == 4 && !get_brief && !get_full) ||
        (args.size() == 5 && (get_brief || get_full)))
      {
      scopeName = args[2].c_str();
      }
    }
  else if (args[1] == "TARGET" && args.size() >= 4)
    {
    scope = cmProperty::TARGET;
    scopeName = args[2].c_str();
    }
  else if (args[1] == "TEST" && args.size() >= 4)
    {
    scope = cmProperty::TEST;
    scopeName = args[2].c_str();
    }
  else if (args[1] == "SOURCE_FILE" && args.size() >= 4)
    {
    scope = cmProperty::SOURCE_FILE;
    scopeName = args[2].c_str();
    }
  else
    {
    this->SetError("called with illegal arguments.");
    return false;
    }
  
  if (get_brief)
    {
    cmPropertyDefinition *def = 
      this->Makefile->GetCMakeInstance()->
      GetPropertyDefinition(property,scope);
    if (def)
      {
      output = def->GetShortDescription();
      }
    }
  else if (get_full)
    {
    cmPropertyDefinition *def = 
      this->Makefile->GetCMakeInstance()->
      GetPropertyDefinition(property,scope);
    if (def)
      {
      output = def->GetFullDescription();
      }
    }
  
  else switch (scope) 
    {
    case cmProperty::VARIABLE:
      {
      if (this->Makefile->GetDefinition(property))
        {
        output = this->Makefile->GetDefinition(property);
        }
      }
      break;
    case cmProperty::TARGET:
      {
      cmTarget *tgt = this->Makefile->GetLocalGenerator()->GetGlobalGenerator()
        ->FindTarget(0, scopeName, true);
      if (tgt)
        {
        cmTarget& target = *tgt;
        const char *prop = target.GetProperty(property);
        if (prop)
          {
          output = prop;
          }
        }
      }
      break;
    case cmProperty::DIRECTORY:
      {
      cmLocalGenerator *lg = this->Makefile->GetLocalGenerator();
      if (args.size() >= 4)
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
        
        // lookup the makefile from the directory name
        lg = 
          this->Makefile->GetLocalGenerator()->GetGlobalGenerator()->
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
      const char *prop = lg->GetMakefile()->GetProperty(property);
      if (prop)
        {
        output = prop;
        }      
      }
      break;
    case cmProperty::GLOBAL:
      {
      const char *prop = 
        this->Makefile->GetCMakeInstance()->GetProperty(property);
      if (prop)
        {
        output = prop;
        }      
      }
      break;
    case cmProperty::TEST:
      {
      cmTest *test = this->Makefile->GetTest(scopeName);
      const char *prop = test->GetProperty(property);
      if (prop)
        {
        output = prop;
        }
      }
      break;
    case cmProperty::SOURCE_FILE:
      {
      cmSourceFile* sf = this->Makefile->GetSource(scopeName);
      const char *prop = sf->GetProperty(property);
      if (prop)
        {
        output = prop;
        }
      }
      break;
    case cmProperty::CACHED_VARIABLE:
      // not handled by GetProperty
      break;
    }

  this->Makefile->AddDefinition(args[0].c_str(), output.c_str());
  return true;
}

