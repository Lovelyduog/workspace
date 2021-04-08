// #ifndef _MYSQL_PLUGIN_HPP
// #define _MYSQL_PLUGIN_HPP
#include <boost/asio.hpp>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/statement.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <boost/thread/thread.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>

#define MAX_DB_NUM 16
#define MAX_TATBLE_NUM 16

namespace db{
    class MysqlPlugin
    {
    public:
        static MysqlPlugin* Getinstance();
        void Init(const std::string& ip, const std::string& user, const std::string& password);
        void ConnectMasterDB();
        bool Connect(const std::string& host, const std::string& user, const std::string& passord);
        sql::ConnectPropertyVal CreateConnectPropertyVal(const std::string &strValue);
        sql::ConnectPropertyVal CreateConnectPropertyVal(int iValue);
        sql::ConnectPropertyVal CreateConnectPropertyValBool(bool bValue);
        void CreateConnectionProperties(const std::string& host, const std::string &strUserName, const std::string &strPassword, sql::ConnectOptionsMap &ConnectionProperties);
        bool execute(int dbPos, const std::string& command);
    private:
        MysqlPlugin();
        ~MysqlPlugin();
        void closeMasterDb();
        bool IsErrorCodeNeedReconnect(int iErrorCode);
        bool FuncHandleSqlException(int dbPos, const std::string &command, sql::SQLException &e);
        void reconnectDb(int iDBPos);
    private:
        boost::asio::io_service _work_io_service;
        boost::scoped_ptr<boost::asio::io_service::work> _work;
        boost::scoped_ptr<boost::thread> _work_thread;
        boost::asio::deadline_timer _check_deadline;
        
        //
        static MysqlPlugin* s_instance;
        sql::mysql::MySQL_Driver* _Drivers[MAX_DB_NUM];
        sql::Connection* _Connections[MAX_DB_NUM];
        boost::mutex _rwMutex[MAX_DB_NUM];
        // 状态
        bool _bInited;
        bool _bConnected;
        bool _bConnecting;
        bool _bClosing;

        //connection
        std::string _connection_ips[MAX_DB_NUM];
        std::string _user;
        std::string _password;

    };  

}//end namespace db
// #endif