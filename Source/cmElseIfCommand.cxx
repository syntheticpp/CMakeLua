/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmElseIfCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2006/09/22 15:23:51 $
  Version:   $Revision: 1.1 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmElseIfCommand.h"

bool cmElseIfCommand::InitialPass(std::vector<std::string> const&)
{
  this->SetError("An ELSEIF command was found outside of a proper "
                 "IF ENDIF structure.");
  return false;
}
