/****************************************************************************

    File: CurrentWorkingDirectory.cpp
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

#include <common/CurrentWorkingDirectory.hpp>
#include <common/Util.hpp>
#include <common/StringUtil.hpp>

#if defined(MACRO_PLATFORM_LINUX)
#   include <linux/limits.h>
#   include <unistd.h>
#elif defined(MACRO_PLATFORM_WINDOWS)
#	include <windows.h>
#endif

#include <mutex>
#include <stdexcept>
#include <cassert>

#if !defined(MACRO_PLATFORM_LINUX) && !defined (MACRO_PLATFORM_WINDOWS)
#   error("Platform Macros are either unspecified, or the platform is not supported.")
#endif

namespace {
    using Error = std::runtime_error;
    void trim_to_parent_path(std::string & path) noexcept;
#ifdef MACRO_PLATFORM_WINDOWS
    int find_null_terminator(const std::string & str);
#endif
} // end of <anonymous> namespace

void get_current_working_directory(std::string & rv) {
    rv.clear();
#   if defined(MACRO_PLATFORM_LINUX)
    //
    //                       LINUX IMPLEMENTATION
    //
    rv.resize(PATH_MAX, '\0');
    // getcwd is thread-safe (by POSIX)
    while (!::getcwd(&rv.front(), rv.size())) {
        rv.resize(rv.size()*2, '\0');
    }

    if (rv.find("(unreachable)") != std::string::npos) {
        throw Error("Current working directory is unreachable.");
    }

    rv.erase(rv.begin() + unsigned(rv.find('\0')), rv.end());
#   elif defined(MACRO_PLATFORM_WINDOWS)
    //
    //                       WINDOWS IMPLEMENTATION
    //
    rv.resize(MAX_PATH, '\0');
    GetCurrentDirectoryA((DWORD)rv.size(), &rv.front());
	// there seems to be a bug in msvc's STL string library, I'll have to code
	// my own as a temporary work around
    rv.erase(rv.begin() + find_null_terminator(rv), rv.end());
#   endif
}

bool set_current_working_directory(const char * path) {
    const int SUCCESS = 1;
    const int FAILURE = 0;
    const int UNINIT  = -1;
    int rv = UNINIT;

#	if defined (MACRO_PLATFORM_LINUX)
    //
    //                  LINUX IMPLEMENTATION
    //
    // we cannot be sure that chdir is thread safe, therefore mutex
    static std::mutex cwd_change_mutex;
    std::lock_guard<std::mutex> lock(cwd_change_mutex); (void)lock;
    rv = (::chdir(path) == 0) ? SUCCESS : FAILURE;
#	elif defined (MACRO_PLATFORM_WINDOWS)
    rv = (SetCurrentDirectoryA(path) == TRUE) ? SUCCESS : FAILURE;
#	endif

    assert(rv != UNINIT);
    return rv == SUCCESS;
}

bool is_absolute_file_path(const char * path) {
#	if defined (MACRO_PLATFORM_LINUX)
    return path[0] == '/';
#	elif defined (MACRO_PLATFORM_WINDOWS)
    if (path[0] == '\0') return false;
    return path[1] == ':';
#	endif

}

std::string get_current_working_directory() {
    std::string rv;
    get_current_working_directory(rv);
    return rv;
}

bool set_current_working_directory(const std::string & path)
    { return set_current_working_directory(path.c_str()); }

bool is_absolute_file_path(const std::string & path)
    { return is_absolute_file_path(path.c_str()); }

DirectoryChangerRaii::DirectoryChangerRaii(const std::string & path):
    DirectoryChangerRaii(path.c_str())
{}

DirectoryChangerRaii::DirectoryChangerRaii(const char * path):
    m_old_directory(util::get_current_working_directory())
{
    std::string filepath = path;
    trim_to_parent_path(filepath);
    // make absolute, if not already
    if (!util::is_absolute_file_path(filepath))
        filepath = m_old_directory + '/' + filepath;
    if (!util::set_current_working_directory(filepath)) {
        throw Error("Failed to change working directory to: \"" +
                    filepath + "\".");
    }
}

DirectoryChangerRaii::~DirectoryChangerRaii() {
    if (!util::set_current_working_directory(m_old_directory)) {
        util::message_assert("Failed to change directory back (programming error.)", false);
    }
}

namespace {

#ifdef MACRO_PLATFORM_WINDOWS
int find_null_terminator(const std::string & str) {
	const char * c = str.data();
	while (*c) ++c;
	return int(c - str.data());
}
#endif

void trim_to_parent_path(std::string & path) noexcept {
    std::size_t selection;

    {
    std::size_t rslash = path.rfind('\\');
    std::size_t fslash = path.rfind('/' );

    if (rslash == std::string::npos) {
        selection = fslash;
    } else if (fslash == std::string::npos) {
        selection = rslash;
    } else {
        selection = std::max(rslash, fslash);
    }
    }

    if (selection == std::string::npos) {
        path.clear();
    } else {
        path.erase(path.begin() + std::string::difference_type(selection), path.end());
    }
}

} // end of <anonymous> namespace
