/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmakemain.cxx,v $
  Language:  C++
  Date:      $Date: 2008-03-11 20:02:10 $
  Version:   $Revision: 1.80 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// include these first, otherwise there will be problems on Windows 
// with GetCurrentDirectory() being redefined 
#ifdef CMAKE_BUILD_WITH_CMAKE
#include "cmDynamicLoader.h"
#include "cmDocumentation.h"
#endif

#include "cmake.h"
#include "cmCacheManager.h"
#include "cmListFileCache.h"
#include "cmakewizard.h"
#include "cmSourceFile.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"

#ifdef CMAKE_BUILD_WITH_CMAKE
//----------------------------------------------------------------------------
static const char * cmDocumentationName[][3] =
{
  {0,
   "  cmake - Cross-Platform Makefile Generator.", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationUsage[][3] =
{
  {0,
   "  cmake [options] <path-to-source>\n"
   "  cmake [options] <path-to-existing-build>", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationDescription[][3] =
{
  {0,
   "The \"cmake\" executable is the CMake command-line interface.  It may "
   "be used to configure projects in scripts.  Project configuration "
   "settings "
   "may be specified on the command line with the -D option.  The -i option "
   "will cause cmake to interactively prompt for such settings.", 0},
  CMAKE_STANDARD_INTRODUCTION,
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationOptions[][3] =
{
  CMAKE_STANDARD_OPTIONS_TABLE,
  {"-E", "CMake command mode.",
   "For true platform independence, CMake provides a list of commands "
   "that can be used on all systems. Run with -E help for the usage "
   "information."},
  {"-i", "Run in wizard mode.",
   "Wizard mode runs cmake interactively without a GUI.  The user is "
   "prompted to answer questions about the project configuration.  "
   "The answers are used to set cmake cache values."},
  {"-L[A][H]", "List non-advanced cached variables.",
   "List cache variables will run CMake and list all the variables from the "
   "CMake cache that are not marked as INTERNAL or ADVANCED. This will "
   "effectively display current CMake settings, which can be then changed "
   "with -D option. Changing some of the variable may result in more "
   "variables being created. If A is specified, then it will display also "
   "advanced variables. If H is specified, it will also display help for "
   "each variable."},
  {"-N", "View mode only.",
   "Only load the cache. Do not actually run configure and generate steps."},
  {"-P <file>", "Process script mode.",
   "Process the given cmake file as a script written in the CMake language.  "
   "No configure or generate step is performed and the cache is not"
   " modified. If variables are defined using -D, this must be done "
   "before the -P argument."},
  {"--graphviz=[file]", "Generate graphviz of dependencies.",
   "Generate a graphviz input file that will contain all the library and "
   "executable dependencies in the project."},
  {"--system-information [file]", "Dump information about this system.",
   "Dump a wide range of information about the current system. If run "
   "from the top of a binary tree for a CMake project it will dump "
   "additional information such as the cache, log files etc."},
  {"--debug-trycompile", "Do not delete the try compile directories..",
   "Do not delete the files and directories created for try_compile calls. "
   "This is useful in debugging failed try_compiles."},
  {"--debug-output", "Put cmake in a debug mode.",
   "Print extra stuff during the cmake run like stack traces with "
   "message(send_error ) calls."},
  {"--help-command cmd [file]", "Print help for a single command and exit.",
   "Full documentation specific to the given command is displayed. "
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-command-list [file]", "List available listfile commands and exit.",
   "The list contains all commands for which help may be obtained by using "
   "the --help-command argument followed by a command name. "
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-commands [file]", "Print help for all commands and exit.",
   "Full documentation specific for all current command is displayed."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-compatcommands [file]", "Print help for compatibility commands. ",
   "Full documentation specific for all compatibility commands is displayed."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-module module [file]", "Print help for a single module and exit.",
   "Full documentation specific to the given module is displayed."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-module-list [file]", "List available modules and exit.",
   "The list contains all modules for which help may be obtained by using "
   "the --help-module argument followed by a module name. "
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-modules [file]", "Print help for all modules and exit.",
   "Full documentation for all modules is displayed. "
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-custom-modules [file]" , "Print help for all custom modules and "
   "exit.",
   "Full documentation for all custom modules is displayed. "
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-property prop [file]", 
   "Print help for a single property and exit.",
   "Full documentation specific to the given property is displayed."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-property-list [file]", "List available properties and exit.",
   "The list contains all properties for which help may be obtained by using "
   "the --help-property argument followed by a property name.  If a file is "
   "specified, the help is written into it."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-properties [file]", "Print help for all properties and exit.",
   "Full documentation for all properties is displayed."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-variable var [file]", 
   "Print help for a single variable and exit.",
   "Full documentation specific to the given variable is displayed."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-variable-list [file]", "List documented variables and exit.",
   "The list contains all variables for which help may be obtained by using "
   "the --help-variable argument followed by a variable name.  If a file is "
   "specified, the help is written into it."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-variables [file]", "Print help for all variables and exit.",
   "Full documentation for all variables is displayed."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationSeeAlso[][3] =
{
  {0, "ccmake", 0},
  {0, "cpack", 0},
  {0, "ctest", 0},
  {0, "cmakecommands", 0},
  {0, "cmakecompat", 0},
  {0, "cmakemodules", 0},
  {0, "cmakeprops", 0},
  {0, "cmakevars", 0},
  {0, 0, 0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationNOTE[][3] =
{
  {0,
   "CMake no longer configures a project when run with no arguments.  "
   "In order to configure the project in the current directory, run\n"
   "  cmake .", 0},
  {0,0,0}
};
#endif

int do_cmake(int ac, char** av);

static cmMakefile* cmakemainGetMakefile(void *clientdata)
{
  cmake* cm = (cmake *)clientdata;
  if(cm && cm->GetDebugOutput())
    {
    cmGlobalGenerator* gg=cm->GetGlobalGenerator();
    if (gg)
      {
      cmLocalGenerator* lg=gg->GetCurrentLocalGenerator();
      if (lg)
        {
        cmMakefile* mf = lg->GetMakefile();
        return mf;
        }
      }
    }
  return 0;
}

static std::string cmakemainGetStack(void *clientdata)
{
  std::string msg;
  cmMakefile* mf=cmakemainGetMakefile(clientdata);
  if (mf)
    {
    msg = mf->GetListFileStack();
    if (!msg.empty())
      {
      msg = "\n   Called from: " + msg;
      }
    }

  return msg;
}

static void cmakemainErrorCallback(const char* m, const char*, bool&, 
                                   void *clientdata)
{
  std::cerr << m << cmakemainGetStack(clientdata) << std::endl << std::flush;
}

static void cmakemainProgressCallback(const char *m, float prog, 
                                      void* clientdata)
{
  cmMakefile* mf = cmakemainGetMakefile(clientdata);
  std::string dir;
  if ((mf) && (strstr(m, "Configuring")==m) && (prog<0))
    {
    dir = " ";
    dir += mf->GetCurrentDirectory();
    }
  else if ((mf) && (strstr(m, "Generating")==m))
    {
    dir = " ";
    dir += mf->GetCurrentOutputDirectory();
    }

  if ((prog < 0) || (!dir.empty()))
    {
    std::cout << "-- " << m << dir << cmakemainGetStack(clientdata)<<std::endl;
    }

  std::cout.flush();
}


int main(int ac, char** av)
{
  cmSystemTools::EnableMSVCDebugHook();
  cmSystemTools::FindExecutableDirectory(av[0]);
  int ret = do_cmake(ac, av);
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmDynamicLoader::FlushCache();
#endif
  return ret;
}

int do_cmake(int ac, char** av)
{
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmDocumentation doc;
#endif
  int nocwd = 0;

  if ( cmSystemTools::GetCurrentWorkingDirectory().size() == 0 )
    {
    std::cerr << "Current working directory cannot be established." 
              << std::endl;
    nocwd = 1;
    }

#ifdef CMAKE_BUILD_WITH_CMAKE
  if(doc.CheckOptions(ac, av) || nocwd)
    { 
    // Construct and print requested documentation.
    cmake hcm;
    hcm.AddCMakePaths();
    doc.SetCMakeRoot(hcm.GetCacheDefinition("CMAKE_ROOT"));

    // the command line args are processed here so that you can do 
    // -DCMAKE_MODULE_PATH=/some/path and have this value accessible here
    std::vector<std::string> args;
    for(int i =0; i < ac; ++i)
      {
      args.push_back(av[i]);
      }
    hcm.SetCacheArgs(args);
    const char* modulePath = hcm.GetCacheDefinition("CMAKE_MODULE_PATH");
    if (modulePath)
      {
      doc.SetCMakeModulePath(modulePath);
      }

    std::vector<cmDocumentationEntry> commands;
    std::vector<cmDocumentationEntry> policies;
    std::vector<cmDocumentationEntry> compatCommands;
    std::vector<cmDocumentationEntry> generators;
    std::map<std::string,cmDocumentationSection *> propDocs;

    hcm.GetPolicyDocumentation(policies);
    hcm.GetCommandDocumentation(commands, true, false);
    hcm.GetCommandDocumentation(compatCommands, false, true);
    hcm.GetPropertiesDocumentation(propDocs);
    hcm.GetGeneratorDocumentation(generators);

    doc.SetName("cmake");
    doc.SetSection("Name",cmDocumentationName);
    doc.SetSection("Usage",cmDocumentationUsage);
    doc.SetSection("Description",cmDocumentationDescription);
    doc.AppendSection("Generators",generators);
    doc.PrependSection("Options",cmDocumentationOptions);
    doc.SetSection("Commands",commands);
    doc.SetSection("Policies",policies);
    doc.AppendSection("Compatibility Commands",compatCommands);
    doc.SetSections(propDocs);

    cmDocumentationEntry e;
    e.Brief = 
      "variables defined by cmake, that give information about the project, "
      "and cmake";
    doc.PrependSection("Variables that Provide Information",e);

    doc.SetSeeAlsoList(cmDocumentationSeeAlso);
    int result = doc.PrintRequestedDocumentation(std::cout)? 0:1;

    // If we were run with no arguments, but a CMakeLists.txt file
    // exists, the user may have been trying to use the old behavior
    // of cmake to build a project in-source.  Print a message
    // explaining the change to standard error and return an error
    // condition in case the program is running from a script.
    if((ac == 1) && cmSystemTools::FileExists("CMakeLists.txt"))
      {
      doc.ClearSections();
      doc.SetSection("NOTE", cmDocumentationNOTE);
      doc.Print(cmDocumentation::UsageForm, std::cerr);
      return 1;
      }
    return result;
    }
#else
  if ( nocwd || ac == 1 )
    {
    std::cout << 
      "Bootstrap CMake should not be used outside CMake build process."
              << std::endl;
    return 0;
    }
#endif
  
  bool wiz = false;
  bool sysinfo = false;
  bool command = false;
  bool list_cached = false;
  bool list_all_cached = false;
  bool list_help = false;
  bool view_only = false;
  bool script_mode = false;
  std::vector<std::string> args;
  for(int i =0; i < ac; ++i)
    {
    if(strcmp(av[i], "-i") == 0)
      {
      wiz = true;
      }
    else if(!command && strcmp(av[i], "--system-information") == 0)
      {
      sysinfo = true;
      }
    // if command has already been set, then
    // do not eat the -E 
    else if (!command && strcmp(av[i], "-E") == 0)
      {
      command = true;
      }
    else if (strcmp(av[i], "-N") == 0)
      {
      view_only = true;
      }
    else if (strcmp(av[i], "-L") == 0)
      {
      list_cached = true;
      }
    else if (strcmp(av[i], "-LA") == 0)
      {
      list_all_cached = true;
      }
    else if (strcmp(av[i], "-LH") == 0)
      {
      list_cached = true;
      list_help = true;
      }
    else if (strcmp(av[i], "-LAH") == 0)
      {
      list_all_cached = true;
      list_help = true;
      }
    else if (strncmp(av[i], "-P", strlen("-P")) == 0)
      {
      if ( i == ac -1 )
        {
        cmSystemTools::Error("No script specified for argument -P");
        }
      else
        {
        script_mode = true;
        args.push_back(av[i]);
        i++;
        args.push_back(av[i]);
        }
      }
    else 
      {
      args.push_back(av[i]);
      }
    }

  if(command)
    {
    int ret = cmake::ExecuteCMakeCommand(args);
    return ret;
    }
  if (wiz)
    {
    cmakewizard wizard;
    return wizard.RunWizard(args); 
    }
  if (sysinfo)
    {
    cmake cm;
    int ret = cm.GetSystemInformation(args);
    return ret; 
    }
  cmake cm;  
  cmSystemTools::SetErrorCallback(cmakemainErrorCallback, (void *)&cm);
  cm.SetProgressCallback(cmakemainProgressCallback, (void *)&cm);
  cm.SetScriptMode(script_mode);

  int res = cm.Run(args, view_only);
  if ( list_cached || list_all_cached )
    {
    cmCacheManager::CacheIterator it = 
      cm.GetCacheManager()->GetCacheIterator();
    std::cout << "-- Cache values" << std::endl;
    for ( it.Begin(); !it.IsAtEnd(); it.Next() )
      {
      cmCacheManager::CacheEntryType t = it.GetType();
      if ( t != cmCacheManager::INTERNAL && t != cmCacheManager::STATIC &&
        t != cmCacheManager::UNINITIALIZED )
        {
        bool advanced = it.PropertyExists("ADVANCED");
        if ( list_all_cached || !advanced)
          {
          if ( list_help )
            {
            std::cout << "// " << it.GetProperty("HELPSTRING") << std::endl;
            }
          std::cout << it.GetName() << ":" << 
            cmCacheManager::TypeToString(it.GetType()) 
            << "=" << it.GetValue() << std::endl;
          if ( list_help )
            {
            std::cout << std::endl;
            }
          }
        }
      }
    }

  // Always return a non-negative value.  Windows tools do not always
  // interpret negative return values as errors.
  if(res != 0)
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

