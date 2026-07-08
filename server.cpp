#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "game.hpp"

namespace {

constexpr int kPort = 8080;
constexpr std::size_t kBufferSize = 8192;

std::string generateSessionId() {
    static const char charset[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(0, sizeof(charset) - 2);

    std::string id;
    id.reserve(16);
    for (int i = 0; i < 16; ++i) {
        id += charset[distribution(generator)];
    }
    return id;
}

std::string trim(const std::string& value) {
    const std::size_t start = value.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    const std::size_t end = value.find_last_not_of(" \t\r\n");
    return value.substr(start, end - start + 1);
}

std::string toLower(std::string value) {
    for (char& ch : value) {
        if (ch >= 'A' && ch <= 'Z') {
            ch = static_cast<char>(ch - 'A' + 'a');
        }
    }
    return value;
}

std::string getMimeType(const std::string& path) {
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".html") {
        return "text/html; charset=utf-8";
    }
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".css") {
        return "text/css; charset=utf-8";
    }
    if (path.size() >= 3 && path.substr(path.size() - 3) == ".js") {
        return "application/javascript; charset=utf-8";
    }
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".png") {
        return "image/png";
    }
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".svg") {
        return "image/svg+xml";
    }
    return "text/plain; charset=utf-8";
}

std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return "";
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

struct HttpRequest {
    std::string method;
    std::string path;
    std::map<std::string, std::string> headers;
    std::string body;
};

bool parseRequest(const std::string& raw, HttpRequest& request) {
    const std::size_t headerEnd = raw.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        return false;
    }

    std::istringstream stream(raw.substr(0, headerEnd));
    std::string requestLine;
    if (!std::getline(stream, requestLine)) {
        return false;
    }
    if (!requestLine.empty() && requestLine.back() == '\r') {
        requestLine.pop_back();
    }

    std::istringstream requestStream(requestLine);
    requestStream >> request.method >> request.path;
    if (request.method.empty() || request.path.empty()) {
        return false;
    }

    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        const std::size_t colon = line.find(':');
        if (colon == std::string::npos) {
            continue;
        }
        std::string key = trim(line.substr(0, colon));
        std::string value = trim(line.substr(colon + 1));
        request.headers[toLower(key)] = value;
    }

    request.body = raw.substr(headerEnd + 4);
    return true;
}

std::string buildResponse(int statusCode, const std::string& statusText,
                          const std::string& contentType, const std::string& body,
                          const std::string& extraHeaders = "") {
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: close\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Access-Control-Allow-Headers: Content-Type, X-Session-Id\r\n";
    response << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    response << extraHeaders;
    response << "\r\n";
    response << body;
    return response.str();
}

std::string jsonError(const std::string& message) {
    return "{\"error\":\"" + jsonEscape(message) + "\"}";
}

int extractChoice(const std::string& body) {
    const std::string key = "\"choice\"";
    const std::size_t keyPos = body.find(key);
    if (keyPos == std::string::npos) {
        return -1;
    }
    const std::size_t colon = body.find(':', keyPos);
    if (colon == std::string::npos) {
        return -1;
    }
    std::size_t pos = colon + 1;
    while (pos < body.size() && (body[pos] == ' ' || body[pos] == '\t')) {
        ++pos;
    }
    int value = 0;
    bool found = false;
    while (pos < body.size() && body[pos] >= '0' && body[pos] <= '9') {
        value = value * 10 + (body[pos] - '0');
        found = true;
        ++pos;
    }
    return found ? value : -1;
}

std::string handleApi(const HttpRequest& request,
                      std::map<std::string, DungeonGame>& sessions,
                      const std::string& sessionId) {
    if (request.method == "OPTIONS") {
        return buildResponse(204, "No Content", "text/plain", "");
    }

    std::string activeSession = sessionId;
    if (activeSession.empty()) {
        activeSession = request.headers.count("x-session-id")
                            ? request.headers.at("x-session-id")
                            : "";
    }

    if (request.path == "/api/new" && request.method == "POST") {
        activeSession = generateSessionId();
        sessions[activeSession] = DungeonGame();
        sessions[activeSession].reset();
        const std::string body =
            "{\"sessionId\":\"" + activeSession + "\"," +
            gameViewToJson(sessions[activeSession].getView()).substr(1);
        return buildResponse(
            200, "OK", "application/json", body,
            "X-Session-Id: " + activeSession + "\r\n");
    }

    if (activeSession.empty() || sessions.find(activeSession) == sessions.end()) {
        return buildResponse(400, "Bad Request", "application/json",
                               jsonError("Missing or invalid session."));
    }

    DungeonGame& game = sessions[activeSession];

    if (request.path == "/api/state" && request.method == "GET") {
        return buildResponse(200, "OK", "application/json",
                             gameViewToJson(game.getView()),
                             "X-Session-Id: " + activeSession + "\r\n");
    }

    if (request.path == "/api/start" && request.method == "POST") {
        if (!game.start()) {
            return buildResponse(400, "Bad Request", "application/json",
                                   jsonError("Game already started."));
        }
        return buildResponse(200, "OK", "application/json",
                             gameViewToJson(game.getView()),
                             "X-Session-Id: " + activeSession + "\r\n");
    }

    if (request.path == "/api/choice" && request.method == "POST") {
        const int choice = extractChoice(request.body);
        if (choice < 1 || choice > 3 || !game.makeChoice(choice)) {
            return buildResponse(400, "Bad Request", "application/json",
                                   jsonError("Invalid choice."));
        }
        return buildResponse(200, "OK", "application/json",
                             gameViewToJson(game.getView()),
                             "X-Session-Id: " + activeSession + "\r\n");
    }

    if (request.path == "/api/continue" && request.method == "POST") {
        if (!game.continueGame()) {
            return buildResponse(400, "Bad Request", "application/json",
                                   jsonError("Cannot continue right now."));
        }
        return buildResponse(200, "OK", "application/json",
                             gameViewToJson(game.getView()),
                             "X-Session-Id: " + activeSession + "\r\n");
    }

    return buildResponse(404, "Not Found", "application/json",
                         jsonError("Unknown API route."));
}

std::string handleStatic(const HttpRequest& request) {
    std::string path = request.path;
    if (path == "/") {
        path = "/index.html";
    }

    if (path.find("..") != std::string::npos) {
        return buildResponse(403, "Forbidden", "text/plain", "Forbidden");
    }

    const std::string filePath = "web" + path;
    const std::string content = readFile(filePath);
    if (content.empty() && path != "/index.html") {
        return buildResponse(404, "Not Found", "text/plain", "Not found");
    }
    if (content.empty()) {
        return buildResponse(404, "Not Found", "text/plain",
                               "Missing web/index.html");
    }

    return buildResponse(200, "OK", getMimeType(filePath), content);
}

void handleClient(int clientSocket, std::map<std::string, DungeonGame>& sessions) {
    std::string raw;
    char buffer[kBufferSize];

    while (raw.find("\r\n\r\n") == std::string::npos) {
        const ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
            close(clientSocket);
            return;
        }
        buffer[bytesRead] = '\0';
        raw.append(buffer, static_cast<std::size_t>(bytesRead));

        const std::size_t headerEnd = raw.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            HttpRequest request;
            if (!parseRequest(raw, request)) {
                const std::string response =
                    buildResponse(400, "Bad Request", "text/plain", "Bad request");
                send(clientSocket, response.c_str(), response.size(), 0);
                close(clientSocket);
                return;
            }

            const auto contentLengthIt = request.headers.find("content-length");
            if (contentLengthIt != request.headers.end()) {
                const std::size_t expectedBodySize =
                    static_cast<std::size_t>(std::stoul(contentLengthIt->second));
                while (request.body.size() < expectedBodySize) {
                    const ssize_t bodyRead =
                        recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
                    if (bodyRead <= 0) {
                        break;
                    }
                    request.body.append(buffer, static_cast<std::size_t>(bodyRead));
                }
            }

            std::string response;
            if (request.path.rfind("/api/", 0) == 0) {
                response = handleApi(request, sessions, "");
            } else {
                response = handleStatic(request);
            }

            send(clientSocket, response.c_str(), response.size(), 0);
            close(clientSocket);
            return;
        }
    }

    close(clientSocket);
}

}  // namespace

int main() {
    const int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Failed to create socket.\n";
        return 1;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(kPort);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        std::cerr << "Failed to bind to port " << kPort << ".\n";
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 8) < 0) {
        std::cerr << "Failed to listen on port " << kPort << ".\n";
        close(serverSocket);
        return 1;
    }

    std::cout << "Dungeon of Shadows GUI server running at http://localhost:"
              << kPort << "\n";

    std::map<std::string, DungeonGame> sessions;

    while (true) {
        sockaddr_in clientAddress{};
        socklen_t clientLength = sizeof(clientAddress);
        const int clientSocket =
            accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddress),
                   &clientLength);
        if (clientSocket < 0) {
            if (errno == EINTR) {
                continue;
            }
            std::cerr << "Accept failed.\n";
            continue;
        }
        handleClient(clientSocket, sessions);
    }

    close(serverSocket);
    return 0;
}
