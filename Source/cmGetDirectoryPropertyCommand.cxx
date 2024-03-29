/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmGetDirectoryPropertyCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2008/01/23 15:27:59 $
  Version:   $Revision: 1.11 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmGetDirectoryPropertyCommand.h"

#include "cmake.h"

// cmGetDirectoryPropertyCommand
bool cmGetDirectoryPropertyCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  std::vector<std::string>::const_iterator i = args.begin();
  std::string variable = *i;
  ++i;
  std::string output = "";
    
  // get the directory argument if there is one
  cmMakefile *dir = this->Makefile;
  if (*i == "DIRECTORY")
    {
    ++i;
    if (i == args.end())
      {
      this->SetError
        ("DIRECTORY argument provided without subsequent arguments");
      return false;
      }
    std::string sd = *i;
    // make sure the start dir is a full path
    if (!cmSystemTools::FileIsFullPath(sd.c_str()))
      {
      sd = this->Makefile->GetStartDirectory();
      sd += "/";
      sd += *i;
      }

    // The local generators are associated with collapsed paths.
    sd = cmSystemTools::CollapseFullPath(sd.c_str());

    // lookup the makefile from the directory name
    cmLocalGenerator *lg = 
      this->Makefile->GetLocalGenerator()->GetGlobalGenerator()->
      FindLocalGenerator(sd.c_str());
    if (!lg)
      {
      this->SetError
        ("DIRECTORY argument provided but requested directory not found. "
         "This could be because the directory argument was invalid or, "
         "it is valid but has not been processed yet.");
      return false;
      }
    dir = lg->GetMakefile();
    ++i;
    }

  // OK, now we have the directory to process, we just get the requested
  // information out of it
  
  if ( *i == "DEFINITION" )
    {
    ++i;
    if (i == args.end())
      {
      this->SetError("A request for a variable definition was made without "
                     "providing the name of the variable to get.");
      return false;
      }
    output = dir->GetSafeDefinition(i->c_str());
    this->Makefile->AddDefinition(variable.c_str(), output.c_str());
    return true;
    }

  const char *prop = dir->GetProperty(i->c_str());
  if (prop)
    {
    this->Makefile->AddDefinition(variable.c_str(), prop);
    return true;
    }
  this->Makefile->AddDefinition(variable.c_str(), "");
  return true;
}

