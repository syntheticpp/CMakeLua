/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmOrderLinkDirectories.h,v $
  Language:  C++
  Date:      $Date: 2006/10/05 19:08:20 $
  Version:   $Revision: 1.20 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmOrderLinkDirectories_h
#define cmOrderLinkDirectories_h

#include <cmStandardIncludes.h>
#include <map>
#include <vector>
#include "cmTarget.h"
#include "cmsys/RegularExpression.hxx"


/** \class cmOrderLinkDirectories
 * \brief Compute the best -L path order
 *
 * This class computes the best order for -L paths.
 * It tries to make sure full path specified libraries are 
 * used.  For example if you have /usr/mylib/libfoo.a on as
 * a link library for a target, and you also have /usr/lib/libbar.a
 * and you also have /usr/lib/libfoo.a, then you would
 * want -L/usr/mylib -L/usr/lib to make sure the correct libfoo.a is 
 * found by the linker.  The algorithm is as follows:
 * - foreach library create a vector of directories it exists in.
 * - foreach directory create a vector of directories that must come
 *   after it, put this in a map<dir, vector<dir>> mapping from a directory
 *   to the vector of directories that it must be before.
 * - put all directories into a vector
 * - sort the vector with a compare function CanBeBefore
 *   CanBeBefore returns true if a directory is OK to be before
 *   another directory.  This is determined by looking at the 
 *   map<dir vector<dir>> and seeing if d1 is in the vector for d2.
 */
class cmOrderLinkDirectories
{
public:
  cmOrderLinkDirectories();
  ///! set link information from the target
  void SetLinkInformation(const char* targetName,
                          const std::vector<std::string>& linkLibraries,
                          const std::vector<std::string>& linkDirectories,
                          const cmTargetManifest& manifest,
                          const char* configSubdir);
  ///! Compute the best order for -L paths from GetLinkLibraries
  bool DetermineLibraryPathOrder();
  ///! Get the results from DetermineLibraryPathOrder
  void GetLinkerInformation(std::vector<cmStdString>& searchPaths,
                            std::vector<cmStdString>& linkItems)
  {
    linkItems = this->LinkItems;
    searchPaths = this->SortedSearchPaths;
  }
  // should be set from CMAKE_STATIC_LIBRARY_SUFFIX,
  // CMAKE_SHARED_LIBRARY_SUFFIX
  // CMAKE_LINK_LIBRARY_SUFFIX
  enum LinkType { LinkUnknown, LinkStatic, LinkShared };
  void AddLinkExtension(const char* e, LinkType type = LinkUnknown)
    {
    if(e && *e)
      {
      if(type == LinkStatic)
        {
        this->StaticLinkExtensions.push_back(e);
        }
      if(type == LinkShared)
        {
        this->SharedLinkExtensions.push_back(e);
        }
      this->LinkExtensions.push_back(e);
      }
    }
  // should be set from CMAKE_STATIC_LIBRARY_PREFIX
  void AddLinkPrefix(const char* s)
    {
    if(s)
      {
      this->LinkPrefixes.insert(s);
      }
    }
  // Return any warnings if the exist
  std::string GetWarnings();
  // return a list of all full path libraries
  void GetFullPathLibraries(std::vector<cmStdString>& libs);

  // Provide flags for switching library link type.
  void SetLinkTypeInformation(LinkType start_link_type,
                              const char* static_link_type_flag,
                              const char* shared_link_type_flag);

  // structure to hold a full path library link item
  struct Library
  {
    cmStdString FullPath;
    cmStdString File;
    cmStdString Path;
  };
  friend struct cmOrderLinkDirectoriesCompare;
  void DebugOn() 
    {
      this->Debug = true;
    }
  
private:
  void CreateRegularExpressions();
  std::string CreateExtensionRegex(std::vector<cmStdString> const& exts);
  void DetermineLibraryPathOrder(std::vector<cmStdString>& searchPaths,
                                 std::vector<cmStdString>& libs,
                                 std::vector<cmStdString>& sortedPaths);
  void PrepareLinkTargets();
  bool LibraryInDirectory(const char* desiredLib,
                          const char* dir, const char* lib);
  void FindLibrariesInSearchPaths();
  void FindIndividualLibraryOrders();
  void PrintMap(const char* name,
                std::map<cmStdString, std::vector<cmStdString> >& m);
  void PrintVector(const char* name,
                   std::vector<std::pair<cmStdString, 
                   std::vector<cmStdString> > >& m);
  void OrderPaths(std::vector<cmStdString>& paths);
  bool FindPathNotInDirectoryToAfterList(cmStdString& path);
  std::string NoCaseExpression(const char* str);
  bool LibraryMayConflict(const char* desiredLib,
                          const char* dir, const char* fname);
private:
  // set of files that will exist when the build occurs
  std::set<cmStdString> ManifestFiles;
  // map from library to directories that it is in other than its full path
  std::map<cmStdString, std::vector<cmStdString> > LibraryToDirectories;
  // map from directory to vector of directories that must be after it
  std::vector<std::pair<cmStdString, std::vector<cmStdString> > > 
  DirectoryToAfterList;
  std::set<cmStdString> DirectoryToAfterListEmitted;
  // map from full path to a Library struct
  std::map<cmStdString, Library> FullPathLibraries;
  // libraries that are found in multiple directories
  std::vector<Library> MultiDirectoryLibraries;
  // libraries that are only found in one directory
  std::vector<Library> SingleDirectoryLibraries;
  // This is a vector of all the link objects -lm or m
  std::vector<cmStdString> LinkItems;
  // Unprocessed link items
  std::vector<cmStdString> RawLinkItems;
  // This vector holds the sorted -L paths
  std::vector<cmStdString> SortedSearchPaths;
  // This vector holds the -F paths
  std::set<cmStdString> EmittedFrameworkPaths;
  // This is the set of -L paths unsorted, but unique
  std::set<cmStdString> LinkPathSet;
  // the names of link extensions
  std::vector<cmStdString> StaticLinkExtensions;
  std::vector<cmStdString> SharedLinkExtensions;
  std::vector<cmStdString> LinkExtensions;
  // the names of link prefixes
  std::set<cmStdString> LinkPrefixes;
  // set of directories that can not be put in the correct order
  std::set<cmStdString> ImpossibleDirectories;
  // Name of target
  cmStdString TargetName;
  // Subdirectory used for this configuration if any.
  cmStdString ConfigSubdir;

  // Link type adjustment.
  LinkType StartLinkType;
  LinkType CurrentLinkType;
  cmStdString StaticLinkTypeFlag;
  cmStdString SharedLinkTypeFlag;
  bool LinkTypeEnabled;
  void SetCurrentLinkType(LinkType lt);

  // library regular expressions
  cmsys::RegularExpression RemoveLibraryExtension;
  cmsys::RegularExpression ExtractStaticLibraryName;
  cmsys::RegularExpression ExtractSharedLibraryName;
  cmsys::RegularExpression ExtractAnyLibraryName;
  cmsys::RegularExpression SplitFramework;
  bool Debug;
};

#endif
