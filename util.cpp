#include "util.h"
#include <QString>
#include <QDebug>

// processANSI()
//
// string  - [IN]  Incoming message from server
// QString - [OUT] Formated with ANSI codes
//
// Convert any present ANSI colour codes so they are converted
// into HTML and displayed in the text area
QString util::highlightStr(std::string &str, highlightTypes type)
{
    std::string format;

    if (type == highlightTypes::TELL)
    {
        format = "<span style=\"color: #db4c2e;\">";
    }
    else if (type == highlightTypes::SHOUT)
    {
        format = "<span style=\"font-weight:bold;\">";
    }

    size_t startPos = str.find_first_of("\r\n");

    if (startPos == std::string::npos)
    {
        startPos = 0;
    }
    str.insert(startPos, format);

    size_t endPos = str.find_last_of("\r\n");
    if (endPos == std::string::npos)
    {
        endPos = str.length();
    }
    str.insert(endPos, "</span>");
    return str.c_str();
}
// processANSI()
//
// string  - [IN]  Incoming message from server
// QString - [OUT] Formated with ANSI codes
//
// Convert any present ANSI colour codes so they are converted 
// into HTML and displayed in the text area
QString util::processANSI(std::string &str)
{
    std::string ESC = "\x1B\x5B";
    std::string END = "\x6D";
    std::string CRLF = "\x0D\x0A";
    QMap<int, QString> mHTMLColors;
    QString tmp = str.c_str();
    size_t len = 0;
    size_t pos = 0;
    size_t end = 0;

    int color = 0;

    // Escape any HTML type characters
    tmp = tmp.toHtmlEscaped();
    str = tmp.toStdString();

    mHTMLColors.insert(30, "#fff"); // Needs to be white as our background is black
    mHTMLColors.insert(31, "#f00");
    mHTMLColors.insert(32, "#0f0");
    mHTMLColors.insert(33, "#ff0");
    mHTMLColors.insert(34, "#00f");
    mHTMLColors.insert(35, "#f0f");
    mHTMLColors.insert(36, "#0ff");

    // Let's get all the colors dealt with
    pos = str.find(ESC, 0);
    end = str.find(END, pos);

    while (pos != std::string::npos)
    {
        std::string s;
        len = end - pos;
        std::string code = str.substr(pos, len+1);
        color = getColorCode(code);
        if (code.compare("\x1B[0m") == 0)
        {
            s = "</span>";
        }
        else
        {
            s = "<span style=\"color:"+ mHTMLColors.value(color).toStdString() +"\">";
        }
        str.replace(pos, code.length(), s);

        pos = str.find(ESC, pos + 1);
        end = str.find(END, pos);
    }

    // process all spaces
    str = replaceMultiSpaces(str);

    // process any \n\r characters
    pos = str.find(CRLF, 0);
    while (pos != std::string::npos)
    {
        str.replace(pos, CRLF.length(), "<br />");
        pos = str.find(CRLF, pos +1);
    }
    // process all telnet codes
    str = replaceTelnetCodes(str);

    return str.c_str();
}

// getColorCode()
//
// string - [IN]   Isolate the ANSI colour code
// int    - [OUT] 
//
int util::getColorCode(std::string &code)
{
    std::string SEMICOLON = "\x3B";
    std::string ESC = "\x5B";
    std::string CHAR_M = "\x6D";  // 'm' in hex
    int color = 30; // Code for reset color

    size_t pos = code.find(SEMICOLON, 0);
    size_t end = code.find(CHAR_M, 0);
    if (pos != std::string::npos)
    {
        color = atoi(code.substr(pos+1, 2).c_str());
    }
    else
    {
        pos = code.find(ESC, 0);
        if (pos != std::string::npos) {
            color = atoi(code.substr(pos+1, end).c_str());
        }
        else
        {
            color = 30;
        }
    }
    return color;
}

// replaceMultiSpaces()
//
// string [IN]  - String to parse
// string [OUT] - String modified without multie whitespace
//
// Sometimes there are double spaces coming from the server
// We need to remove the extra whitespace or formating is messed up
std::string util::replaceMultiSpaces(std::string &str)
{
    std::string twoNBSP = "&nbsp;&nbsp;";
    std::string NBSP = "&nbsp;";
    std::string MultiSpace = "\x20\x20";
    std::string NBSPspace = "&nbsp ";
    size_t pos = str.find(MultiSpace, 0);

    while (pos != std::string::npos)
    {
        str.replace(pos, MultiSpace.length(), twoNBSP);
        pos = str.find(MultiSpace, pos+1);
    }

    pos = str.find(NBSPspace, 0);
    while (pos != std::string::npos)
    {
        str.replace(pos, NBSPspace.length(), twoNBSP);
        pos = str.find(NBSPspace, pos+1);
    }

    return str;
}

// replaceTelnetCodes()
//
// string [IN]  - Incoming text from server
// string [OUT] - Formatted text with telnet codes removed
//
// Telnet codes can play havoc with the display, we we will iterate
// over them and return a cleaner string
std::string util::replaceTelnetCodes(std::string &str)
{
    std::vector<std::string>escapeCodes;
    escapeCodes.push_back("\x0D\x0A\x1B");
    escapeCodes.push_back("\xFF\xFB\x01");
    escapeCodes.push_back("\x1F\x1F\x1F");
    escapeCodes.push_back("\xFF\xFC\x01");
    escapeCodes.push_back("\xFF\xFC\x03");
    escapeCodes.push_back("\xEF\xBF\xBD");   // Replacement Character
    std::vector<std::string>::iterator It;
    size_t pos = 0;

    for (It = escapeCodes.begin(); It != escapeCodes.end(); ++It)
    {
        pos = str.find(*It, 0);
        while (pos != std::string::npos)
        {
            str.replace(pos, (*It).length(), "");
            pos = str.find(*It, pos + 1);
        }
    }

    return str;
}
