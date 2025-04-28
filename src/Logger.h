#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <sstream>

class ErrorLogMessage : public std::basic_ostringstream<char> {
	public:
		~ErrorLogMessage() {
			std::cerr << "Fatal error: " << str().c_str();
			exit(EXIT_FAILURE);
		}
};

#define DIE ErrorLogMessage()

#endif //LOGGER_H
