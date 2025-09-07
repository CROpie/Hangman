#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include <cstdlib>
#include <sstream> // ostringstream
#include <random>

#include "dbconn.hpp"

#include "json.hpp"
using json = nlohmann::json;

class DatabaseService {
    public:
        dbConn::Connector& dbConnector;

        DatabaseService(dbConn::Connector& dbConnector);

        std::string getWordFromDB();

    private:
        int getRandom(int N);

};