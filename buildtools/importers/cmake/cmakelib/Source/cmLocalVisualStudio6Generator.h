/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmLocalVisualStudio6Generator.h,v $
  Language:  C++
  Date:      $Date: 2006/04/19 14:34:41 $
  Version:   $Revision: 1.14 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmLocalVisualStudio6Generator_h
#define cmLocalVisualStudio6Generator_h

#include "cmLocalGenerator.h"

class cmMakeDepend;
class cmTarget;
class cmSourceFile;
class cmSourceGroup;
class cmCustomCommand;

/** \class cmLocalVisualStudio6Generator
 * \brief Write a LocalUnix makefiles.
 *
 * cmLocalVisualStudio6Generator produces a LocalUnix makefile from its
 * member this->Makefile.
 */
class cmLocalVisualStudio6Generator : public cmLocalGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmLocalVisualStudio6Generator();

  virtual ~cmLocalVisualStudio6Generator();
  
  /**
   * Generate the makefile for this directory. 
   */
  virtual void Generate();

  void OutputDSPFile();

  enum BuildType {STATIC_LIBRARY, DLL, EXECUTABLE, WIN32_EXECUTABLE, UTILITY};

  /**
   * Specify the type of the build: static, dll, or executable.
   */
  void SetBuildType(BuildType, const char* libName, cmTarget&);

  /**
   * Return array of created DSP names in a STL vector.
   * Each executable must have its own dsp.
   */
  std::vector<std::string> GetCreatedProjectNames() 
    {
    return this->CreatedProjectNames;
    }

private:
  std::string DSPHeaderTemplate;
  std::string DSPFooterTemplate;
  std::vector<std::string> CreatedProjectNames;
  
  void CreateSingleDSP(const char *lname, cmTarget &tgt);
  void WriteDSPFile(std::ostream& fout, const char *libName, 
                    cmTarget &tgt);
  void WriteDSPBeginGroup(std::ostream& fout, 
                          const char* group,
                          const char* filter);
  void WriteDSPEndGroup(std::ostream& fout);

  void WriteDSPHeader(std::ostream& fout, const char *libName,
                      cmTarget &tgt, std::vector<cmSourceGroup> &sgs);

  void WriteDSPFooter(std::ostream& fout);
  void AddDSPBuildRule(cmTarget& tgt);
  void WriteCustomRule(std::ostream& fout,
                       const char* source,
                       const char* command,
                       const char* comment,
                       const std::vector<std::string>& depends,
                       const std::vector<std::string>& outputs,
                       const char* flags);
  void AddUtilityCommandHack(cmTarget& target, int count,
                             std::vector<std::string>& depends,
                             const cmCustomCommand& origCommand);
  void WriteGroup(const cmSourceGroup *sg, cmTarget target,
                  std::ostream &fout, const char *libName);
  std::string CreateTargetRules(cmTarget &target, 
                                const char *libName);
  void ComputeLinkOptions(cmTarget& target, const char* configName,
                          const std::string extraOptions,
                          std::string& options);
  std::string IncludeOptions;
  std::vector<std::string> Configurations;
};

#endif

