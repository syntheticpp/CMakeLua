/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmCTestScriptHandler.h,v $
  Language:  C++
  Date:      $Date: 2007/06/08 20:06:33 $
  Version:   $Revision: 1.19 $

  Copyright (c) 2002 Kitware, Inc. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef cmCTestScriptHandler_h
#define cmCTestScriptHandler_h


#include "cmCTestGenericHandler.h"
#include "cmListFileCache.h"

class cmMakefile;
class cmLocalGenerator;
class cmGlobalGenerator;
class cmake;
class cmCTestCommand;

/** \class cmCTestScriptHandler
 * \brief A class that handles ctest -S invocations
 *
 * CTest script is controlled using several variables that script has to
 * specify and some optional ones. Required ones are:
 *   CTEST_SOURCE_DIRECTORY - Source directory of the project
 *   CTEST_BINARY_DIRECTORY - Binary directory of the project
 *   CTEST_COMMAND          - Testing commands
 *
 * Optional variables are:
 *   CTEST_BACKUP_AND_RESTORE
 *   CTEST_CMAKE_COMMAND
 *   CTEST_CMAKE_OUTPUT_FILE_NAME
 *   CTEST_CONTINUOUS_DURATION
 *   CTEST_CONTINUOUS_MINIMUM_INTERVAL
 *   CTEST_CVS_CHECKOUT
 *   CTEST_CVS_COMMAND
 *   CTEST_UPDATE_COMMAND
 *   CTEST_DASHBOARD_ROOT
 *   CTEST_ENVIRONMENT
 *   CTEST_INITIAL_CACHE
 *   CTEST_START_WITH_EMPTY_BINARY_DIRECTORY
 *   CTEST_START_WITH_EMPTY_BINARY_DIRECTORY_ONCE
 *
 * In addition the following variables can be used. The number can be 1-10.
 *   CTEST_EXTRA_UPDATES_1
 *   CTEST_EXTRA_UPDATES_2
 *   ...
 *   CTEST_EXTRA_UPDATES_10
 *
 * CTest script can use the following arguments CTest provides:
 *   CTEST_SCRIPT_ARG
 *   CTEST_SCRIPT_DIRECTORY
 *   CTEST_SCRIPT_NAME
 *
 */
class cmCTestScriptHandler : public cmCTestGenericHandler
{
public:
  cmTypeMacro(cmCTestScriptHandler, cmCTestGenericHandler);

  /**
   * Add a script to run, and if is should run in the current process
   */
  void AddConfigurationScript(const char *, bool pscope);

  /**
   * Run a dashboard using a specified confiuration script
   */
  int ProcessHandler();

  /*
   * Run a script
   */
  static bool RunScript(cmCTest* ctest, const char *script, bool InProcess);
  int RunCurrentScript();

  /*
   * Empty Binary Directory
   */
  static bool EmptyBinaryDirectory(const char *dir);

  /*
   * Some elapsed time handling functions
   */
  static void SleepInSeconds(unsigned int secondsToWait);
  void UpdateElapsedTime();

  /** 
   * Return the time remaianing that the script is allowed to run in
   * seconds if the user has set the variable CTEST_TIME_LIMIT. If that has
   * not been set it returns 1e7 seconds
   */
  double GetRemainingTimeAllowed();

  cmCTestScriptHandler();
  ~cmCTestScriptHandler();

  void Initialize();

  void CreateCMake();
  void GetCommandDocumentation(std::vector<cmDocumentationEntry>& v) const;

private:
  // reads in a script
  int ReadInScript(const std::string& total_script_arg);
  int ExecuteScript(const std::string& total_script_arg);

  // extract vars from the script to set ivars
  int ExtractVariables();

  // perform a CVS checkout of the source dir
  int CheckOutSourceDir();

  // perform any extra cvs updates that were requested
  int PerformExtraUpdates();

  // backup and restore dirs
  int BackupDirectories();
  void RestoreBackupDirectories();

  int RunConfigurationScript(const std::string& script, bool pscope);
  int RunConfigurationDashboard();

  // Add ctest command
  void AddCTestCommand(cmCTestCommand* command);

  std::vector<cmStdString> ConfigurationScripts;
  std::vector<bool> ScriptProcessScope;

  bool Backup;
  bool EmptyBinDir;
  bool EmptyBinDirOnce;

  cmStdString SourceDir;
  cmStdString BinaryDir;
  cmStdString BackupSourceDir;
  cmStdString BackupBinaryDir;
  cmStdString CTestRoot;
  cmStdString CVSCheckOut;
  cmStdString CTestCmd;
  cmStdString UpdateCmd;
  cmStdString CTestEnv;
  cmStdString InitCache;
  cmStdString CMakeCmd;
  cmStdString CMOutFile;
  std::vector<cmStdString> ExtraUpdates;

  double MinimumInterval;
  double ContinuousDuration;

  // what time in seconds did this script start running
  double ScriptStartTime;

  cmMakefile *Makefile;
  cmLocalGenerator *LocalGenerator;
  cmGlobalGenerator *GlobalGenerator;
  cmake *CMake;
};

#endif
