/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmSourceFileLocation.cxx,v $
  Language:  C++
  Date:      $Date: 2007/06/18 15:59:23 $
  Version:   $Revision: 1.2 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmSourceFileLocation.h"

#include "cmMakefile.h"
#include "cmSystemTools.h"

//----------------------------------------------------------------------------
cmSourceFileLocation
::cmSourceFileLocation(cmMakefile* mf, const char* name): Makefile(mf)
{
  this->AmbiguousDirectory = !cmSystemTools::FileIsFullPath(name);
  this->AmbiguousExtension = true;
  this->Directory = cmSystemTools::GetFilenamePath(name);
  this->Name = cmSystemTools::GetFilenameName(name);
  this->UpdateExtension(name);
}

//----------------------------------------------------------------------------
void cmSourceFileLocation::Update(const char* name)
{
  if(this->AmbiguousDirectory)
    {
    this->UpdateDirectory(name);
    }
  if(this->AmbiguousExtension)
    {
    this->UpdateExtension(name);
    }
}

//----------------------------------------------------------------------------
void cmSourceFileLocation::Update(cmSourceFileLocation const& loc)
{
  if(this->AmbiguousDirectory && !loc.AmbiguousDirectory)
    {
    this->Directory = loc.Directory;
    this->AmbiguousDirectory = false;
    }
  if(this->AmbiguousExtension && !loc.AmbiguousExtension)
    {
    this->Name = loc.Name;
    this->AmbiguousExtension = false;
    }
}

//----------------------------------------------------------------------------
void cmSourceFileLocation::DirectoryUseSource()
{
  if(this->AmbiguousDirectory)
    {
    this->Directory =
      cmSystemTools::CollapseFullPath(
        this->Directory.c_str(), this->Makefile->GetCurrentDirectory());
    this->AmbiguousDirectory = false;
    }
}

//----------------------------------------------------------------------------
void cmSourceFileLocation::DirectoryUseBinary()
{
  if(this->AmbiguousDirectory)
    {
    this->Directory =
      cmSystemTools::CollapseFullPath(
        this->Directory.c_str(), this->Makefile->GetCurrentOutputDirectory());
    this->AmbiguousDirectory = false;
    }
}

//----------------------------------------------------------------------------
void cmSourceFileLocation::UpdateExtension(const char* name)
{
  // Check the extension.
  std::string ext = cmSystemTools::GetFilenameLastExtension(name);
  if(!ext.empty()) { ext = ext.substr(1); }

  // TODO: Let enable-language specify extensions for each language.
  cmMakefile const* mf = this->Makefile;
  const std::vector<std::string>& srcExts = mf->GetSourceExtensions();
  const std::vector<std::string>& hdrExts = mf->GetHeaderExtensions();
  if(std::find(srcExts.begin(), srcExts.end(), ext) != srcExts.end() ||
     std::find(hdrExts.begin(), hdrExts.end(), ext) != hdrExts.end())
    {
    // This is a known extension.  Use the given filename with extension.
    this->Name = cmSystemTools::GetFilenameName(name);
    this->AmbiguousExtension = false;
    }
}

//----------------------------------------------------------------------------
void cmSourceFileLocation::UpdateDirectory(const char* name)
{
  // If a full path was given we know the directory.
  if(cmSystemTools::FileIsFullPath(name))
    {
    this->Directory = cmSystemTools::GetFilenamePath(name);
    this->AmbiguousDirectory = false;
    }
}

//----------------------------------------------------------------------------
bool cmSourceFileLocation::Matches(cmSourceFileLocation const& loc)
{
  if(this->AmbiguousExtension || loc.AmbiguousExtension)
    {
    // Need to compare without the file extension.
    std::string thisName;
    if(this->AmbiguousExtension)
      {
      thisName = this->Name;
      }
    else
      {
      thisName = cmSystemTools::GetFilenameWithoutLastExtension(this->Name);
      }
    std::string locName;
    if(loc.AmbiguousExtension)
      {
      locName = loc.Name;
      }
    else
      {
      locName = cmSystemTools::GetFilenameWithoutLastExtension(loc.Name);
      }
    if(thisName != locName)
      {
      return false;
      }
    }
  else
    {
    // Compare with extension.
    if(this->Name != loc.Name)
      {
      return false;
      }
    }

  if(!this->AmbiguousDirectory && !loc.AmbiguousDirectory)
    {
    // Both sides have absolute directories.
    if(this->Directory != loc.Directory)
      {
      return false;
      }
    }
  else if(this->AmbiguousDirectory && loc.AmbiguousDirectory &&
          this->Makefile == loc.Makefile)
    {
    // Both sides have directories relative to the same location.
    if(this->Directory != loc.Directory)
      {
      return false;
      }
    }
  else if(this->AmbiguousDirectory && loc.AmbiguousDirectory)
    {
    // Each side has a directory relative to a different location.
    // This can occur when referencing a source file from a different
    // directory.  This is not yet allowed.
    abort();
    }
  else if(this->AmbiguousDirectory)
    {
    // Compare possible directory combinations.
    std::string srcDir =
      cmSystemTools::CollapseFullPath(
        this->Directory.c_str(), this->Makefile->GetCurrentDirectory());
    std::string binDir =
      cmSystemTools::CollapseFullPath(
        this->Directory.c_str(), this->Makefile->GetCurrentOutputDirectory());
    if(srcDir != loc.Directory &&
       binDir != loc.Directory)
      {
      return false;
      }
    }
  else if(loc.AmbiguousDirectory)
    {
    // Compare possible directory combinations.
    std::string srcDir =
      cmSystemTools::CollapseFullPath(
        loc.Directory.c_str(), loc.Makefile->GetCurrentDirectory());
    std::string binDir =
      cmSystemTools::CollapseFullPath(
        loc.Directory.c_str(), loc.Makefile->GetCurrentOutputDirectory());
    if(srcDir != this->Directory &&
       binDir != this->Directory)
      {
      return false;
      }
    }

  // File locations match.
  this->Update(loc);
  return true;
}
