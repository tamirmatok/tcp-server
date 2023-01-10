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
	//else if (requestType == "POST") {

	//}
	//else if (requestType == "DELETE") {

	//}
	//else if (requestType == "TRACE") {

	//}
	//else if (requestType == "HEAD") {

	//}
	//else if (requestType == "OPTIONS") {

	//}
	//else if (requestType == "PUT") {

	//}
	return response;
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

	return createResponse(status, "text/html", to_string(body.length()), body, false);
}

string createResponse(string status, string contentType, string contentLen, string body, bool isOptions)
{
	time_t rawtime;
	time(&rawtime);
	string date(ctime(&rawtime));

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

string getFileName(string language){

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