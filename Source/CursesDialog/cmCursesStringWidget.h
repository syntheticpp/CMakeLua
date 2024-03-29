/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmCursesStringWidget.h,v $
  Language:  C++
  Date:      $Date: 2006/03/16 15:44:55 $
  Version:   $Revision: 1.6 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __cmCursesStringWidget_h
#define __cmCursesStringWidget_h

#include "cmCursesWidget.h"

class cmCursesMainForm;

/** \class cmCursesStringWidget
 * \brief A simple entry widget.
 *
 * cmCursesStringWdiget is a simple text entry widget.
 */

class cmCursesStringWidget : public cmCursesWidget
{
public:
  cmCursesStringWidget(int width, int height, int left, int top);
  
  /**
   * Handle user input. Called by the container of this widget
   * when this widget has focus. Returns true if the input was
   * handled.
   */
  virtual bool HandleInput(int& key, cmCursesMainForm* fm, WINDOW* w);

  /**
   * Set/Get the string.
   */
  void SetString(const char* value);
  const char* GetString();
  virtual const char* GetValue();

  /**
   * Set/Get InEdit flag. Can be used to tell the widget to leave
   * edit mode (in case of a resize for example).
   */
  void SetInEdit(bool inedit) { this->InEdit = inedit; }
  bool GetInEdit() { return this->InEdit; }

  /**
   * This method is called when different keys are pressed. The
   * subclass can have a special implementation handler for this.
   */
  virtual void OnTab(cmCursesMainForm* fm, WINDOW* w);
  virtual void OnReturn(cmCursesMainForm* fm, WINDOW* w);
  virtual void OnType(int& key, cmCursesMainForm* fm, WINDOW* w);

  /**
   * If there are any, print the widget specific commands
   * in the toolbar and return true. Otherwise, return false
   * and the parent widget will print.
   */
  virtual bool PrintKeys();

protected:
  cmCursesStringWidget(const cmCursesStringWidget& from);
  void operator=(const cmCursesStringWidget&);

  // true if the widget is in edit mode
  bool InEdit;
  char* OriginalString;
  bool Done;
};

#endif // __cmCursesStringWidget_h
