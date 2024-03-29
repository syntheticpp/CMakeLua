/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmGlobalKdevelopGenerator.h,v $
  Language:  C++
<<<<<<< cmGlobalKdevelopGenerator.h
  Date:      $Date: 2007/08/26 23:27:33 $
  Version:   $Revision: 1.7 $
=======
  Date:      $Date: 2008-03-27 21:40:43 $
  Version:   $Revision: 1.8 $
>>>>>>> 1.8

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  Copyright (c) 2004 Alexander Neundorf, neundorf@kde.org. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmGlobalKdevelopGenerator_h
#define cmGlobalKdevelopGenerator_h

#include "cmExternalMakefileProjectGenerator.h"

class cmLocalGenerator;

/** \class cmGlobalKdevelopGenerator
 * \brief Write Unix Makefiles accompanied by KDevelop3 project files.
 *
 * cmGlobalKdevelopGenerator produces a project file for KDevelop 3 (KDevelop
 * > 3.1.1).  The project is based on the "Custom Makefile based C/C++"
 * project of KDevelop.  Such a project consists of Unix Makefiles in the
 * build directory together with a <your_project>.kdevelop project file,
 * which contains the project settings and a <your_project>.kdevelop.filelist
 * file, which lists the source files relative to the kdevelop project
 * directory. The kdevelop project directory is the base source directory.
 */
class cmGlobalKdevelopGenerator : public cmExternalMakefileProjectGenerator
{
public:
  cmGlobalKdevelopGenerator();

  virtual const char* GetName() const
                          { return cmGlobalKdevelopGenerator::GetActualName();}
  static const char* GetActualName()                     { return "KDevelop3";}
  static cmExternalMakefileProjectGenerator* New() 
                                      { return new cmGlobalKdevelopGenerator; }
  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry, 
                                const char* fullName) const;

  virtual void Generate();
private:
  /*** Create the foo.kdevelop.filelist file, return false if it doesn't
    succeed.  If the file already exists the contents will be merged.  
    */
  bool CreateFilelistFile(const std::vector<cmLocalGenerator*>& lgs,
                          const std::string& outputDir, 
                          const std::string& projectDirIn,
                          const std::string& projectname,
                          std::string& cmakeFilePattern,
                          std::string& fileToOpen);

  /** Create the foo.kdevelop file. This one calls MergeProjectFiles()
    if it already exists, otherwise createNewProjectFile() The project
    files will be created in \a outputDir (in the build tree), the
    kdevelop project dir will be set to \a projectDir (in the source
    tree). \a cmakeFilePattern consists of a lists of all cmake
    listfiles used by this CMakeLists.txt */
  void CreateProjectFile(const std::string& outputDir,
                         const std::string& projectDir,
                         const std::string& projectname, 
                         const std::string& executable, 
                         const std::string& cmakeFilePattern,
                         const std::string& fileToOpen);

  /*** Reads the old foo.kdevelop line by line and only replaces the
       "important" lines 
       */
  void MergeProjectFiles(const std::string& outputDir,
                         const std::string& projectDir,
                         const std::string& filename,
                         const std::string& executable,
                         const std::string& cmakeFilePattern,
                         const std::string& fileToOpen,
                         const std::string& sessionFilename);
  ///! Creates a new foo.kdevelop and a new foo.kdevses file
  void CreateNewProjectFile(const std::string& outputDir, 
                            const std::string& projectDir, 
                            const std::string& filename,
                            const std::string& executable, 
                            const std::string& cmakeFilePattern,
                            const std::string& fileToOpen,
                            const std::string& sessionFilename);

  std::vector<std::string> Blacklist;
};

#endif
