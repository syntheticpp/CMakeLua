/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmCTestReadCustomFilesCommand.h,v $
  Language:  C++
  Date:      $Date: 2006/03/10 20:03:09 $
  Version:   $Revision: 1.2 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmCTestReadCustomFilesCommand_h
#define cmCTestReadCustomFilesCommand_h

#include "cmCTestCommand.h"

/** \class cmCTestReadCustomFiles
 * \brief Run a ctest script
 *
 * cmLibrarysCommand defines a list of executable (i.e., test)
 * programs to create.
 */
class cmCTestReadCustomFilesCommand : public cmCTestCommand
{
public:

  cmCTestReadCustomFilesCommand() {}
  
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    cmCTestReadCustomFilesCommand* ni = new cmCTestReadCustomFilesCommand;
    ni->CTest = this->CTest;
    return ni;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "CTEST_READ_CUSTOM_FILES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "read CTestCustom files.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  CTEST_READ_CUSTOM_FILES( directory ... )\n"
      "Read all the CTestCustom.ctest or CTestCustom.cmake files from "
      "the given directory.";
    }

  cmTypeMacro(cmCTestReadCustomFilesCommand, cmCTestCommand);

};


#endif
