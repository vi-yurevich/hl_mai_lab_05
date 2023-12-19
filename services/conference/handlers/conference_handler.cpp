#ifndef CONFERENCE_HANDLER_H
#define CONFERENCE_HANDLER_H

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include <iostream>
#include <iostream>
#include <fstream>

using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTMLForm;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::NameValueCollection;
using Poco::Net::ServerSocket;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;

#include "../../../database/user.h"
#include "../../../database/lecture.h"
#include "../../../database/conference.h"

static bool hasSubstr(const std::string &str, const std::string &substr)
{
    if (str.size() < substr.size())
        return false;
    for (size_t i = 0; i <= str.size() - substr.size(); ++i)
    {
        bool ok{true};
        for (size_t j = 0; ok && (j < substr.size()); ++j)
            ok = (str[i + j] == substr[j]);
        if (ok)
            return true;
    }
    return false;
}

class ConferenceHandler : public HTTPRequestHandler
{
private:
    bool check_conference_id(const std::string &conference_id, std::string &reason)
    {
        long id = atol(conference_id.c_str());
        if (id < 0 && !database::Conference::check_conference_exists_by_id(id))
        {
            std::ostringstream oss;
            oss << "Conference with id: " << conference_id << "doesn't exist.";
            reason = oss.str();
            return false;
        }

        return true;
    };

    bool check_lecture_id(const std::string &lecture_id, std::string &reason)
    {
        long id = atol(lecture_id.c_str());
        if (id < 0 && !database::Lecture::check_lecture_exists_by_id(id))
        {
            std::ostringstream oss;
            oss << "Lecture with id: " << lecture_id << "doesn't exist.";
            reason = oss.str();
            return false;
        }

        return true;
    };

public:
    ConferenceHandler(const std::string &format) : _format(format)
    {
    }

    void handleRequest(HTTPServerRequest &request,
                       HTTPServerResponse &response)
    {
        HTMLForm form(request, request.stream());
        try
        {
            if (form.has("conference_id") 
                    && (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET)
                    && hasSubstr(request.getURI(), "/conference/lectures"))
            {
                long id = atol(form.get("conference_id").c_str());

                auto result = database::Lecture::search_by_conference_id(id);
                if (result)
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(
                        database::Lecture::vectorToJSON(result.value()), 
                        ostr);
                    return;
                }
                else
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("type", "/errors/not_found");
                    root->set("title", "Internal exception");
                    root->set("status", "404");
                    root->set("detail", "conference not found");
                    root->set("instance", "/user");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }
            }
            else if (form.has("conference_id")
                    && form.has("lecture_id")
                    && (request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT)
                    && hasSubstr(request.getURI(), "/conference/add_lecture"))
                {
                    bool check_result = true;
                    std::string message;
                    std::string reason;

                    if (!check_conference_id(form.get("conference_id"), reason))
                    {
                        check_result = false;
                        message += reason;
                        message += "<br>";
                    }
                    if (!check_lecture_id(form.get("lecture_id"), reason))
                    {
                        check_result = false;
                        message += reason;
                        message += "<br>";
                    }

                    if (check_result)
                    {
                        long conference_id = atol(form.get("conference_id").c_str());
                        long lecture_id = atol(form.get("lecture_id").c_str());
                        atol(form.get("conference_id").c_str());
                        database::Lecture::update_conference_id(lecture_id, conference_id);
                        
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        response.setChunkedTransferEncoding(true);
                        response.send();

                        return;
                    }
                    else
                    {
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                        std::ostream &ostr = response.send();
                        ostr << message;
                        response.send();
                        return;
                    }
                }
            
        }
        catch (...)
        {
        }

        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/not_found");
        root->set("title", "Internal exception");
        root->set("status", Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
        root->set("detail", "request not found");
        root->set("instance", "/con");
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

private:
    std::string _format;
};
#endif