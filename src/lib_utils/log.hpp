#pragma once

#include "format.hpp"
#include <ostream>

enum Level {
	Quiet = -1,
	Error = 0,
	Warning,
	Info,
	Debug
};

class Log {
	public:
		template<typename... Arguments>
		static void msg(Level level, const std::string& fmt, Arguments... args) {
			if ((level != Quiet) && (level <= globalLogLevel)) {
				get(level) << getColorBegin(level) << getTime() << format(fmt, args...) << getColorEnd(level) << std::endl;
				get(level).flush();
			}
		}

		void setLevel(Level level);
		Level getLevel();

	private:
		Log();
		~Log();
		static std::ostream& get(Level level);
		static std::string getTime();
		static std::string getColorBegin(Level level);
		static std::string getColorEnd(Level level);

		static Level globalLogLevel;
};
