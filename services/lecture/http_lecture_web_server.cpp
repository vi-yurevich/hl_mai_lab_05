#ifndef HTTP_LECTURE_WEBSERVERS
#define HTTP_LECTURE_WEBSERVERS

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
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

using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::ServerSocket;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;

#include "http_lecture_request_factory.cpp"

// #include "../../database/user.h"


class HTTPLectureWebServer : public Poco::Util::ServerApplication
{
public:
    int main([[maybe_unused]] const std::vector<std::string> &args)
    {
        // std::cout << "current id:" << database::User::get_last_inserted_user_id() << std::endl;
        // std::cout << database::User::get_id_for_insert_user() << std::endl;
        // std::cout << "after get id:" << database::User::get_last_inserted_user_id() << std::endl;

        ServerSocket svs(Poco::Net::SocketAddress("0.0.0.0", 8081));
        HTTPServer srv(new HTTPLectureRequestFactory(DateTimeFormat::SORTABLE_FORMAT), svs, new HTTPServerParams);
        srv.start();
        waitForTerminationRequest();
        srv.stop();

        return Application::EXIT_OK;
    }
};
#endif // !HTTP_LECTURE_WEBSERVER
