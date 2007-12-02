/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmFindPackageCommand.h,v $
  Language:  C++
  Date:      $Date: 2007/10/10 15:47:43 $
  Version:   $Revision: 1.14 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmFindPackageCommand_h
#define cmFindPackageCommand_h

#include "cmCommand.h"

/** \class cmFindPackageCommand
 * \brief Load settings from an external project.
 *
 * cmFindPackageCommand
 */
class cmFindPackageCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmFindPackageCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "find_package";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Load settings for an external project.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  find_package(<name> [major.minor] [QUIET] [NO_MODULE]\n"
      "               [[REQUIRED|COMPONENTS] [components...]])\n"
      "Finds and loads settings from an external project.  <name>_FOUND will "
      "be set to indicate whether the package was found.  Settings that "
      "can be used when <name>_FOUND is true are package-specific.  The "
      "package is found through several steps.  "
      "Directories listed in CMAKE_MODULE_PATH are searched for files called "
      "\"Find<name>.cmake\".  If such a file is found, it is read and "
      "processed by CMake, and is responsible for finding the package.  "
      "This first step may be skipped by using the NO_MODULE option.  "
      "If no such file is found, it is expected that the package is another "
      "project built by CMake that has a \"<name>Config.cmake\" file.  "
      "A cache entry called <name>_DIR is created and is expected to be set "
      "to the directory containing this file.  If the file is found, it is "
      "read and processed by CMake to load the settings of the package.  If "
      "<name>_DIR has not been set during a configure step, the command "
      "will generate an error describing the problem unless the QUIET "
      "argument is specified.  If <name>_DIR has been set to a directory "
      "not containing a \"<name>Config.cmake\" file, an error is always "
      "generated.  If REQUIRED is specified and the package is not found, "
      "a FATAL_ERROR is generated and the configure step stops executing.  "
      "A package-specific list of components may be listed after the "
      "REQUIRED option, or after the COMPONENTS option if no REQUIRED "
      "option is given.";
    }
  
  cmTypeMacro(cmFindPackageCommand, cmCommand);
private:
  void AppendSuccessInformation(bool quiet);
  void AppendToProperty(const char* propertyName);
  bool FindModule(bool& found, bool quiet, bool required);
  bool FindConfig();
  std::string SearchForConfig() const;
  bool ReadListFile(const char* f);

  cmStdString Name;
  cmStdString Variable;
  cmStdString Config;
  std::vector<cmStdString> Builds;
  std::vector<cmStdString> Prefixes;
  std::vector<cmStdString> Relatives;
};


#endif
