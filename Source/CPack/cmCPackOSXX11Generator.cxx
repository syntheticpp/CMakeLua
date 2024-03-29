/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmCPackOSXX11Generator.cxx,v $
  Language:  C++
  Date:      $Date: 2007/10/31 12:50:17 $
  Version:   $Revision: 1.5 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmCPackOSXX11Generator.h"

#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackLog.h"

#include <cmsys/SystemTools.hxx>
#include <cmsys/Glob.hxx>

//----------------------------------------------------------------------
cmCPackOSXX11Generator::cmCPackOSXX11Generator()
{
}

//----------------------------------------------------------------------
cmCPackOSXX11Generator::~cmCPackOSXX11Generator()
{
}

//----------------------------------------------------------------------
int cmCPackOSXX11Generator::CompressFiles(const char* outFileName,
  const char* toplevel,
  const std::vector<std::string>& files)
{
  (void) files; // TODO: Fix api to not need files.
  (void) toplevel; // TODO: Use toplevel

  const char* cpackPackageExecutables
    = this->GetOption("CPACK_PACKAGE_EXECUTABLES");
  if ( cpackPackageExecutables )
    {
    cmCPackLogger(cmCPackLog::LOG_DEBUG, "The cpackPackageExecutables: "
      << cpackPackageExecutables << "." << std::endl);
    cmOStringStream str;
    cmOStringStream deleteStr;
    std::vector<std::string> cpackPackageExecutablesVector;
    cmSystemTools::ExpandListArgument(cpackPackageExecutables,
      cpackPackageExecutablesVector);
    if ( cpackPackageExecutablesVector.size() % 2 != 0 )
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "CPACK_PACKAGE_EXECUTABLES should contain pairs of <executable> and "
        "<icon name>." << std::endl);
      return 0;
      }
    std::vector<std::string>::iterator it;
    for ( it = cpackPackageExecutablesVector.begin();
      it != cpackPackageExecutablesVector.end();
      ++it )
      {
      std::string cpackExecutableName = *it;
      ++ it;
      this->SetOptionIfNotSet("CPACK_EXECUTABLE_NAME", 
        cpackExecutableName.c_str());
      }
    }

  // Disk image directories
  std::string diskImageDirectory = toplevel;
  std::string diskImageBackgroundImageDir
    = diskImageDirectory + "/.background";


  // App bundle directories
  std::string packageDirFileName = toplevel;
  packageDirFileName += "/";
  packageDirFileName += this->GetOption("CPACK_PACKAGE_FILE_NAME");
  packageDirFileName += ".app";
  std::string contentsDirectory = packageDirFileName + "/Contents";
  std::string resourcesDirectory = contentsDirectory + "/Resources";
  std::string appDirectory = contentsDirectory + "/MacOS";

  const char* dir = resourcesDirectory.c_str();
  const char* appdir = appDirectory.c_str();
  const char* contDir = contentsDirectory.c_str();
  const char* iconFile = this->GetOption("CPACK_PACKAGE_ICON");
  if ( iconFile )
    {
    std::string iconFileName = cmsys::SystemTools::GetFilenameName(
      iconFile);
    if ( !cmSystemTools::FileExists(iconFile) )
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find icon file: "
        << iconFile << ". Please check CPACK_PACKAGE_ICON setting."
        << std::endl);
      return 0;
      }
    std::string destFileName = resourcesDirectory + "/" + iconFileName;
    this->ConfigureFile(iconFile, destFileName.c_str(), true);
    this->SetOptionIfNotSet("CPACK_APPLE_GUI_ICON", iconFileName.c_str());
    }

  std::string applicationsLinkName = diskImageDirectory + "/Applications";
  cmSystemTools::CreateSymlink("/Applications", applicationsLinkName.c_str());

  if (
    !this->CopyResourcePlistFile("VolumeIcon.icns", 
                                 diskImageDirectory.c_str(),
                                 ".VolumeIcon.icns", true ) ||
    !this->CopyResourcePlistFile("DS_Store", diskImageDirectory.c_str(),
      ".DS_Store", true ) ||
    !this->CopyResourcePlistFile("background.png",
      diskImageBackgroundImageDir.c_str(), "background.png", true ) ||
    !this->CopyResourcePlistFile("RuntimeScript", dir) ||
    !this->CopyResourcePlistFile("OSXX11.Info.plist", contDir,
      "Info.plist" ) ||
    !this->CopyResourcePlistFile("OSXScriptLauncher", appdir, 
      this->GetOption("CPACK_PACKAGE_FILE_NAME"), true)
  )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem copying the resource files"
      << std::endl);
    return 0;
    }

  std::string output;
  std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  tmpFile += "/hdiutilOutput.log";
  cmOStringStream dmgCmd;
  dmgCmd << "\"" << this->GetOption("CPACK_INSTALLER_PROGRAM_DISK_IMAGE")
         << "\" create -ov -format UDZO -srcfolder \"" 
         << diskImageDirectory.c_str() 
         << "\" \"" << outFileName << "\"";
  int retVal = 1;
  cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                "Compress disk image using command: " 
                << dmgCmd.str().c_str() << std::endl);
  bool res = cmSystemTools::RunSingleCommand(dmgCmd.str().c_str(), &output,
    &retVal, 0, this->GeneratorVerbose, 0);
  if ( !res || retVal )
    {
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << dmgCmd.str().c_str() << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running hdiutil command: "
      << dmgCmd.str().c_str() << std::endl
      << "Please check " << tmpFile.c_str() << " for errors" << std::endl);
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------
int cmCPackOSXX11Generator::InitializeInternal()
{
  cmCPackLogger(cmCPackLog::LOG_DEBUG,
    "cmCPackOSXX11Generator::Initialize()" << std::endl);
  std::vector<std::string> path;
  std::string pkgPath = cmSystemTools::FindProgram("hdiutil", path, false);
  if ( pkgPath.empty() )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find hdiutil compiler"
      << std::endl);
    return 0;
    }
  this->SetOptionIfNotSet("CPACK_INSTALLER_PROGRAM_DISK_IMAGE", 
                          pkgPath.c_str());

  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
/*
bool cmCPackOSXX11Generator::CopyCreateResourceFile(const char* name)
{
  std::string uname = cmSystemTools::UpperCase(name);
  std::string cpackVar = "CPACK_RESOURCE_FILE_" + uname;
  const char* inFileName = this->GetOption(cpackVar.c_str());
  if ( !inFileName )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "CPack option: " << cpackVar.c_str()
                  << " not specified. It should point to " 
                  << (name ? name : "(NULL)")
                  << ".rtf, " << name
                  << ".html, or " << name << ".txt file" << std::endl);
    return false;
    }
  if ( !cmSystemTools::FileExists(inFileName) )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find " 
                  << (name ? name : "(NULL)")
                  << " resource file: " << inFileName << std::endl);
    return false;
    }
  std::string ext = cmSystemTools::GetFilenameLastExtension(inFileName);
  if ( ext != ".rtfd" && ext != ".rtf" && ext != ".html" && ext != ".txt" )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Bad file extension specified: "
      << ext << ". Currently only .rtfd, .rtf, .html, and .txt files allowed."
      << std::endl);
    return false;
    }

  std::string destFileName = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  destFileName += "/Resources/";
  destFileName += name + ext;


  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Configure file: " 
                << (inFileName ? inFileName : "(NULL)")
                << " to " << destFileName.c_str() << std::endl);
  this->ConfigureFile(inFileName, destFileName.c_str());
  return true;
}
*/

//----------------------------------------------------------------------
bool cmCPackOSXX11Generator::CopyResourcePlistFile(const char* name,
  const char* dir, const char* outputFileName /* = 0 */,
  bool copyOnly /* = false */)
{
  std::string inFName = "CPack.";
  inFName += name;
  inFName += ".in";
  std::string inFileName = this->FindTemplate(inFName.c_str());
  if ( inFileName.empty() )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find input file: "
      << inFName << std::endl);
    return false;
    }

  if ( !outputFileName )
    {
    outputFileName = name;
    }

  std::string destFileName = dir;
  destFileName += "/";
  destFileName += outputFileName;

  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Configure file: "
    << inFileName.c_str() << " to " << destFileName.c_str() << std::endl);
  this->ConfigureFile(inFileName.c_str(), destFileName.c_str(), copyOnly);
  return true;
}

//----------------------------------------------------------------------
const char* cmCPackOSXX11Generator::GetPackagingInstallPrefix()
{
  this->InstallPrefix = "/";
  this->InstallPrefix += this->GetOption("CPACK_PACKAGE_FILE_NAME");
  this->InstallPrefix += ".app/Contents/Resources";
  return this->InstallPrefix.c_str();
}
