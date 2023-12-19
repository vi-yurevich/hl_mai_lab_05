#include <string>
#include <iostream>
#include <fstream>

#include "./../database/database.h"
#include "./../database/user.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

using namespace Poco::Data::Keywords;

auto main() -> int
{
    try
    {
        std::string json;
        std::ifstream is("/load_data/test_data_for_user_table.json");
        std::istream_iterator<char> eos;
        std::istream_iterator<char> iit(is);
        while (iit != eos)
            json.push_back(*(iit++));
        is.close();
        
        std::vector<database::User> users = database::User::fromJSON_Array(json);

        std::cout << "Begin inserting ...." << std::endl;

        for (auto &user: users)
            user.save_to_mysql();

        std::cout << "Inserted " << users.size() << " records" << std::endl; 
    }
    catch (Poco::Data::MySQL::ConnectionException &e)
    {
        std::cout << "connection:" << e.what() << std::endl;
    }
    catch (Poco::Data::MySQL::StatementException &e)
    {

        std::cout << "statement:" << e.what() << std::endl;
    }
    catch (std::exception* ex)
    {
        std::cout << "exception:" << ex->what() << std::endl;
    }
    return 0;
}