/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmExportLibraryDependencies.h,v $
  Language:  C++
  Date:      $Date: 2008/01/23 15:27:59 $
  Version:   $Revision: 1.9 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmExportLibraryDependenciesCommand_h
#define cmExportLibraryDependenciesCommand_h

#include "cmCommand.h"

/** \class cmExportLibraryDependenciesCommand
 * \brief Add a test to the lists of tests to run.
 *
 * cmExportLibraryDependenciesCommand adds a test to the list of tests to run
 * 
 */
class cmExportLibraryDependenciesCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmExportLibraryDependenciesCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * This is called at the end after all the information
   * specified by the command is accumulated. 
   */
  virtual void FinalPass();

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "export_library_dependencies";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return 
      "Write out the dependency information for all targets of a project.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  export_library_dependencies(<file> [APPEND])\n"
      "Create a file named <file> that can be included into a CMake listfile "
      "with the INCLUDE command.  The file will contain a number of SET "
      "commands that will set all the variables needed for library dependency "
      "information.  This should be the last command in the top level "
      "CMakeLists.txt file of the project.  If the APPEND option is "
      "specified, the SET commands will be appended to the given file "
      "instead of replacing it.";
    }
  
  cmTypeMacro(cmExportLibraryDependenciesCommand, cmCommand);

private:
  std::string Filename;
  bool Append;
  void ConstFinalPass() const;
};


#endif
