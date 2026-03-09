#pragma once

class Cell;
class QJsonDocument;

class CellCardSerializer
{
public:
    void Serialize(const Cell& cell, QJsonDocument& outputDoc);
private:

};
