#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#define FMS_TRC           do { ::fms::Dbg::inst().trc(__FILE__, __LINE__);      } while(0);
#define FMS_DBG(msg, ...) do { ::fms::Dbg::inst().dbg(msg, __FILE__, __LINE__,  ##__VA_ARGS__); } while(0);
#define FMS_INF(msg, ...) do { ::fms::Dbg::inst().inf(msg, __FILE__, __LINE__,  ##__VA_ARGS__); } while(0);
#define FMS_WRN(msg, ...) do { ::fms::Dbg::inst().wrn(msg, __FILE__, __LINE__,  ##__VA_ARGS__); } while(0);
#define FMS_ERR(msg, ...) do { ::fms::Dbg::inst().err(msg, __FILE__, __LINE__,  ##__VA_ARGS__); } while(0);
#define FMS_HEXDMP(start, end) do {::fms::Dbg::inst().hexdump(start, end, __FILE__, __LINE__); } while(0);

namespace fms {
	class Dbg {
	public:
		static Dbg& inst()
 {
			static Dbg instance;
			return instance;
		}
		Dbg(Dbg const&)               = delete;
        void operator=(Dbg const&)    = delete;
		enum Level {
			TRC,
			DBG,
			INF,
			WRN,
			ERR
		};
		template<typename It>
		void hexdump(It begin, It end, const char *file, int line) {
			std::stringstream stream;
			int i = 1;
			for (auto it = begin; it != end; it++) {
				stream << std::hex <<  std::setw(2) << std::setfill('0') << unsigned(*it);
				if (i % 4 == 0) stream << " ";
				if (i % 16 == 0) stream << std::endl;
				i++;
			}
			va_list emptyVaList;
			log(DBG, stream.str().c_str(), file, line, emptyVaList);
		}
		void trc(const char* file, int line, ...) {
			va_list args;
			va_start(args, line);
			log(TRC, "<trace>", file, line, args);
			va_end(args);
		}
		void dbg(const char* str, const char* file, int line, ...) {
			va_list args;
			va_start(args, line);
			log(DBG, str, file, line, args);
			va_end(args);
		}
		void inf(const char* str, const char* file, int line, ...) {
			va_list args;
			va_start(args, line);
			log(INF, str, file, line, args);
			va_end(args);
		}
		void wrn(const char* str, const char* file, int line, ...) {
			va_list args;
			va_start(args, line);
			log(WRN, str, file, line, args);
			va_end(args);
		}
		void err(const char* str, const char* file, int line, ...) {
			va_list args;
			va_start(args, line);
			log(ERR, str, file, line, args);
			va_end(args);
		}

		void log(Level level, const char* data, const char* file, int line, va_list args) {
			std::string out;
			if (supportsAnsi()) out = "\e[36m";
			out += (file + prefixLenght );
			out += ":";
			out += std::to_string(line);

			switch(level) {
				case TRC:
					if (supportsAnsi()) out += "\e[35m ";
					out += "[TRC]";
					break;
				case DBG:
					if (supportsAnsi()) out += "\e[32m ";
					out += "[DBG]";
					break;
				case INF:
					if (supportsAnsi()) out += "\e[34m ";
					out += "[INF]";
					break;
				case WRN:
					if (supportsAnsi()) out += "\e[33m ";
					out += "[WRN]";
				case ERR:
					if (supportsAnsi()) out += "\e[31m ";
					out += "[ERR]";
					break;
			}
			if (supportsAnsi()) out += "\e[0m";
			out += " ";
			size_t size = 200;
			char formatted_out[size];
			vsnprintf(formatted_out, size, data, args);
			out += formatted_out;

			outputLine(out);

		}
	protected:
		int prefixLenght;
		virtual void outputLine(std::string& str) const {
			std::cerr << str << std::endl;
		}

		virtual bool supportsAnsi() const { return true; }

	private:
		Dbg() {
			this->prefixLenght = std::string(__FILE__).find("core/debug.h");
			std::string versionInf = "FMS ";
			versionInf += __TIMESTAMP__;
			inf(versionInf.c_str(), __FILE__, __LINE__);
		}
	};

}

#endif //DEBUG_H
