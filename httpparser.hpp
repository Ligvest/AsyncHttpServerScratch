#pragma once

#include <fstream>
#include <iostream>
#include <sstream>

const std::string rootFolder();

struct HttpRequest {
    std::string type;        // GET
    std::string url;         // /index.html
    std::string version;     // HTTP/1.0
    std::string parameters;  // params=asdfs
};

struct HttpResponse {
    std::string status;       // 200 OK
    std::string connection;   // close
    std::string contentType;  // text/htmla
    std::string content;      // message
};

class HttpParser {
    // ctors and dtors
public:
    HttpParser(std::string rootFolder) : rootFolder_(rootFolder) {}

    // methods
public:
    std::string readFile(std::string filename);
    bool isFileExists(std::string filename);
    void parseUrlAndParameters(std::istringstream& ss, HttpRequest& httpRequest);
    HttpRequest parseHttpHeader(const std::string& rawHeader);
    std::string formResponse(const HttpRequest& httpRequest);

private:
    const std::string rootFolder_;
};

bool HttpParser::isFileExists(std::string filename) {
    std::ifstream infile(filename);
    return infile.good();
}

void HttpParser::parseUrlAndParameters(std::istringstream& ss, HttpRequest& httpRequest) {
    ss >> httpRequest.url;
    std::size_t paramBeginIndex = httpRequest.url.find('?');
    if (paramBeginIndex != std::string::npos) {                                // params found
        httpRequest.parameters = httpRequest.url.substr(paramBeginIndex + 1);  // cut url
        httpRequest.url = httpRequest.url.substr(0, paramBeginIndex);          // cut params
    }

    if ((httpRequest.url == "/index.html") || (httpRequest.url == "/")) {
        httpRequest.url = "/index.html";
    }
}

HttpRequest HttpParser::parseHttpHeader(const std::string& rawHeader) {
    HttpRequest httpRequest;
    std::istringstream rawStream(rawHeader);
    rawStream >> httpRequest.type;                  // request type
    parseUrlAndParameters(rawStream, httpRequest);  // url and parameters
    rawStream >> httpRequest.version;               // http version
    return httpRequest;
}

std::string HttpParser::formResponse(const HttpRequest& httpRequest) {
    HttpResponse httpResponse;
    std::string answer;

    if (isFileExists(rootFolder_ + httpRequest.url)) {
        httpResponse.content = readFile(rootFolder_ + httpRequest.url);
        httpResponse.status = "200 OK";
        httpResponse.connection = "close";
        httpResponse.contentType = "text/html";

        answer = httpRequest.version + ' ' + httpResponse.status + "\r\n" +
                 "Content-Length: " + std::to_string(httpResponse.content.size()) + "\r\n" +
                 "Connection: " + httpResponse.connection + "\r\n" +
                 "Content-Type: " + httpResponse.contentType + "\r\n" + "\r\n" +
                 httpResponse.content;
    } else {
        answer = "HTTP/1.0 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n";
    }
    return answer;
}

std::string HttpParser::readFile(std::string filename) {
    // Read from file to std::string
    std::ifstream inputFileStream(filename);
    std::stringstream ss;
    ss << inputFileStream.rdbuf();
    return ss.str();
}
