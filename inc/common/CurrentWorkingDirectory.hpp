/****************************************************************************

    File: CurrentWorkingDirectory.hpp
    Author: Andrew Janke
    License: GPLv3

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*****************************************************************************/

#pragma once

#include <string>

/** @note Implementation is platform specific
 *  @return Process's current working directory.
 */
std::string get_current_working_directory();

/** @note Implementation is platform specific
 *  @param path Set to process's current working directory.
 */
void get_current_working_directory(std::string & path);

/** Attempts to set the process's currect working directory.
 *  @note This function is thread safe (using one mutex for this global
 *        resource)
 *  @param path relative/absolute new current working directory
 *  @return Returns true if the currect working directory was successfully
 *          changed.
 */
bool set_current_working_directory(const std::string & path);

/** @copydoc bool set_current_working_directory(const std::string&) */
bool set_current_working_directory(const char * path);

/** Makes a quick platform specific check on whether a path is given as an
 *  absolute path.
 *  @note   Does not check if the path exist or not!
 *  @param  path Path to test.
 *  @return Returns true if absolute path, false otherwise.
 */
bool is_absolute_file_path(const std::string & path);

/** @copydoc bool file_path_is_absolute(const std::string&) */
bool is_absolute_file_path(const char * path);

/** Automatic directory changing class. This RAII object will change the
 *  process' working directory until this object is destoryed.
 */
class DirectoryChangerRaii {
public:
    DirectoryChangerRaii(const std::string & path);
    DirectoryChangerRaii(const char * path);
    ~DirectoryChangerRaii();

private:
    std::string m_old_directory;
};
