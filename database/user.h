#ifndef USER_H
#define USER_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"
#include <optional>

namespace database
{
    class User{
        private:
            long _id;
            std::string _first_name;
            std::string _last_name;
            std::string _email;
            std::string _phone_number;
            std::string _login;
            std::string _password;

        public:
            bool operator< (const User &other) const;

            static void preload(const std::string &file_name);

            static User fromJSON(const std::string & str);
            static std::vector<User> fromJSON_Array(const std::string &json_array_str);

            long               get_id() const;
            const std::string &get_first_name() const;
            const std::string &get_last_name() const;
            const std::string &get_email() const;
            const std::string &get_phone_number() const;
            const std::string &get_login() const;
            const std::string &get_password() const;

            long        &id();
            std::string &first_name();
            std::string &last_name();
            std::string &email();
            std::string &phone_number();
            std::string &login();
            std::string &password();

            static long get_id_for_insert_user();

            static std::optional<User> read_by_id(long id);
            static std::vector<User> read_all();
            static std::vector<User> search(std::string first_name,std::string last_name);
            static std::vector<User> search_by_login(std::string login);
            static bool check_user_exists_by_id(long id);
            void save_to_mysql();

            void save_to_cache();
            static std::optional<User> read_from_cache_by_id(long id);
            void send_to_queue();

            Poco::JSON::Object::Ptr toJSON() const;
            static Poco::JSON::Array::Ptr vectorToJSON_and_hide_password(std::vector<User>& users);

    };
}

#endif