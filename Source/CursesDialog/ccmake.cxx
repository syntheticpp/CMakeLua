/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: ccmake.cxx,v $
  Language:  C++
  Date:      $Date: 2007/12/13 22:56:50 $
  Version:   $Revision: 1.37 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "../cmCacheManager.h"
#include "../cmSystemTools.h"
#include "../cmake.h"
#include "../cmDocumentation.h"

#include <signal.h>
#include <sys/ioctl.h>

#include "cmCursesMainForm.h"
#include "cmCursesStandardIncludes.h"

#include <form.h>

//----------------------------------------------------------------------------
static const char * cmDocumentationName[][3] =
{
  {0,
   "  ccmake - Curses Interface for CMake.", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationUsage[][3] =
{
  {0,
   "  ccmake <path-to-source>\n"
   "  ccmake <path-to-existing-build>", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationDescription[][3] =
{
  {0,
   "The \"ccmake\" executable is the CMake curses interface.  Project "
   "configuration settings may be specified interactively through "
   "this GUI.  Brief instructions are provided at the bottom of the "
   "terminal when the program is running.", 0},
  CMAKE_STANDARD_INTRODUCTION,
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationOptions[][3] =
{
  CMAKE_STANDARD_OPTIONS_TABLE,
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationSeeAlso[][3] =
{
  {0, "cmake", 0},
  {0, "ctest", 0},
  {0, 0, 0}
};

cmCursesForm* cmCursesForm::CurrentForm=0;

extern "C"
{

void onsig(int)
{
  if (cmCursesForm::CurrentForm)
    {
    endwin();
    initscr(); /* Initialization */ 
    noecho(); /* Echo off */ 
    cbreak(); /* nl- or cr not needed */ 
    keypad(stdscr,TRUE); /* Use key symbols as 
                            KEY_DOWN*/ 
    refresh();
    int x,y;
    getmaxyx(stdscr, y, x);
    cmCursesForm::CurrentForm->Render(1,1,x,y);
    cmCursesForm::CurrentForm->UpdateStatusBar();
    }
  signal(SIGWINCH, onsig);
}
 
}

void CMakeErrorHandler(const char* message, const char* title, bool&, void* clientData)
{
  cmCursesForm* self = static_cast<cmCursesForm*>( clientData );
  self->AddError(message, title);
}

int main(int argc, char** argv)
{
  cmSystemTools::FindExecutableDirectory(argv[0]);
  cmDocumentation doc;
  if(doc.CheckOptions(argc, argv))
    {
    cmake hcm;
    std::vector<cmDocumentationEntry> commands;
    std::vector<cmDocumentationEntry> compatCommands;
    std::vector<cmDocumentationEntry> generators;
    hcm.GetCommandDocumentation(commands, true, false);
    hcm.GetCommandDocumentation(compatCommands, false, true);
    hcm.GetGeneratorDocumentation(generators);
    doc.SetName("ccmake");
    doc.SetSection("Name",cmDocumentationName);
    doc.SetSection("Usage",cmDocumentationUsage);
    doc.SetSection("Description",cmDocumentationDescription);
    doc.SetSection("Generators",generators);
    doc.SetSection("Options",cmDocumentationOptions);
    doc.SetSection("Command",commands);
    doc.SetSection("Compatibility Commands",compatCommands);
    doc.SetSeeAlsoList(cmDocumentationSeeAlso);
    return doc.PrintRequestedDocumentation(std::cout)? 0:1;
    }  
  
  bool debug = false;
  unsigned int i;
  int j;
  std::vector<std::string> args;
  for(j =0; j < argc; ++j)
    {
    if(strcmp(argv[j], "-debug") == 0)
      {
      debug = true;
      }
    else
      {
      args.push_back(argv[j]);
      }
    }

  std::string cacheDir = cmSystemTools::GetCurrentWorkingDirectory();
  for(i=1; i < args.size(); ++i)
    {
    std::string arg = args[i];
    if(arg.find("-B",0) == 0)
      {
      cacheDir = arg.substr(2);
      }
    }

  cmSystemTools::DisableRunCommandOutput();

  if (debug)
    {
    cmCursesForm::DebugStart();
    }

  initscr(); /* Initialization */ 
  noecho(); /* Echo off */ 
  cbreak(); /* nl- or cr not needed */ 
  keypad(stdscr,TRUE); /* Use key symbols as 
                          KEY_DOWN*/ 

  signal(SIGWINCH, onsig);

  int x,y;
  getmaxyx(stdscr, y, x);
  if ( x < cmCursesMainForm::MIN_WIDTH  || 
       y < cmCursesMainForm::MIN_HEIGHT )
    {
    endwin();
    std::cerr << "Window is too small. A size of at least "
              << cmCursesMainForm::MIN_WIDTH << " x " 
              <<  cmCursesMainForm::MIN_HEIGHT
              << " is required to run ccmake." <<  std::endl;
    return 1;
    }


  cmCursesMainForm* myform;

  myform = new cmCursesMainForm(args, x);
  if(myform->LoadCache(cacheDir.c_str()))
    {
    curses_clear();
    touchwin(stdscr);
    endwin();
    delete myform;
    std::cerr << "Error running cmake::LoadCache().  Aborting.\n";
    return 1;
    }

  cmSystemTools::SetErrorCallback(CMakeErrorHandler, myform);

  cmCursesForm::CurrentForm = myform;

  myform->InitializeUI();
  if ( myform->Configure(1) == 0 )
    {
    myform->Render(1, 1, x, y);
    myform->HandleInput();
    }
  
  // Need to clean-up better
  curses_clear();
  touchwin(stdscr);
  endwin();
  delete cmCursesForm::CurrentForm;
  cmCursesForm::CurrentForm = 0;

  std::cout << std::endl << std::endl;
  
  return 0;

}
