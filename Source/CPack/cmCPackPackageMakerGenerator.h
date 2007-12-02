/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmCPackPackageMakerGenerator.h,v $
  Language:  C++
  Date:      $Date: 2007/11/05 21:55:45 $
  Version:   $Revision: 1.11 $

  Copyright (c) 2002 Kitware, Inc. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef cmCPackPackageMakerGenerator_h
#define cmCPackPackageMakerGenerator_h


#include "cmCPackGenerator.h"

/** \class cmCPackPackageMakerGenerator
 * \brief A generator for PackageMaker files
 *
 * http://developer.apple.com/documentation/Darwin
 * /Reference/ManPages/man1/packagemaker.1.html
 */
class cmCPackPackageMakerGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackPackageMakerGenerator, cmCPackGenerator);

  /**
   * Construct generator
   */
  cmCPackPackageMakerGenerator();
  virtual ~cmCPackPackageMakerGenerator();

protected:
  virtual int InitializeInternal();
  int CompressFiles(const char* outFileName, const char* toplevel,
    const std::vector<std::string>& files);
  virtual const char* GetOutputExtension() { return ".dmg"; }
  virtual const char* GetOutputPostfix() { return "darwin"; }

  bool CopyCreateResourceFile(const char* name);
  bool CopyResourcePlistFile(const char* name);

  double PackageMakerVersion;
};

#endif
