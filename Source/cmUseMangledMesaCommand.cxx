/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmUseMangledMesaCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2008/01/23 15:27:59 $
  Version:   $Revision: 1.18 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmUseMangledMesaCommand.h"
#include "cmSystemTools.h"

#include <cmsys/RegularExpression.hxx>

// cmUseMangledMesaCommand
bool cmUseMangledMesaCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{ 
  // expected two arguments:
  // arguement one: the full path to gl_mangle.h
  // arguement two : directory for output of edited headers
  if(args.size() != 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  const char* inputDir = args[0].c_str();
  std::string glh = inputDir;
  glh += "/";
  glh += "gl.h";
  if(!cmSystemTools::FileExists(glh.c_str()))
    {
    std::string e = "Bad path to Mesa, could not find: ";
    e += glh;
    e += " ";
    this->SetError(e.c_str());
    return false;
    }
  const char* destDir = args[1].c_str();
  std::vector<std::string> files;
  cmSystemTools::Glob(inputDir, "\\.h$", files);
  if(files.size() == 0)
    {
    cmSystemTools::Error("Could not open Mesa Directory ", inputDir);
    return false;
    }
  cmSystemTools::MakeDirectory(destDir);
  for(std::vector<std::string>::iterator i = files.begin();
      i != files.end(); ++i)
    {
    std::string path = inputDir;
    path += "/";
    path += *i;
    this->CopyAndFullPathMesaHeader(path.c_str(), destDir);
    }
  
  return true;
}

void 
cmUseMangledMesaCommand::
CopyAndFullPathMesaHeader(const char* source,
                          const char* outdir)
{
  std::string dir, file;
  cmSystemTools::SplitProgramPath(source, dir, file);
  std::string outFile = outdir;
  outFile += "/";
  outFile += file;
  std::string tempOutputFile = outFile;
  tempOutputFile += ".tmp";
  std::ofstream fout(tempOutputFile.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Could not open file for write in copy operation: ", 
                         tempOutputFile.c_str(), outdir);
    cmSystemTools::ReportLastSystemError("");
    return;
    }
  std::ifstream fin(source);
  if(!fin)
    {
    cmSystemTools::Error("Could not open file for read in copy operation",
                         source);
    return;
    }
  // now copy input to output and expand variables in the
  // input file at the same time
  std::string inLine;  
  // regular expression for any #include line
  cmsys::RegularExpression includeLine(
    "^[ \t]*#[ \t]*include[ \t]*[<\"]([^\">]+)[\">]");
  // regular expression for gl/ or GL/ in a file (match(1) of above)
  cmsys::RegularExpression glDirLine("(gl|GL)(/|\\\\)([^<\"]+)");
  // regular expression for gl GL or xmesa in a file (match(1) of above)
  cmsys::RegularExpression glLine("(gl|GL|xmesa)");
  while(cmSystemTools::GetLineFromStream(fin,inLine))
    {
    if(includeLine.find(inLine.c_str()))
      {
      std::string includeFile = includeLine.match(1);
      if(glDirLine.find(includeFile.c_str()))
        {
        std::string gfile = glDirLine.match(3);
        fout << "#include \"" << outdir << "/" << gfile.c_str() << "\"\n";
        }
      else if(glLine.find(includeFile.c_str()))
        {
        fout << "#include \"" << outdir << "/" << 
          includeLine.match(1).c_str() << "\"\n";
        }
      else
        {
        fout << inLine << "\n";
        }
      }
    else
      {
      fout << inLine << "\n";
      }
    }
  // close the files before attempting to copy
  fin.close();
  fout.close();
  cmSystemTools::CopyFileIfDifferent(tempOutputFile.c_str(),
                                     outFile.c_str());
  cmSystemTools::RemoveFile(tempOutputFile.c_str());
}

