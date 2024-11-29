#include <utility>
#include <foxogram/HttpClient.h>
#include <foxogram/exceptions.h>
#include <ixwebsocket/IXNetSystem.h>

bool isWSAInitialized = false;

namespace foxogram {
    const std::string &Payload::getMethod() const {
        return method;
    }

    const ix::WebSocketHttpHeaders &Payload::getHeaders() const {
        return headers;
    }

    const std::string &Payload::getBody() const {
        return body;
    }

    const std::string &Payload::getUrl() const {
        return url;
    }

    void Payload::addAuth(const std::string &token) {
        this->headers.insert({{"Authorization", "Bearer " + token}});
    }

    Payload::Payload(std::string method, const std::string& path, const std::string& token) {
        this->method = std::move(method);
        this->url = this->baseUrl + path;
        this->addAuth(token);
    }

    Payload::Payload(std::string method, const std::string& path,
                     std::map<std::string, std::string> headers, const std::string& token) {
        this->method = std::move(method);
        this->url = this->baseUrl + path;
        this->headers.merge(headers);
        this->addAuth(token);
    }

    Payload::Payload(std::string method, const std::string& path,
                     const nlohmann::json& body, const std::string& token) {
        this->method = std::move(method);
        this->url = this->baseUrl + path;
        std::string strBody;
        for (const auto& pair : body.get<std::map<std::string, std::string>>()) {
            strBody += pair.first + "=" + pair.second + "+";
        }
        this->body = strBody.substr(0, strBody.size()-1);
        this->bodyJson = body;
        this->addAuth(token);
    }

    Payload::Payload(std::string method, const std::string& path, std::map<std::string, std::string> headers,
                     const nlohmann::json& body, const std::string& token) {
        this->method = std::move(method);
        this->url = this->baseUrl + path;
        this->headers.merge(headers);
        std::string strBody;
        for (const auto& pair : body.get<std::map<std::string, std::string>>()) {
            strBody += pair.first + "=" + pair.second + "+";
        }
        this->body = strBody.substr(0, strBody.size()-1);
        this->bodyJson = body;
        this->addAuth(token);
    }

    const nlohmann::json &Payload::getBodyJson() const {
        return bodyJson;
    }

    nlohmann::json HttpClient::request(const Payload& payload) {
        if (!isWSAInitialized) {
            ix::initNetSystem();
            isWSAInitialized = true;
        }
        ix::HttpClient httpClient;
        ix::HttpRequestArgsPtr args = httpClient.createRequest();
        args->extraHeaders = payload.getHeaders();
        ix::HttpResponsePtr r;
        if (payload.getMethod() == "GET") {
            r = httpClient.get(payload.getUrl()+"?"+payload.getBody(), args);
        } else if (payload.getMethod() == "POST") {
            r = httpClient.post(payload.getUrl(), to_string(payload.getBodyJson()), args);
        } else if (payload.getMethod() == "PUT") {
            r = httpClient.put(payload.getUrl(), to_string(payload.getBodyJson()), args);
        } else if (payload.getMethod() == "DELETE") {
            r = httpClient.Delete(payload.getUrl(), args);
        } else if (payload.getMethod() == "PATCH") {
            r = httpClient.patch(payload.getUrl(), to_string(payload.getBodyJson()), args);
        } else {
            throw std::invalid_argument("Invalid method");
        }
        if (!r->errorMsg.empty()) {
            throw HttpException(r->errorMsg);
        }
        nlohmann::json j = nlohmann::json::parse(r->body);
        return j;
    }

}