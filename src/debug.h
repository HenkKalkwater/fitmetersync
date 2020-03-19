#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <string>

#define FMS_TRC      do { ::FMS::Dbg::inst().trc(__FILE__, __LINE__);      } while(0);
#define FMS_DBG(msg) do { ::FMS::Dbg::inst().dbg(msg, __FILE__, __LINE__); } while(0);
#define FMS_INF(msg) do { ::FMS::Dbg::inst().inf(msg, __FILE__, __LINE__); } while(0);
#define FMS_WRN(msg) do { ::FMS::Dbg::inst().wrn(msg, __FILE__, __LINE__); } while(0);
#define FMS_ERR(msg) do { ::FMS::Dbg::inst().err(msg, __FILE__, __LINE__); } while(0);

namespace FMS {
	class Dbg {
	public:
		static Dbg& inst() {
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
		void trc(const char* file, int line) {
			log(TRC, "<trace>", file, line);
		}
		void dbg(const char* str, const char* file, int line) {
			log(DBG, str, file, line);
		}
		void inf(const char* str, const char* file, int line) {
			log(INF, str, file, line);
		}
		void wrn(const char* str, const char* file, int line) {
			log(WRN, str, file, line);
		}
		void err(const char* str, const char* file, int line) {
			log(ERR, str, file, line);
		}
		
		void log(Level level, const char* data, const char* file, int line) {
			std::string out;
			
			out += (file + prefixLenght);
			out += ":";
			out += std::to_string(line);
			
			switch(level) {
				case TRC:
					if (supportsAnsi()) out += "\e[35m ";
					break;
				case DBG:
					if (supportsAnsi()) out += "\e[32m ";
					else out += " DBG: ";
					break;
				case INF:
					if (supportsAnsi()) out += "\e[34m ";
					else out += " INF: ";
					break;
				case WRN:
					if (supportsAnsi()) out += "\e[33m ";
					else out += "WRN: ";
				case ERR:
					if (supportsAnsi()) out += "\e[31m ";
					else out += "ERR: ";
					break;
			}
			out += data;
			if (supportsAnsi()) out += "\e[0m";
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
			this->prefixLenght = std::string(__FILE__).find("src/debug.h");
			std::string versionInf = "JDBX ";
			versionInf += __TIMESTAMP__;
			dbg(versionInf.c_str(), __FILE__, __LINE__);
		}
	};

}

#endif //DEBUG_H
