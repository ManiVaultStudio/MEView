#pragma once

#include <QString>

class NeuronDescriptor;

class Query
{
public:
    QString loadQueryFromFile(std::string fileName);

    std::vector<NeuronDescriptor> send();
private:
    QString content;
};
