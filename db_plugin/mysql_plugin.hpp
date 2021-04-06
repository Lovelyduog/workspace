// #ifndef _MYSQL_PLUGIN_HPP
// #define _MYSQL_PLUGIN_HPP

#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/statement.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
using namespace sql;

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace db{
    class MysqlPlugin
    {
    public:
        static MysqlPlugin* Getinstance();
        bool Connect(const std::string& host, const std::string& user, const std::string& passord);
    private:
        MysqlPlugin();
    private:
        static MysqlPlugin* s_instance;
        sql::mysql::MySQL_Driver* _Driver;
        sql::Connection* _Connection;
    };  

}//end namespace db
// #endif