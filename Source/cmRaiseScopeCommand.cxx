/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmRaiseScopeCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2007/12/03 17:43:52 $
  Version:   $Revision: 1.1 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmRaiseScopeCommand.h"

// cmRaiseScopeCommand
bool cmRaiseScopeCommand
::InitialPass(std::vector<std::string> const& args)
{
  unsigned int i =0;
  for(; i < args.size(); ++i)
    {
    this->Makefile->RaiseScope(args[i].c_str());
    }
  return true;
}

