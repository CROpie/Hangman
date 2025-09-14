#include "databaseService.h"

DatabaseService::DatabaseService(dbConn::Connector& dbConnector)
    : dbConnector(dbConnector) {}

std::string DatabaseService::getWordFromDB() {
    // number of words in the DB
    int N = 5;
    int num = getRandom(N);

    std::ostringstream sqlString;
    sqlString << "SELECT * FROM hangTable WHERE id = " << num << ";";
    json response = dbConnector.connectQueryClose(sqlString.str());
    std::string word = response[0].value("word", "");
    if (word.empty()) throw std::runtime_error("unable to retrieve word from DB");

    return word;
}

int DatabaseService::getRandom(int N) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, N);

    return dist(gen);
}
