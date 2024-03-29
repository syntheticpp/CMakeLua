/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmLocalUnixMakefileGenerator3.h,v $
  Language:  C++
  Date:      $Date: 2008/01/13 21:36:20 $
  Version:   $Revision: 1.82 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmLocalUnixMakefileGenerator3_h
#define cmLocalUnixMakefileGenerator3_h

#include "cmLocalGenerator.h"

class cmCustomCommand;
class cmDependInformation;
class cmDepends;
class cmMakefileTargetGenerator;
class cmTarget;
class cmSourceFile;

/** \class cmLocalUnixMakefileGenerator3
 * \brief Write a LocalUnix makefiles.
 *
 * cmLocalUnixMakefileGenerator3 produces a LocalUnix makefile from its
 * member Makefile.
 */
class cmLocalUnixMakefileGenerator3 : public cmLocalGenerator
{
public:
  cmLocalUnixMakefileGenerator3();
  virtual ~cmLocalUnixMakefileGenerator3();

  /**
   * Process the CMakeLists files for this directory to fill in the
   * Makefile ivar 
   */
  virtual void Configure();

  /**
   * Generate the makefile for this directory. 
   */
  virtual void Generate();

  
  // this returns the relative path between the HomeOutputDirectory and this
  // local generators StartOutputDirectory
  const std::string &GetHomeRelativeOutputPath();

  // Write out a make rule 
  void WriteMakeRule(std::ostream& os,
                     const char* comment,
                     const char* target,
                     const std::vector<std::string>& depends,
                     const std::vector<std::string>& commands,
                     bool symbolic,
                     bool in_help = false);
  
  // write the main variables used by the makefiles
  void WriteMakeVariables(std::ostream& makefileStream);

  // write the progress variables used by the makefiles
  void WriteProgressVariables(unsigned long total, unsigned long &current);
  void WriteAllProgressVariable();

  /**
   * If true, then explicitly pass MAKEFLAGS on the make all target for makes
   * that do not use environment variables.
   *
   */
  void SetPassMakeflags(bool s){this->PassMakeflags = s;}
  bool GetPassMakeflags() { return this->PassMakeflags; }
  
  /**
   * Set the flag used to keep the make program silent.
   */
  void SetMakeSilentFlag(const char* s) { this->MakeSilentFlag = s; }
  std::string &GetMakeSilentFlag() { return this->MakeSilentFlag; }

  /**
   * Set to true if the shell being used is the windows shell.
   * This controls if statements in the makefile and the SHELL variable.
   * The default is false.
   */
  void SetWindowsShell(bool v)  {this->WindowsShell = v;}

  /**
   * Set to true if the make tool being used is Watcom WMake.
   */
  void SetWatcomWMake(bool v)  {this->WatcomWMake = v;}

  /**
   * Set to true if the make tool being used is MinGW Make.
   */
  void SetMinGWMake(bool v)  {this->MinGWMake = v;}

  /**
   * Set to true if the make tool being used is NMake.
   */
  void SetNMake(bool v)  {this->NMake = v;}

  /**
   * Set to true if the shell being used is the MSYS shell.
   * This controls if statements in the makefile and the SHELL variable.
   * The default is false.
   */
  void SetMSYSShell(bool v)  {this->MSYSShell = v;}

  /**
   * If set to true, then NULL is set to nil for non Windows_NT.
   * This uses make syntax used by nmake and borland.
   * The default is false.
   */
  void SetDefineWindowsNULL(bool v)  {this->DefineWindowsNULL = v;}

  /**
   * If set to true, cd dir && command is used to 
   * run commands in a different directory.
   */
  void SetUnixCD(bool v)  {this->UnixCD = v;}

  /**
   * Set Support Verbose Variable.  If true, then .SILENT will
   * be not end with :  i.e. .SILENT: or .SILENT
   */
  void SetSilentNoColon(bool v)  {this->SilentNoColon = v;}

  /**
   * Set the command to use for native make shell echo.  The value
   * should include all parts of the command up to the beginning of
   * the message (including a whitespace separator).
   */
  void SetNativeEchoCommand(const char* cmd, bool isWindows)
    { this->NativeEchoCommand = cmd; this->NativeEchoWindows = isWindows; }

  /**
   * Set the string used to include one makefile into another default
   * is include.
   */
  void SetIncludeDirective(const char* s) { this->IncludeDirective = s; }
  const char *GetIncludeDirective() { return this->IncludeDirective.c_str(); }

  /**
   * Set max makefile variable size, default is 0 which means unlimited.
   */
  void SetMakefileVariableSize(int s) { this->MakefileVariableSize = s; }

  /**
   * If ignore lib prefix is true, then do not strip lib from the name
   * of a library.
   */
  void SetIgnoreLibPrefix(bool s) { this->IgnoreLibPrefix = s; }

  /**
   * Set whether passing a make target on a command line requires an
   * extra level of escapes.
   */
  void SetMakeCommandEscapeTargetTwice(bool b)
    { this->MakeCommandEscapeTargetTwice = b; }

  /**
   * Set whether the Borland curly brace command line hack should be
   * applied.
   */
  void SetBorlandMakeCurlyHack(bool b)
    { this->BorlandMakeCurlyHack = b; }

  // used in writing out Cmake files such as WriteDirectoryInformation
  static void WriteCMakeArgument(std::ostream& os, const char* s);

  /** creates the common disclainer text at the top of each makefile */
  void WriteDisclaimer(std::ostream& os);

  // write a  comment line #====... in the stream
  void WriteDivider(std::ostream& os);

  /** used to create a recursive make call */
  std::string GetRecursiveMakeCall(const char *makefile, const char* tgt);    
  
  // append flags to a string
  virtual void AppendFlags(std::string& flags, const char* newFlags);

  // append an echo command
  enum EchoColor { EchoNormal, EchoDepend, EchoBuild, EchoLink,
                   EchoGenerate, EchoGlobal };
  void AppendEcho(std::vector<std::string>& commands, const char* text,
                  EchoColor color = EchoNormal);

  virtual std::string GetTargetDirectory(cmTarget const& target) const;

    // create a command that cds to the start dir then runs the commands
  void CreateCDCommand(std::vector<std::string>& commands, 
                       const char *targetDir, const char *returnDir);

  static std::string ConvertToQuotedOutputPath(const char* p);

  std::string CreateMakeVariable(const char* sin, const char* s2in);

  /** Called from command-line hook to bring dependencies up to date
      for a target.  */
  virtual bool UpdateDependencies(const char* tgtInfo,
                                  bool verbose, bool color);

  /** Called from command-line hook to clear dependencies.  */
  virtual void ClearDependencies(cmMakefile* mf, bool verbose);
  
  /** write some extra rules such as make test etc */
  void WriteSpecialTargetsTop(std::ostream& makefileStream);
  void WriteSpecialTargetsBottom(std::ostream& makefileStream);

  std::string GetRelativeTargetDirectory(cmTarget const& target);

  // File pairs for implicit dependency scanning.  The key of the map
  // is the depender and the value is the explicit dependee.
  struct ImplicitDependFileMap: public std::map<cmStdString, cmStdString> {};
  struct ImplicitDependLanguageMap:
    public std::map<cmStdString, ImplicitDependFileMap> {};
  struct ImplicitDependTargetMap:
    public std::map<cmStdString, ImplicitDependLanguageMap> {};
  ImplicitDependLanguageMap const& GetImplicitDepends(cmTarget const& tgt);

  void AddImplicitDepends(cmTarget const& tgt, const char* lang,
                          const char* obj, const char* src);

  void AppendGlobalTargetDepends(std::vector<std::string>& depends,
                                 cmTarget& target);

  // write the target rules for the local Makefile into the stream
  void WriteLocalAllRules(std::ostream& ruleFileStream);
  
  struct LocalObjectEntry
  {
    cmTarget* Target;
    std::string Language;
    LocalObjectEntry(): Target(0), Language() {}
    LocalObjectEntry(cmTarget* t, const char* lang):
      Target(t), Language(lang) {}
  };
  struct LocalObjectInfo: public std::vector<LocalObjectEntry>
  {
    bool HasSourceExtension;
  };
  std::map<cmStdString, LocalObjectInfo> const& GetLocalObjectFiles()
    { return this->LocalObjectFiles;}

  std::vector<cmStdString> const& GetLocalHelp() { return this->LocalHelp; }

  // return info about progress actions
  unsigned long GetNumberOfProgressActions();
  unsigned long GetNumberOfProgressActionsForTarget(const char *);

  /** Get whether to create rules to generate preprocessed and
      assembly sources.  This could be converted to a variable lookup
      later.  */
  bool GetCreatePreprocessedSourceRules()
    {
    return !this->SkipPreprocessedSourceRules;
    }
  bool GetCreateAssemblySourceRules()
    {
    return !this->SkipAssemblySourceRules;
    }
  // Get the directories into which the .o files will go for this target
  void GetTargetObjectFileDirectories(cmTarget* target,
                                      std::vector<std::string>& dirs);
protected:
  void WriteLocalMakefile();
  
  
  // write the target rules for the local Makefile into the stream
  void WriteLocalMakefileTargets(std::ostream& ruleFileStream,
                                 std::set<cmStdString> &emitted);

  // this method Writes the Directory informaiton files
  void WriteDirectoryInformationFile();


  // write the depend info 
  void WriteDependLanguageInfo(std::ostream& cmakefileStream, cmTarget &tgt);
  
  // write the local help rule
  void WriteHelpRule(std::ostream& ruleFileStream);
  
  // this converts a file name that is relative to the StartOuputDirectory
  // into a full path
  std::string ConvertToFullPath(const std::string& localPath);

  
  void WriteConvenienceRule(std::ostream& ruleFileStream,
                            const char* realTarget,
                            const char* helpTarget);

  void WriteTargetDependRule(std::ostream& ruleFileStream,
                             cmTarget& target);
  void WriteTargetCleanRule(std::ostream& ruleFileStream,
                            cmTarget& target,
                            const std::vector<std::string>& files);
  void WriteTargetRequiresRule(std::ostream& ruleFileStream,
                               cmTarget& target,
                               const std::vector<std::string>& objects);
  void WriteObjectConvenienceRule(std::ostream& ruleFileStream,
                                  const char* comment, const char* output,
                                  LocalObjectInfo const& info);
  
  std::string GetObjectFileName(cmTarget& target,
                                const cmSourceFile& source,
                                std::string* nameWithoutTargetDir = 0,
                                bool* hasSourceExtension = 0);

  void AppendRuleDepend(std::vector<std::string>& depends,
                        const char* ruleFileName);
  void AppendCustomDepends(std::vector<std::string>& depends,
                           const std::vector<cmCustomCommand>& ccs);
  void AppendCustomDepend(std::vector<std::string>& depends,
                          const cmCustomCommand& cc);
  void AppendCustomCommands(std::vector<std::string>& commands,
                            const std::vector<cmCustomCommand>& ccs);
  void AppendCustomCommand(std::vector<std::string>& commands,
                           const cmCustomCommand& cc,
                           bool echo_comment=false);
  void AppendCleanCommand(std::vector<std::string>& commands,
                          const std::vector<std::string>& files,
                          cmTarget& target, const char* filename =0);

  std::map<cmStdString, std::vector<int> > ProgressFiles;

  // Helper methods for dependeny updates.
  bool ScanDependencies(const char* targetDir);
  void CheckMultipleOutputs(bool verbose);

private:
  friend class cmMakefileTargetGenerator;
  friend class cmMakefileExecutableTargetGenerator;
  friend class cmMakefileLibraryTargetGenerator;
  friend class cmMakefileUtilityTargetGenerator;
  friend class cmGlobalUnixMakefileGenerator3;

  ImplicitDependTargetMap ImplicitDepends;

  //==========================================================================
  // Configuration settings.
  int MakefileVariableSize;
  std::string IncludeDirective;
  std::string MakeSilentFlag;
  std::string ConfigurationName;
  std::string NativeEchoCommand;
  bool NativeEchoWindows;
  bool DefineWindowsNULL;
  bool UnixCD;
  bool PassMakeflags;
  bool SilentNoColon;
  bool MakeCommandEscapeTargetTwice;
  bool BorlandMakeCurlyHack;
  //==========================================================================

  std::string HomeRelativeOutputPath;

  /* Copy the setting of CMAKE_COLOR_MAKEFILE from the makefile at the
     beginning of generation to avoid many duplicate lookups.  */
  bool ColorMakefile;

  /* Copy the setting of CMAKE_SKIP_PREPROCESSED_SOURCE_RULES and
     CMAKE_SKIP_ASSEMBLY_SOURCE_RULES at the beginning of generation to
     avoid many duplicate lookups.  */
  bool SkipPreprocessedSourceRules;
  bool SkipAssemblySourceRules;

  std::map<cmStdString, LocalObjectInfo> LocalObjectFiles;
  std::vector<cmStdString> LocalHelp;

  /* does the work for each target */
  std::vector<cmMakefileTargetGenerator *> TargetGenerators;
  std::map<cmStdString, cmStdString> MakeVariableMap;
  std::map<cmStdString, cmStdString> ShortMakeVariableMap;
};

#endif
