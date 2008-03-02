/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmMakefile.cxx,v $
  Language:  C++
  Date:      $Date: 2008/03/01 21:21:41 $
  Version:   $Revision: 1.441 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmMakefile.h"
#include "cmVersion.h"
#include "cmCommand.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocation.h"
#include "cmSystemTools.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmCommands.h"
#include "cmCacheManager.h"
#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"
#include "cmCommandArgumentParserHelper.h"
#include "cmTest.h"
#ifdef CMAKE_BUILD_WITH_CMAKE
#  include "cmVariableWatch.h"
#endif
#include "cmInstallGenerator.h"
#include "cmake.h"
#include <stdlib.h> // required for atoi

#include <cmsys/RegularExpression.hxx>

#include <cmsys/auto_ptr.hxx>

#include <ctype.h> // for isspace

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

// default is not to be building executables
cmMakefile::cmMakefile()
{
  this->DefinitionStack.push_back(DefinitionMap());

  // Setup the default include file regular expression (match everything).
  this->IncludeFileRegularExpression = "^.*$";
  // Setup the default include complaint regular expression (match nothing).
  this->ComplainFileRegularExpression = "^$";
  // Source and header file extensions that we can handle

  // Set up a list of source and header extensions
  // these are used to find files when the extension
  // is not given
  // The "c" extension MUST precede the "C" extension.
  this->SourceFileExtensions.push_back( "c" );
  this->SourceFileExtensions.push_back( "C" );

  this->SourceFileExtensions.push_back( "c++" );
  this->SourceFileExtensions.push_back( "cc" );
  this->SourceFileExtensions.push_back( "cpp" );
  this->SourceFileExtensions.push_back( "cxx" );
  this->SourceFileExtensions.push_back( "m" );
  this->SourceFileExtensions.push_back( "M" );
  this->SourceFileExtensions.push_back( "mm" );

  this->HeaderFileExtensions.push_back( "h" );
  this->HeaderFileExtensions.push_back( "hh" );
  this->HeaderFileExtensions.push_back( "h++" );
  this->HeaderFileExtensions.push_back( "hm" );
  this->HeaderFileExtensions.push_back( "hpp" );
  this->HeaderFileExtensions.push_back( "hxx" );
  this->HeaderFileExtensions.push_back( "in" );
  this->HeaderFileExtensions.push_back( "txx" );

  this->DefineFlags = " ";
  this->LocalGenerator = 0;

#if defined(CMAKE_BUILD_WITH_CMAKE)
  this->AddSourceGroup("", "^.*$");
  this->AddSourceGroup
    ("Source Files",
     "\\.(C|M|c|c\\+\\+|cc|cpp|cxx|m|mm|rc|def|r|odl|idl|hpj|bat)$");
  this->AddSourceGroup("Header Files",
                       "\\.(h|hh|h\\+\\+|hm|hpp|hxx|in|txx|inl)$");
  this->AddSourceGroup("CMake Rules", "\\.rule$");
  this->AddSourceGroup("Resources", "\\.plist$");
#endif
  this->AddDefaultDefinitions();
  this->Initialize();
  this->PreOrder = false;
}

cmMakefile::cmMakefile(const cmMakefile& mf)
{
  this->Prefix = mf.Prefix;
  this->AuxSourceDirectories = mf.AuxSourceDirectories;
  this->cmStartDirectory = mf.cmStartDirectory;
  this->StartOutputDirectory = mf.StartOutputDirectory;
  this->cmHomeDirectory = mf.cmHomeDirectory;
  this->HomeOutputDirectory = mf.HomeOutputDirectory;
  this->cmCurrentListFile = mf.cmCurrentListFile;
  this->ProjectName = mf.ProjectName;
  this->Targets = mf.Targets;
  this->SourceFiles = mf.SourceFiles;
  this->Tests = mf.Tests;
  this->IncludeDirectories = mf.IncludeDirectories;
  this->LinkDirectories = mf.LinkDirectories;
  this->SystemIncludeDirectories = mf.SystemIncludeDirectories;
  this->ListFiles = mf.ListFiles;
  this->OutputFiles = mf.OutputFiles;
  this->LinkLibraries = mf.LinkLibraries;
  this->InstallGenerators = mf.InstallGenerators;
  this->IncludeFileRegularExpression = mf.IncludeFileRegularExpression;
  this->ComplainFileRegularExpression = mf.ComplainFileRegularExpression;
  this->SourceFileExtensions = mf.SourceFileExtensions;
  this->HeaderFileExtensions = mf.HeaderFileExtensions;
  this->DefineFlags = mf.DefineFlags;

#if defined(CMAKE_BUILD_WITH_CMAKE)
  this->SourceGroups = mf.SourceGroups;
#endif

  this->DefinitionStack.push_back(mf.DefinitionStack.back());
  this->LocalGenerator = mf.LocalGenerator;
  this->FunctionBlockers = mf.FunctionBlockers;
  this->DataMap = mf.DataMap;
  this->MacrosMap = mf.MacrosMap;
  this->SubDirectoryOrder = mf.SubDirectoryOrder;
  this->TemporaryDefinitionKey = mf.TemporaryDefinitionKey;
  this->Properties = mf.Properties;
  this->PreOrder = mf.PreOrder;
  this->ListFileStack = mf.ListFileStack;
  this->Initialize();
}

//----------------------------------------------------------------------------
void cmMakefile::Initialize()
{
  this->cmDefineRegex.compile("#cmakedefine[ \t]+([A-Za-z_0-9]*)");
  this->cmDefine01Regex.compile("#cmakedefine01[ \t]+([A-Za-z_0-9]*)");
  this->cmAtVarRegex.compile("(@[A-Za-z_0-9/.+-]+@)");
}

unsigned int cmMakefile::GetCacheMajorVersion()
{
  return this->GetCacheManager()->GetCacheMajorVersion();
}

unsigned int cmMakefile::GetCacheMinorVersion()
{
  return this->GetCacheManager()->GetCacheMinorVersion();
}

bool cmMakefile::NeedCacheCompatibility(int major, int minor)
{
  return this->GetCacheManager()->NeedCacheCompatibility(major, minor);
}

cmMakefile::~cmMakefile()
{
  for(std::vector<cmInstallGenerator*>::iterator
        i = this->InstallGenerators.begin();
      i != this->InstallGenerators.end(); ++i)
    {
    delete *i;
    }
  for(std::vector<cmSourceFile*>::iterator i = this->SourceFiles.begin();
      i != this->SourceFiles.end(); ++i)
    {
    delete *i;
    }
  for(std::vector<cmTest*>::iterator i = this->Tests.begin();
      i != this->Tests.end(); ++i)
    {
    delete *i;
    }
  for(std::vector<cmTarget*>::iterator
        i = this->ImportedTargetsOwned.begin();
      i != this->ImportedTargetsOwned.end(); ++i)
    {
    delete *i;
    }
  for(unsigned int i=0; i < this->UsedCommands.size(); i++)
    {
    delete this->UsedCommands[i];
    }
  for(DataMapType::const_iterator d = this->DataMap.begin();
      d != this->DataMap.end(); ++d)
    {
    if(d->second)
      {
      delete d->second;
      }
    }
  std::list<cmFunctionBlocker *>::iterator pos;
  for (pos = this->FunctionBlockers.begin();
       pos != this->FunctionBlockers.end(); ++pos)
    {
    cmFunctionBlocker* b = *pos;
    delete b;
    }
  this->FunctionBlockers.clear();
}

void cmMakefile::PrintStringVector(const char* s,
                                   const std::vector<std::string>& v) const
{
  std::cout << s << ": ( \n";
  for(std::vector<std::string>::const_iterator i = v.begin();
      i != v.end(); ++i)
    {
    std::cout << (*i).c_str() << " ";
    }
  std::cout << " )\n";
}

void cmMakefile
::PrintStringVector(const char* s,
                    const std::vector<std::pair<cmStdString, bool> >& v) const
{
  std::cout << s << ": ( \n";
  for(std::vector<std::pair<cmStdString, bool> >::const_iterator i
        = v.begin(); i != v.end(); ++i)
    {
    std::cout << i->first.c_str() << " " << i->second;
    }
  std::cout << " )\n";
}


// call print on all the classes in the makefile
void cmMakefile::Print()
{
  // print the class lists
  std::cout << "classes:\n";

  std::cout << " this->Targets: ";
  for (cmTargets::iterator l = this->Targets.begin();
       l != this->Targets.end(); l++)
    {
    std::cout << l->first << std::endl;
    }

  std::cout << " this->StartOutputDirectory; " <<
    this->StartOutputDirectory.c_str() << std::endl;
  std::cout << " this->HomeOutputDirectory; " <<
    this->HomeOutputDirectory.c_str() << std::endl;
  std::cout << " this->cmStartDirectory; " <<
    this->cmStartDirectory.c_str() << std::endl;
  std::cout << " this->cmHomeDirectory; " <<
    this->cmHomeDirectory.c_str() << std::endl;
  std::cout << " this->ProjectName; "
            <<  this->ProjectName.c_str() << std::endl;
  this->PrintStringVector("this->IncludeDirectories;",
                          this->IncludeDirectories);
  this->PrintStringVector("this->LinkDirectories", this->LinkDirectories);
#if defined(CMAKE_BUILD_WITH_CMAKE)
  for( std::vector<cmSourceGroup>::const_iterator i =
         this->SourceGroups.begin(); i != this->SourceGroups.end(); ++i)
    {
    std::cout << "Source Group: " << i->GetName() << std::endl;
    }
#endif
}

bool cmMakefile::CommandExists(const char* name) const
{
  return this->GetCMakeInstance()->CommandExists(name);
}

bool cmMakefile::ExecuteCommand(const cmListFileFunction& lff,
                                cmExecutionStatus &status)
{
  bool result = true;

  // quick return if blocked
  if(this->IsFunctionBlocked(lff,status))
    {
    // No error.
    return result;
    }
  
  std::string name = lff.Name;

  // execute the command
  cmCommand *rm =
    this->GetCMakeInstance()->GetCommand(name.c_str());
  if(rm)
    {
    // const char* versionValue
      // = this->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
    // int major = 0;
    // int minor = 0;
    // if ( versionValue )
      // {
      // sscanf(versionValue, "%d.%d", &major, &minor);
      // }
    cmCommand* usedCommand = rm->Clone();
    usedCommand->SetMakefile(this);
    bool keepCommand = false;
    if(usedCommand->GetEnabled() && !cmSystemTools::GetFatalErrorOccured()  &&
       (!this->GetCMakeInstance()->GetScriptMode() ||
        usedCommand->IsScriptable()))
      {
      if(!usedCommand->InvokeInitialPass(lff.Arguments,status))
        {
        cmOStringStream error;
        error << "Error in cmake code at\n"
              << lff.FilePath << ":" << lff.Line << ":\n"
              << usedCommand->GetError() << std::endl
              << "   Called from: " << this->GetListFileStack().c_str();
        cmSystemTools::Error(error.str().c_str());
        result = false;
        if ( this->GetCMakeInstance()->GetScriptMode() )
          {
          cmSystemTools::SetFatalErrorOccured();
          }
        }
      else
        {
        // use the command
        keepCommand = true;
        this->UsedCommands.push_back(usedCommand);
        }
      }
    else if ( this->GetCMakeInstance()->GetScriptMode()
              && !usedCommand->IsScriptable() )
      {
      cmOStringStream error;
      error << "Error in cmake code at\n"
            << lff.FilePath << ":" << lff.Line << ":\n"
            << "Command " << usedCommand->GetName()
            << "() is not scriptable" << std::endl;
      cmSystemTools::Error(error.str().c_str());
      result = false;
      cmSystemTools::SetFatalErrorOccured();
      }
    // if the Cloned command was not used
    // then delete it
    if(!keepCommand)
      {
      delete usedCommand;
      }
    }
  else
    {
    if(!cmSystemTools::GetFatalErrorOccured())
      {
      cmOStringStream error;
      error << "Error in cmake code at\n"
            << lff.FilePath << ":" << lff.Line << ":\n"
            << "Unknown CMake command \"" << lff.Name.c_str() << "\".";
      cmSystemTools::Error(error.str().c_str());
      result = false;
      cmSystemTools::SetFatalErrorOccured();
      }
    }

  return result;
}

// Parse the given CMakeLists.txt file executing all commands
//
bool cmMakefile::ReadListFile(const char* filename_in,
                              const char *external_in,
                              std::string* fullPath)
{
  std::string currentParentFile
    = this->GetSafeDefinition("CMAKE_PARENT_LIST_FILE");
  std::string currentFile
    = this->GetSafeDefinition("CMAKE_CURRENT_LIST_FILE");
  this->AddDefinition("CMAKE_PARENT_LIST_FILE", filename_in);

  // used to watch for blockers going out of scope
  // e.g. mismatched IF statement
  std::set<cmFunctionBlocker *> originalBlockers;

  const char* external = 0;
  std::string external_abs;

  const char* filename = filename_in;
  std::string filename_abs;

  if (external_in)
    {
    external_abs =
      cmSystemTools::CollapseFullPath(external_in,
                                      this->cmStartDirectory.c_str());
    external = external_abs.c_str();
    if (filename_in)
      {
      filename_abs =
        cmSystemTools::CollapseFullPath(filename_in,
                                        this->cmStartDirectory.c_str());
      filename = filename_abs.c_str();
      }
    }

  // keep track of the current file being read
  if (filename)
    {
    if(this->cmCurrentListFile != filename)
      {
      this->cmCurrentListFile = filename;
      }
    // loop over current function blockers and record them
    std::list<cmFunctionBlocker *>::iterator pos;
    for (pos = this->FunctionBlockers.begin();
         pos != this->FunctionBlockers.end(); ++pos)
      {
      originalBlockers.insert(*pos);
      }
    }

  // Now read the input file
  const char *filenametoread= filename;

  if( external)
    {
    filenametoread= external;
    }

  this->AddDefinition("CMAKE_CURRENT_LIST_FILE", filenametoread);

  // try to see if the list file is the top most
  // list file for a project, and if it is, then it
  // must have a project command.   If there is not
  // one, then cmake will provide one via the
  // cmListFileCache class.
  bool requireProjectCommand = false;
  if(!external && this->cmStartDirectory == this->cmHomeDirectory)
    {
    if(cmSystemTools::LowerCase(
      cmSystemTools::GetFilenameName(filename)) == "cmakelists.txt")
      {
      requireProjectCommand = true;
      }
    }

  // push the listfile onto the stack
  this->ListFileStack.push_back(filenametoread);
  if(fullPath!=0)
    {
    *fullPath=filenametoread;
    }


  // *** Here is the core listfile reading code ***
  // is it a lua file ?
  if (cmSystemTools::GetFilenameLastExtension(filenametoread) == ".lua")
    {
    lua_State *L = this->GetCMakeInstance()->GetLuaState();
    this->ListFiles.push_back( filenametoread);

	// Run utility helper
	std::string lua_helper_file;
	lua_helper_file = this->GetModulesFile("LuaPublicAPIHelper.lua");
	std::cerr << "Path to lua_helper file is: " << lua_helper_file << std::endl;
	
	int s = luaL_loadfile(L, lua_helper_file.c_str());
    if ( s==0 )
	{
      // execute Lua program
      s = lua_pcall(L, 0, 0, 0);
		if ( s!=0 )
		{
			std::cerr << "-- " << lua_tostring(L, -1) << std::endl;
			lua_pop(L, 1);
		}
	}
	else
	{
			std::cerr << "-- " << lua_tostring(L, -1) << std::endl;
			lua_pop(L, 1);
	}



    // get the current makefile setting
    lua_pushstring(L,"cmCurrentMakefile");
    lua_gettable(L, LUA_REGISTRYINDEX);
    cmMakefile *previousMF = 
      static_cast<cmMakefile *>(lua_touserdata(L,-1));

    // make sure the current Makefile is set
    lua_pushstring(L,"cmCurrentMakefile");
    lua_pushlightuserdata(L, this);
    lua_settable(L, LUA_REGISTRYINDEX);

    // load and run the lua
    int result = luaL_dofile(L, filenametoread);

    // restore the prior makefile setting
    lua_pushstring(L,"cmCurrentMakefile");
    lua_pushlightuserdata(L, previousMF);
    lua_settable(L, LUA_REGISTRYINDEX);

    if (result)
      {
      cmSystemTools::Error("Error in Lua code: ",
                           lua_tostring(L, -1));
      lua_pop(L, 1);  /* pop error message from the stack */

      // pop the listfile off the stack
      this->ListFileStack.pop_back();
      if(fullPath!=0)
        {
        *fullPath = "";
        }
      this->AddDefinition("CMAKE_PARENT_LIST_FILE", currentParentFile.c_str());
      this->AddDefinition("CMAKE_CURRENT_LIST_FILE", currentFile.c_str());
      return false;
      }
    }
  else
    {
  cmListFile cacheFile;
  if( !cacheFile.ParseFile(filenametoread, requireProjectCommand) )
    {
    // pop the listfile off the stack
    this->ListFileStack.pop_back();
    if(fullPath!=0)
      {
      *fullPath = "";
      }
    this->AddDefinition("CMAKE_PARENT_LIST_FILE", currentParentFile.c_str());
    this->AddDefinition("CMAKE_CURRENT_LIST_FILE", currentFile.c_str());
    return false;
    }
  // add this list file to the list of dependencies
  this->ListFiles.push_back( filenametoread);
  const size_t numberFunctions = cacheFile.Functions.size();
  for(size_t i =0; i < numberFunctions; ++i)
    {
    cmExecutionStatus status;
    this->ExecuteCommand(cacheFile.Functions[i],status);
    if (status.GetReturnInvoked() ||
        cmSystemTools::GetFatalErrorOccured() )
      {
      // pop the listfile off the stack
      this->ListFileStack.pop_back();
      this->AddDefinition("CMAKE_PARENT_LIST_FILE",
                          currentParentFile.c_str());
      this->AddDefinition("CMAKE_CURRENT_LIST_FILE", currentFile.c_str());
      return true;
      }
    }
  }
  // *** end of listfile code

  // send scope ended to and function blockers
  if (filename)
    {
    // loop over all function blockers to see if any block this command
    std::list<cmFunctionBlocker *>::iterator pos;
    for (pos = this->FunctionBlockers.begin();
         pos != this->FunctionBlockers.end(); ++pos)
      {
      // if this blocker was not in the original then send a
      // scope ended message
      if (originalBlockers.find(*pos) == originalBlockers.end())
        {
        (*pos)->ScopeEnded(*this);
        }
      }
    }

  this->AddDefinition("CMAKE_PARENT_LIST_FILE", currentParentFile.c_str());
  this->AddDefinition("CMAKE_CURRENT_LIST_FILE", currentFile.c_str());

  // pop the listfile off the stack
  this->ListFileStack.pop_back();

  return true;
}


void cmMakefile::AddCommand(cmCommand* wg)
{
  this->GetCMakeInstance()->AddCommand(wg);
}

// Set the make file
void cmMakefile::SetLocalGenerator(cmLocalGenerator* lg)
{
  this->LocalGenerator = lg;
}

bool cmMakefile::NeedBackwardsCompatibility(unsigned int major,
                                            unsigned int minor,
                                            unsigned int patch)
{
  if(this->LocalGenerator)
    {
    return
      this->LocalGenerator->NeedBackwardsCompatibility(major, minor, patch);
    }
  else
    {
    return false;
    }
}

void cmMakefile::FinalPass()
{
  // do all the variable expansions here
  this->ExpandVariables();

  // give all the commands a chance to do something
  // after the file has been parsed before generation
  for(std::vector<cmCommand*>::iterator i = this->UsedCommands.begin();
      i != this->UsedCommands.end(); ++i)
    {
    (*i)->FinalPass();
    }

}

// Generate the output file
void cmMakefile::ConfigureFinalPass()
{
  this->FinalPass();
  const char* oldValue
    = this->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
  if (oldValue && atof(oldValue) <= 1.2)
    {
    cmSystemTools::Error("You have requested backwards compatibility "
                         "with CMake version 1.2 or earlier. This version "
                         "of CMake only supports backwards compatibility "
                         "with CMake 1.4 or later. For compatibility with "
                         "1.2 or earlier please use CMake 2.0");
    }
  for (cmTargets::iterator l = this->Targets.begin();
       l != this->Targets.end(); l++)
    {
    l->second.AnalyzeLibDependencies(*this);
    }
}

//----------------------------------------------------------------------------
void
cmMakefile::AddCustomCommandToTarget(const char* target,
                                     const std::vector<std::string>& depends,
                                     const cmCustomCommandLines& commandLines,
                                     cmTarget::CustomCommandType type,
                                     const char* comment,
                                     const char* workingDir,
                                     bool escapeOldStyle)
{
  // Find the target to which to add the custom command.
  cmTargets::iterator ti = this->Targets.find(target);
  if(ti != this->Targets.end())
    {
    // Add the command to the appropriate build step for the target.
    std::vector<std::string> no_output;
    cmCustomCommand cc(no_output, depends, commandLines, comment, workingDir);
    cc.SetEscapeOldStyle(escapeOldStyle);
    switch(type)
      {
      case cmTarget::PRE_BUILD:
        ti->second.GetPreBuildCommands().push_back(cc);
        break;
      case cmTarget::PRE_LINK:
        ti->second.GetPreLinkCommands().push_back(cc);
        break;
      case cmTarget::POST_BUILD:
        ti->second.GetPostBuildCommands().push_back(cc);
        break;
      }
    }
}

//----------------------------------------------------------------------------
void
cmMakefile::AddCustomCommandToOutput(const std::vector<std::string>& outputs,
                                     const std::vector<std::string>& depends,
                                     const char* main_dependency,
                                     const cmCustomCommandLines& commandLines,
                                     const char* comment,
                                     const char* workingDir,
                                     bool replace,
                                     bool escapeOldStyle)
{
  // Make sure there is at least one output.
  if(outputs.empty())
    {
    cmSystemTools::Error("Attempt to add a custom rule with no output!");
    return;
    }

  // Choose a source file on which to store the custom command.
  cmSourceFile* file = 0;
  if(main_dependency && main_dependency[0])
    {
    // The main dependency was specified.  Use it unless a different
    // custom command already used it.
    file = this->GetSource(main_dependency);
    if(file && file->GetCustomCommand() && !replace)
      {
      // The main dependency already has a custom command.
      if(commandLines == file->GetCustomCommand()->GetCommandLines())
        {
        // The existing custom command is identical.  Silently ignore
        // the duplicate.
        return;
        }
      else
        {
        // The existing custom command is different.  We need to
        // generate a rule file for this new command.
        file = 0;
        }
      }
    else
      {
      // The main dependency does not have a custom command or we are
      // allowed to replace it.  Use it to store the command.
      file = this->GetOrCreateSource(main_dependency);
      }
    }

  // Generate a rule file if the main dependency is not available.
  if(!file)
    {
    // Construct a rule file associated with the first output produced.
    std::string outName = outputs[0];
    outName += ".rule";

    // Check if the rule file already exists.
    file = this->GetSource(outName.c_str());
    if(file && file->GetCustomCommand() && !replace)
      {
      // The rule file already exists.
      if(commandLines != file->GetCustomCommand()->GetCommandLines())
        {
        cmSystemTools::Error("Attempt to add a custom rule to output \"",
                             outName.c_str(),
                             "\" which already has a custom rule.");
        }
      return;
      }

    // Create a cmSourceFile for the rule file.
    file = this->GetOrCreateSource(outName.c_str(), true);
    }

  // Always create the output sources and mark them generated.
  for(std::vector<std::string>::const_iterator o = outputs.begin();
      o != outputs.end(); ++o)
    {
    if(cmSourceFile* out = this->GetOrCreateSource(o->c_str(), true))
      {
      out->SetProperty("GENERATED", "1");
      }
    }

  // Construct a complete list of dependencies.
  std::vector<std::string> depends2(depends);
  if(main_dependency && main_dependency[0])
    {
    depends2.push_back(main_dependency);
    }

  // Attach the custom command to the file.
  if(file)
    {
    cmCustomCommand* cc =
      new cmCustomCommand(outputs, depends2, commandLines,
                          comment, workingDir);
    cc->SetEscapeOldStyle(escapeOldStyle);
    file->SetCustomCommand(cc);
    }
}

//----------------------------------------------------------------------------
void
cmMakefile::AddCustomCommandToOutput(const char* output,
                                     const std::vector<std::string>& depends,
                                     const char* main_dependency,
                                     const cmCustomCommandLines& commandLines,
                                     const char* comment,
                                     const char* workingDir,
                                     bool replace,
                                     bool escapeOldStyle)
{
  std::vector<std::string> outputs;
  outputs.push_back(output);
  this->AddCustomCommandToOutput(outputs, depends, main_dependency,
                                 commandLines, comment, workingDir,
                                 replace, escapeOldStyle);
}

//----------------------------------------------------------------------------
void
cmMakefile::AddCustomCommandOldStyle(const char* target,
                                     const std::vector<std::string>& outputs,
                                     const std::vector<std::string>& depends,
                                     const char* source,
                                     const cmCustomCommandLines& commandLines,
                                     const char* comment)
{
  // Translate the old-style signature to one of the new-style
  // signatures.
  if(strcmp(source, target) == 0)
    {
    // In the old-style signature if the source and target were the
    // same then it added a post-build rule to the target.  Preserve
    // this behavior.
    this->AddCustomCommandToTarget(target, depends, commandLines,
                                   cmTarget::POST_BUILD, comment, 0);
    return;
    }

  // Each output must get its own copy of this rule.
  cmsys::RegularExpression sourceFiles("\\.(C|M|c|c\\+\\+|cc|cpp|cxx|m|mm|"
                                       "rc|def|r|odl|idl|hpj|bat|h|h\\+\\+|"
                                       "hm|hpp|hxx|in|txx|inl)$");
  for(std::vector<std::string>::const_iterator oi = outputs.begin();
      oi != outputs.end(); ++oi)
    {
    // Get the name of this output.
    const char* output = oi->c_str();

    // Choose whether to use a main dependency.
    if(sourceFiles.find(source))
      {
      // The source looks like a real file.  Use it as the main dependency.
      this->AddCustomCommandToOutput(output, depends, source,
                                     commandLines, comment, 0);
      }
    else
      {
      // The source may not be a real file.  Do not use a main dependency.
      const char* no_main_dependency = 0;
      std::vector<std::string> depends2 = depends;
      depends2.push_back(source);
      this->AddCustomCommandToOutput(output, depends2, no_main_dependency,
                                     commandLines, comment, 0);
      }

    // If the rule was added to the source (and not a .rule file),
    // then add the source to the target to make sure the rule is
    // included.
    std::string sname = output;
    sname += ".rule";
    if(!this->GetSource(sname.c_str()))
      {
      if (this->Targets.find(target) != this->Targets.end())
        {
        this->Targets[target].AddSource(source);
        }
      else
        {
        cmSystemTools::Error("Attempt to add a custom rule to a target "
                             "that does not exist yet for target ", target);
        return;
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmMakefile::AddUtilityCommand(const char* utilityName,
                                   bool excludeFromAll,
                                   const std::vector<std::string>& depends,
                                   const char* workingDirectory,
                                   const char* command,
                                   const char* arg1,
                                   const char* arg2,
                                   const char* arg3,
                                   const char* arg4)
{
  // Construct the command line for the custom command.
  cmCustomCommandLine commandLine;
  commandLine.push_back(command);
  if(arg1)
    {
    commandLine.push_back(arg1);
    }
  if(arg2)
    {
    commandLine.push_back(arg2);
    }
  if(arg3)
    {
    commandLine.push_back(arg3);
    }
  if(arg4)
    {
    commandLine.push_back(arg4);
    }
  cmCustomCommandLines commandLines;
  commandLines.push_back(commandLine);

  // Call the real signature of this method.
  this->AddUtilityCommand(utilityName, excludeFromAll, workingDirectory,
                          depends, commandLines);
}

//----------------------------------------------------------------------------
void cmMakefile::AddUtilityCommand(const char* utilityName,
                                   bool excludeFromAll,
                                   const char* workingDirectory,
                                   const std::vector<std::string>& depends,
                                   const cmCustomCommandLines& commandLines,
                                   bool escapeOldStyle, const char* comment)
{
  // Create a target instance for this utility.
  cmTarget* target = this->AddNewTarget(cmTarget::UTILITY, utilityName);
  if (excludeFromAll)
    {
    target->SetProperty("EXCLUDE_FROM_ALL", "TRUE");
    }
  if(!comment)
    {
    // Use an empty comment to avoid generation of default comment.
    comment = "";
    }

  // Store the custom command in the target.
  std::string force = this->GetStartOutputDirectory();
  force += cmake::GetCMakeFilesDirectory();
  force += "/";
  force += utilityName;
  const char* no_main_dependency = 0;
  bool no_replace = false;
  this->AddCustomCommandToOutput(force.c_str(), depends,
                                 no_main_dependency,
                                 commandLines, comment,
                                 workingDirectory, no_replace,
                                 escapeOldStyle);
  cmSourceFile* sf = target->AddSource(force.c_str());

  // The output is not actually created so mark it symbolic.
  if(sf)
    {
    sf->SetProperty("SYMBOLIC", "1");
    }
  else
    {
    cmSystemTools::Error("Could not get source file entry for ",
                         force.c_str());
    }
}

void cmMakefile::AddDefineFlag(const char* flag)
{
  if (!flag)
    {
    return;
    }

  // If this is really a definition, update COMPILE_DEFINITIONS.
  if(this->ParseDefineFlag(flag, false))
    {
    return;
    }

  // remove any \n\r
  std::string ret = flag;
  std::string::size_type pos = 0;
  while((pos = ret.find('\n', pos)) != std::string::npos)
    {
    ret[pos] = ' ';
    pos++;
    }
  pos = 0;
  while((pos = ret.find('\r', pos)) != std::string::npos)
    {
    ret[pos] = ' ';
    pos++;
    }

  this->DefineFlags += " ";
  this->DefineFlags += ret;
}


void cmMakefile::RemoveDefineFlag(const char* flag)
{
  // Check the length of the flag to remove.
  std::string::size_type len = strlen(flag);
  if(len < 1)
    {
    return;
    }

  // If this is really a definition, update COMPILE_DEFINITIONS.
  if(this->ParseDefineFlag(flag, true))
    {
    return;
    }

  // Remove all instances of the flag that are surrounded by
  // whitespace or the beginning/end of the string.
  for(std::string::size_type lpos = this->DefineFlags.find(flag, 0);
      lpos != std::string::npos; lpos = this->DefineFlags.find(flag, lpos))
    {
    std::string::size_type rpos = lpos + len;
    if((lpos <= 0 || isspace(this->DefineFlags[lpos-1])) &&
       (rpos >= this->DefineFlags.size() || isspace(this->DefineFlags[rpos])))
      {
      this->DefineFlags.erase(lpos, len);
      }
    else
      {
      ++lpos;
      }
    }
}

bool cmMakefile::ParseDefineFlag(std::string const& def, bool remove)
{
  // Create a regular expression to match valid definitions.
  // Definitions with non-trivial values must not be matched because
  // escaping them could break compatibility with escapes added by
  // users.
  static cmsys::RegularExpression
    regex("^[-/]D[A-Za-z_][A-Za-z0-9_]*(=[A-Za-z0-9_.]+)?$");

  // Make sure the definition matches.
  if(!regex.find(def.c_str()))
    {
    return false;
    }

  // VS6 IDE does not support definitions with values.
  if((strcmp(this->LocalGenerator->GetGlobalGenerator()->GetName(),
             "Visual Studio 6") == 0) &&
     (def.find("=") != def.npos))
    {
    return false;
    }

  // Get the definition part after the flag.
  const char* define = def.c_str() + 2;

  if(remove)
    {
    if(const char* cdefs = this->GetProperty("COMPILE_DEFINITIONS"))
      {
      // Expand the list.
      std::vector<std::string> defs;
      cmSystemTools::ExpandListArgument(cdefs, defs);

      // Recompose the list without the definition.
      std::string ndefs;
      const char* sep = "";
      for(std::vector<std::string>::const_iterator di = defs.begin();
          di != defs.end(); ++di)
        {
        if(*di != define)
          {
          ndefs += sep;
          sep = ";";
          ndefs += *di;
          }
        }

      // Store the new list.
      this->SetProperty("COMPILE_DEFINITIONS", ndefs.c_str());
      }
    }
  else
    {
    // Append the definition to the directory property.
    this->AppendProperty("COMPILE_DEFINITIONS", define);
    }

  return true;
}

void cmMakefile::AddLinkLibrary(const char* lib,
                                cmTarget::LinkLibraryType llt)
{
  cmTarget::LibraryID tmp;
  tmp.first = lib;
  tmp.second = llt;
  this->LinkLibraries.push_back(tmp);
}

void cmMakefile::AddLinkLibraryForTarget(const char *target,
                                         const char* lib,
                                         cmTarget::LinkLibraryType llt)
{
  cmTargets::iterator i = this->Targets.find(target);
  if ( i != this->Targets.end())
    {
    cmTarget* tgt =
      this->GetCMakeInstance()->GetGlobalGenerator()->FindTarget(0,lib);
    if(tgt)
      {
      bool allowModules = true;
      const char* versionValue
        = this->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
      if (versionValue &&  (atof(versionValue) >= 2.4) )
        {
        allowModules = false;
        }
      // if it is not a static or shared library then you can not link to it
      if(!((tgt->GetType() == cmTarget::STATIC_LIBRARY) ||
           (tgt->GetType() == cmTarget::SHARED_LIBRARY) ||
           tgt->IsExecutableWithExports()))
        {
        cmOStringStream e;
        e << "Attempt to add link target " << lib << " of type: "
          << cmTarget::TargetTypeNames[static_cast<int>(tgt->GetType())]
          << "\nto target " << target
          << ". One can only link to STATIC or SHARED libraries, or "
          << "to executables with the ENABLE_EXPORTS property set.";
        // in older versions of cmake linking to modules was allowed
        if( tgt->GetType() == cmTarget::MODULE_LIBRARY )
          {
          e <<
            "\nTo allow linking of modules set "
            "CMAKE_BACKWARDS_COMPATIBILITY to 2.2 or lower\n";
          }
        // if no modules are allowed then this is always an error
        if(!allowModules ||
           // if we allow modules but the type is not a module then it is
           // still an error
           (allowModules && tgt->GetType() != cmTarget::MODULE_LIBRARY))
          {
          cmSystemTools::Error(e.str().c_str());
          }
        }
      }
    i->second.AddLinkLibrary( *this, target, lib, llt );
    }
  else
    {
    cmOStringStream e;
    e << "Attempt to add link library \""
      << lib << "\" to target \""
      << target << "\" which is not built by this project.";
    cmSystemTools::Error(e.str().c_str());
    }
}

void cmMakefile::AddLinkDirectoryForTarget(const char *target,
                                           const char* d)
{
  cmTargets::iterator i = this->Targets.find(target);
  if ( i != this->Targets.end())
    {
    i->second.AddLinkDirectory( d );
    }
  else
    {
    cmSystemTools::Error
      ("Attempt to add link directories to non-existant target: ",
       target, " for directory ", d);
    }
}

void cmMakefile::AddLinkLibrary(const char* lib)
{
  this->AddLinkLibrary(lib,cmTarget::GENERAL);
}

void cmMakefile::AddLinkDirectory(const char* dir)
{
  // Don't add a link directory that is already present.  Yes, this
  // linear search results in n^2 behavior, but n won't be getting
  // much bigger than 20.  We cannot use a set because of order
  // dependency of the link search path.

  if(!dir)
    {
    return;
    }
  // remove trailing slashes
  if(dir[strlen(dir)-1] == '/')
    {
    std::string newdir = dir;
    newdir = newdir.substr(0, newdir.size()-1);
    if(std::find(this->LinkDirectories.begin(),
                 this->LinkDirectories.end(),
                 newdir.c_str()) == this->LinkDirectories.end())
      {
      this->LinkDirectories.push_back(newdir);
      }
    }
  else
    {
    if(std::find(this->LinkDirectories.begin(),
                 this->LinkDirectories.end(), dir)
       == this->LinkDirectories.end())
      {
      this->LinkDirectories.push_back(dir);
      }
    }
}

void cmMakefile::InitializeFromParent()
{
  cmMakefile *parent = this->LocalGenerator->GetParent()->GetMakefile();

  // copy the definitions
  this->DefinitionStack.front() = parent->DefinitionStack.back();

  // copy include paths
  this->IncludeDirectories = parent->IncludeDirectories;
  this->SystemIncludeDirectories = parent->SystemIncludeDirectories;

  // define flags
  this->DefineFlags = parent->DefineFlags;

  // compile definitions property and per-config versions
  {
  this->SetProperty("COMPILE_DEFINITIONS",
                    parent->GetProperty("COMPILE_DEFINITIONS"));
  std::vector<std::string> configs;
  if(const char* configTypes =
     this->GetDefinition("CMAKE_CONFIGURATION_TYPES"))
    {
    cmSystemTools::ExpandListArgument(configTypes, configs);
    }
  else if(const char* buildType =
          this->GetDefinition("CMAKE_BUILD_TYPE"))
    {
    configs.push_back(buildType);
    }
  for(std::vector<std::string>::const_iterator ci = configs.begin();
      ci != configs.end(); ++ci)
    {
    std::string defPropName = "COMPILE_DEFINITIONS_";
    defPropName += cmSystemTools::UpperCase(*ci);
    this->SetProperty(defPropName.c_str(),
                      parent->GetProperty(defPropName.c_str()));
    }
  }

  // link libraries
  this->LinkLibraries = parent->LinkLibraries;

  // link directories
  this->LinkDirectories = parent->LinkDirectories;

  // the initial project name
  this->ProjectName = parent->ProjectName;

  // Copy include regular expressions.
  this->IncludeFileRegularExpression = parent->IncludeFileRegularExpression;
  this->ComplainFileRegularExpression = parent->ComplainFileRegularExpression;

  // Imported targets.
  this->ImportedTargets = parent->ImportedTargets;
}

void cmMakefile::ConfigureSubDirectory(cmLocalGenerator *lg2)
{
  // copy our variables from the child makefile
  lg2->GetMakefile()->InitializeFromParent();
  lg2->GetMakefile()->MakeStartDirectoriesCurrent();
  if (this->GetCMakeInstance()->GetDebugOutput())
    {
    std::string msg="   Entering             ";
    msg += lg2->GetMakefile()->GetCurrentDirectory();
    cmSystemTools::Message(msg.c_str());
    }
  // finally configure the subdir
  lg2->Configure();
  if (this->GetCMakeInstance()->GetDebugOutput())
    {
    std::string msg="   Returning to         ";
    msg += this->GetCurrentDirectory();
    cmSystemTools::Message(msg.c_str());
    }
}

void cmMakefile::AddSubDirectory(const char* sub,
                                 bool excludeFromAll, bool preorder)
{
  // the source path must be made full if it isn't already
  std::string srcPath = sub;
  if (!cmSystemTools::FileIsFullPath(srcPath.c_str()))
    {
    srcPath = this->GetCurrentDirectory();
    srcPath += "/";
    srcPath += sub;
    }

  // binary path must be made full if it isn't already
  std::string binPath = sub;
  if (!cmSystemTools::FileIsFullPath(binPath.c_str()))
    {
    binPath = this->GetCurrentOutputDirectory();
    binPath += "/";
    binPath += sub;
    }


  this->AddSubDirectory(srcPath.c_str(), binPath.c_str(),
                        excludeFromAll, preorder, false);
}


void cmMakefile::AddSubDirectory(const char* srcPath, const char *binPath,
                                 bool excludeFromAll, bool preorder,
                                 bool immediate)
{
  std::vector<cmLocalGenerator *>& children =
    this->LocalGenerator->GetChildren();
  // has this directory already been added? If so error
  unsigned int i;
  for (i = 0; i < children.size(); ++i)
    {
    if (srcPath == children[i]->GetMakefile()->GetStartDirectory())
      {
      cmSystemTools::Error
        ("Attempt to add subdirectory multiple times for directory.\n",
         srcPath);
      return;
      }
    }

  // create a new local generator and set its parent
  cmLocalGenerator *lg2 =
    this->LocalGenerator->GetGlobalGenerator()->CreateLocalGenerator();
  lg2->SetParent(this->LocalGenerator);
  this->LocalGenerator->GetGlobalGenerator()->AddLocalGenerator(lg2);

  // set the subdirs start dirs
  lg2->GetMakefile()->SetStartDirectory(srcPath);
  lg2->GetMakefile()->SetStartOutputDirectory(binPath);
  if(excludeFromAll)
    {
    lg2->GetMakefile()->SetProperty("EXCLUDE_FROM_ALL", "TRUE");
    }
  lg2->GetMakefile()->SetPreOrder(preorder);

  if (immediate)
    {
    this->ConfigureSubDirectory(lg2);
    }
}

void cmMakefile::AddIncludeDirectory(const char* inc, bool before)
{
  // if there is a newline then break it into multiple arguments
  if (!inc)
    {
    return;
    }

  // Don't add an include directory that is already present.  Yes,
  // this linear search results in n^2 behavior, but n won't be
  // getting much bigger than 20.  We cannot use a set because of
  // order dependency of the include path.
  std::vector<std::string>::iterator i =
    std::find(this->IncludeDirectories.begin(),
              this->IncludeDirectories.end(), inc);
  if(i == this->IncludeDirectories.end())
    {
    if (before)
      {
      // WARNING: this *is* expensive (linear time) since it's a vector
      this->IncludeDirectories.insert(this->IncludeDirectories.begin(), inc);
      }
    else
      {
      this->IncludeDirectories.push_back(inc);
      }
    }
  else
    {
    if(before)
      {
      // if this before and already in the path then remove it
      this->IncludeDirectories.erase(i);
      // WARNING: this *is* expensive (linear time) since it's a vector
      this->IncludeDirectories.insert(this->IncludeDirectories.begin(), inc);
      }
    }
}

//----------------------------------------------------------------------------
void cmMakefile::AddSystemIncludeDirectory(const char* dir)
{
  this->SystemIncludeDirectories.insert(dir);
}

//----------------------------------------------------------------------------
bool cmMakefile::IsSystemIncludeDirectory(const char* dir)
{
  return (this->SystemIncludeDirectories.find(dir) !=
          this->SystemIncludeDirectories.end());
}

void cmMakefile::AddDefinition(const char* name, const char* value)
{
  if (!value )
    {
    return;
    }

#ifdef CMAKE_STRICT
  if (this->GetCMakeInstance())
    {
    this->GetCMakeInstance()->
      RecordPropertyAccess(name,cmProperty::VARIABLE);
    }
#endif

  this->TemporaryDefinitionKey = name;
  this->DefinitionStack.back()[this->TemporaryDefinitionKey] = value;

#ifdef CMAKE_BUILD_WITH_CMAKE
  cmVariableWatch* vv = this->GetVariableWatch();
  if ( vv )
    {
    vv->VariableAccessed(this->TemporaryDefinitionKey,
                         cmVariableWatch::VARIABLE_MODIFIED_ACCESS,
                         value,
                         this);
    }
#endif
}


void cmMakefile::AddCacheDefinition(const char* name, const char* value,
                                    const char* doc,
                                    cmCacheManager::CacheEntryType type)
{
  const char* val = value;
  cmCacheManager::CacheIterator it =
    this->GetCacheManager()->GetCacheIterator(name);
  if(!it.IsAtEnd() && (it.GetType() == cmCacheManager::UNINITIALIZED) &&
     it.Initialized())
    {
    val = it.GetValue();
    if ( type == cmCacheManager::PATH || type == cmCacheManager::FILEPATH )
      {
      std::vector<std::string>::size_type cc;
      std::vector<std::string> files;
      std::string nvalue = "";
      cmSystemTools::ExpandListArgument(val, files);
      for ( cc = 0; cc < files.size(); cc ++ )
        {
        files[cc] = cmSystemTools::CollapseFullPath(files[cc].c_str());
        if ( cc > 0 )
          {
          nvalue += ";";
          }
        nvalue += files[cc];
        }

      this->GetCacheManager()->AddCacheEntry(name, nvalue.c_str(), doc, type);
      val = it.GetValue();
      }

    }
  this->GetCacheManager()->AddCacheEntry(name, val, doc, type);
  // if there was a definition then remove it
  this->DefinitionStack.back().erase( DefinitionMap::key_type(name));
}


void cmMakefile::AddDefinition(const char* name, bool value)
{
  if(value)
    {
    this->DefinitionStack.back()
      .erase( DefinitionMap::key_type(name));
    this->DefinitionStack.back()
      .insert(DefinitionMap::value_type(name, "ON"));
    }
  else
    {
    this->DefinitionStack.back()
      .erase( DefinitionMap::key_type(name));
    this->DefinitionStack.back()
      .insert(DefinitionMap::value_type(name, "OFF"));
    }
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmVariableWatch* vv = this->GetVariableWatch();
  if ( vv )
    {
    vv->VariableAccessed(name, cmVariableWatch::VARIABLE_MODIFIED_ACCESS,
      value?"ON":"OFF", this);
    }
#endif
}


void cmMakefile::AddCacheDefinition(const char* name,
                                    bool value,
                                    const char* doc)
{
  bool val = value;
  cmCacheManager::CacheIterator it =
    this->GetCacheManager()->GetCacheIterator(name);
  if(!it.IsAtEnd() && (it.GetType() == cmCacheManager::UNINITIALIZED) &&
     it.Initialized())
    {
    val = it.GetValueAsBool();
    }
  this->GetCacheManager()->AddCacheEntry(name, val, doc);
  this->AddDefinition(name, val);
}

void cmMakefile::RemoveDefinition(const char* name)
{
  this->DefinitionStack.back().erase(DefinitionMap::key_type(name));
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmVariableWatch* vv = this->GetVariableWatch();
  if ( vv )
    {
    vv->VariableAccessed(name, cmVariableWatch::VARIABLE_REMOVED_ACCESS,
      0, this);
    }
#endif
}

void cmMakefile::SetProjectName(const char* p)
{
  this->ProjectName = p;
}


void cmMakefile::AddGlobalLinkInformation(const char* name, cmTarget& target)
{
  // for these targets do not add anything
  switch(target.GetType())
    {
    case cmTarget::UTILITY:
    case cmTarget::GLOBAL_TARGET:
      return;
    default:;
    }
  std::vector<std::string>::iterator j;
  for(j = this->LinkDirectories.begin();
      j != this->LinkDirectories.end(); ++j)
    {
    target.AddLinkDirectory(j->c_str());
    }
  target.MergeLinkLibraries( *this, name, this->LinkLibraries );
}


void cmMakefile::AddLibrary(const char* lname, cmTarget::TargetType type,
                            const std::vector<std::string> &srcs,
                            bool excludeFromAll)
{
  // wrong type ? default to STATIC
  if (    (type != cmTarget::STATIC_LIBRARY) 
       && (type != cmTarget::SHARED_LIBRARY) 
       && (type != cmTarget::MODULE_LIBRARY))
    {
    type = cmTarget::STATIC_LIBRARY;
    }

  cmTarget* target = this->AddNewTarget(type, lname);
  // Clear its dependencies. Otherwise, dependencies might persist
  // over changes in CMakeLists.txt, making the information stale and
  // hence useless.
  target->ClearDependencyInformation( *this, lname );
  if(excludeFromAll)
    {
    target->SetProperty("EXCLUDE_FROM_ALL", "TRUE");
    }
  target->AddSources(srcs);
  this->AddGlobalLinkInformation(lname, *target);
}

cmTarget* cmMakefile::AddExecutable(const char *exeName,
                                    const std::vector<std::string> &srcs,
                                    bool excludeFromAll)
{
  cmTarget* target = this->AddNewTarget(cmTarget::EXECUTABLE, exeName);
  if(excludeFromAll)
    {
    target->SetProperty("EXCLUDE_FROM_ALL", "TRUE");
    }
  target->AddSources(srcs);
  this->AddGlobalLinkInformation(exeName, *target);
  return target;
}

//----------------------------------------------------------------------------
cmTarget*
cmMakefile::AddNewTarget(cmTarget::TargetType type, const char* name)
{
  cmTargets::iterator it;
  cmTarget target;
  target.SetType(type, name);
  target.SetMakefile(this);
  it=this->Targets.insert(
      cmTargets::value_type(target.GetName(), target)).first;
  this->LocalGenerator->GetGlobalGenerator()->AddTarget(*it);
  return &it->second;
}

cmSourceFile *cmMakefile::GetSourceFileWithOutput(const char *cname)
{
  std::string name = cname;
  std::string out;

  // look through all the source files that have custom commands
  // and see if the custom command has the passed source file as an output
  // keep in mind the possible .rule extension that may be tacked on
  for(std::vector<cmSourceFile*>::const_iterator i =
        this->SourceFiles.begin(); i != this->SourceFiles.end(); ++i)
    {
    // does this source file have a custom command?
    if ((*i)->GetCustomCommand())
      {
      // is the output of the custom command match the source files name
      const std::vector<std::string>& outputs =
        (*i)->GetCustomCommand()->GetOutputs();
      for(std::vector<std::string>::const_iterator o = outputs.begin();
          o != outputs.end(); ++o)
        {
        out = *o;
        std::string::size_type pos = out.rfind(name);
        // If the output matches exactly
        if (pos != out.npos &&
            pos == out.size() - name.size() &&
            (pos ==0 || out[pos-1] == '/'))
          {
          return *i;
          }
        }
      }
    }

  // otherwise return NULL
  return 0;
}

#if defined(CMAKE_BUILD_WITH_CMAKE)
cmSourceGroup* cmMakefile::GetSourceGroup(const std::vector<std::string>&name)
{
  cmSourceGroup* sg = 0;

  // first look for source group starting with the same as the one we wants
  for (std::vector<cmSourceGroup>::iterator sgIt = this->SourceGroups.begin();
       sgIt != this->SourceGroups.end(); ++sgIt)

    {
    std::string sgName = sgIt->GetName();
    if(sgName == name[0])
      {
      sg = &(*sgIt);
      break;
      }
    }

  if(sg != 0)
    {
    // iterate through its children to find match source group
    for(unsigned int i=1; i<name.size(); ++i)
      {
      sg = sg->lookupChild(name[i].c_str());
      if(sg == 0)
        {
        break;
        }
      }
    }
  return sg;
}

 void cmMakefile::AddSourceGroup(const char* name,
                                 const char* regex)
{
  if (name)
    {
    std::vector<std::string> nameVector;
    nameVector.push_back(name);
    AddSourceGroup(nameVector, regex);
    }
}

void cmMakefile::AddSourceGroup(const std::vector<std::string>& name,
                                const char* regex)
{
  cmSourceGroup* sg = 0;
  std::vector<std::string> currentName;
  int i = 0;
  const int lastElement = static_cast<int>(name.size()-1);
  for(i=lastElement; i>=0; --i)
    {
    currentName.assign(name.begin(), name.begin()+i+1);
    sg = this->GetSourceGroup(currentName);
    if(sg != 0)
      {
      break;
      }
    }

  // i now contains the index of the last found component
  if(i==lastElement)
    {
    // group already exists, replace its regular expression
    if ( regex )
      {
      // We only want to set the regular expression.  If there are already
      // source files in the group, we don't want to remove them.
      sg->SetGroupRegex(regex);
      }
    return;
    }
  else if(i==-1)
    {
    // group does not exists nor belong to any existing group
    // add its first component
    this->SourceGroups.push_back(cmSourceGroup(name[0].c_str(), regex));
    sg = this->GetSourceGroup(currentName);
    i = 0; // last component found
    }

  // build the whole source group path
  for(++i; i<=lastElement; ++i)
    {
    sg->AddChild(cmSourceGroup(name[i].c_str(), 0));
    sg = sg->lookupChild(name[i].c_str());
    }

  sg->SetGroupRegex(regex);
}

#endif

void cmMakefile::AddExtraDirectory(const char* dir)
{
  this->AuxSourceDirectories.push_back(dir);
}


// expance CMAKE_BINARY_DIR and CMAKE_SOURCE_DIR in the
// include and library directories.

void cmMakefile::ExpandVariables()
{
  // Now expand variables in the include and link strings
  for(std::vector<std::string>::iterator d = this->IncludeDirectories.begin();
      d != this->IncludeDirectories.end(); ++d)
    {
    this->ExpandVariablesInString(*d, true, true);
    }
  for(std::vector<std::string>::iterator d = this->LinkDirectories.begin();
      d != this->LinkDirectories.end(); ++d)
    {
    this->ExpandVariablesInString(*d, true, true);
    }
  for(cmTarget::LinkLibraryVectorType::iterator l =
        this->LinkLibraries.begin();
      l != this->LinkLibraries.end(); ++l)
    {
    this->ExpandVariablesInString(l->first, true, true);
    }
}

bool cmMakefile::IsOn(const char* name) const
{
  const char* value = this->GetDefinition(name);
  return cmSystemTools::IsOn(value);
}

bool cmMakefile::IsSet(const char* name) const
{
  const char* value = this->GetDefinition(name);
  if ( !value )
    {
    return false;
    }

  if ( ! *value )
    {
    return false;
    }

  if ( cmSystemTools::IsNOTFOUND(value) )
    {
    return false;
    }

  return true;
}

bool cmMakefile::CanIWriteThisFile(const char* fileName)
{
  if ( !this->IsOn("CMAKE_DISABLE_SOURCE_CHANGES") )
    {
    return true;
    }
  // If we are doing an in-source build, than the test will always fail
  if ( cmSystemTools::SameFile(this->GetHomeDirectory(),
                               this->GetHomeOutputDirectory()) )
    {
    if ( this->IsOn("CMAKE_DISABLE_IN_SOURCE_BUILD") )
      {
      return false;
      }
    return true;
    }

  // Check if this is subdirectory of the source tree but not a
  // subdirectory of a build tree
  if ( cmSystemTools::IsSubDirectory(fileName,
      this->GetHomeDirectory()) &&
    !cmSystemTools::IsSubDirectory(fileName,
      this->GetHomeOutputDirectory()) )
    {
    return false;
    }
  return true;
}

const char* cmMakefile::GetRequiredDefinition(const char* name) const
{
  const char* ret = this->GetDefinition(name);
  if(!ret)
    {
    cmSystemTools::Error("Error required internal CMake variable not "
                         "set, cmake may be not be built correctly.\n",
                         "Missing variable is:\n",
                         name);
    return "";
    }
  return ret;
}

bool cmMakefile::IsDefinitionSet(const char* name) const
{
  const char* def = 0;
  DefinitionMap::const_iterator pos = 
    this->DefinitionStack.back().find(name);
  if(pos != this->DefinitionStack.back().end())
    {
    def = (*pos).second.c_str();
    }
  else
    {
    def = this->GetCacheManager()->GetCacheValue(name);
    }
#ifdef CMAKE_BUILD_WITH_CMAKE
  if(cmVariableWatch* vv = this->GetVariableWatch())
    {
    if(!def)
      {
      vv->VariableAccessed
        (name, cmVariableWatch::UNKNOWN_VARIABLE_DEFINED_ACCESS,
         def, this);
      }
    }
#endif
  return def?true:false;
}

const char* cmMakefile::GetDefinition(const char* name) const
{
#ifdef CMAKE_STRICT
  if (this->GetCMakeInstance())
    {
    this->GetCMakeInstance()->
      RecordPropertyAccess(name,cmProperty::VARIABLE);
    }
#endif
  const char* def = 0;
  DefinitionMap::const_iterator pos = 
    this->DefinitionStack.back().find(name);
  if(pos != this->DefinitionStack.back().end())
    {
    def = (*pos).second.c_str();
    }
  else
    {
    def = this->GetCacheManager()->GetCacheValue(name);
    }
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmVariableWatch* vv = this->GetVariableWatch();
  if ( vv )
    {
    if ( def )
      {
      vv->VariableAccessed(name, cmVariableWatch::VARIABLE_READ_ACCESS,
        def, this);
      }
    else
      {
      // are unknown access allowed
      DefinitionMap::const_iterator pos2 =
        this->DefinitionStack.back()
        .find("CMAKE_ALLOW_UNKNOWN_VARIABLE_READ_ACCESS");
      if (pos2 != this->DefinitionStack.back().end() &&
          cmSystemTools::IsOn((*pos2).second.c_str()))
        {
        vv->VariableAccessed(name,
          cmVariableWatch::ALLOWED_UNKNOWN_VARIABLE_READ_ACCESS, def, this);
        }
      else
        {
        vv->VariableAccessed(name,
          cmVariableWatch::UNKNOWN_VARIABLE_READ_ACCESS, def, this);
        }
      }
    }
#endif
  return def;
}

const char* cmMakefile::GetSafeDefinition(const char* def) const
{
  const char* ret = this->GetDefinition(def);
  if(!ret)
    {
    return "";
    }
  return ret;
}

std::vector<std::string> cmMakefile
::GetDefinitions(int cacheonly /* = 0 */) const
{
  std::map<cmStdString, int> definitions;
  if ( !cacheonly )
    {
    DefinitionMap::const_iterator it;
    for ( it = this->DefinitionStack.back().begin();
          it != this->DefinitionStack.back().end(); it ++ )
      {
      definitions[it->first] = 1;
      }
    }
  cmCacheManager::CacheIterator cit =
    this->GetCacheManager()->GetCacheIterator();
  for ( cit.Begin(); !cit.IsAtEnd(); cit.Next() )
    {
    definitions[cit.GetName()] = 1;
    }

  std::vector<std::string> res;

  std::map<cmStdString, int>::iterator fit;
  for ( fit = definitions.begin(); fit != definitions.end(); fit ++ )
    {
    res.push_back(fit->first);
    }
  return res;
}


const char *cmMakefile::ExpandVariablesInString(std::string& source)
{
  return this->ExpandVariablesInString(source, false, false);
}

const char *cmMakefile::ExpandVariablesInString(std::string& source,
                                                bool escapeQuotes,
                                                bool noEscapes,
                                                bool atOnly,
                                                const char* filename,
                                                long line,
                                                bool removeEmpty,
                                                bool replaceAt)
{
  if ( source.empty() || source.find_first_of("$@\\") == source.npos)
    {
    return source.c_str();
    }

  // Special-case the @ONLY mode.
  if(atOnly)
    {
    if(!noEscapes || !removeEmpty || !replaceAt)
      {
      // This case should never be called.  At-only is for
      // configure-file/string which always does no escapes.
      abort();
      }

    // Store an original copy of the input.
    std::string input = source;

    // Start with empty output.
    source = "";

    // Look for one @VAR@ at a time.
    const char* in = input.c_str();
    while(this->cmAtVarRegex.find(in))
      {
      // Get the range of the string to replace.
      const char* first = in + this->cmAtVarRegex.start();
      const char* last =  in + this->cmAtVarRegex.end();

      // Store the unchanged part of the string now.
      source.append(in, first-in);

      // Lookup the definition of VAR.
      std::string var(first+1, last-first-2);
      if(const char* val = this->GetDefinition(var.c_str()))
        {
        // Store the value in the output escaping as requested.
        if(escapeQuotes)
          {
          source.append(cmSystemTools::EscapeQuotes(val));
          }
        else
          {
          source.append(val);
          }
        }

      // Continue looking for @VAR@ further along the string.
      in = last;
      }

    // Append the rest of the unchanged part of the string.
    source.append(in);

    return source.c_str();
    }

  // This method replaces ${VAR} and @VAR@ where VAR is looked up
  // with GetDefinition(), if not found in the map, nothing is expanded.
  // It also supports the $ENV{VAR} syntax where VAR is looked up in
  // the current environment variables.

  cmCommandArgumentParserHelper parser;
  parser.SetMakefile(this);
  parser.SetLineFile(line, filename);
  parser.SetEscapeQuotes(escapeQuotes);
  parser.SetNoEscapeMode(noEscapes);
  parser.SetReplaceAtSyntax(replaceAt);
  parser.SetRemoveEmpty(removeEmpty);
  int res = parser.ParseString(source.c_str(), 0);
  if ( res )
    {
    source = parser.GetResult();
    }
  else
    {
    cmOStringStream error;
    error << "Syntax error in cmake code at\n"
          << (filename?filename:"(no filename given)")
          << ":" << line << ":\n"
          << parser.GetError() << ", when parsing string \""
          << source.c_str() << "\"";
    const char* versionValue
      = this->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
    int major = 0;
    int minor = 0;
    if ( versionValue )
      {
      sscanf(versionValue, "%d.%d", &major, &minor);
      }
    if ( major < 2 || major == 2 && minor < 1 )
      {
      cmSystemTools::Error(error.str().c_str());
      cmSystemTools::SetFatalErrorOccured();
      return source.c_str();
      }
    else
      {
      cmSystemTools::Message(error.str().c_str());
      }
    }
  return source.c_str();
}

void cmMakefile::RemoveVariablesInString(std::string& source,
                                         bool atOnly) const
{
  if(!atOnly)
    {
    cmsys::RegularExpression var("(\\${[A-Za-z_0-9]*})");
    while (var.find(source))
      {
      source.erase(var.start(),var.end() - var.start());
      }
    }

  if(!atOnly)
    {
    cmsys::RegularExpression varb("(\\$ENV{[A-Za-z_0-9]*})");
    while (varb.find(source))
      {
      source.erase(varb.start(),varb.end() - varb.start());
      }
    }
  cmsys::RegularExpression var2("(@[A-Za-z_0-9]*@)");
  while (var2.find(source))
    {
    source.erase(var2.start(),var2.end() - var2.start());
    }
}

/**
 * Add the default definitions to the makefile.  These values must not
 * be dependent on anything that isn't known when this cmMakefile instance
 * is constructed.
 */
void cmMakefile::AddDefaultDefinitions()
{
/* Up to CMake 2.4 here only WIN32, UNIX and APPLE were set.
  With CMake must separate between target and host platform. In most cases
  the tests for WIN32, UNIX and APPLE will be for the target system, so an 
  additional set of variables for the host system is required ->
  CMAKE_HOST_WIN32, CMAKE_HOST_UNIX, CMAKE_HOST_APPLE.
  WIN32, UNIX and APPLE are now set in the platform files in 
  Modules/Platforms/.
  To keep cmake scripts (-P) and custom language and compiler modules
  working, these variables are still also set here in this place, but they
  will be reset in CMakeSystemSpecificInformation.cmake before the platform 
  files are executed. */
#if defined(_WIN32) || defined(__CYGWIN__)
  this->AddDefinition("WIN32", "1");
  this->AddDefinition("CMAKE_HOST_WIN32", "1");
#else
  this->AddDefinition("UNIX", "1");
  this->AddDefinition("CMAKE_HOST_UNIX", "1");
#endif
  // Cygwin is more like unix so enable the unix commands
#if defined(__CYGWIN__)
  this->AddDefinition("UNIX", "1");
  this->AddDefinition("CMAKE_HOST_UNIX", "1");
#endif
#if defined(__APPLE__)
  this->AddDefinition("APPLE", "1");
  this->AddDefinition("CMAKE_HOST_APPLE", "1");
#endif

  char temp[1024];
  sprintf(temp, "%d", cmVersion::GetMinorVersion());
  this->AddDefinition("CMAKE_MINOR_VERSION", temp);
  sprintf(temp, "%d", cmVersion::GetMajorVersion());
  this->AddDefinition("CMAKE_MAJOR_VERSION", temp);
  sprintf(temp, "%d", cmVersion::GetPatchVersion());
  this->AddDefinition("CMAKE_PATCH_VERSION", temp);

  this->AddDefinition("CMAKE_FILES_DIRECTORY",
                      cmake::GetCMakeFilesDirectory());
}

#if defined(CMAKE_BUILD_WITH_CMAKE)
/**
 * Find a source group whose regular expression matches the filename
 * part of the given source name.  Search backward through the list of
 * source groups, and take the first matching group found.  This way
 * non-inherited SOURCE_GROUP commands will have precedence over
 * inherited ones.
 */
cmSourceGroup&
cmMakefile::FindSourceGroup(const char* source,
                            std::vector<cmSourceGroup> &groups)
{
  // First search for a group that lists the file explicitly.
  for(std::vector<cmSourceGroup>::reverse_iterator sg = groups.rbegin();
      sg != groups.rend(); ++sg)
    {
    cmSourceGroup *result = sg->MatchChildrenFiles(source);
    if(result)
      {
      return *result;
      }
    }

  // Now search for a group whose regex matches the file.
  for(std::vector<cmSourceGroup>::reverse_iterator sg = groups.rbegin();
      sg != groups.rend(); ++sg)
    {
    cmSourceGroup *result = sg->MatchChildrenRegex(source);
    if(result)
      {
      return *result;
      }
    }


  // Shouldn't get here, but just in case, return the default group.
  return groups.front();
}
#endif

bool cmMakefile::IsFunctionBlocked(const cmListFileFunction& lff,
                                   cmExecutionStatus &status)
{
  // if there are no blockers get out of here
  if (this->FunctionBlockers.begin() == this->FunctionBlockers.end())
    {
    return false;
    }

  // loop over all function blockers to see if any block this command
  // evaluate in reverse, this is critical for balanced IF statements etc
  std::list<cmFunctionBlocker *>::reverse_iterator pos;
  for (pos = this->FunctionBlockers.rbegin();
       pos != this->FunctionBlockers.rend(); ++pos)
    {
    if((*pos)->IsFunctionBlocked(lff, *this, status))
      {
      return true;
      }
    }

  return false;
}

void cmMakefile::ExpandArguments(
  std::vector<cmListFileArgument> const& inArgs,
  std::vector<std::string>& outArgs)
{
  std::vector<cmListFileArgument>::const_iterator i;
  std::string value;
  outArgs.reserve(inArgs.size());
  for(i = inArgs.begin(); i != inArgs.end(); ++i)
    {
    // Expand the variables in the argument.
    value = i->Value;
    this->ExpandVariablesInString(value, false, false, false,
                                  i->FilePath, i->Line,
                                  false, true);

    // If the argument is quoted, it should be one argument.
    // Otherwise, it may be a list of arguments.
    if(i->Quoted)
      {
      outArgs.push_back(value);
      }
    else
      {
      cmSystemTools::ExpandListArgument(value, outArgs);
      }
    }
}

void cmMakefile::RemoveFunctionBlocker(const cmListFileFunction& lff)
{
  // loop over all function blockers to see if any block this command
  std::list<cmFunctionBlocker *>::reverse_iterator pos;
  for (pos = this->FunctionBlockers.rbegin();
       pos != this->FunctionBlockers.rend(); ++pos)
    {
    if ((*pos)->ShouldRemove(lff, *this))
      {
      cmFunctionBlocker* b = *pos;
      this->FunctionBlockers.remove(b);
      delete b;
      break;
      }
    }

  return;
}

void cmMakefile::SetHomeDirectory(const char* dir)
{
  this->cmHomeDirectory = dir;
  cmSystemTools::ConvertToUnixSlashes(this->cmHomeDirectory);
  this->AddDefinition("CMAKE_SOURCE_DIR", this->GetHomeDirectory());
  if ( !this->GetDefinition("CMAKE_CURRENT_SOURCE_DIR") )
    {
    this->AddDefinition("CMAKE_CURRENT_SOURCE_DIR", this->GetHomeDirectory());
    }
}

void cmMakefile::SetHomeOutputDirectory(const char* lib)
{
  this->HomeOutputDirectory = lib;
  cmSystemTools::ConvertToUnixSlashes(this->HomeOutputDirectory);
  this->AddDefinition("CMAKE_BINARY_DIR", this->GetHomeOutputDirectory());
  if ( !this->GetDefinition("CMAKE_CURRENT_BINARY_DIR") )
    {
    this->AddDefinition("CMAKE_CURRENT_BINARY_DIR",
                        this->GetHomeOutputDirectory());
    }
}


/**
 * Register the given cmData instance with its own name.
 */
void cmMakefile::RegisterData(cmData* data)
{
  std::string name = data->GetName();
  DataMapType::const_iterator d = this->DataMap.find(name);
  if((d != this->DataMap.end()) && (d->second != 0) && (d->second != data))
    {
    delete d->second;
    }
  this->DataMap[name] = data;
}


/**
 * Register the given cmData instance with the given name.  This can be used
 * to register a NULL pointer.
 */
void cmMakefile::RegisterData(const char* name, cmData* data)
{
  DataMapType::const_iterator d = this->DataMap.find(name);
  if((d != this->DataMap.end()) && (d->second != 0) && (d->second != data))
    {
    delete d->second;
    }
  this->DataMap[name] = data;
}


/**
 * Lookup a cmData instance previously registered with the given name.  If
 * the instance cannot be found, return NULL.
 */
cmData* cmMakefile::LookupData(const char* name) const
{
  DataMapType::const_iterator d = this->DataMap.find(name);
  if(d != this->DataMap.end())
    {
    return d->second;
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
cmSourceFile* cmMakefile::GetSource(const char* sourceName)
{
  cmSourceFileLocation sfl(this, sourceName);
  for(std::vector<cmSourceFile*>::const_iterator
        sfi = this->SourceFiles.begin();
      sfi != this->SourceFiles.end(); ++sfi)
    {
    cmSourceFile* sf = *sfi;
    if(sf->Matches(sfl))
      {
      return sf;
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
cmSourceFile* cmMakefile::GetOrCreateSource(const char* sourceName,
                                            bool generated)
{
  if(cmSourceFile* esf = this->GetSource(sourceName))
    {
    return esf;
    }
  else
    {
    cmSourceFile* sf = new cmSourceFile(this, sourceName);
    if(generated)
      {
      sf->SetProperty("GENERATED", "1");
      }
    this->SourceFiles.push_back(sf);
    return sf;
    }
}

void cmMakefile::EnableLanguage(std::vector<std::string> const &  lang, 
                               bool optional)
{
  this->AddDefinition("CMAKE_CFG_INTDIR",
  this->LocalGenerator->GetGlobalGenerator()->GetCMakeCFGInitDirectory());
  this->LocalGenerator->GetGlobalGenerator()->EnableLanguage(lang, this, 
                                                             optional);
}

void cmMakefile::ExpandSourceListArguments(
  std::vector<std::string> const& arguments,
  std::vector<std::string>& newargs, unsigned int /* start */)
{
  // now expand the args
  unsigned int i;
  for(i = 0; i < arguments.size(); ++i)
    {
    // List expansion will have been done already.
    newargs.push_back(arguments[i]);
    }
}

int cmMakefile::TryCompile(const char *srcdir, const char *bindir,
                           const char *projectName, const char *targetName,
                           const std::vector<std::string> *cmakeArgs,
                           std::string *output)
{
  // does the binary directory exist ? If not create it...
  if (!cmSystemTools::FileIsDirectory(bindir))
    {
    cmSystemTools::MakeDirectory(bindir);
    }

  // change to the tests directory and run cmake
  // use the cmake object instead of calling cmake
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(bindir);

  // make sure the same generator is used
  // use this program as the cmake to be run, it should not
  // be run that way but the cmake object requires a vailid path
  std::string cmakeCommand = this->GetDefinition("CMAKE_COMMAND");
  cmake cm;
  cm.SetIsInTryCompile(true);
  cmGlobalGenerator *gg = cm.CreateGlobalGenerator
    (this->LocalGenerator->GetGlobalGenerator()->GetName());
  if (!gg)
    {
    cmSystemTools::Error(
      "Internal CMake error, TryCompile bad GlobalGenerator");
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }
  cm.SetGlobalGenerator(gg);

  // do a configure
  cm.SetHomeDirectory(srcdir);
  cm.SetHomeOutputDirectory(bindir);
  cm.SetStartDirectory(srcdir);
  cm.SetStartOutputDirectory(bindir);
  cm.SetCMakeCommand(cmakeCommand.c_str());
  cm.LoadCache();
  // if cmake args were provided then pass them in
  if (cmakeArgs)
    {
    cm.SetCacheArgs(*cmakeArgs);
    }
  // to save time we pass the EnableLanguage info directly
  gg->EnableLanguagesFromGenerator
    (this->LocalGenerator->GetGlobalGenerator());

  if (cm.Configure() != 0)
    {
    cmSystemTools::Error(
      "Internal CMake error, TryCompile configure of cmake failed");
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }

  if (cm.Generate() != 0)
    {
    cmSystemTools::Error(
      "Internal CMake error, TryCompile generation of cmake failed");
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }

  // finally call the generator to actually build the resulting project
  int ret =
    this->LocalGenerator->GetGlobalGenerator()->TryCompile(srcdir,bindir,
                                                           projectName,
                                                           targetName,
                                                           output,
                                                           this);

  cmSystemTools::ChangeDirectory(cwd.c_str());
  return ret;
}

cmake *cmMakefile::GetCMakeInstance() const
{
  if ( this->LocalGenerator && this->LocalGenerator->GetGlobalGenerator() )
    {
    return this->LocalGenerator->GetGlobalGenerator()->GetCMakeInstance();
    }
  return 0;
}

#ifdef CMAKE_BUILD_WITH_CMAKE
cmVariableWatch *cmMakefile::GetVariableWatch() const
{
  if ( this->GetCMakeInstance() &&
       this->GetCMakeInstance()->GetVariableWatch() )
    {
    return this->GetCMakeInstance()->GetVariableWatch();
    }
  return 0;
}
#endif

void cmMakefile::AddMacro(const char* name, const char* signature)
{
  if ( !name || !signature )
    {
    return;
    }
  this->MacrosMap[name] = signature;
}

void cmMakefile::GetListOfMacros(std::string& macros)
{
  StringStringMap::iterator it;
  macros = "";
  int cc = 0;
  for ( it = this->MacrosMap.begin(); it != this->MacrosMap.end(); ++it )
    {
    if ( cc > 0 )
      {
      macros += ";";
      }
    macros += it->first;
    cc ++;
    }
}

cmCacheManager *cmMakefile::GetCacheManager() const
{
  return this->GetCMakeInstance()->GetCacheManager();
}

void cmMakefile::DisplayStatus(const char* message, float s)
{
  this->GetLocalGenerator()->GetGlobalGenerator()
    ->GetCMakeInstance()->UpdateProgress(message, s);
}

std::string cmMakefile::GetModulesFile(const char* filename)
{
  std::vector<std::string> modulePath;
  const char* def = this->GetDefinition("CMAKE_MODULE_PATH");
  if(def)
    {
    cmSystemTools::ExpandListArgument(def, modulePath);
    }

  // Also search in the standard modules location.
  def = this->GetDefinition("CMAKE_ROOT");
  if(def)
    {
    std::string rootModules = def;
    rootModules += "/Modules";
    modulePath.push_back(rootModules);
    }
  //std::string  Look through the possible module directories.
  for(std::vector<std::string>::iterator i = modulePath.begin();
      i != modulePath.end(); ++i)
    {
    std::string itempl = *i;
    cmSystemTools::ConvertToUnixSlashes(itempl);
    itempl += "/";
    itempl += filename;
    if(cmSystemTools::FileExists(itempl.c_str()))
      {
      return itempl;
      }
    }
  return "";
}

void cmMakefile::ConfigureString(const std::string& input,
                                 std::string& output, bool atOnly,
                                 bool escapeQuotes)
{
  // Split input to handle one line at a time.
  std::string::const_iterator lineStart = input.begin();
  while(lineStart != input.end())
    {
    // Find the end of this line.
    std::string::const_iterator lineEnd = lineStart;
    while(lineEnd != input.end() && *lineEnd != '\n')
      {
      ++lineEnd;
      }

    // Copy the line.
    std::string line(lineStart, lineEnd);

    // Skip the newline character.
    bool haveNewline = (lineEnd != input.end());
    if(haveNewline)
      {
      ++lineEnd;
      }

    // Replace #cmakedefine instances.
    if(this->cmDefineRegex.find(line))
      {
      const char* def =
        this->GetDefinition(this->cmDefineRegex.match(1).c_str());
      if(!cmSystemTools::IsOff(def))
        {
        cmSystemTools::ReplaceString(line, "#cmakedefine", "#define");
        output += line;
        }
      else
        {
        output += "/* #undef ";
        output += this->cmDefineRegex.match(1);
        output += " */";
        }
      }
    else if(this->cmDefine01Regex.find(line))
      {
      const char* def =
        this->GetDefinition(this->cmDefine01Regex.match(1).c_str());
      cmSystemTools::ReplaceString(line, "#cmakedefine01", "#define");
      output += line;
      if(!cmSystemTools::IsOff(def))
        {
        output += " 1";
        }
      else
        {
        output += " 0";
        }
      }
    else
      {
      output += line;
      }

    if(haveNewline)
      {
      output += "\n";
      }

    // Move to the next line.
    lineStart = lineEnd;
    }

  // Perform variable replacements.
  this->ExpandVariablesInString(output, escapeQuotes, true,
                                atOnly, 0, -1, true);
}

int cmMakefile::ConfigureFile(const char* infile, const char* outfile,
                              bool copyonly, bool atOnly, bool escapeQuotes)
{
  int res = 1;
  if ( !this->CanIWriteThisFile(outfile) )
    {
    cmSystemTools::Error("Attempt to write file: ",
                         outfile, " into a source directory.");
    return 0;
    }
  if ( !cmSystemTools::FileExists(infile) )
    {
    cmSystemTools::Error("File ", infile, " does not exist.");
    return 0;
    }
  std::string soutfile = outfile;
  std::string sinfile = infile;
  this->AddCMakeDependFile(infile);
  cmSystemTools::ConvertToUnixSlashes(soutfile);
  mode_t perm = 0;
  cmSystemTools::GetPermissions(sinfile.c_str(), perm);
  std::string::size_type pos = soutfile.rfind('/');
  if(pos != std::string::npos)
    {
    std::string path = soutfile.substr(0, pos);
    cmSystemTools::MakeDirectory(path.c_str());
    }

  if(copyonly)
    {
    if ( !cmSystemTools::CopyFileIfDifferent(sinfile.c_str(),
                                             soutfile.c_str()))
      {
      return 0;
      }
    }
  else
    {
    std::string tempOutputFile = soutfile;
    tempOutputFile += ".tmp";
    std::ofstream fout(tempOutputFile.c_str());
    if(!fout)
      {
      cmSystemTools::Error(
        "Could not open file for write in copy operation ",
        tempOutputFile.c_str());
      cmSystemTools::ReportLastSystemError("");
      return 0;
      }
    std::ifstream fin(sinfile.c_str());
    if(!fin)
      {
      cmSystemTools::Error("Could not open file for read in copy operation ",
                           sinfile.c_str());
      return 0;
      }

    // now copy input to output and expand variables in the
    // input file at the same time
    std::string inLine;
    std::string outLine;
    while( cmSystemTools::GetLineFromStream(fin, inLine) )
      {
      outLine = "";
      this->ConfigureString(inLine, outLine, atOnly, escapeQuotes);
      fout << outLine.c_str() << "\n";
      }
    // close the files before attempting to copy
    fin.close();
    fout.close();
    if ( !cmSystemTools::CopyFileIfDifferent(tempOutputFile.c_str(),
                                             soutfile.c_str()) )
      {
      res = 0;
      }
    else
      {
      cmSystemTools::SetPermissions(soutfile.c_str(), perm);
      }
    cmSystemTools::RemoveFile(tempOutputFile.c_str());
    }
  return res;
}

void cmMakefile::AddWrittenFile(const char* file)
{ this->GetCMakeInstance()->AddWrittenFile(file); }

bool cmMakefile::HasWrittenFile(const char* file)
{ return this->GetCMakeInstance()->HasWrittenFile(file); }

bool cmMakefile::CheckInfiniteLoops()
{
  std::vector<std::string>::iterator it;
  for ( it = this->ListFiles.begin();
        it != this->ListFiles.end();
        ++ it )
    {
    if ( this->HasWrittenFile(it->c_str()) )
      {
      cmOStringStream str;
      str << "File " << it->c_str() <<
        " is written by WRITE_FILE (or FILE WRITE) command and should "
        "not be used as input to CMake. Please use CONFIGURE_FILE to "
        "be safe. Refer to the note next to FILE WRITE command.";
      cmSystemTools::Error(str.str().c_str());
      return false;
      }
    }
  return true;
}

void cmMakefile::SetProperty(const char* prop, const char* value)
{
  if (!prop)
    {
    return;
    }
  
  // handle special props
  std::string propname = prop;
  if ( propname == "INCLUDE_DIRECTORIES" )
    {
    std::vector<std::string> varArgsExpanded;
    cmSystemTools::ExpandListArgument(value, varArgsExpanded);
    this->SetIncludeDirectories(varArgsExpanded);
    return;
    }

  if ( propname == "LINK_DIRECTORIES" )
    {
    std::vector<std::string> varArgsExpanded;
    cmSystemTools::ExpandListArgument(value, varArgsExpanded);
    this->SetLinkDirectories(varArgsExpanded);
    return;
    }
  
  if ( propname == "INCLUDE_REGULAR_EXPRESSION" )
    {
    this->SetIncludeRegularExpression(value);
    return;
    }

  if ( propname == "ADDITIONAL_MAKE_CLEAN_FILES" )
    {
    // This property is not inherrited
    if ( strcmp(this->GetCurrentDirectory(), 
                this->GetStartDirectory()) != 0 )
      {
      return;
      }
    }

  this->Properties.SetProperty(prop,value,cmProperty::DIRECTORY);
}

void cmMakefile::AppendProperty(const char* prop, const char* value)
{
  if (!prop)
    {
    return;
    }

  // handle special props
  std::string propname = prop;
  if ( propname == "INCLUDE_DIRECTORIES" )
    {
    std::vector<std::string> varArgsExpanded;
    cmSystemTools::ExpandListArgument(value, varArgsExpanded);
    for(std::vector<std::string>::const_iterator vi = varArgsExpanded.begin();
        vi != varArgsExpanded.end(); ++vi)
      {
      this->AddIncludeDirectory(vi->c_str());
      }
    return;
    }

  if ( propname == "LINK_DIRECTORIES" )
    {
    std::vector<std::string> varArgsExpanded;
    cmSystemTools::ExpandListArgument(value, varArgsExpanded);
    for(std::vector<std::string>::const_iterator vi = varArgsExpanded.begin();
        vi != varArgsExpanded.end(); ++vi)
      {
      this->AddLinkDirectory(vi->c_str());
      }
    return;
    }

  this->Properties.AppendProperty(prop,value,cmProperty::DIRECTORY);
}

const char *cmMakefile::GetPropertyOrDefinition(const char* prop)
{
  const char *ret = this->GetProperty(prop, cmProperty::DIRECTORY);
  if (!ret)
    {
    ret = this->GetDefinition(prop);
    }
  return ret;
}

const char *cmMakefile::GetProperty(const char* prop)
{
  return this->GetProperty(prop, cmProperty::DIRECTORY);
}

const char *cmMakefile::GetProperty(const char* prop,
                                    cmProperty::ScopeType scope)
{
  // watch for specific properties
  static std::string output;
  output = "";
  if (!strcmp("PARENT_DIRECTORY",prop))
    {
    output = this->LocalGenerator->GetParent()
      ->GetMakefile()->GetStartDirectory();
    return output.c_str();
    }
  else if (!strcmp("INCLUDE_REGULAR_EXPRESSION",prop) )
    {
    output = this->GetIncludeRegularExpression();
    return output.c_str();
    }
  else if (!strcmp("LISTFILE_STACK",prop))
    {
    for (std::deque<cmStdString>::iterator i = this->ListFileStack.begin();
         i != this->ListFileStack.end(); ++i)
      {
      if (i != this->ListFileStack.begin())
        {
        output += ";";
        }
      output += *i;
      }
    return output.c_str();
    }
  else if (!strcmp("VARIABLES",prop) || !strcmp("CACHE_VARIABLES",prop))
    {
    int cacheonly = 0;
    if ( !strcmp("CACHE_VARIABLES",prop) )
      {
      cacheonly = 1;
      }
    std::vector<std::string> vars = this->GetDefinitions(cacheonly);
    for (unsigned int cc = 0; cc < vars.size(); cc ++ )
      {
      if ( cc > 0 )
        {
        output += ";";
        }
      output += vars[cc];
      }
    return output.c_str();
    }
  else if (!strcmp("MACROS",prop))
    {
    this->GetListOfMacros(output);
    return output.c_str();
    }
  else if (!strcmp("DEFINITIONS",prop))
    {
    output = this->GetDefineFlags();
    return output.c_str();
    }
  else if (!strcmp("INCLUDE_DIRECTORIES",prop) )
    {
    cmOStringStream str;
    for (std::vector<std::string>::const_iterator 
         it = this->GetIncludeDirectories().begin();
         it != this->GetIncludeDirectories().end();
         ++ it )
      {
      if ( it != this->GetIncludeDirectories().begin())
        {
        str << ";";
        }
      str << it->c_str();
      }
    output = str.str();
    return output.c_str();
    }
  else if (!strcmp("LINK_DIRECTORIES",prop))
    {
    cmOStringStream str;
    for (std::vector<std::string>::const_iterator 
         it = this->GetLinkDirectories().begin();
         it != this->GetLinkDirectories().end();
         ++ it )
      {
      if ( it != this->GetLinkDirectories().begin())
        {
        str << ";";
        }
      str << it->c_str();
      }
    output = str.str();
    return output.c_str();
    }

  bool chain = false;
  const char *retVal =
    this->Properties.GetPropertyValue(prop, scope, chain);
  if (chain)
    {
    if(this->LocalGenerator->GetParent())
      {
      return this->LocalGenerator->GetParent()->GetMakefile()->
        GetProperty(prop, scope);
      }
    return this->GetCMakeInstance()->GetProperty(prop,scope);
    }

  return retVal;
}

bool cmMakefile::GetPropertyAsBool(const char* prop)
{
  return cmSystemTools::IsOn(this->GetProperty(prop));
}


cmTarget* cmMakefile::FindTarget(const char* name)
{
  cmTargets& tgts = this->GetTargets();

  cmTargets::iterator i = tgts.find ( name );
  if ( i != tgts.end() )
    {
    return &i->second;
    }

  return 0;
}

cmTest* cmMakefile::CreateTest(const char* testName)
{
  if ( !testName )
    {
    return 0;
    }
  cmTest* test = this->GetTest(testName);
  if ( test )
    {
    return test;
    }
  test = new cmTest;
  test->SetName(testName);
  test->SetMakefile(this);
  this->Tests.push_back(test);
  return test;
}

cmTest* cmMakefile::GetTest(const char* testName) const
{
  if ( !testName )
    {
    return 0;
    }
  std::vector<cmTest*>::const_iterator it;
  for ( it = this->Tests.begin(); it != this->Tests.end(); ++ it )
    {
    if ( strcmp((*it)->GetName(), testName) == 0 )
      {
      return *it;
      }
    }
  return 0;
}

const std::vector<cmTest*> *cmMakefile::GetTests() const
{
  return &this->Tests;
}

std::vector<cmTest*> *cmMakefile::GetTests()
{
  return &this->Tests;
}

std::string cmMakefile::GetListFileStack()
{
  cmOStringStream tmp;
  size_t depth = this->ListFileStack.size();
  if (depth > 0)
    {
    std::deque<cmStdString>::iterator it = this->ListFileStack.end();
    do
      {
      if (depth != this->ListFileStack.size())
        {
        tmp << "\n                ";
        }
      --it;
      tmp << "[";
      tmp << depth;
      tmp << "]\t";
      tmp << *it;
      depth--;
      }
    while (it != this->ListFileStack.begin());
    }
  return tmp.str();
}


void cmMakefile::PushScope()
{
  // Get the index of the next stack entry.
  std::vector<DefinitionMap>::size_type index = this->DefinitionStack.size();

  // Allocate a new stack entry.
  this->DefinitionStack.push_back(DefinitionMap());

  // Copy the previous top to the new top.
  this->DefinitionStack[index] = this->DefinitionStack[index-1];
}

void cmMakefile::PopScope()
{
  this->DefinitionStack.pop_back();
}

void cmMakefile::RaiseScope(const char *var, const char *varDef)
{
  if (!var || !strlen(var))
    {
    return;
    }

  // multiple scopes in this directory?
  if (this->DefinitionStack.size() > 1)
    {
    if(varDef)
      {
      this->DefinitionStack[this->DefinitionStack.size()-2][var] = varDef;
      }
    else
      {
      this->DefinitionStack[this->DefinitionStack.size()-2].erase(var);
      }
    }
  // otherwise do the parent (if one exists)
  else if (this->LocalGenerator->GetParent())
    {
    cmMakefile *parent = this->LocalGenerator->GetParent()->GetMakefile();
    if (parent)
      {
      if(varDef)
        {
        parent->AddDefinition(var,varDef);
        }
      else
        {
        parent->RemoveDefinition(var);
        }
      }
    }
}


// define properties
void cmMakefile::DefineProperties(cmake *cm)
{
  cm->DefineProperty
    ("ADDITIONAL_MAKE_CLEAN_FILES", cmProperty::DIRECTORY,
     "Additional files to clean during the make clean stage.",
     "A list of files that will be cleaned as a part of the "
     "\"make clean\" stage. ");

  cm->DefineProperty
    ("CLEAN_NO_CUSTOM", cmProperty::DIRECTORY,
     "Should the output of custom commands be left.",
     "If this is true then the outputs of custom commands for this "
     "directory will not be removed during the \"make clean\" stage. ");

  cm->DefineProperty
    ("LISTFILE_STACK", cmProperty::DIRECTORY,
     "The current stack of listfiles being processed.",
     "This property is mainly useful when trying to debug errors "
     "in your CMake scripts. It returns a list of what list files "
     "are currently being processed, in order. So if one listfile "
     "does an INCLUDE command then that is effectively pushing "
     "the included listfile onto the stack.");

  cm->DefineProperty
    ("TEST_INCLUDE_FILE", cmProperty::DIRECTORY,
     "A cmake file that will be included when ctest is run.",
     "If you specify TEST_INCLUDE_FILE, that file will be "
     "included and processed when ctest is run on the directory.");

  cm->DefineProperty
    ("COMPILE_DEFINITIONS", cmProperty::DIRECTORY,
     "Preprocessor definitions for compiling a directory's sources.",
     "The COMPILE_DEFINITIONS property may be set to a list of preprocessor "
     "definitions using the syntax VAR or VAR=value.  Function-style "
     "definitions are not supported.  CMake will automatically escape "
     "the value correctly for the native build system (note that CMake "
     "language syntax may require escapes to specify some values).  "
     "This property may be set on a per-configuration basis using the name "
     "COMPILE_DEFINITIONS_<CONFIG> where <CONFIG> is an upper-case name "
     "(ex. \"COMPILE_DEFINITIONS_DEBUG\").  "
     "This property will be initialized in each directory by its value "
     "in the directory's parent.\n"
     "CMake will automatically drop some definitions that "
     "are not supported by the native build tool.  "
     "The VS6 IDE does not support definitions with values "
     "(but NMake does).\n"
     "Dislaimer: Most native build tools have poor support for escaping "
     "certain values.  CMake has work-arounds for many cases but some "
     "values may just not be possible to pass correctly.  If a value "
     "does not seem to be escaped correctly, do not attempt to "
     "work-around the problem by adding escape sequences to the value.  "
     "Your work-around may break in a future version of CMake that "
     "has improved escape support.  Instead consider defining the macro "
     "in a (configured) header file.  Then report the limitation.");

  cm->DefineProperty
    ("COMPILE_DEFINITIONS_<CONFIG>", cmProperty::DIRECTORY,
     "Per-configuration preprocessor definitions in a directory.",
     "This is the configuration-specific version of COMPILE_DEFINITIONS.  "
     "This property will be initialized in each directory by its value "
     "in the directory's parent.\n");

  cm->DefineProperty
    ("EXCLUDE_FROM_ALL", cmProperty::DIRECTORY,
     "Exclude the directory from the all target of its parent.",
     "A property on a directory that indicates if its targets are excluded "
     "from the default build target. If it is not, then with a Makefile "
     "for example typing make will cause the targets to be built. "
     "The same concept applies to the default build of other generators.",
     false);
}

//----------------------------------------------------------------------------
cmTarget*
cmMakefile::AddImportedTarget(const char* name, cmTarget::TargetType type)
{
  // Create the target.
  cmsys::auto_ptr<cmTarget> target(new cmTarget);
  target->SetType(type, name);
  target->SetMakefile(this);
  target->MarkAsImported();

  // Add to the set of available imported targets.
  this->ImportedTargets[name] = target.get();

  // Transfer ownership to this cmMakefile object.
  this->ImportedTargetsOwned.push_back(target.get());
  return target.release();
}

//----------------------------------------------------------------------------
cmTarget* cmMakefile::FindTargetToUse(const char* name)
{
  // Look for an imported target.  These take priority because they
  // are more local in scope and do not have to be globally unique.
  std::map<cmStdString, cmTarget*>::const_iterator
    imported = this->ImportedTargets.find(name);
  if(imported != this->ImportedTargets.end())
    {
    return imported->second;
    }

  // Look for a target built in this project.
  return this->LocalGenerator->GetGlobalGenerator()->FindTarget(0, name);
}

//----------------------------------------------------------------------------
bool cmMakefile::EnforceUniqueName(std::string const& name, std::string& msg,
                                   bool isCustom)
{
  if(cmTarget* existing = this->FindTargetToUse(name.c_str()))
    {
    // The name given conflicts with an existing target.  Produce an
    // error in a compatible way.
    if(existing->IsImported())
      {
      // Imported targets were not supported in previous versions.
      // This is new code, so we can make it an error.
      cmOStringStream e;
      e << "cannot create target \"" << name
        << "\" because an imported target with the same name already exists.";
      msg = e.str();
      return false;
      }
    else 
      {
      // target names must be globally unique
      switch (this->GetPolicyStatus(cmPolicies::CMP_0002))
        {
        case cmPolicies::WARN:
          msg = this->GetPolicies()->
            GetPolicyWarning(cmPolicies::CMP_0002);
        case cmPolicies::OLD:
          return true;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
          msg = this->GetPolicies()->
            GetRequiredPolicyError(cmPolicies::CMP_0002);
          return false;
        case cmPolicies::NEW:
          break;
        }
      
      // The conflict is with a non-imported target.
      // Allow this if the user has requested support.
      cmake* cm =
        this->LocalGenerator->GetGlobalGenerator()->GetCMakeInstance();
      if(isCustom && existing->GetType() == cmTarget::UTILITY &&
         this != existing->GetMakefile() &&
         cm->GetPropertyAsBool("ALLOW_DUPLICATE_CUSTOM_TARGETS"))
        {
        return true;
        }

      // Produce an error that tells the user how to work around the
      // problem.
      cmOStringStream e;
      e << "cannot create target \"" << name
        << "\" because another target with the same name already exists.  "
        << "The existing target is ";
      switch(existing->GetType())
        {
        case cmTarget::EXECUTABLE:
          e << "an executable ";
          break;
        case cmTarget::STATIC_LIBRARY:
          e << "a static library ";
          break;
        case cmTarget::SHARED_LIBRARY:
          e << "a shared library ";
          break;
        case cmTarget::MODULE_LIBRARY:
          e << "a module library ";
          break;
        case cmTarget::UTILITY:
          e << "a custom target ";
          break;
        default: break;
        }
      e << "created in source directory \""
        << existing->GetMakefile()->GetCurrentDirectory() << "\".\n"
        << "\n";
      e <<
        "Logical target names must be globally unique because:\n"
        "  - Unique names may be referenced unambiguously both in CMake\n"
        "    code and on make tool command lines.\n"
        "  - Logical names are used by Xcode and VS IDE generators\n"
        "    to produce meaningful project names for the targets.\n"
        "The logical name of executable and library targets does not "
        "have to correspond to the physical file names built.  "
        "Consider using the OUTPUT_NAME target property to create two "
        "targets with the same physical name while keeping logical "
        "names distinct.  "
        "Custom targets must simply have globally unique names.\n"
        "\n"
        "If you are building an older project it is possible that "
        "it violated this rule but was working accidentally because "
        "CMake did not previously diagnose this problem.  "
        "Set CMAKE_BACKWARDS_COMPATIBILITY to 2.4 or lower to disable "
        "this error.\n";
      if(isCustom && existing->GetType() == cmTarget::UTILITY)
        {
        e <<
          "\n"
          "For projects that care only about Makefile generators and do "
          "not wish to support Xcode or VS IDE generators, one may add\n"
          "  set_property(GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS 1)\n"
          "to the top of the project to allow duplicate custom targets "
          "(target names must still be unique within each directory).  "
          "However, setting this property will cause non-Makefile generators "
          "to produce an error and refuse to generate the project.";
        }
        msg = e.str();
      return false;
      }
    }
  return true;
}

cmPolicies::PolicyStatus cmMakefile
::GetPolicyStatus(cmPolicies::PolicyID id)
{
  cmPolicies::PolicyStatus status = cmPolicies::REQUIRED_IF_USED;
  PolicyMap::iterator mappos;
  int vecpos;
  bool done = false;

  // check our policy stack first
  for (vecpos = this->PolicyStack.size(); vecpos >= 0 && !done; vecpos--)
  {
    mappos = this->PolicyStack[vecpos].find(id);
    if (mappos != this->PolicyStack[vecpos].end())
    {
      status = mappos->second;
      done = true;
    }
  }
  
  // if not found then 
  if (!done)
  {
    // pass the buck to our parent if we have one
    if (this->LocalGenerator->GetParent())
    {
      cmMakefile *parent = 
        this->LocalGenerator->GetParent()->GetMakefile();
      return parent->GetPolicyStatus(id);
    }
    // otherwise use the default
    else
    {
      status = this->GetPolicies()->GetPolicyStatus(id);
    }
  }
  
  // warn if we see a REQUIRED_IF_USED above a OLD or WARN
  if (!this->GetPolicies()->IsValidUsedPolicyStatus(id,status))
  {
    return cmPolicies::REQUIRED_IF_USED;
  }
  
  return status;
}

bool cmMakefile::SetPolicy(const char *id, 
                           cmPolicies::PolicyStatus status)
{
  cmPolicies::PolicyID pid;
  if (!this->GetPolicies()->GetPolicyID(id, /* out */ pid))
  {
    cmSystemTools::Error("Invalid policy string used. Invalid string was "
      , id);
    return false;
  }
  return this->SetPolicy(pid,status);
}

bool cmMakefile::SetPolicy(cmPolicies::PolicyID id, 
                           cmPolicies::PolicyStatus status)
{
  // setting a REQUIRED_ALWAYS policy to WARN or OLD is an insta error
  if (this->GetPolicies()->
      IsValidPolicyStatus(id,status))
  {
    this->PolicyStack.back()[id] = status;
    return true;
  }
  return false;
}

bool cmMakefile::PushPolicy()
{
  // Allocate a new stack entry.
  this->PolicyStack.push_back(PolicyMap());
  return true;
}

bool cmMakefile::PopPolicy()
{
  if (PolicyStack.size() == 0)
  {
    cmSystemTools::Error("Attempt to pop the policy stack past "
      "it's beginning.");
    return false;
  }
  this->PolicyStack.pop_back();
  return true;
}

bool cmMakefile::SetPolicyVersion(const char *version)
{
  return this->GetCMakeInstance()->GetPolicies()->
    ApplyPolicyVersion(this,version);
}

cmPolicies *cmMakefile::GetPolicies()
{ 
  if (!this->GetCMakeInstance())
  {
    return 0;
  }
  return this->GetCMakeInstance()->GetPolicies();
}
