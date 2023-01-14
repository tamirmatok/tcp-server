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
	//else if (requestType == "HEAD") {

	//}
	else if (requestType == "OPTIONS") {
		//TODO: make sure body {}
		createResponse(RESPONSE_OK, "text/html", "", true);
	}
	//else if (requestType == "PUT") {

	//}
	return response;
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


void getHeadersHelper(Recv_headers& headers, char* msg_received)
{
	string buffer(msg_received);

	size_t accept = buffer.find("Accept:") + 7; //saves location after the 'accept:'
	headers.accept = strtok(&msg_received[accept], " \r\n");

	size_t host = buffer.find("Host:") + 5;
	headers.host = strtok(&msg_received[host], " \r\n");

	size_t connection = buffer.find("Connection:") + 11;
	headers.connection = strtok(&msg_received[connection], " \r\n");

	size_t content_Type = buffer.find("Content-Type:");
	if (content_Type != string::npos) { headers.Content_Type = strtok(&msg_received[content_Type + 13], " \r\n"); }
	else { headers.Content_Type = "text"; }

	//TODO:test res for content_len with real req.
	size_t content_len = buffer.find("Content-Length:") + 15;
	headers.content_len = strtok(&msg_received[content_len], " \r\n");

	size_t body = buffer.find(string("\r\n\r\n")) + 4;
	headers.body.assign(&msg_received[body]);

	size_t language = buffer.find("?lang=");
	if (language != string::npos) { headers.language = strtok(&msg_received[language + 6], " \r\n"); }
	else { headers.language.clear(); }

	size_t file_id = buffer.find("?fileName=");
	if (file_id != string::npos) { headers.file_name = strtok(&msg_received[file_id + 10], "\r\n"); }
	else {
		 headers.file_name.clear(); 
	}
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