/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmGetPropertyCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2008-04-01 19:22:30 $
  Version:   $Revision: 1.9 $

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

#include "cmLuaUtils.h"


static int cmGetPropertyLua(lua_State *L) 
{
  // build a list file function 
  cmListFileFunction lff;
  cmExecutionStatus status;
  
  lff.Name = lua_tostring(L, lua_upvalueindex(1));
  
  // stick in a temp var
  lff.Arguments.push_back
    (cmListFileArgument("__GET_PROPERTY_LUA_TEMP", false, 0, 0));

  int i;
  int top = lua_gettop(L);
  for (i = 1; i <= top; i++) 
    { 
    const char *arg = lua_tostring(L, i);
    lff.Arguments.push_back(cmListFileArgument(arg, false,
                                               0, 0));
    }

  // get the current makefile
  lua_pushstring(L,"cmCurrentMakefile");
  lua_gettable(L, LUA_REGISTRYINDEX);
  cmMakefile *mf = static_cast<cmMakefile *>(lua_touserdata(L,-1));

  // pass it to ExecuteCommand
  mf->ExecuteCommand(lff, status);
  // What should I do with status?

  // get the return value
  const char *result = mf->GetDefinition("__GET_PROPERTY_LUA_TEMP");
  lua_pushstring(L, result);

  return 1;  /* number of results */
}

// special lua code
cmGetPropertyCommand::cmGetPropertyCommand()
{
  this->InfoType = OutValue;
  this->LuaFunction = cmGetPropertyLua;
}


//----------------------------------------------------------------------------
bool cmGetPropertyCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // The cmake variable in which to store the result.
  this->Variable = args[0];

  // Get the scope from which to get the property.
  cmProperty::ScopeType scope;
  if(args[1] == "GLOBAL")
    {
    scope = cmProperty::GLOBAL;
    }
  else if(args[1] == "DIRECTORY")
    {
    scope = cmProperty::DIRECTORY;
    }
  else if(args[1] == "TARGET")
    {
    scope = cmProperty::TARGET;
    }
  else if(args[1] == "SOURCE")
    {
    scope = cmProperty::SOURCE_FILE;
    }
  else if(args[1] == "TEST")
    {
    scope = cmProperty::TEST;
    }
  else if(args[1] == "VARIABLE")
    {
    scope = cmProperty::VARIABLE;
    }
  else
    {
    cmOStringStream e;
    e << "given invalid scope " << args[1] << ".  "
      << "Valid scopes are "
      << "GLOBAL, DIRECTORY, TARGET, SOURCE, TEST, VARIABLE.";
    this->SetError(e.str().c_str());
    return false;
    }

  // Parse remaining arguments.
  enum Doing { DoingNone, DoingName, DoingProperty, DoingType };
  Doing doing = DoingName;
  for(unsigned int i=2; i < args.size(); ++i)
    {
    if(args[i] == "PROPERTY")
      {
      doing = DoingProperty;
      }
    else if(args[i] == "BRIEF_DOCS")
      {
      doing = DoingNone;
      this->InfoType = OutBriefDoc;
      }
    else if(args[i] == "FULL_DOCS")
      {
      doing = DoingNone;
      this->InfoType = OutFullDoc;
      }
    else if(args[i] == "SET")
      {
      doing = DoingNone;
      this->InfoType = OutSet;
      }
    else if(args[i] == "DEFINED")
      {
      doing = DoingNone;
      this->InfoType = OutDefined;
      }
    else if(doing == DoingName)
      {
      doing = DoingNone;
      this->Name = args[i];
      }
    else if(doing == DoingProperty)
      {
      doing = DoingNone;
      this->PropertyName = args[i];
      }
    else
      {
      cmOStringStream e;
      e << "given invalid argument \"" << args[i] << "\".";
      this->SetError(e.str().c_str());
      return false;
      }
    }

  // Make sure a property name was found.
  if(this->PropertyName.empty())
    {
    this->SetError("not given a PROPERTY <name> argument.");
    return false;
    }

  // Compute requested output.
  if(this->InfoType == OutBriefDoc)
    {
    // Lookup brief documentation.
    std::string output;
    if(cmPropertyDefinition* def =
       this->Makefile->GetCMakeInstance()->
       GetPropertyDefinition(this->PropertyName.c_str(), scope))
      {
      output = def->GetShortDescription();
      }
    else
      {
      output = "NOTFOUND";
      }
    this->Makefile->AddDefinition(this->Variable.c_str(), output.c_str());
    }
  else if(this->InfoType == OutFullDoc)
    {
    // Lookup full documentation.
    std::string output;
    if(cmPropertyDefinition* def =
       this->Makefile->GetCMakeInstance()->
       GetPropertyDefinition(this->PropertyName.c_str(), scope))
      {
      output = def->GetFullDescription();
      }
    else
      {
      output = "NOTFOUND";
      }
    this->Makefile->AddDefinition(this->Variable.c_str(), output.c_str());
    }
  else if(this->InfoType == OutDefined)
    {
    // Lookup if the property is defined
    if(this->Makefile->GetCMakeInstance()->
       GetPropertyDefinition(this->PropertyName.c_str(), scope))
      {
      this->Makefile->AddDefinition(this->Variable.c_str(), "1");
      }
    else
      {
      this->Makefile->AddDefinition(this->Variable.c_str(), "0");
      }
    }
  else
    {
    // Dispatch property getting.
    switch(scope)
      {
      case cmProperty::GLOBAL:      return this->HandleGlobalMode();
      case cmProperty::DIRECTORY:   return this->HandleDirectoryMode();
      case cmProperty::TARGET:      return this->HandleTargetMode();
      case cmProperty::SOURCE_FILE: return this->HandleSourceMode();
      case cmProperty::TEST:        return this->HandleTestMode();
      case cmProperty::VARIABLE:    return this->HandleVariableMode();

      case cmProperty::CACHED_VARIABLE:
        break; // should never happen
      }
    }

  return true;
}

//----------------------------------------------------------------------------
bool cmGetPropertyCommand::StoreResult(const char* value)
{
  if(this->InfoType == OutSet)
    {
    this->Makefile->AddDefinition(this->Variable.c_str(), value? "1":"0");
    }
  else // if(this->InfoType == OutValue)
    {
    this->Makefile->AddDefinition(this->Variable.c_str(), value);
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmGetPropertyCommand::HandleGlobalMode()
{
  if(!this->Name.empty())
    {
    this->SetError("given name for GLOBAL scope.");
    return false;
    }

  // Get the property.
  cmake* cm = this->Makefile->GetCMakeInstance();
  return this->StoreResult(cm->GetProperty(this->PropertyName.c_str()));
}

//----------------------------------------------------------------------------
bool cmGetPropertyCommand::HandleDirectoryMode()
{
  // Default to the current directory.
  cmMakefile* mf = this->Makefile;

  // Lookup the directory if given.
  if(!this->Name.empty())
    {
    // Construct the directory name.  Interpret relative paths with
    // respect to the current directory.
    std::string dir = this->Name;
    if(!cmSystemTools::FileIsFullPath(dir.c_str()))
      {
      dir = this->Makefile->GetCurrentDirectory();
      dir += "/";
      dir += this->Name;
      }

    // The local generators are associated with collapsed paths.
    dir = cmSystemTools::CollapseFullPath(dir.c_str());

    // Lookup the generator.
    if(cmLocalGenerator* lg =
       (this->Makefile->GetLocalGenerator()
        ->GetGlobalGenerator()->FindLocalGenerator(dir.c_str())))
      {
      // Use the makefile for the directory found.
      mf = lg->GetMakefile();
      }
    else
      {
      // Could not find the directory.
      this->SetError
        ("DIRECTORY scope provided but requested directory was not found. "
         "This could be because the directory argument was invalid or, "
         "it is valid but has not been processed yet.");
      return false;
      }
    }

  // Get the property.
  return this->StoreResult(mf->GetProperty(this->PropertyName.c_str()));
}

//----------------------------------------------------------------------------
bool cmGetPropertyCommand::HandleTargetMode()
{
  if(this->Name.empty())
    {
    this->SetError("not given name for TARGET scope.");
    return false;
    }

  if(cmTarget* target = this->Makefile->FindTargetToUse(this->Name.c_str()))
    {
    return this->StoreResult(target->GetProperty(this->PropertyName.c_str()));
    }
  else
    {
    cmOStringStream e;
    e << "could not find TARGET " << this->Name
      << ".  Perhaps it has not yet been created.";
    this->SetError(e.str().c_str());
    return false;
    }
}

//----------------------------------------------------------------------------
bool cmGetPropertyCommand::HandleSourceMode()
{
  if(this->Name.empty())
    {
    this->SetError("not given name for SOURCE scope.");
    return false;
    }

  // Get the source file.
  if(cmSourceFile* sf =
     this->Makefile->GetOrCreateSource(this->Name.c_str()))
    {
    return
      this->StoreResult(sf->GetPropertyForUser(this->PropertyName.c_str()));
    }
  else
    {
    cmOStringStream e;
    e << "given SOURCE name that could not be found or created: "
      << this->Name;
    this->SetError(e.str().c_str());
    return false;
    }
}

//----------------------------------------------------------------------------
bool cmGetPropertyCommand::HandleTestMode()
{
  if(this->Name.empty())
    {
    this->SetError("not given name for TEST scope.");
    return false;
    }

  // Loop over all tests looking for matching names.
  std::vector<cmTest*> const& tests = *this->Makefile->GetTests();
  for(std::vector<cmTest*>::const_iterator ti = tests.begin();
      ti != tests.end(); ++ti)
    {
    cmTest* test = *ti;
    if(test->GetName() == this->Name)
      {
      return this->StoreResult(test->GetProperty(this->PropertyName.c_str()));
      }
    }

  // If not found it is an error.
  cmOStringStream e;
  e << "given TEST name that does not exist: " << this->Name;
  this->SetError(e.str().c_str());
  return false;
}

//----------------------------------------------------------------------------
bool cmGetPropertyCommand::HandleVariableMode()
{
  if(!this->Name.empty())
    {
    this->SetError("given name for VARIABLE scope.");
    return false;
    }

  return this->StoreResult
    (this->Makefile->GetDefinition(this->PropertyName.c_str()));
}
