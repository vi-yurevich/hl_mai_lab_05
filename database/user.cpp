#include "user.h"
#include "database.h"
#include "cache.h"
#include "../config/config.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <cppkafka/cppkafka.h>

#include <sstream>
#include <fstream>
#include <exception>
#include <algorithm>
#include <future>
#include <mutex>
#include <stdlib.h>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database
{
    bool User::operator< (const User &other) const {
        return _id < other.get_id();
    }  
    
    void User::preload(const std::string &file)
    {
        try
        {

            Poco::Data::Session session = database::Database::get().create_session();
            std::string json;
            std::ifstream is(file);
            std::istream_iterator<char> eos;
            std::istream_iterator<char> iit(is);
            while (iit != eos)
                json.push_back(*(iit++));
            is.close();

            Poco::JSON::Parser parser;
            Poco::Dynamic::Var result = parser.parse(json);
            Poco::JSON::Array::Ptr arr = result.extract<Poco::JSON::Array::Ptr>();

            size_t i{0};
            for (i = 0; i < arr->size(); ++i)
            {
                Poco::JSON::Object::Ptr object = arr->getObject(i);
                long id = database::User::get_id_for_insert_user();
                std::string first_name = object->getValue<std::string>("first_name");
                std::string last_name = object->getValue<std::string>("last_name");
                std::string phone_number = object->getValue<std::string>("phone_number");
                std::string email = object->getValue<std::string>("email");
                std::string login = object->getValue<std::string>("login");
                std::string password = object->getValue<std::string>("password");
                std::string sharding_hint = database::Database::sharding_hint_for_user(id);

                std::string select_str = "INSERT INTO User (id, first_name,last_name,email,phone_number,login,password) VALUES(?, ?, ?, ?, ?, ?, ?)";
                select_str += sharding_hint;

                Poco::Data::Statement insert(session);
                insert << select_str,
                use(id),
                use(first_name),
                use(last_name),
                use(email),
                use(phone_number),
                use(login),
                use(password);

                insert.execute();
            }

            std::cout << "Inserted " << i << " records" << std::endl;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }
    
    Poco::JSON::Object::Ptr User::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("first_name", _first_name);
        root->set("last_name", _last_name);
        root->set("email", _email);
        root->set("phone_number", _phone_number);
        root->set("login", _login);
        root->set("password", _password);

        return root;
    }

    Poco::JSON::Array::Ptr User::vectorToJSON_and_hide_password(std::vector<User>& users) {
        Poco::JSON::Array::Ptr json_user_array = new Poco::JSON::Array();

        for (const auto& user : users) {

            Poco::JSON::Object::Ptr json_user = user.toJSON();
            json_user->set("password", "*******");

            json_user_array->add(
                Poco::Dynamic::Var(json_user)
            );
        }

        return json_user_array;
    }

    User User::fromJSON(const std::string &str)
    {
        User user;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        user.id() = object->getValue<long>("id");
        user.first_name() = object->getValue<std::string>("first_name");
        user.last_name() = object->getValue<std::string>("last_name");
        user.email() = object->getValue<std::string>("email");
        user.phone_number() = object->getValue<std::string>("phone_number");
        user.login() = object->getValue<std::string>("login");
        user.password() = object->getValue<std::string>("password");

        return user;
    }

    std::vector<User> User::fromJSON_Array(const std::string &json_array_str) {
        User user;
        std::vector<User> users;

        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(json_array_str);
        Poco::JSON::Array::Ptr json_array = result.extract<Poco::JSON::Array::Ptr>();

        std::cout << "Begin inserting ...." << std::endl;
        size_t i{0};
        for (i = 0; i < json_array->size(); ++i)
        {
            Poco::JSON::Object::Ptr json_object = json_array->getObject(i);
            user.first_name() = json_object->getValue<std::string>("first_name");
            user.last_name() = json_object->getValue<std::string>("last_name");
            user.email() = json_object->getValue<std::string>("email");
            user.phone_number() = json_object->getValue<std::string>("phone_number");
            user.login() = json_object->getValue<std::string>("login");
            user.password() = json_object->getValue<std::string>("password");
            users.push_back(user);
        }

        return users;
    }
/*
    long User::get_last_inserted_user_id() 
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            long last_inserted_user_id;
            select << "SELECT `last_id` FROM `Last_inserted_user_id` WHERE `id` = 1;",
                into(last_inserted_user_id),
                range(0, 1); //  iterate over result set one row at a time

            select.execute();
            return last_inserted_user_id;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }
*/
/*
    void User::update_last_inserted_user_id(long new_id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement update(session);
            update << "UPDATE `Last_inserted_user_id` SET `last_id` = ? WHERE `id` = 1;",
                use(new_id);

            update.execute();
            return;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }
*/
    long User::get_id_for_insert_user() {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            long last_inserted_user_id;
            long updated_id;
            
            Poco::Data::Statement select(session);
            select << "SELECT `last_id` FROM `Last_inserted_user_id` WHERE `id` = 1;",
                into(last_inserted_user_id),
                range(0, 1);

            Poco::Data::Statement update(session);
            update << "UPDATE `Last_inserted_user_id` SET `last_id` = ? WHERE `id` = 1;",
                use(updated_id);

            Poco::Data::Statement start_transaction(session);
            start_transaction << "START TRANSACTION;";

            Poco::Data::Statement commit_transaction(session);
            commit_transaction << "COMMIT;";


            // std::cout << "У сессии есть возможности транзакции:" << session.canTransact() << std::endl;
            // std::cout << "Уровень изоляции транзакции:" << session.getTransactionIsolation() << std::endl;
            // std::cout << "READ_UNCOMMITTED:" << session.isTransactionIsolation(Poco::Data::Session::TRANSACTION_READ_UNCOMMITTED) << std::endl;
            // std::cout << "READ_COMMITED:" << session.isTransactionIsolation(Poco::Data::Session::TRANSACTION_READ_COMMITTED) << std::endl;
            // std::cout << "REPEATABLE_READ:" << session.isTransactionIsolation(Poco::Data::Session::TRANSACTION_REPEATABLE_READ) << std::endl;
            // std::cout << "SERIALIZABLE:" << session.isTransactionIsolation(Poco::Data::Session::TRANSACTION_SERIALIZABLE) << std::endl;
            // std::cout << "" << session. << std::endl;



            session.begin();

            // start_transaction.execute();
            // std::cout << "Транзакция идёт: " << session.isTransaction() << std::endl;

            select.execute();
        
            // std::cout << "read id: " << last_inserted_user_id << std::endl;
            // std::cout << "row have been locked\n";
            // std::this_thread::sleep_for(30000ms);

            updated_id = last_inserted_user_id + 1;
            update.execute();
            // commit_transaction.execute();
            session.commit();

            // std::cout << "transction have been commited" << std::endl;

            return updated_id;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }
    
    std::optional<User> User::read_by_id(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            User a;

            std::string sharding_hint = database::Database::sharding_hint_for_user(id);
            std::string select_str = "SELECT id, first_name, last_name, email, phone_number, login, password FROM User where id=?";
            select_str += sharding_hint;
            std::cout << select_str << std::endl;

            select << select_str,
                into(a._id),
                into(a._first_name),
                into(a._last_name),
                into(a._email),
                into(a._phone_number),
                into(a._login),
                into(a._password),
                use(id),
                range(0, 1); //  iterate over result set one row at a time

            select.execute();
            Poco::Data::RecordSet rs(select);
            if (rs.moveFirst()) return a;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
            
        }
        return {};
    }

    std::vector<User> User::read_all()
    {
        try
        {
            std::vector<User> result;
            std::vector<std::string> hints = database::Database::get_all_hints();

            std::vector<std::future<std::vector<database::User>>> futures;

            for (const std::string &hint : hints)
            {
                auto handle = std::async(
                    std::launch::async, 
                    [hint]() -> std::vector<database::User>
                    {
                        std::vector<database::User> result;
                        Poco::Data::Session session = database::Database::get().create_session();
                        Statement select(session);
                        std::string select_str = "SELECT id, first_name, last_name, email, phone_number, login, password FROM User";
                        select_str += hint;
                        select << select_str;

                        select.execute();
                        Poco::Data::RecordSet record_set(select);

                        bool more = record_set.moveFirst();
                        while (more)
                        {
                            database::User user;
                            user.id() = record_set[0].convert<long>(); 
                            user.first_name() = record_set[1].convert<std::string>(); 
                            user.last_name() = record_set[2].convert<std::string>(); 
                            user.email() = record_set[3].convert<std::string>(); 
                            user.phone_number() = record_set[5].convert<std::string>(); 
                            user.login() = record_set[5].convert<std::string>(); 
                            user.password() = record_set[6].convert<std::string>();
                            result.push_back(user); 
                            more = record_set.moveNext();
                        }
                        return result; 
                    });

                futures.emplace_back(std::move(handle));
            }

            for (auto &res : futures)
            {
                std::vector<database::User> v = res.get();
                std::copy(
                    std::begin(v),
                    std::end(v),
                    std::back_inserter(result)
                );
            }

            std::vector<database::User> result_distincted;
            std::sort(std::begin(result), std::end(result));
            long previous_id = -1;
            
            std::copy_if(
                std::begin(result),
                std::end(result),
                std::back_inserter(result_distincted),
                [&previous_id](database::User user)
                {
                    if (previous_id != user.get_id())
                    {
                        previous_id = user.get_id();
                        return true;
                    }
                    else 
                        return false;
                });
            
            return result_distincted;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::vector<User> User::search_by_login(std::string login)
    {
        try
        {
            std::vector<User> result;
            std::vector<std::string> hints = database::Database::get_all_hints();

            std::vector<std::future<std::vector<database::User>>> futures;

            for (const std::string &hint : hints)
            {
                auto handle = std::async(
                    std::launch::async, 
                    [hint, login]() -> std::vector<database::User>
                    {
                        std::vector<database::User> result;
                        Poco::Data::Session session = database::Database::get().create_session();
                        Statement select(session);
                        std::string select_str = "SELECT id, first_name, last_name, email, phone_number, login, password FROM User";
                        select_str += " WHERE login = '" + login + "'";
                        select_str += hint;
                        select << select_str;                        

                        select.execute();
                        Poco::Data::RecordSet record_set(select);

                        bool more = record_set.moveFirst();
                        while (more)
                        {
                            database::User user;
                            user.id() = record_set[0].convert<long>(); 
                            user.first_name() = record_set[1].convert<std::string>(); 
                            user.last_name() = record_set[2].convert<std::string>(); 
                            user.email() = record_set[3].convert<std::string>(); 
                            user.phone_number() = record_set[5].convert<std::string>(); 
                            user.login() = record_set[5].convert<std::string>(); 
                            user.password() = record_set[6].convert<std::string>();
                            result.push_back(user); 
                            more = record_set.moveNext();
                        }
                        return result; 
                    });

                futures.emplace_back(std::move(handle));
            }

            for (auto &res : futures)
            {
                std::vector<database::User> v = res.get();
                std::copy(
                    std::begin(v),
                    std::end(v),
                    std::back_inserter(result)
                );
            }

            std::vector<database::User> result_distincted;
            std::sort(std::begin(result), std::end(result));
            long previous_id = -1;

            std::copy_if(
                std::begin(result),
                std::end(result),
                std::back_inserter(result_distincted),
                [&previous_id](database::User user)
                {
                    if (previous_id != user.get_id())
                    {
                        previous_id = user.get_id();
                        return true;
                    }
                    else 
                        return false;
                });
            
            return result_distincted;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }
    
    std::vector<User> User::search(std::string first_name, std::string last_name)
    {
        try
        {
            std::vector<User> result;
            std::vector<std::string> hints = database::Database::get_all_hints();

            std::vector<std::future<std::vector<database::User>>> futures;

            for (const std::string &hint : hints)
            {
                auto handle = std::async(
                    std::launch::async, 
                    [hint, first_name, last_name]() -> std::vector<database::User>
                    {
                        std::vector<database::User> result;
                        Poco::Data::Session session = database::Database::get().create_session();
                        Statement select(session);
                        std::string select_str = "SELECT id, first_name, last_name, email, phone_number, login, password FROM User";
                        select_str += " WHERE first_name LIKE '" + first_name + "%'";
                        select_str += " AND last_name LIKE '" + last_name + "%';";
                        select_str += hint;
                        select << select_str;

                        select.execute();
                        Poco::Data::RecordSet record_set(select);

                        bool more = record_set.moveFirst();
                        while (more)
                        {
                            database::User user;
                            user.id() = record_set[0].convert<long>(); 
                            user.first_name() = record_set[1].convert<std::string>(); 
                            user.last_name() = record_set[2].convert<std::string>(); 
                            user.email() = record_set[3].convert<std::string>(); 
                            user.phone_number() = record_set[5].convert<std::string>(); 
                            user.login() = record_set[5].convert<std::string>(); 
                            user.password() = record_set[6].convert<std::string>();
                            result.push_back(user); 
                            more = record_set.moveNext();
                        }
                        return result; 
                    });

                futures.emplace_back(std::move(handle));
            }

            for (auto &res : futures)
            {
                std::vector<database::User> v = res.get();
                std::copy(
                    std::begin(v),
                    std::end(v),
                    std::back_inserter(result)
                );
            }

            std::vector<database::User> result_distincted;
            std::sort(std::begin(result), std::end(result));
            long previous_id = -1;
            
            std::copy_if(
                std::begin(result),
                std::end(result),
                std::back_inserter(result_distincted),
                [&previous_id](database::User user)
                {
                    if (previous_id != user.get_id())
                    {
                        previous_id = user.get_id();
                        return true;
                    }
                    else 
                        return false;
                });
            
            return result_distincted;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    bool User::check_user_exists_by_id(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            int count = 0;
            
            std::string sharding_hint = database::Database::sharding_hint_for_user(id);
            std::string select_str = "SELECT COUNT(*) FROM `User` WHERE id = ?";
            select_str += sharding_hint;
            std::cout << select_str << std::endl;

            select << select_str,
                into(count),
                use(id);
            
            select.execute();

            if (count > 0)
                return true;
            else
                return false;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void User::save_to_mysql()
    {

        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);
            
            // _id = database::User::get_id_for_insert_user();
            std::string sharding_hint = database::Database::sharding_hint_for_user(_id);
            

            std::string select_str = "INSERT INTO User (id, first_name,last_name,email,phone_number,login,password) VALUES(?, ?, ?, ?, ?, ?, ?)";
            select_str += sharding_hint;
            std::cout << select_str << std::endl;

            insert << select_str,
                use(_id),
                use(_first_name),
                use(_last_name),
                use(_email),
                use(_phone_number),
                use(_login),
                use(_password);

            insert.execute();

            std::cout << "inserted:" << _id << std::endl;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::optional<User> User::read_from_cache_by_id(long id)
    {

        try
        {
            std::string result;
            if (database::Cache::get().get(id, result))
                return fromJSON(result);
            else
                return std::optional<User>();
        }
        catch (std::exception& err)
        {
           // std::cerr << "error:" << err.what() << std::endl;
            return std::optional<User>();
        }
    }

     void User::save_to_cache()
    {
        std::stringstream ss;
        Poco::JSON::Stringifier::stringify(toJSON(), ss);
        std::string message = ss.str();
        database::Cache::get().put(_id, message);
    }

    void User::send_to_queue()
    {
        static cppkafka::Configuration config = {
            {"metadata.broker.list", Config::get().get_queue_host()},
            {"acks", "all"}};
        static cppkafka::Producer producer(config);
        static std::mutex mtx;
        static int message_key{0};
        using Hdr = cppkafka::MessageBuilder::HeaderType;

        std::lock_guard<std::mutex> lock(mtx);
        std::stringstream ss;
        Poco::JSON::Stringifier::stringify(toJSON(), ss);
        std::string message = ss.str();
        bool not_sent = true;

        cppkafka::MessageBuilder builder(Config::get().get_queue_topic());
        std::string mk = std::to_string(++message_key);
        builder.key(mk);                                       // set some key
        builder.header(Hdr{"producer_type", "user writer"});   // set some custom header
        builder.payload(message);                              // set message

        while (not_sent)
        {
            try
            {
                producer.produce(builder);
                not_sent = false;

                std::cout << mk << "->" << message << std::endl; 
            }
            catch (...)
            {
            }
        }
    }
    
    long User::get_id() const
    {
        return _id;
    }

    const std::string &User::get_first_name() const
    {
        return _first_name;
    }

    const std::string &User::get_last_name() const
    {
        return _last_name;
    }

    const std::string &User::get_email() const
    {
        return _email;
    }

    const std::string &User::get_phone_number() const
    {
        return _phone_number;
    }    
    
    const std::string &User::get_login() const
    {
        return _login;
    }

    const std::string &User::get_password() const
    {
        return _password;
    }

    long &User::id()
    {
        return _id;
    }

    std::string &User::first_name()
    {
        return _first_name;
    }

    std::string &User::last_name()
    {
        return _last_name;
    }

    std::string &User::email()
    {
        return _email;
    }

    std::string &User::phone_number()
    {
        return _phone_number;
    }    
    
    std::string &User::login()
    {
        return _login;
    }

    std::string &User::password()
    {
        return _password;
    }
}