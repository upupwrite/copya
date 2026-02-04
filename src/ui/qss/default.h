#pragma once
#include <QString>

namespace DefaultQss
{
inline const QString TOOLSET = R"(
QToolButton {
    text-align: left;
    padding: 5px;
    border: 1px solid #ddd;
    border-radius: 3px;
    margin: 2px;
}
QToolButton:hover {
    background-color: #f0f0f0;
    border-color: #bbb;
        })";
inline const QString TOOLSETNULL = "QToolButton { color: #2c3e50; }";
inline const QString FINDBOX = "";
inline const QString CLEANFIND = "";
}  // namespace DefaultQss
