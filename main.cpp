#include <iostream>
#include "db_plugin/mysql_plugin.hpp"

int main()
{
    db::MysqlPlugin *mysqlinter = db::MysqlPlugin::Getinstance();
    if (mysqlinter)
    {
        mysqlinter->Init("tcp://127.0.0.1:3306", "shenglish", "2285575104");
    }
    
    return 0;
}