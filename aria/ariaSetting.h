#ifndef ARIASETTING_H
#define ARIASETTING_H

#include <map>
#include <string>

class ariaSetting
{
public:
	static ariaSetting& instance();

	std::string appPath();

	std::string downloadPath();

	std::map<std::string, std::string> &setting() {return _settings;}
private:
	ariaSetting();

	std::string _appPath;
	std::map<std::string, std::string> _settings;
};

#endif // ARIASETTING_H
