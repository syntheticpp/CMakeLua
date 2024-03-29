/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmCPackLog.cxx,v $
  Language:  C++
  Date:      $Date: 2006/03/10 18:06:26 $
  Version:   $Revision: 1.7 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "cmCPackLog.h"

#include "cmGeneratedFileStream.h"

//----------------------------------------------------------------------
cmCPackLog::cmCPackLog()
{
  this->Verbose = false;
  this->Debug = false;
  this->Quiet = false;
  this->NewLine = true;

  this->LastTag = cmCPackLog::NOTAG;
#undef cerr
#undef cout
  this->DefaultOutput = &std::cout;
  this->DefaultError = &std::cerr;

  this->LogOutput = 0;
  this->LogOutputCleanup = false;
}

//----------------------------------------------------------------------
cmCPackLog::~cmCPackLog()
{
  this->SetLogOutputStream(0);
}

//----------------------------------------------------------------------
void cmCPackLog::SetLogOutputStream(std::ostream* os)
{
  if ( this->LogOutputCleanup && this->LogOutput )
    {
    delete this->LogOutput;
    }
  this->LogOutputCleanup = false;
  this->LogOutput = os;
}

//----------------------------------------------------------------------
bool cmCPackLog::SetLogOutputFile(const char* fname)
{
  cmGeneratedFileStream *cg = 0;
  if ( fname )
    {
    cg = new cmGeneratedFileStream(fname);
    }
  if ( cg && !*cg )
    {
    delete cg;
    cg = 0;
    }
  this->SetLogOutputStream(cg);
  if ( !cg )
    {
    return false;
    }
  this->LogOutputCleanup = true;
  return true;
}

//----------------------------------------------------------------------
void cmCPackLog::Log(int tag, const char* file, int line,
  const char* msg, size_t length)
{
  // By default no logging
  bool display = false;

  // Display file and line number if debug
  bool useFileAndLine = this->Debug;

  bool output  = false;
  bool debug   = false;
  bool warning = false;
  bool error   = false;
  bool verbose = false;

  // When writing in file, add list of tags whenever tag changes.
  std::string tagString;
  bool needTagString = false;
  if ( this->LogOutput && this->LastTag != tag )
    {
    needTagString = true;
    }

  if ( tag & LOG_OUTPUT )
    {
    output = true;
    display = true;
    if ( needTagString )
      {
      if ( tagString.size() > 0 ) { tagString += ","; }
      tagString = "VERBOSE";
      }
    }
  if ( tag & LOG_WARNING )
    {
    warning = true;
    display = true;
    if ( needTagString )
      {
      if ( tagString.size() > 0 ) { tagString += ","; }
      tagString = "WARNING";
      }
    }
  if ( tag & LOG_ERROR )
    {
    error = true;
    display = true;
    if ( needTagString )
      {
      if ( tagString.size() > 0 ) { tagString += ","; }
      tagString = "ERROR";
      }
    }
  if ( tag & LOG_DEBUG && this->Debug )
    {
    debug = true;
    display = true;
    if ( needTagString )
      {
      if ( tagString.size() > 0 ) { tagString += ","; }
      tagString = "DEBUG";
      }
    useFileAndLine = true;
    }
  if ( tag & LOG_VERBOSE && this->Verbose )
    {
    verbose = true;
    display = true;
    if ( needTagString )
      {
      if ( tagString.size() > 0 ) { tagString += ","; }
      tagString = "VERBOSE";
      }
    }
  if ( this->Quiet )
    {
    display = false;
    }
  if ( this->LogOutput )
    {
    if ( needTagString )
      {
      *this->LogOutput << "[" << file << ":" << line << " "
        << tagString << "] ";
      }
    this->LogOutput->write(msg, length);
    }
  this->LastTag = tag;
  if ( !display )
    {
    return;
    }
  if ( this->NewLine )
    {
    if ( error && !this->ErrorPrefix.empty() )
      {
      *this->DefaultError << this->ErrorPrefix.c_str();
      }
    else if ( warning && !this->WarningPrefix.empty() )
      {
      *this->DefaultError << this->WarningPrefix.c_str();
      }
    else if ( output && !this->OutputPrefix.empty() )
      {
      *this->DefaultOutput << this->OutputPrefix.c_str();
      }
    else if ( verbose && !this->VerbosePrefix.empty() )
      {
      *this->DefaultOutput << this->VerbosePrefix.c_str();
      }
    else if ( debug && !this->DebugPrefix.empty() )
      {
      *this->DefaultOutput << this->DebugPrefix.c_str();
      }
    else if ( !this->Prefix.empty() )
      {
      *this->DefaultOutput << this->Prefix.c_str();
      }
    if ( useFileAndLine )
      {
      if ( error || warning )
        {
        *this->DefaultError << file << ":" << line << " ";
        }
      else
        {
        *this->DefaultOutput << file << ":" << line << " ";
        }
      }
    }
  if ( error || warning )
    {
    this->DefaultError->write(msg, length);
    this->DefaultError->flush();
    }
  else
    {
    this->DefaultOutput->write(msg, length);
    this->DefaultOutput->flush();
    }
  if ( msg[length-1] == '\n' || length > 2 )
    {
    this->NewLine = true;
    }
}
