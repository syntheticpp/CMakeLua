/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmRemoveDefinitionsCommand.h,v $
  Language:  C++
  Date:      $Date: 2007/10/10 15:47:43 $
  Version:   $Revision: 1.5 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmRemoveDefinitionsCommand_h
#define cmRemoveDefinitionsCommand_h

#include "cmCommand.h"

/** \class cmRemoveDefinitionsCommand
 * \brief Specify a list of compiler defines
 *
 * cmRemoveDefinitionsCommand specifies a list of compiler defines. 
 * These defines will
 * be removed from the compile command.  
 */
class cmRemoveDefinitionsCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmRemoveDefinitionsCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "remove_definitions";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return 
      "Removes -D define flags to the command line of C and C++ compilers.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  remove_definitions(-DFOO -DBAR ...)\n"
      "Removes flags from command line of C and C++ compilers.  "
      "This command can be used to remove any flag from a compile line, "
      "but the -D flag is accepted by most C/C++ compilers.  "
      "Other flags may not be as portable.";
    }
  
  cmTypeMacro(cmRemoveDefinitionsCommand, cmCommand);
};



#endif
