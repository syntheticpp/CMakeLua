/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmDependsC.h,v $
  Language:  C++
  Date:      $Date: 2007-02-05 14:48:38 $
  Version:   $Revision: 1.20 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmDependsC_h
#define cmDependsC_h

#include "cmDepends.h"
#include <cmsys/RegularExpression.hxx>
#include <queue>

/** \class cmDependsC
 * \brief Dependency scanner for C and C++ object files.
 */
class cmDependsC: public cmDepends
{
public:
  /** Checking instances need to know the build directory name and the
      relative path from the build directory to the target file.  */
  cmDependsC();
  cmDependsC(std::vector<std::string> const& includes,
             const char* scanRegex, const char* complainRegex,
             const cmStdString& cachFileName);

  /** Virtual destructor to cleanup subclasses properly.  */
  virtual ~cmDependsC();

protected:
  typedef std::vector<char> t_CharBuffer;

  // Implement writing/checking methods required by superclass.
  virtual bool WriteDependencies(const char *src,
                                 const char *file,
                                 std::ostream& makeDepends,
                                 std::ostream& internalDepends);

  // Method to scan a single file.
  void Scan(std::istream& is, const char* directory,
    const cmStdString& fullName);

  // The include file search path.
  std::vector<std::string> const* IncludePath;

  // Regular expression to identify C preprocessor include directives.
  cmsys::RegularExpression IncludeRegexLine;

  // Regular expressions to choose which include files to scan
  // recursively and which to complain about not finding.
  cmsys::RegularExpression IncludeRegexScan;
  cmsys::RegularExpression IncludeRegexComplain;
  const std::string IncludeRegexLineString;
  const std::string IncludeRegexScanString;
  const std::string IncludeRegexComplainString;

public:
  // Data structures for dependency graph walk.
  struct UnscannedEntry
  {
    cmStdString FileName;
    cmStdString QuotedLocation;
  };

  struct cmIncludeLines
  {
    cmIncludeLines(): Used(false) {}
    std::vector<UnscannedEntry> UnscannedEntries;
    bool Used;
  };
protected:
  std::set<cmStdString> Encountered;
  std::queue<UnscannedEntry> Unscanned;
  t_CharBuffer Buffer;

  std::map<cmStdString, cmIncludeLines *> FileCache;
  std::map<cmStdString, cmStdString> HeaderLocationCache;

  cmStdString CacheFileName;

  void WriteCacheFile() const;
  void ReadCacheFile();
private:
  cmDependsC(cmDependsC const&); // Purposely not implemented.
  void operator=(cmDependsC const&); // Purposely not implemented.
};

#endif
