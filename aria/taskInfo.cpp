#include "taskInfo.h"


std::string UriTask::getUri()
{
	return url;
	//std::string ret;
	//for(int i = 0; i < url.size() - 1; i++)
	//	ret += url[i] + ";";
	//ret += url.back();
	//return ret;
}

std::string BtTask::getUri()
{
	return torrent;
}
