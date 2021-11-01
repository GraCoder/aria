#include "taskInfo.h"

#include <filesystem>

std::string UriTask::getUri()
{
	return url;
	//std::string ret;
	//for(int i = 0; i < url.size() - 1; i++)
	//	ret += url[i] + ";";
	//ret += url.back();
	//return ret;
}

std::string UriTask::getLocal()
{
	auto path = std::filesystem::path(dir).append(name);
	return path.generic_string();
}

std::string BtTask::getUri()
{
	return torrent;
}

std::string BtTask::getLocal()
{

}
