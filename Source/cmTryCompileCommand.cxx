/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmTryCompileCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/24 15:27:51 $
  Version:   $Revision: 1.62 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmTryCompileCommand.h"

// cmTryCompileCommand
bool cmTryCompileCommand::InitialPass(std::vector<std::string> const& argv)
{
  if(argv.size() < 3)
    {
    return false;
    }

  this->TryCompileCode(argv);

  // if They specified clean then we clean up what we can
  if (this->SrcFileSignature)
    {
    if(!this->Makefile->GetCMakeInstance()->GetDebugTryCompile())
      {
      this->CleanupFiles(this->BinaryDirectory.c_str());
      }
    }
  return true;
}

