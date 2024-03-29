/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmDependsJavaParserHelper.h,v $
  Language:  C++
  Date:      $Date: 2006/05/10 19:01:22 $
  Version:   $Revision: 1.3 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmDependsJavaParserHelper_h 
#define cmDependsJavaParserHelper_h

#include "cmStandardIncludes.h"

#define YYSTYPE cmDependsJavaParserHelper::ParserType
#define YYSTYPE_IS_DECLARED
#define YY_EXTRA_TYPE cmDependsJavaParserHelper*
#define YY_DECL int cmDependsJava_yylex(YYSTYPE* yylvalp, yyscan_t yyscanner)

/** \class cmDependsJavaParserHelper
 * \brief Helper class for parsing java source files
 *
 * Finds dependencies for java file and list of outputs
 */

class cmDependsJavaParserHelper
{
public:
  typedef struct {
    char* str;
  } ParserType;

  cmDependsJavaParserHelper();
  ~cmDependsJavaParserHelper();

  int ParseString(const char* str, int verb);
  int ParseFile(const char* file);

  // For the lexer:
  void AllocateParserType(cmDependsJavaParserHelper::ParserType* pt, 
    const char* str, int len = 0);

  int LexInput(char* buf, int maxlen);
  void Error(const char* str);

  // For yacc
  void AddClassFound(const char* sclass);
  void PrepareElement(ParserType* opt);
  void DeallocateParserType(char** pt);
  void CheckEmpty(int line, int cnt, ParserType* pt);
  void StartClass(const char* cls);
  void EndClass();
  void AddPackagesImport(const char* sclass);
  void SetCurrentPackage(const char* pkg) { this->CurrentPackage = pkg; }
  const char* GetCurrentPackage() { return this->CurrentPackage.c_str(); }
  void SetCurrentCombine(const char* cmb) { this->CurrentCombine = cmb; }
  const char* GetCurrentCombine() { return this->CurrentCombine.c_str(); }
  void UpdateCombine(const char* str1, const char* str2);

  std::vector<cmStdString>& GetClassesFound() { return this->ClassesFound; }

  std::vector<cmStdString> GetFilesProduced();

private:
  class CurrentClass
    {
  public:
    cmStdString Name;
    std::vector<CurrentClass> NestedClasses;
    CurrentClass() {}
    void AddFileNamesForPrinting(std::vector<cmStdString> *files, 
                                 const char* prefix, const char* sep);
    };
  cmStdString CurrentPackage;
  cmStdString::size_type InputBufferPos;
  cmStdString InputBuffer;
  std::vector<char> OutputBuffer;
  std::vector<cmStdString> ClassesFound;
  std::vector<cmStdString> PackagesImport;
  cmStdString CurrentCombine;

  std::vector<CurrentClass> ClassStack;

  int CurrentLine;
  int UnionsAvailable;
  int LastClassId;
  int CurrentDepth;
  int Verbose;

  std::vector<char*> Allocates;

  void PrintClasses();

  void Print(const char* place, const char* str);
  void CombineUnions(char** out, const char* in1, char** in2, 
                     const char* sep);
  void SafePrintMissing(const char* str, int line, int cnt);

  void CleanupParser();
};

#endif

