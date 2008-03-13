#include "cmPolicies.h"
#include "cmake.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmVersion.h"
#include <map>
#include <set>
#include <queue>
#include <assert.h>

const char* cmPolicies::PolicyStatusNames[] = {
  "OLD", "WARN", "NEW", "REQUIRED_IF_USED", "REQUIRED_ALWAYS"
};

class cmPolicy
{
public:  
  cmPolicy(cmPolicies::PolicyID iD, 
            const char *idString,
            const char *shortDescription,
            const char *longDescription,
            unsigned int majorVersionIntroduced,
            unsigned int minorVersionIntroduced,
            unsigned int patchVersionIntroduced,
            cmPolicies::PolicyStatus status)
  {
    if (!idString || !shortDescription || ! longDescription)
    {
      cmSystemTools::Error("Attempt to define a policy without "
        "all parameters being specified!");
      return;
    }
    this->ID = iD;
    this->IDString = idString;
    this->ShortDescription = shortDescription;
    this->LongDescription = longDescription;
    this->MajorVersionIntroduced = majorVersionIntroduced;
    this->MinorVersionIntroduced = minorVersionIntroduced;
    this->PatchVersionIntroduced = patchVersionIntroduced;
    this->Status = status;
  }

  std::string GetVersionString()
  {
    cmOStringStream error;
    error << this->MajorVersionIntroduced << "." <<
      this->MinorVersionIntroduced << "." <<
      this->PatchVersionIntroduced; 
    return error.str();
  }

  bool IsPolicyNewerThan(unsigned int majorV, 
                         unsigned int minorV,
                         unsigned int patchV)
  {
    if (majorV < this->MajorVersionIntroduced)
    {
      return true;
    }
    if (majorV > this->MajorVersionIntroduced)
    {
      return false;
    }
    if (minorV < this->MinorVersionIntroduced)
    {
      return true;
    }
    if (minorV > this->MinorVersionIntroduced)
    {
      return false;
    }
    return (patchV < this->PatchVersionIntroduced);
  }
  
  cmPolicies::PolicyID ID;
  std::string IDString;
  std::string ShortDescription;
  std::string LongDescription;
  unsigned int MajorVersionIntroduced;
  unsigned int MinorVersionIntroduced;
  unsigned int PatchVersionIntroduced;
  cmPolicies::PolicyStatus Status;
};

cmPolicies::cmPolicies()
{
  // define all the policies
  this->DefinePolicy(
    CMP0000, "CMP0000",
    "A policy version number must be specified.",
    "CMake requires that projects specify the version of CMake to which "
    "they have been written.  "
    "This policy has been put in place to help CMake maintain backwards "
    "compatibility with existing projects while allowing it to evolve "
    "more rapidly.\n"
    "The easiest way to specify a policy version number is to "
    "call the cmake_policy command at the top of your CMakeLists file:\n"
    "  cmake_policy(VERSION <major>.<minor>)\n"
    "where <major>.<minor> is the version of CMake you want to support.  "
    "The cmake_minimum_required command may also be used; see its "
    "documentation for details.",
    2,6,0, cmPolicies::WARN
    );

  this->DefinePolicy(
    CMP0001, "CMP0001",
    "CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.",
    "The OLD behavior is to check CMAKE_BACKWARDS_COMPATIBILITY and present "
    "it to the user.  "
    "The NEW behavior is to ignore CMAKE_BACKWARDS_COMPATIBILITY "
    "completely.\n"
    "In CMake 2.4 and below the variable CMAKE_BACKWARDS_COMPATIBILITY was "
    "used to request compatibility with earlier versions of CMake.  "
    "In CMake 2.6 and above all compatibility issues are handled by policies "
    "and the cmake_policy command.  "
    "However, CMake must still check CMAKE_BACKWARDS_COMPATIBILITY for "
    "projects written for CMake 2.4 and below.",
    2,6,0, cmPolicies::WARN
    );

  this->DefinePolicy(
    CMP0002, "CMP0002",
    "Logical target names must be globally unique.",
    "Targets names created with "
    "add_executable, add_library, or add_custom_target "
    "are logical build target names.  "
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
    "Custom targets must simply have globally unique names (unless one "
    "uses the global property ALLOW_DUPLICATE_CUSTOM_TARGETS with a "
    "Makefiles generator).",
    2,6,0, cmPolicies::WARN
    );

  this->DefinePolicy(
    CMP0003, "CMP0003",
    "Libraries linked via full path no longer produce linker search paths.",
    "This policy affects how libraries whose full paths are NOT known "
    "are found at link time, but was created due to a change in how CMake "
    "deals with libraries whose full paths are known.  "
    "Consider the code\n"
    "  target_link_libraries(myexe /path/to/libA.so)\n"
    "CMake 2.4 and below implemented linking to libraries whose full paths "
    "are known by splitting them on the link line into separate components "
    "consisting of the linker search path and the library name.  "
    "The example code might have produced something like\n"
    "  ... -L/path/to -lA ...\n"
    "in order to link to library A.  "
    "An analysis was performed to order multiple link directories such that "
    "the linker would find library A in the desired location, but there "
    "are cases in which this does not work.  "
    "CMake versions 2.6 and above use the more reliable approach of passing "
    "the full path to libraries directly to the linker in most cases.  "
    "The example code now produces something like\n"
    "  ... /path/to/libA.so ....\n"
    "Unfortunately this change can break code like\n"
    "  target_link_libraries(myexe /path/to/libA.so B)\n"
    "where \"B\" is meant to find \"/path/to/libB.so\".  "
    "This code is wrong because the user is asking the linker to find "
    "library B but has not provided a linker search path (which may be "
    "added with the link_directories command).  "
    "However, with the old linking implementation the code would work "
    "accidentally because the linker search path added for library A "
    "allowed library B to be found."
    "\n"
    "In order to support projects depending on linker search paths "
    "added by linking to libraries with known full paths, the OLD "
    "behavior for this policy will add the linker search paths even "
    "though they are not needed for their own libraries.  "
    "When this policy is set to OLD, CMake will produce a link line such as\n"
    "  ... -L/path/to /path/to/libA.so -lB ...\n"
    "which will allow library B to be found as it was previously.  "
    "When this policy is set to NEW, CMake will produce a link line such as\n"
    "  ... /path/to/libA.so -lB ...\n"
    "which more accurately matches what the project specified."
    "\n"
    "The setting for this policy used when generating the link line is that "
    "in effect when the target is created by an add_executable or "
    "add_library command.  For the example described above, the code\n"
    "  cmake_policy(SET CMP0003 OLD) # or cmake_policy(VERSION 2.4)\n"
    "  add_executable(myexe myexe.c)\n"
    "  target_link_libraries(myexe /path/to/libA.so B)\n"
    "will work and suppress the warning for this policy.  "
    "It may also be updated to work with the corrected linking approach:\n"
    "  cmake_policy(SET CMP0003 NEW) # or cmake_policy(VERSION 2.6)\n"
    "  link_directories(/path/to) # needed to find library B\n"
    "  add_executable(myexe myexe.c)\n"
    "  target_link_libraries(myexe /path/to/libA.so B)\n"
    "Even better, library B may be specified with a full path:\n"
    "  add_executable(myexe myexe.c)\n"
    "  target_link_libraries(myexe /path/to/libA.so /path/to/libB.so)\n"
    "When all items on the link line have known paths CMake does not check "
    "this policy so it has no effect.",
    2,6,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0004, "CMP0004",
    "Libraries linked may not have leading or trailing whitespace.",
    "CMake versions 2.4 and below silently removed leading and trailing "
    "whitespace from libraries linked with code like\n"
    "  target_link_libraries(myexe \" A \")\n"
    "This could lead to subtle errors in user projects.\n"
    "The OLD behavior for this policy is to silently remove leading and "
    "trailing whitespace.  "
    "The NEW behavior for this policy is to diagnose the existence of "
    "such whitespace as an error.  "
    "The setting for this policy used when checking the library names is "
    "that in effect when the target is created by an add_executable or "
    "add_library command.",
    2,6,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0005, "CMP0005",
    "Preprocessor definition values are now escaped automatically.",
    "This policy determines whether or not CMake should generate escaped "
    "preprocessor definition values added via add_definitions.  "
    "CMake versions 2.4 and below assumed that only trivial values would "
    "be given for macros in add_definitions calls.  "
    "It did not attempt to escape non-trivial values such as string "
    "literals in generated build rules.  "
    "CMake versions 2.6 and above support escaping of most values, but "
    "cannot assume the user has not added escapes already in an attempt to "
    "work around limitations in earlier versions.\n"
    "The OLD behavior for this policy is to place definition values given "
    "to add_definitions directly in the generated build rules without "
    "attempting to escape anything.  "
    "The NEW behavior for this policy is to generate correct escapes "
    "for all native build tools automatically.  "
    "See documentation of the COMPILE_DEFINITIONS target property for "
    "limitations of the escaping implementation.",
    2,6,0, cmPolicies::WARN);
}

cmPolicies::~cmPolicies()
{
  // free the policies
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator i 
    = this->Policies.begin();
  for (;i != this->Policies.end(); ++i)
  {
    delete i->second;
  }
}

void cmPolicies::DefinePolicy(cmPolicies::PolicyID iD,
                              const char *idString, 
                              const char *shortDescription,
                              const char *longDescription,
                              unsigned int majorVersionIntroduced,
                              unsigned int minorVersionIntroduced,
                              unsigned int patchVersionIntroduced,
                              cmPolicies::PolicyStatus status)
{
  // a policy must be unique and can only be defined once
  if (this->Policies.find(iD) != this->Policies.end())
  {
    cmSystemTools::Error("Attempt to redefine a CMake policy for policy "
      "ID ", this->GetPolicyIDString(iD).c_str());
    return;
  }
  
  this->Policies[iD] = new cmPolicy(iD, idString,
                                    shortDescription,
                                    longDescription,
                                    majorVersionIntroduced,
                                    minorVersionIntroduced,
                                    patchVersionIntroduced,
                                    status);
  this->PolicyStringMap[idString] = iD;
}

bool cmPolicies::ApplyPolicyVersion(cmMakefile *mf, 
                                    const char *version)
{
  std::string ver = "2.4.0";

  if (version && strlen(version) > 0)
  {
    ver = version;
  }

  unsigned int majorVer = 2;
  unsigned int minorVer = 0;
  unsigned int patchVer = 0;

  // parse the string
  if(sscanf(ver.c_str(), "%u.%u.%u",
            &majorVer, &minorVer, &patchVer) < 2)
    {
    return false;
    }
  
  // it is an error if the policy version is less than 2.4
  if (majorVer < 2 || majorVer == 2 && minorVer < 4)
    {
    mf->IssueMessage(cmake::FATAL_ERROR,
      "An attempt was made to set the policy version of CMake to something "
      "earlier than \"2.4\".  "
      "In CMake 2.4 and below backwards compatibility was handled with the "
      "CMAKE_BACKWARDS_COMPATIBILITY variable.  "
      "In order to get compatibility features supporting versions earlier "
      "than 2.4 set policy CMP0001 to OLD to tell CMake to check the "
      "CMAKE_BACKWARDS_COMPATIBILITY variable.  "
      "One way to so this is to set the policy version to 2.4 exactly."
      );
    }

  // now loop over all the policies and set them as appropriate
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator i 
    = this->Policies.begin();
  for (;i != this->Policies.end(); ++i)
  {
    if (i->second->IsPolicyNewerThan(majorVer,minorVer,patchVer))
    {
      if (!mf->SetPolicy(i->second->ID, cmPolicies::WARN))
      {
        return false;
      }
    }
    else
    {
      if (!mf->SetPolicy(i->second->ID, cmPolicies::NEW))
      {
        return false;
      }
    }
  }
  return true;
}

// is this a valid status the listfile can set this policy to?
bool cmPolicies::IsValidPolicyStatus(cmPolicies::PolicyID id, 
                                     cmPolicies::PolicyStatus status)
{
  // if they are setting a feature to anything other than OLD or WARN and the
  // feature is not known about then that is an error
  if (this->Policies.find(id) == this->Policies.end())
  {
    if (status == cmPolicies::WARN ||
        status == cmPolicies::OLD)
    {
      return true;
    }
    cmOStringStream error;
    error << 
      "Error: an attempt was made to enable the new behavior for " <<
      "a new feature that is in a later version of CMake than "
      "what you are runing, please upgrade to a newer version "
      "of CMake.";
    cmSystemTools::Error(error.str().c_str());
    return false;  
  }

  // now we know the feature is defined, so the only issue is if someone is
  // setting it to WARN or OLD when the feature is REQUIRED_ALWAYS
  if ((status == cmPolicies::WARN || 
      status == cmPolicies::OLD) && 
      this->Policies[id]->Status == cmPolicies::REQUIRED_ALWAYS)
  {
    cmOStringStream error;
    error << 
      "Error: an attempt was made to enable the old behavior for " <<
      "a feature that is no longer supported. The feature in " <<
      "question is feature " <<
      id <<
      " which had new behavior introduced in CMake version " <<
      this->Policies[id]->GetVersionString() <<
      " please either update your CMakeLists files to conform to " <<
      "the new behavior " <<
      "or use an older version of CMake that still supports " <<
      "the old behavior. Run cmake --help-policies " <<
      id << " for more information.";
    cmSystemTools::Error(error.str().c_str());
    return false;
  }
  
  return true;
}

// is this a valid status the listfile can set this policy to?
bool cmPolicies::IsValidUsedPolicyStatus(cmPolicies::PolicyID id, 
                                         cmPolicies::PolicyStatus status)
{
  // if they are setting a feature to anything other than OLD or WARN and the
  // feature is not known about then that is an error
  if (this->Policies.find(id) == this->Policies.end())
  {
    if (status == cmPolicies::WARN ||
        status == cmPolicies::OLD)
    {
      return true;
    }
    cmOStringStream error;
    error << 
      "Error: an attempt was made to enable the new behavior for " <<
      "a new feature that is in a later version of CMake than "
      "what you are runing, please upgrade to a newer version "
      "of CMake.";
    cmSystemTools::Error(error.str().c_str());
    return false;  
  }

  // now we know the feature is defined, so the only issue is if someone is
  // setting it to WARN or OLD when the feature is REQUIRED_ALWAYS
  if ((status == cmPolicies::WARN || 
      status == cmPolicies::OLD) && 
      (this->Policies[id]->Status == cmPolicies::REQUIRED_ALWAYS ||
       this->Policies[id]->Status == cmPolicies::REQUIRED_IF_USED))
  {
    cmOStringStream error;
    error << 
      "Error: an attempt was made to enable the old behavior for " <<
      "a feature that is no longer supported. The feature in " <<
      "question is feature " <<
      id <<
      " which had new behavior introduced in CMake version " <<
      this->Policies[id]->GetVersionString() <<
      " please either update your CMakeLists files to conform to " <<
      "the new behavior " <<
      "or use an older version of CMake that still supports " <<
      "the old behavior. Run cmake --help-policies " <<
      id << " for more information.";
    cmSystemTools::Error(error.str().c_str());
    return false;
  }
  
  return true;
}

bool cmPolicies::GetPolicyID(const char *id, cmPolicies::PolicyID &pid)
{
  if (!id || strlen(id) < 1)
  {
    return false;
  }
  std::map<std::string,cmPolicies::PolicyID>::iterator pos = 
    this->PolicyStringMap.find(id);
  if (pos == this->PolicyStringMap.end())
  {
    return false;
  }
  pid = pos->second;
  return true;
}

std::string cmPolicies::GetPolicyIDString(cmPolicies::PolicyID pid)
{
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator pos = 
    this->Policies.find(pid);
  if (pos == this->Policies.end())
  {
    return "";
  }
  return pos->second->IDString;
}


///! return a warning string for a given policy
std::string cmPolicies::GetPolicyWarning(cmPolicies::PolicyID id)
{
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator pos = 
    this->Policies.find(id);
  if (pos == this->Policies.end())
  {
    cmSystemTools::Error(
      "Request for warning text for undefined policy!");
    return "Request for warning text for undefined policy!";
  }

  cmOStringStream msg;
  msg <<
    "Policy " << pos->second->IDString << " is not set: "
    "" << pos->second->ShortDescription << "  "
    "Run \"cmake --help-policy " << pos->second->IDString << "\" for "
    "policy details.  "
    "Use the cmake_policy command to set the policy "
    "and suppress this warning.";
  return msg.str();
}
  
  
///! return an error string for when a required policy is unspecified
std::string cmPolicies::GetRequiredPolicyError(cmPolicies::PolicyID id)
{
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator pos = 
    this->Policies.find(id);
  if (pos == this->Policies.end())
  {
    cmSystemTools::Error(
      "Request for error text for undefined policy!");
    return "Request for warning text for undefined policy!";
  }

  cmOStringStream error;
  error <<
    "Policy " << pos->second->IDString << " is not set to NEW: "
    "" << pos->second->ShortDescription << "  "
    "Run \"cmake --help-policy " << pos->second->IDString << "\" for "
    "policy details.  "
    "CMake now requires this policy to be set to NEW by the project.  "
    "The policy may be set explicitly using the code\n"
    "  cmake_policy(SET " << pos->second->IDString << " NEW)\n"
    "or by upgrading all policies with the code\n"
    "  cmake_policy(VERSION " << pos->second->GetVersionString() <<
    ") # or later\n"
    "Run \"cmake --help-command cmake_policy\" for more information.";
  return error.str();
}

///! Get the default status for a policy
cmPolicies::PolicyStatus 
cmPolicies::GetPolicyStatus(cmPolicies::PolicyID id)
{
  // if the policy is not know then what?
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator pos = 
    this->Policies.find(id);
  if (pos == this->Policies.end())
  {
    // TODO is this right?
    return cmPolicies::WARN;
  }
  
  return pos->second->Status;
}

void cmPolicies::GetDocumentation(std::vector<cmDocumentationEntry>& v)
{
  // now loop over all the policies and set them as appropriate
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator i 
    = this->Policies.begin();
  for (;i != this->Policies.end(); ++i)
  {
    cmOStringStream full;
    full << i->second->LongDescription;
    full << "\nThis policy was introduced in CMake version ";
    full << i->second->GetVersionString() << ".  ";
    full << "CMake version " << cmVersion::GetMajorVersion()
         << "." << cmVersion::GetMinorVersion() << " ";
    // add in some more text here based on status
    switch (i->second->Status)
    {
      case cmPolicies::WARN:
        full << "defaults to WARN for this policy.  "
             << "Use the cmake_policy command to set it to OLD or NEW.";
        break;
      case cmPolicies::OLD:
        full << "defaults to the OLD behavior for this policy.";
        break;
      case cmPolicies::NEW:
        full << "defaults to the NEW behavior for this policy.";
        break;
      case cmPolicies::REQUIRED_IF_USED:
        full << "requires the policy to be set to NEW if you use it.  "
             << "Use the cmake_policy command to set it to NEW.";
        break;
      case cmPolicies::REQUIRED_ALWAYS:
        full << "requires the policy to be set to NEW.  "
             << "Use the cmake_policy command to set it to NEW.";
        break;
    }
          
    cmDocumentationEntry e(i->second->IDString.c_str(),
                           i->second->ShortDescription.c_str(),
                           full.str().c_str());
    v.push_back(e);
  }
}
