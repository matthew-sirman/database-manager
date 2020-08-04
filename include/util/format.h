//
// Created by matthew on 06/07/2020.
//

#ifndef UTIL_FORMAT_H
#define UTIL_FORMAT_H

#include <string>
#include <stdexcept>

template<typename...Args>
std::string format(std::string fmt, const Args&... args);

template<typename T>
std::string __get_format_arg(unsigned index, const T &t) {
	if (index != 0) {
		throw std::out_of_range("Format index out of range");
	}
	std::stringstream ss;
	ss << t;
	return ss.str();
}

template<typename T, typename ...Args>
std::string __get_format_arg(unsigned index, const T &t, const Args &... args) {
	if (index == 0) {
		std::stringstream ss;
		ss << t;
		return ss.str();
	}
	return __get_format_arg(index - 1, args...);
}

template<typename ...Args>
inline std::string format(std::string fmt, const Args & ...args) {
	std::stringstream ss;
	size_t pos = 0;
	unsigned arg = 0;

	while ((pos = fmt.find("}}", pos)) < fmt.size()) {
		fmt.erase(pos, 1);
	}

	while ((pos = fmt.find('{')) != -1) {
		if (fmt.at(pos + 1) == '{') {
			ss << fmt.substr(0, ++pos);
			fmt.erase(0, pos + 1);
		} else {
			ss << fmt.substr(0, pos);
			fmt.erase(0, pos + 1);
			pos = fmt.find('}');
			std::string arg_spec = fmt.substr(0, pos);
			if (arg_spec.empty()) {
				ss << __get_format_arg(arg++, args...);
			} else {
				ss << __get_format_arg(std::stoi(arg_spec), args...);
			}
			fmt.erase(0, pos + 1);
		}
	}
	ss << fmt;
	return ss.str();
}

#endif //UTIL_FORMAT_H
