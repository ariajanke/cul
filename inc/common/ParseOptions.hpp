/****************************************************************************

    File: ParseOptions.hpp
    Author: Aria Janke
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
#include <initializer_list>

template <typename T>
using ParseFunc = void(*)(T &, char ** beg, char ** end);

template <typename T>
struct OptionTableEntry {
    const char * longname;
    char abbr;
    ParseFunc<T> parser;
};

template <typename T>
using OptionsTable = std::initializer_list<OptionTableEntry<T>>;

/** Parses program options and their arguments to a user specified type.
 *  @tparam OptionsType 
 *  @param  argc
 *  @param  argv
 */
template <typename OptionsType>
OptionsType parse_options
	(int argc, char ** argv, OptionsTable<OptionsType>);

class ParseOptionsPriv {
	template <typename OptionsType>
	friend OptionsType parse_options
		(int argc, char ** argv, OptionsTable<OptionsType> options_table);
		
	static bool is_same(const char * a, const char * b);

	enum { k_is_long, k_is_short, k_is_not_option };

	static decltype(k_is_long) detect_option_type(const char * opt);
	
	template <typename OptionsType>
	static ParseFunc<OptionsType> do_inbetweens
		(OptionsType * options, const char * opt, 
		 OptionsTable<OptionsType> options_table);

	template <typename OptionsType>
	static ParseFunc<OptionsType> get_long_option
		(const char * opt, OptionsTable<OptionsType> options_table);
};

template <typename OptionsType>
OptionsType parse_options
    (int argc, char ** argv, OptionsTable<OptionsType> options_table)
{
	// generally this follows a "wait until we get a whole list of 
	// arguments before executing the option"
	using PFunc = ParseFunc<OptionsType>;
	using Pop = ParseOptionsPriv;
    
    OptionsType rv;
    PFunc last_parser = nullptr;
    decltype(argv) last = nullptr;
    auto end = argv + argc;
    for (; argv != end; ++argv) {
        if (!last) last = argv;
        
        PFunc sel_f = nullptr;
        auto opt_type = Pop::detect_option_type(*argv);
        switch (opt_type) {
        case Pop::k_is_long:
			sel_f = Pop::get_long_option(*argv + 2, options_table);
            break;
        case Pop::k_is_short:
			sel_f = Pop::do_inbetweens(&rv, *argv + 1, options_table);
            break;
        case Pop::k_is_not_option: break;
        }
        
        // if we found an option, process the last one
        if (sel_f || opt_type != Pop::k_is_not_option) {
            if (last_parser) {
                last_parser(rv, last, last ? argv : nullptr);
            }
            
			last = nullptr;		
            last_parser = sel_f;
        }
    }
    // process any remaining option
    if (last_parser) {
		last_parser(rv, last, last ? end : nullptr);
	}
    return rv;
}

/* static */ inline bool ParseOptionsPriv::is_same(const char * a, const char * b) {
	while (*a && *b) {
		if (*a++ != *b++) return false;
	}
	return *a == *b;
}

/* static */ inline decltype(ParseOptionsPriv::k_is_long) ParseOptionsPriv::
	detect_option_type(const char * opt)
{
	if (opt[0] == '-') {
		return (opt[1] == '-') ? k_is_long : k_is_short;
	}
	return k_is_not_option;
}

template <typename OptionsType>
/* static */ ParseFunc<OptionsType> ParseOptionsPriv::do_inbetweens
	(OptionsType * options, const char * opt, 
	 OptionsTable<OptionsType> options_table)
{
	ParseFunc<OptionsType> last_f = nullptr;
	for (; *opt; ++opt) {
		for (const auto & entry : options_table) {
			if (entry.abbr == *opt) {
				if (last_f) last_f(*options, nullptr, nullptr);
				last_f = entry.parser;
				break;
			}
		}
	}
	return last_f;
}

template <typename OptionsType>
/* static */ ParseFunc<OptionsType> ParseOptionsPriv::get_long_option
	(const char * opt, OptionsTable<OptionsType> options_table)
{
	for (const auto & entry : options_table) {
		if (is_same(entry.longname, opt)) {
			return entry.parser;
		}
	}
	return nullptr;
}
