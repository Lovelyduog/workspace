#include "mysql_plugin.hpp"
#include <iostream>

namespace db{

    MysqlPlugin* MysqlPlugin::s_instance;

    MysqlPlugin::MysqlPlugin()
    {

    }

    MysqlPlugin* MysqlPlugin::Getinstance()
    {
        if(s_instance == nullptr)
        {
            s_instance = new MysqlPlugin();
        }
    }

    bool MysqlPlugin::Connect(const std::string& host, const std::string& user, const std::string& passord)
    {
        _Driver = sql::mysql::get_mysql_driver_instance();
        if(_Driver)
        {
            _Connection = _Driver->connect(host, user, passord);

            if (_Connection)
            {
                sql::Statement *state = _Connection->createStatement();
                if (state)
                {
                    std::cout << "ready for execute operation" << std::endl;
                }
                
            }
            
        }
        return true;
    }

}//end namespace db
