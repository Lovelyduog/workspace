#include "mysql_plugin.hpp"
#include <iostream>
#include<sstream>
namespace db{

    MysqlPlugin* MysqlPlugin::s_instance;

    MysqlPlugin::MysqlPlugin():
    _work_io_service(),_work(new boost::asio::io_service::work(_work_io_service)),
    _work_thread(new boost::thread(boost::bind(&boost::asio::io_service::run, &_work_io_service))),
    _check_deadline(_work_io_service)
    {
        _bInited = false;
        _bConnecting = false;
        _bConnected = false;
        _bClosing = false;

        for(int i = 0; i < MAX_DB_NUM; ++i)
        {
            _Drivers[i] = nullptr;
        }

        for(int i = 0; i < MAX_DB_NUM; ++i)
        {
            _Connections[i] = nullptr;
        }

    }

    void MysqlPlugin::Init(const std::string& ip, const std::string& user, const std::string& password)
    {
        if(_bInited)
        {
            return;
        }

        for(int i = 0; i < MAX_DB_NUM; ++i)
        {
            _connection_ips[i] = ip;
        }
        _user = user;
        _password = password;
        ConnectMasterDB();
        _bInited = true;
    }

    MysqlPlugin* MysqlPlugin::Getinstance()
    {
        if(s_instance == nullptr)
        {
            s_instance = new MysqlPlugin;
        }
        return s_instance;
    }

    void MysqlPlugin::ConnectMasterDB()
    {
        if (!_bConnected && !_bClosing)
        {
            _bConnecting = true;;
        }
        else
        {
            return;
        }
        
        if(_bConnected)
        {
            _bConnecting = false;
            return;
        }

        sql::Statement* state = nullptr;
        {  
            for (int i = 0; i < MAX_DB_NUM; ++i)
            {
                boost::mutex::scoped_lock lock(_rwMutex[i]);

                try
                {
                    if (_Drivers[i] != nullptr && !_Connections[i]->isClosed())
                    {
                        continue;
                    }

                    if(_Drivers[i] != nullptr)
                    {
                        _Drivers[i] = nullptr;
                    }

                    _Drivers[i] = sql::mysql::get_mysql_driver_instance();
                    if(_Drivers[i] == nullptr)
                    {
                        _bConnecting = false;
                        return;
                    }

                    if (_Connections[i] != nullptr)
                    {
                        if(!_Connections[i]->isClosed())
                        {
                            _Connections[i]->close();
                        }
                        delete _Connections[i];
                        _Connections[i] = nullptr;
                    }
                    
                    sql::ConnectOptionsMap ConnectionProperties;
                    CreateConnectionProperties(_connection_ips[i], _user, _password, ConnectionProperties);
                    _Connections[i] = _Drivers[i]->connect(ConnectionProperties);

                    if (_Connections[i] == nullptr)
                    {
                        _bConnecting = false;
                        return;
                    }
                    
                    state = _Connections[i]->createStatement();
                    if (state == nullptr)
                    {
                        _bConnecting = false;
                        return;
                    }

                    std::ostringstream osCreateDb;
                    osCreateDb << "create database if not exists Tx_" << i <<";";
                    state->execute(osCreateDb.str());
                    std::ostringstream osUseDb;
                    osUseDb << "use Tx_ " << i << ";";
                    state->execute(osUseDb.str());
                    state->close();
                    delete state;
                    state = nullptr;
                    
                }
                catch(sql::SQLException& e)
                {
                    if (nullptr != state)
                    {
                        state->close();
                        delete state;
                        state = nullptr;
                    }
                    _bConnected = false;
                }
                catch(...)
                {
                    if (nullptr != state)
                    {
                        state->close();
                        delete state;
                        state = nullptr;
                    }
                    _bConnected = false;
                }
                
            }
            
        }

        if(_bConnecting)
        {
            _bConnecting = false;
            _bConnected = true;
        } 
    }


    void MysqlPlugin::CreateConnectionProperties(const std::string& host, const std::string &strUserName, const std::string &strPassword, sql::ConnectOptionsMap &ConnectionProperties)
    {
        ConnectionProperties["hostName"] = CreateConnectPropertyVal(host);
        ConnectionProperties["userName"] = CreateConnectPropertyVal(strUserName);
        ConnectionProperties["password"] = CreateConnectPropertyVal(strPassword);
        int iTimeOut = 2;
        ConnectionProperties["OPT_WRITE_TIMEOUT"] = CreateConnectPropertyVal(iTimeOut);
        ConnectionProperties["OPT_READ_TIMEOUT"] = CreateConnectPropertyVal(iTimeOut);
        bool bCompress = true;
        ConnectionProperties["CLIENT_COMPRESS"] = CreateConnectPropertyValBool(bCompress);
        bool bMultiStatements = true;
        ConnectionProperties["CLIENT_MULTI_STATEMENTS"] = CreateConnectPropertyValBool(bMultiStatements);
    }


    sql::ConnectPropertyVal MysqlPlugin::CreateConnectPropertyVal(const std::string &strValue)
    {
        sql::ConnectPropertyVal val(strValue);
        return val;
    }

    sql::ConnectPropertyVal MysqlPlugin::CreateConnectPropertyVal(int iValue)
    {
        sql::ConnectPropertyVal val(iValue);
        return val;
    }

    sql::ConnectPropertyVal MysqlPlugin::CreateConnectPropertyValBool(bool bValue)
    {
        sql::ConnectPropertyVal val(bValue);
        return val;
    }
    bool MysqlPlugin::Connect(const std::string& host, const std::string& user, const std::string& passord)
    {
        
  

    }

    bool MysqlPlugin::execute(int dbPos, const std::string& command)
    {

    }

}//end namespace db
