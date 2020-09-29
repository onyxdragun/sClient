#ifndef UTIL_H
#define UTIL_H

#include <ostream>
#include <string>
#include <vector>
#include <sstream>
#include <QString>
#include <QMap>
#include <QTextDocument>

class util : public QObject {
    Q_OBJECT

public:

    enum highlightTypes {
        TELL = 0,
        SHOUT
    };

    Q_ENUM(highlightTypes)

    static QString processANSI(std::string&);
    static QString highlightStr(std::string&, highlightTypes);
    static int getColorCode(std::string&);
    static std::string replaceMultiSpaces(std::string&);
    static std::string replaceTelnetCodes(std::string&);

};

#endif // UTIL_H
