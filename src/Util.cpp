#include <common/Util.hpp>
#include <common/CurrentWorkingDirectory.hpp>
#include <common/StringUtil.hpp>

#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>

#include <cstring>
#include <cassert>
#include <cmath>

namespace {

using Error = std::runtime_error;

bool is_dir_slash(char c) { return c == '\\' || c == '/'; }

} // end of <anonymous> namespace
	
float  square_root(float  x) { return std::sqrt(x); }
double square_root(double x) { return std::sqrt(x); }
int    square_root(int    x) { return std::sqrt(x); }

float  sine(float  x) { return std::sin(x); }
double sine(double x) { return std::sin(x); }

float  cosine(float  x) { return std::cos(x); }
double cosine(double x) { return std::cos(x); }


float  arc_cosine(float  x) { return std::acos(x); }
double arc_cosine(double x) { return std::acos(x); }

void message_assert(const char * msg, bool cond) {
    if (cond) return;
	std::cerr << msg << std::endl;
	std::terminate();
}

void fix_path
    (const std::string & referee, const std::string & referer,
     std::string & dest_path)
{
    // parameter checking
    bool is_path = false;
    for (char c : referer) {
        if (is_dir_slash(c)) {
            is_path = true;
            break;
        }
    }

    if (!is_path) {
        dest_path = referee;
        return;
    }

    // ex: referee "./path/data.txt", referer: "~/cat/dog.xml"
    // out: "~/cat/path/data.txt"
    auto itr = referer.end();
    while (!is_dir_slash(*--itr)) {}
    if (itr == referer.begin()) return;
    dest_path = referee;
    dest_path.insert(dest_path.begin(), referer.begin(), itr + 1);
}

std::string load_file_contents_to_string(const char * filename) {
    std::ifstream fin;
    fin.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fin.open(filename);
    std::stringstream sstrm;
    sstrm << fin.rdbuf();
    fin.close();
    return sstrm.str();
}
