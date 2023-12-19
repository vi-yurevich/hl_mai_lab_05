#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <memory>
#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/SessionPool.h>
#include <Poco/Data/Transaction.h>

namespace database{
    class Database{
        private:
            std::string _connection_string;
            std::unique_ptr<Poco::Data::SessionPool> _pool;
            Database();
        public:
            static Database& get();
            Poco::Data::Session create_session();
            Poco::Data::Transaction create_transaction(Poco::Data::Session& session);
            static size_t get_numbers_of_shard();
            static std::string sharding_hint_for_user(long id);
            static std::vector<std::string> get_all_hints();
    };
}
#endif