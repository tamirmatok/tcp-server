#include "requests_handler.h"

const string RESPONSE_OK = "200 OK";
const string RESPONSE_CREATED_FILE = "201 Created";
const string RESPONSE_NO_CONTENT = "204 No Content";
const string RESPONSE_FILE_NOT_FOUND = "404 Not Found";
const string SERVER_ERROR = "500 Internal Server Error";


string requestHandler(Recv_headers& msg_headers)
{
	string requestType = msg_headers.type;
	string response;

	if (requestType == "GET") {
		response = handleGetRequest(msg_headers);
	}
	else if (requestType == "POST") {
		response = handlePostRequest(msg_headers);
	}
	else if (requestType == "DELETE") {
		response = handleDeleteRequest(msg_headers);
	}
	else if (requestType == "TRACE") {
		response = handleTraceRequest(msg_headers);
	}
	else if (requestType == "HEAD") {
		response = handleHeadRequest(msg_headers);
	}
	else if (requestType == "OPTIONS") {
		//TODO: make sure body {}
		createResponse(RESPONSE_OK, "text/html", "", true);
	}
	else if (requestType == "PUT") {
		response = handlePutRequest(msg_headers);
	}
	return response;
}

string createResponseForHead(Recv_headers headers, string status)
{
	time_t rawtime;
	time(&rawtime);
	string date(ctime(&rawtime));

	string ans = "HTTP/1.1 ";
	ans += (status + "\r\n");
	ans += "Host: Web Server\r\n";
	ans += ("Date: " + date);
	ans += (string("Content-Type: ") + headers.Content_Type + "; charset=utf-8");

	if (status == RESPONSE_OK)
	{
		ans += "\r\n";
		ans += ("Last Modified: ");
		ans += (getLastChangeTime(headers.file_name));
		ans += "\r\n";
		ans += ("Content-Length: ");
		ans += to_string(getFileLen(headers.file_name));

		if (!headers.language.empty()) {
			ans += "\r\n";
			ans += ("Langauge: ");
			ans += (headers.language);
		}
	}

	ans += "\r\n";
	ans += "\r\n";

	return ans;
}

string createResponseForPut(Recv_headers& msg_headers, string status )
{
	time_t rawtime;
	time(&rawtime);
	string date(ctime(&rawtime));
	string contentLen = to_string(status.length());

	string ans = "HTTP/1.1 ";
	ans += (status + "\r\n");
	ans += "Host: Web Server\r\n";
	ans += ("Date: " + date);
	ans += (string("Content-Type: ") + msg_headers.Content_Type + +"; charset=utf-8" + "\r\n");
	ans += (string("Content-Length: ") + contentLen + "\r\n");
	ans += ("Content-Location:");
	ans += get_full_path(msg_headers.file_name);

	ans += "\r\n";
	ans += "\r\n";

	ans += status;

	return ans;
}

string createResponse(string status, string contentType, string body, bool isOptions)
{
	time_t rawtime;
	time(&rawtime);
	string date(ctime(&rawtime));
	string contentLen = to_string(body.length());

	string ans = "HTTP/1.1 ";
	ans += (status + "\r\n");
	ans += "Host: Web Server\r\n";
	ans += ("Date: " + date);
	ans += (string("Content-Type: ") + contentType + +"; charset=utf-8" + "\r\n");
	ans += (string("Content-Length: ") + contentLen + "\r\n");

	if (isOptions)
		ans += ("Allow: GET,POST,PUT,HEAD,TRACE, OPTIONS, DELETE\r\n");
	

	ans += "\r\n";
	ans += body;

	return ans;
}

string createFile(string& file_name, string body, string lang)
{
	FILE* fp;
	string status;
	string file_name_copy;
	
	// extract file name
	for(int i = 0;i< file_name.length();++i)
	{
		if (file_name[i] == ' ')
			break;
		file_name_copy.push_back(file_name[i]);
	}

	if (!lang.empty())
	{
		file_name_copy.resize(file_name_copy.find("."));
		file_name_copy.append("-").append(lang).append(".txt");	
	}

	if (body.empty()) return RESPONSE_NO_CONTENT;

	fp = fopen(file_name_copy.c_str(), "r+");

	if (fp == nullptr)
	{
		status = RESPONSE_CREATED_FILE;
		fp = fopen(file_name_copy.c_str(), "w");
		if (fp == nullptr) return RESPONSE_FILE_NOT_FOUND;
	}
	else
	{ 
		//TODO: rewrite file if exicet
		fclose(fp);
		status = RESPONSE_OK;
		fp = freopen(file_name_copy.c_str(), "w", fp);
	}

	fprintf(fp, "%s", body.data());
	fclose(fp);

	return status;
}

// ------------- WEB SERVER METHODS -------------------------
string handlePutRequest(Recv_headers& msg_headers)
{
	string status = createFile(msg_headers.file_name, msg_headers.body, msg_headers.language);
	return 	createResponseForPut(msg_headers, status);
}

string handleGetRequest(Recv_headers& msg_headers)
{
	string status = RESPONSE_OK;
	string body = "";
	int fileSize = 0;
	string fileName = getFileName(msg_headers.language);
	string fullPath = get_full_path(fileName);
	ifstream myFile(get_full_path(fileName));

	if (!myFile)
	{
		status = RESPONSE_FILE_NOT_FOUND;
	}
	else
	{
		myFile.seekg(0, ios::end);
		fileSize = myFile.tellg();
		myFile.seekg(0, ios::beg);
		while (myFile.good())
		{
			string line = "";
			getline(myFile, line);
			body += (line + "\n");
		}
		myFile.close();
	}

	return createResponse(status, "text/html", body, false);
}

string handleTraceRequest(Recv_headers headers)
{
	return createResponse(RESPONSE_OK, "massage/http", headers.raw_msg, false);
}

string handlePostRequest(Recv_headers headers)
{
	//return the body of the message
	string body = "body received: " + headers.body;
	return createResponse(RESPONSE_OK, "massage/http", body, false);
}

string handleDeleteRequest(Recv_headers headers)
{
	string body = "";
	string fullPathToFile = get_full_path(headers.file_name);
	string status = Delete_File(fullPathToFile);

	return createResponse(status, "message/http", body, false);
}

string handleHeadRequest(Recv_headers headers)
{
	string status;
	string response;
	string full_file_name(headers.file_name);

	if (!headers.language.empty())
	{
		full_file_name.resize(full_file_name.find("."));
		full_file_name.append("-").append(headers.language).append(".txt");
	}

	status = getFileStatus(full_file_name);
	response = createResponseForHead(headers, status);

	return response;
}

//---------------------------------------------------------

string getLastChangeTime(string file_name)
{
	string modified_time;
	struct stat filestat;
	stat(file_name.data(), &filestat);
	modified_time.append(ctime(&filestat.st_mtime));
	modified_time[modified_time.length() - 1] = ' ';
	return modified_time;
}

// checkes if file exist
string getFileStatus(string file_name)
{
	FILE* fp;
	fp = fopen(file_name.data(), "r");

	if (fp == nullptr)
		return RESPONSE_FILE_NOT_FOUND;
	else
	{
		fclose(fp);
		return RESPONSE_OK;
	}
}

unsigned int getFileLen(string file_name)
{
	FILE* fp;
	fp = fopen(file_name.data(), "r");
	if (fp == nullptr) exit(1);
	fseek(fp, 0L, SEEK_END);
	int sz = ftell(fp);
	fclose(fp);
	return sz;
}

string getFileName(string language) {

	string fileName = "";

	if (language == "en") {
		fileName = "en.txt";
	}
	else if (language == "fr") {
		fileName = "fr.txt";
	}
	else if (language == "he") {
		fileName = "he.txt";
	}
	return fileName;
}

string get_full_path(string fileName) {
	char buff[FILENAME_MAX]; //create string buffer to hold path
	GetCurrentDir(buff, FILENAME_MAX);
	string current_working_dir(buff);
	return current_working_dir + "\\" + fileName;
}

// delete file from current directory.
string Delete_File(string filename)
{
	FILE* fp;
	fp = fopen(filename.data(), "r");
	if (fp == nullptr)
		return RESPONSE_FILE_NOT_FOUND;
	else {
		fclose(fp);
		if (remove(filename.data()) != 0)
		{
			return SERVER_ERROR;
		}
		else
			cout << endl << filename + " deleted succesfully" << endl;
			return RESPONSE_OK;
	}
}