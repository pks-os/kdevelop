/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmIncludeExternalMSProjectCommand.h,v $
  Language:  C++
  Date:      $Date: 2006/05/11 19:50:11 $
  Version:   $Revision: 1.6 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmIncludeExternalMSProjectCommand_h
#define cmIncludeExternalMSProjectCommand_h

#include "cmCommand.h"

/** \class cmIncludeExternalMSProjectCommand
 * \brief Specify an external MS project file for inclusion in the workspace.
 *
 * cmIncludeExternalMSProjectCommand is used to specify an externally
 * generated Microsoft project file for inclusion in the default workspace
 * generated by CMake.
 */
class cmIncludeExternalMSProjectCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmIncludeExternalMSProjectCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "INCLUDE_EXTERNAL_MSPROJECT";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Include an external Microsoft project file in a workspace.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  INCLUDE_EXTERNAL_MSPROJECT(projectname location\n"
      "                             dep1 dep2 ...)\n"
      "Includes an external Microsoft project in the generated workspace "
      "file.  Currently does nothing on UNIX.";
    }
  
  cmTypeMacro(cmIncludeExternalMSProjectCommand, cmCommand);
};



#endif
