/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmFindPackageCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2007/12/15 19:16:45 $
  Version:   $Revision: 1.26 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmFindPackageCommand.h"
#include <cmsys/RegularExpression.hxx>

#ifdef CMAKE_BUILD_WITH_CMAKE
#include "cmVariableWatch.h"
#endif

void cmFindPackageNeedBackwardsCompatibility(const std::string& variable,
  int access_type, void*, const char* newValue,
  const cmMakefile*)
{
  (void)newValue;
#ifdef CMAKE_BUILD_WITH_CMAKE
  if(access_type == cmVariableWatch::UNKNOWN_VARIABLE_READ_ACCESS)
    {
    std::string message = "An attempt was made to access a variable: ";
    message += variable;
    message +=
      " that has not been defined. This variable is created by the "
      "FIND_PACKAGE command. CMake version 1.6 always converted the "
      "variable name to upper-case, but this behavior is no longer the "
      "case.  To fix this you might need to set the cache value of "
      "CMAKE_BACKWARDS_COMPATIBILITY to 1.6 or less.  If you are writing a "
      "CMake listfile, you should change the variable reference to use "
      "the case of the argument to FIND_PACKAGE.";
    cmSystemTools::Error(message.c_str());
    }
#else
  (void)variable;
  (void)access_type;
#endif
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Record options.
  this->Name = args[0];
  bool quiet = false;
  bool required = false;
  bool no_module = false;
  std::string components;
  const char* components_sep = "";

  // Parse the arguments.
  bool doing_components = false;
  cmsys::RegularExpression version("^[0-9.]+$");
  bool haveVersion = false;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    if(args[i] == "QUIET")
      {
      quiet = true;
      doing_components = false;
      }
    else if(args[i] == "NO_MODULE")
      {
      no_module = true;
      doing_components = false;
      }
    else if(args[i] == "REQUIRED")
      {
      required = true;
      doing_components = true;
      }
    else if(args[i] == "COMPONENTS")
      {
      doing_components = true;
      }
    else if(doing_components)
      {
      // Set a variable telling the find script this component
      // is required.
      std::string req_var = Name + "_FIND_REQUIRED_" + args[i];
      this->Makefile->AddDefinition(req_var.c_str(), "1");

      // Append to the list of required components.
      components += components_sep;
      components += args[i];
      components_sep = ";";
      }
    else if(!haveVersion && version.find(args[i].c_str()))
      {
      haveVersion = true;
      }
    else
      {
      cmOStringStream e;
      e << "called with invalid argument \"" << args[i].c_str() << "\"";
      this->SetError(e.str().c_str());
      return false;
      }
    }

  // Store the list of components.
  std::string components_var = Name + "_FIND_COMPONENTS";
  this->Makefile->AddDefinition(components_var.c_str(), components.c_str());

  // See if there is a Find<name>.cmake module.
  if(!no_module)
    {
    bool foundModule = false;
    if(!this->FindModule(foundModule, quiet, required))
      {
      this->AppendSuccessInformation(quiet);
      return false;
      }
    if(foundModule)
      {
      this->AppendSuccessInformation(quiet);
      return true;
      }
    }

  // No find module.  Assume the project has a CMake config file.  Use
  // a <NAME>_DIR cache variable to locate it.
  this->Variable = this->Name;
  this->Variable += "_DIR";
  this->Config = this->Name;
  this->Config += "Config.cmake";

  // Support old capitalization behavior.
  std::string upperDir = cmSystemTools::UpperCase(this->Name);
  std::string upperFound = cmSystemTools::UpperCase(this->Name);
  upperDir += "_DIR";
  upperFound += "_FOUND";
  bool needCompatibility = false;
  if(!(upperDir == this->Variable))
    {
    const char* versionValue =
      this->Makefile->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
    if(atof(versionValue) < 1.7)
      {
      needCompatibility = true;
      }
    }

  // Try to find the config file.
  const char* def = this->Makefile->GetDefinition(this->Variable.c_str());
  if(needCompatibility && cmSystemTools::IsOff(def))
    {
    // Use the setting of the old name of the variable to provide the
    // value of the new.
    const char* oldDef = this->Makefile->GetDefinition(upperDir.c_str());
    if(!cmSystemTools::IsOff(oldDef))
      {
      this->Makefile->AddDefinition(this->Variable.c_str(), oldDef);
      def = this->Makefile->GetDefinition(this->Variable.c_str());
      }
    }
  if(cmSystemTools::IsOff(def))
    {
    if(!this->FindConfig())
      {
      this->AppendSuccessInformation(quiet);
      return false;
      }
    }

  // If the config file was found, load it.
  bool result = true;
  bool found = false;
  def = this->Makefile->GetDefinition(this->Variable.c_str());
  if(!cmSystemTools::IsOff(def))
    {
    std::string f = def;
    cmSystemTools::ConvertToUnixSlashes(f);
    f += "/";
    f += this->Config;
    if(!cmSystemTools::FileIsFullPath(f.c_str()))
      {
      f = "/" + f;
      f = this->Makefile->GetCurrentDirectory() + f;
      }

    if(cmSystemTools::FileExists(f.c_str()))
      {
      if(this->ReadListFile(f.c_str()))
        {
        found = true;
        }
      else
        {
        result = false;
        }
      }
    else
      {
      cmOStringStream e;
      e << this->Variable << " is set to \"" << def << "\", which is "
        << "not a directory containing " << this->Config;
      cmSystemTools::Error(e.str().c_str());
      if(required)
        {
        cmSystemTools::SetFatalErrorOccured();
        }
      result = true;
      }
    }
  else if(!quiet || required)
    {
    cmOStringStream e;
    e << "FIND_PACKAGE could not find Find" << this->Name
      << ".cmake nor config file " << this->Config << ".\n"
      << "Adjust CMAKE_MODULE_PATH to find Find" << this->Name
      << ".cmake or set " << this->Variable
      << "\nto the directory containing " << this->Config
      << " in order to use " << this->Name << ".";
    cmSystemTools::Error(e.str().c_str());
    if(required)
      {
      cmSystemTools::SetFatalErrorOccured();
      }
    result = true;
    }

  // Set a variable marking whether the package was found.
  std::string foundVar = this->Name;
  foundVar += "_FOUND";
  this->Makefile->AddDefinition(foundVar.c_str(), found? "1":"0");

  if(needCompatibility)
    {
    // Listfiles will be looking for the capitalized version of the
    // name.  Provide it.
    this->Makefile->AddDefinition
      (upperDir.c_str(),
       this->Makefile->GetDefinition(this->Variable.c_str()));
    this->Makefile->AddDefinition
      (upperFound.c_str(),
       this->Makefile->GetDefinition(foundVar.c_str()));
    }

#ifdef CMAKE_BUILD_WITH_CMAKE
  if(!(upperDir == this->Variable))
    {
    if(needCompatibility)
      {
      // Listfiles may use the capitalized version of the name.
      // Remove any previously added watch.
      this->Makefile->GetVariableWatch()->RemoveWatch(
        upperDir.c_str(),
        cmFindPackageNeedBackwardsCompatibility
        );
      }
    else
      {
      // Listfiles should not be using the capitalized version of the
      // name.  Add a watch to warn the user.
      this->Makefile->GetVariableWatch()->AddWatch(
        upperDir.c_str(),
        cmFindPackageNeedBackwardsCompatibility
        );
      }
    }
#endif

  this->AppendSuccessInformation(quiet);
  return result;
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::FindModule(bool& found, bool quiet, bool required)
{
  std::string module = "Find";
  module += this->Name;
  module += ".cmake";
  std::string mfile = this->Makefile->GetModulesFile(module.c_str());
  if ( mfile.size() )
    {
    if(quiet)
      {
      // Tell the module that is about to be read that it should find
      // quietly.
      std::string quietly = this->Name;
      quietly += "_FIND_QUIETLY";
      this->Makefile->AddDefinition(quietly.c_str(), "1");
      }

    if(required)
      {
      // Tell the module that is about to be read that it should report
      // a fatal error if the package is not found.
      std::string req = this->Name;
      req += "_FIND_REQUIRED";
      this->Makefile->AddDefinition(req.c_str(), "1");
      }

    // Load the module we found.
    found = true;
    return this->ReadListFile(mfile.c_str());
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::FindConfig()
{
  std::string help = "The directory containing ";
  help += this->Config;
  help += ".";

  // Construct the list of relative paths to each prefix to be
  // searched.
  std::string rel = "/lib/";
  rel += cmSystemTools::LowerCase(this->Name);
  this->Relatives.push_back(rel);
  rel = "/lib/";
  rel += this->Name;
  this->Relatives.push_back(rel);

  // It is likely that CMake will have recently built the project.
  for(int i=1; i <= 10; ++i)
    {
    cmOStringStream r;
    r << "[HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\"
      "Settings\\StartPath;WhereBuild" << i << "]";
    std::string entry = r.str();
    cmSystemTools::ExpandRegistryValues(entry);
    cmSystemTools::ConvertToUnixSlashes(entry);
    if(cmSystemTools::FileIsDirectory(entry.c_str()))
      {
      this->Builds.push_back(entry);
      }
    }

  // The project may be installed.  Use the system search path to
  // construct a list of possible install prefixes.
  std::vector<std::string> systemPath;
  cmSystemTools::GetPath(systemPath);
  for(std::vector<std::string>::iterator i = systemPath.begin();
      i != systemPath.end(); ++i)
    {
    *i += "/..";
    if(cmSystemTools::FileIsDirectory(i->c_str()))
      {
      this->Prefixes.push_back(cmSystemTools::CollapseFullPath(i->c_str()));
      }
    }
#if !defined(WIN32) || defined(__CYGWIN__)
  this->Prefixes.push_back("/usr/local");
  this->Prefixes.push_back("/usr");
#endif

  // Look for the project's configuration file.
  std::string init = this->SearchForConfig();

  // Store the entry in the cache so it can be set by the user.
  this->Makefile->AddCacheDefinition(this->Variable.c_str(),
                                 init.c_str(),
                                 help.c_str(),
                                 cmCacheManager::PATH);
  return true;
}

//----------------------------------------------------------------------------
std::string cmFindPackageCommand::SearchForConfig() const
{
  // Check the environment variable.
  std::string env;
  if(cmSystemTools::GetEnv(this->Variable.c_str(), env) && env.length() > 0)
    {
    cmSystemTools::ConvertToUnixSlashes(env);
    std::string f = env;
    f += "/";
    f += this->Config;
    if(cmSystemTools::FileExists(f.c_str()))
      {
      return env;
      }
    }

  // Search the build directories.
  for(std::vector<cmStdString>::const_iterator b = this->Builds.begin();
      b != this->Builds.end(); ++b)
    {
    std::string f = *b;
    f += "/";
    f += this->Config;
    if(cmSystemTools::FileExists(f.c_str()))
      {
      return *b;
      }
    }

  // Search paths relative to each installation prefix.
  for(std::vector<cmStdString>::const_iterator p = this->Prefixes.begin();
      p != this->Prefixes.end(); ++p)
    {
    std::string prefix = *p;
    for(std::vector<cmStdString>::const_iterator r = this->Relatives.begin();
        r != this->Relatives.end(); ++r)
      {
      std::string dir = prefix;
      dir += *r;
      std::string f = dir;
      f += "/";
      f += this->Config;
      if(cmSystemTools::FileExists(f.c_str()))
        {
        return dir;
        }
      }
    }

  return this->Variable + "-NOTFOUND";
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::ReadListFile(const char* f)
{
  if(this->Makefile->ReadListFile(this->Makefile->GetCurrentListFile(),f))
    {
    return true;
    }
  std::string e = "Error reading CMake code from \"";
  e += f;
  e += "\".";
  this->SetError(e.c_str());
  return false;
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::AppendToProperty(const char* propertyName)
{
  std::string propertyValue;
  const char *prop =
      this->Makefile->GetCMakeInstance()->GetProperty(propertyName);
  if (prop && *prop)
    {
    propertyValue = prop;

    std::vector<std::string> contents;
    cmSystemTools::ExpandListArgument(propertyValue, contents, false);

    bool alreadyInserted = false;
    for(std::vector<std::string>::const_iterator it = contents.begin();
      it != contents.end(); ++ it )
      {
      if (*it == this->Name)
        {
        alreadyInserted = true;
        break;
        }
      }
    if (!alreadyInserted)
      {
      propertyValue += ";";
      propertyValue += this->Name;
      }
    }
  else
    {
    propertyValue = this->Name;
    }
  this->Makefile->GetCMakeInstance()->SetProperty(propertyName,
                                                  propertyValue.c_str());
 }

//----------------------------------------------------------------------------
void cmFindPackageCommand::AppendSuccessInformation(bool quiet)
{
  std::string found = this->Name;
  found += "_FOUND";
  std::string upperFound = cmSystemTools::UpperCase(found);

  const char* upperResult = this->Makefile->GetDefinition(upperFound.c_str());
  const char* result = this->Makefile->GetDefinition(found.c_str());
  if ((cmSystemTools::IsOn(result)) || (cmSystemTools::IsOn(upperResult)))
    {
    this->AppendToProperty("PACKAGES_FOUND");
    if (!quiet)
      {
      this->AppendToProperty("ENABLED_FEATURES");
      }
    }
  else
    {
    this->AppendToProperty("PACKAGES_NOT_FOUND");
    if (!quiet)
      {
      this->AppendToProperty("DISABLED_FEATURES");
      }
    }
}
