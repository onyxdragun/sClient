#ifndef UTIL_H
#define UTIL_H

#include <ostream>
#include <string>
#include <vector>
#include <sstream>
#include <QString>
#include <QMap>
#include <QTextDocument>

class util {

public:

    static QString processANSI(std::string);
    static int getColorCode(std::string);
    static std::string replaceMultiSpaces(std::string);
};

#endif // UTIL_H
