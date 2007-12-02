/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmGetSourceFilePropertyCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2007/07/10 18:05:06 $
  Version:   $Revision: 1.11 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmGetSourceFilePropertyCommand.h"

#include "cmSourceFile.h"

// cmSetSourceFilePropertyCommand
bool cmGetSourceFilePropertyCommand::InitialPass(
  std::vector<std::string> const& args)
{
  if(args.size() != 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  const char* var = args[0].c_str();
  const char* file = args[1].c_str();
  cmSourceFile* sf = this->Makefile->GetSource(file);

  // for the location we must create a source file first
  if (!sf && args[2] == "LOCATION")
    {
    sf = this->Makefile->GetOrCreateSource(file);
    }
  if(sf)
    {
    if(args[2] == "LOCATION")
      {
      // Make sure the location is known.  Update: this is a hack to work
      // around a problem with const methods in cmSourceFile, by design
      // GetProperty("LOCATION") should work but right now it has to be
      // "primed" by calling GetFullPath() first on a non-const cmSourceFile
      // instance. This is because LOCATION is a computed-on-demand
      // property. Either GetProperty needs to be non-const or the map
      // needs to be changed to be mutable etc. for computed properties to
      // work properly.
      sf->GetFullPath();
      }
    const char *prop = sf->GetProperty(args[2].c_str());
    if (prop)
      {
      this->Makefile->AddDefinition(var, prop);
      return true;
      }
    }

  this->Makefile->AddDefinition(var, "NOTFOUND");
  return true;
}

