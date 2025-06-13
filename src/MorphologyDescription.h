#pragma once

#include <graphics/Vector2f.h>

#include <QStringList>

#include <array>

class ApicalDendriteDescription
{
public:
    mv::Vector2f bias;
    std::array<float, 4> depthPc;
    float earlyBranchPath;
    float emdWithBasalDendrite;
    mv::Vector2f extent;
    float fracAboveBasalDendrite;
    float fracBelowBasalDendrite;
    float fracIntersectBasalDendrite;
    int maxBranchOrder;
    float maxEuclideanDistance;
    float maxPathDistance;
    float meanContraction;
    float meanDiameter;
    float meanMomentsAlongMaxDistanceProjection;
    int numBranches;
    float numOuterBifurcations;
    mv::Vector2f somaPercentile;
    float stdMomentsAlongMaxDistanceProjection;
    float totalLength;
    float totalSurfaceArea;
};

class BasalDendriteDescription
{
public:
    mv::Vector2f bias;
    int calculateNumberOfStems;
    mv::Vector2f extent;
    float fracAboveApicalDendrite;
    float fracBelowApicalDendrite;
    float fracIntersectApicalDendrite;
    int maxBranchOrder;
    float maxEuclideanDistance;
    float maxPathDistance;
    float meanContraction;
    float meanDiameter;
    int numBranches;
    mv::Vector2f somaPercentile;
    float stemExitDown;
    float stemExitSide;
    float stemExitUp;
    float totalLength;
    float totalSurfaceArea;
};

class MorphologyDescription
{
public:
    void setData(const std::vector<QString>& header, const std::vector<float>& values);

    const ApicalDendriteDescription& getApicalDendriteDescription() { return _apicalDendriteDesc; }
    const BasalDendriteDescription& getBasalDendriteDescription() { return _basalDendriteDesc; }

private:
    ApicalDendriteDescription   _apicalDendriteDesc;
    BasalDendriteDescription    _basalDendriteDesc;
};
