/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmEndFunctionCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2007/12/03 17:47:22 $
  Version:   $Revision: 1.1 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmEndFunctionCommand.h"

bool cmEndFunctionCommand
::InvokeInitialPass(std::vector<cmListFileArgument> const&)
{
  this->SetError("An ENDFUNCTION command was found outside of a proper "
                 "FUNCTION ENDFUNCTION structure. Or its arguments did not "
                 "match the opening FUNCTION command.");
  return false;
}

