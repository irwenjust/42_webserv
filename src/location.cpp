#include "location.hpp"

Location::Location(ServerConf *srvConf)
	: _serverConfig(srvConf)
	, _autoIndex(false)
	, _autoIndexSet(false)
	, _redirectCode(0)
	, _session(false)
	, _sessionSet(false)
{
}

Location::Location(const Location &original)
	: _serverConfig(original._serverConfig)
	, _path(original._path)
	, _rootPath(original._rootPath)
	, _autoIndex(original._autoIndex)
	, _methods(original._methods)
{
}

Location::Location()
{
}

Location::~Location()
{
}

Location &Location::operator=(const Location &original)
{
	if (this != &original)
	{
		this->~Location();
		new (this) Location(original);
	}
	return (*this);
}

void Location::parseLocation(std::ifstream &configFile, std::string &location_line)
{
	std::string line;

	_addPath(location_line);
	Utils::skipEmptyLines(configFile, line);
	if (!configFile)
		throw std::runtime_error("parseLocation: Empty location block!");
	if (!std::regex_match(line, std::regex("\t\\{\\s*")))
		throw std::runtime_error("parseLocation: No '{' opening location block!");

	while (Utils::skipEmptyLines(configFile, line), configFile)
	{
		std::smatch match_res;
		std::regex ptrn("^\t{2}(\\w+).*");
		if (!std::regex_match(line, match_res, ptrn))
			break;
		// std::cout << "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmethod: " <<match_res[1] << std::endl;
		if (match_res[1] == "root")
			_addRoot(line);
		else if (match_res[1] == "index")
			_addIndex(line);
		else if (match_res[1] == "autoindex")
			_addAutoIndex(line);
		else if (match_res[1] == "session")
			_addSession(line);
		else if (match_res[1] == "allow_methods")
			_addMethods(line);
		else if (match_res[1] == "return")
			_addRedirect(line);
		else if (match_res[1] == "upload")
			_addUpload(line);
		else if (match_res[1] == "cgi_pass")
			_addCgi(line);
		else
			throw std::runtime_error(
				"parseLocation: Unknown element in location block: " +
				line);
	}
	if (!std::regex_match(line, std::regex("\t\\}\\s*")))
		throw std::runtime_error("parseLocation: No '}' closing location block!");

	// if (_methods.empty() || 
	// !((_rootPath.size() && _autoIndexSet && !_redirectPath.size()) || (!_rootPath.size() && !_autoIndexSet && _redirectPath.size())))
	// 	throw std::runtime_error("parseLocation: Invalid location block");
}

// Setters
void Location::_addPath(std::string &line)
{
	std::regex ptrn("^\\s*location\\s+(\\S+)\\s*$");
	std::smatch match_res;

	if (!std::regex_match(line, match_res, ptrn))
		throw std::runtime_error(
			"_addPath: Expected format: \"location [/path/];\"");
	_path = match_res[1];
}

void Location::_addRoot(std::string &line)
{
	std::regex ptrn("^\\s*root\\s+(.*?)\\s*;?\\s*$");
	std::smatch match_res;
	struct stat mode;

	if (_rootPath.size())
		throw std::runtime_error(
			"_addRoot: Cannot add location root multiple times!");
	if (!std::regex_match(line, match_res, ptrn))
		throw std::runtime_error(
			"_addRoot: Expected format: \"root [directory];\"");
	if (stat(match_res.str(1).c_str(), &mode) != 0)
		throw(std::runtime_error("_addRoot: Specified path doesn't exist!"));
	if (!(mode.st_mode & S_IFDIR))
		throw(std::runtime_error("_addRoot: Specified path isn't a directory!"));
	_rootPath = match_res[1];
	_rootPath = std::filesystem::canonical(_rootPath).generic_string();
}

void Location::_addIndex(std::string &line)
{
	std::regex ptrn("^\\s*index\\s+([\\w\\.-]+\\.(html|htm|txt|bad_extension))\\s*;?\\s*$");
	std::smatch match_res;

	if (!_index.empty())
		throw std::runtime_error(
			"_addIndex: Cannot add location index multiple times!");
	if (!std::regex_match(line, match_res, ptrn))
		throw std::runtime_error("_addIndex: Expected format: \"index file.(html|htm|txt);\"");
	_index = match_res[1];
}

void Location::_addAutoIndex(std::string &line)
{
	std::regex ptrn("^\\s*autoindex\\s+(on|off)\\s*;\\s*$");
	std::smatch match_res;

	if (_autoIndexSet)
		throw std::runtime_error(
			"_addAutoIndex: Cannot add autoindex element multiple times!");
	if (!std::regex_match(line, match_res, ptrn))
		throw std::runtime_error(
			"_addAutoIndex: Expected format: \"autoindex [on/off];\"");
	if (match_res[1] == "on")
		_autoIndex = 1;
	else
		_autoIndex = 0;
	_autoIndexSet = true;
}

void Location::_addSession(std::string &line)
{
	std::regex ptrn("^\\s*session\\s+(on|off)\\s*;\\s*$");
	std::smatch match_res;

	if (_sessionSet)
		throw std::runtime_error(
			"_addSession: Cannot add sesseion element multiple times!");
	if (!std::regex_match(line, match_res, ptrn))
		throw std::runtime_error(
			"_addSession: Expected format: \"session [on/off];\"");
	if (match_res[1] == "on")
		_session = 1;
	else
		_session = 0;
	_sessionSet = true;
}

void Location::_addMethods(std::string &line)
{
	std::regex ptrn_global("^\\s*allow_methods\\s+(GET|POST|DELETE|PUT)(\\s+(GET|POST|DELETE|PUT))*\\s*;?\\s*$");
	std::regex ptrn_local("\\s+(GET|POST|DELETE|PUT)");

	if (_methods.size())
		throw std::runtime_error(
			"_addMethods: Cannot add location methods multiple times!");
	if (!std::regex_match(line, ptrn_global))
		throw std::runtime_error(
			"_addMethod: Expected format: \"method [list of methods (GET/POST/DELETE)];\"");
	for (std::sregex_iterator itr =
		     std::sregex_iterator(line.begin(), line.end(), ptrn_local);
	     itr != std::sregex_iterator(); itr++)
	{
		if (std::find(_methods.begin(), _methods.end(), (*itr)[1]) !=
		    _methods.end())
			throw std::runtime_error(
				"_addMethods: Adding duplicate location methods!");
		_methods.push_back((*itr)[1]);
	}
}

void Location::_addRedirect(std::string &line)
{
	std::regex ptrn("^\t{2}return\\s+(301|302)\\s+([a-zA-Z0-9\\.\\/:]*)\\s*;\\s*$");
	std::smatch match_res;

	if (_redirectCode)
		throw std::runtime_error(
			"_addRedirect: Cannot add multiple redirects to location!");
	if (!std::regex_match(line, match_res, ptrn))
		throw std::runtime_error(
			"_addRedirect: Expected format: \"return [code] [HTTP/HTTPS URL];\"");
	_redirectCode = stringToType<int>(match_res[1]);
	_redirectPath = match_res[2];
}

void Location::_addUpload(std::string &line)
{
	std::regex ptrn("\t{2}upload\\s+(.*)\\s*;\\s*");
	std::smatch match_res;
	struct stat mode;

	if (_uploadPath.length())
		throw std::runtime_error(
			"_addUpload: Cannot add multiple upload paths to location!");
	if (!std::regex_match(line, match_res, ptrn))
		throw std::runtime_error(
			"_addUpload: Expected format: \"upload [directory];\"");
	if (stat(match_res.str(1).c_str(), &mode) != 0)
		throw(std::runtime_error("_addUpload: Specified path doesn't exist!"));
	if (!(mode.st_mode & S_IFDIR))
		throw(std::runtime_error(
			"_addUpload: Specified path isn't a directory!"));
	_uploadPath = match_res[1];
	_uploadPath = std::filesystem::canonical(_uploadPath).generic_string();
}

void Location::_addCgi(std::string &line)
{
	// Accept: "cgi_pass /path/to/interpreter;"
	std::regex ptrn("^\\s*cgi_pass\\s+([^\\s]+)\\s*;?\\s*$");
	std::smatch match_res;
	struct stat mode;

	if (!_cgi.empty())
		throw std::runtime_error("_addCgi: Cannot add location cgi multiple times!");

	if (!std::regex_match(line, match_res, ptrn))
		throw std::runtime_error("_addCgi: Expected format: \"cgi_pass /path/to/interpreter;\"");

	std::string path = match_res[1];
	if (stat(path.c_str(), &mode) != 0)
		throw std::runtime_error("_addCgi: Specified path doesn't exist: " + path);

	if (!S_ISREG(mode.st_mode))
		throw std::runtime_error("_addCgi: Specified path isn't a file!");

	// You can set this to a default extension like .bla
	_cgi[".bla"] = path;
}

// Getters
bool Location::getAutoIndex()
{
	return _autoIndex;
}

void Location::dump(void)
{
	std::cout << "-- Location.dump() --" << std::endl;
	for (const auto &m : this->_methods)
	{
		std::cout << "\tmethod: " << m << std::endl;
	}
	std::cout << "\tpath: " << _path << std::endl;
	std::cout << "\trootPath: " << _rootPath << std::endl;
	std::cout << "\tuploadPath: " << _uploadPath << std::endl;

	std::cout << "---------------------" << std::endl;
}