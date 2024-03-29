/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmAuxSourceDirectoryCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2008/01/23 15:27:59 $
  Version:   $Revision: 1.26 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmAuxSourceDirectoryCommand.h"
#include "cmSourceFile.h"

#include <cmsys/Directory.hxx>

// cmAuxSourceDirectoryCommand
bool cmAuxSourceDirectoryCommand::InitialPass
(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2 || args.size() > 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  std::string sourceListValue;
  std::string templateDirectory = args[0];
  this->Makefile->AddExtraDirectory(templateDirectory.c_str());
  std::string tdir;
  if(!cmSystemTools::FileExists(templateDirectory.c_str()))
    {
    tdir = this->Makefile->GetCurrentDirectory();
    tdir += "/";
    tdir += templateDirectory;
    }
  else
    {
    tdir = templateDirectory;
    }

  // was the list already populated
  const char *def = this->Makefile->GetDefinition(args[1].c_str());  
  if (def)
    {
    sourceListValue = def;
    }
  
  // Load all the files in the directory
  cmsys::Directory dir;
  if(dir.Load(tdir.c_str()))
    {
    size_t numfiles = dir.GetNumberOfFiles();
    for(size_t i =0; i < numfiles; ++i)
      {
      std::string file = dir.GetFile(static_cast<unsigned long>(i));
      // Split the filename into base and extension
      std::string::size_type dotpos = file.rfind(".");
      if( dotpos != std::string::npos )
        {
        std::string ext = file.substr(dotpos+1);
        std::string base = file.substr(0, dotpos);
        // Process only source files
        if( base.size() != 0
            && std::find( this->Makefile->GetSourceExtensions().begin(),
                          this->Makefile->GetSourceExtensions().end(), ext )
                 != this->Makefile->GetSourceExtensions().end() )
          {
          std::string fullname = templateDirectory;
          fullname += "/";
          fullname += file;
          // add the file as a class file so 
          // depends can be done
          cmSourceFile* sf =
            this->Makefile->GetOrCreateSource(fullname.c_str());
          sf->SetProperty("ABSTRACT","0");
          if(!sourceListValue.empty())
            {
            sourceListValue += ";";
            }
          sourceListValue += fullname;
          }
        }
      }
    }
  this->Makefile->AddDefinition(args[1].c_str(), sourceListValue.c_str());  
  return true;
}

