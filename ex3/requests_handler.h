#pragma once

#include "server.h"
#include <fstream>
#include <string>
#include <iostream>
#include <direct.h>
#define GetCurrentDir _getcwd


extern const string RESPONSE_OK;
extern const string RESPONSE_CREATED_FILE;
extern const string RESPONSE_NO_CONTENT;
extern const string RESPONSE_FILE_NOT_FOUND;
extern const string SERVER_ERROR;

string requestHandler(Recv_headers& msg_headers);
string get_full_path(string fileName);
string handleGetRequest(Recv_headers& msg_headers);
string getFileName(string language);
string createResponse(string status, string contentType, string body, bool isOptions);
void getHeadersHelper(Recv_headers& headers, char* msg_received);
string handleTraceRequest(Recv_headers headers);
string handlePostRequest(Recv_headers headers);
string Delete_File(string filename);
string handleDeleteRequest(Recv_headers headers);