#include "MorphologyDescription.h"

#include <unordered_map>

#include <QDebug>

std::vector<QString> propertyList = {
    "apical_dendrite_bias_x",
    "apical_dendrite_bias_y",
    "apical_dendrite_depth_pc_0",
    "apical_dendrite_depth_pc_1",
    "apical_dendrite_depth_pc_2",
    "apical_dendrite_depth_pc_3",
    "apical_dendrite_early_branch_path",
    "apical_dendrite_emd_with_basal_dendrite",
    "apical_dendrite_extent_x",
    "apical_dendrite_extent_y",
    "apical_dendrite_frac_above_basal_dendrite",
    "apical_dendrite_frac_below_basal_dendrite",
    "apical_dendrite_frac_intersect_basal_dendrite",
    "apical_dendrite_max_branch_order",
    "apical_dendrite_max_euclidean_distance",
    "apical_dendrite_max_path_distance",
    "apical_dendrite_mean_contraction",
    "apical_dendrite_mean_diameter",
    "apical_dendrite_mean_moments_along_max_distance_projection",
    "apical_dendrite_num_branches",
    "apical_dendrite_num_outer_bifurcations",
    "apical_dendrite_soma_percentile_x",
    "apical_dendrite_soma_percentile_y",
    "apical_dendrite_std_moments_along_max_distance_projection",
    "apical_dendrite_total_length",
    "apical_dendrite_total_surface_area",
    "axon_exit_distance",
    "axon_exit_theta",
    "basal_dendrite_bias_x",
    "basal_dendrite_bias_y",
    "basal_dendrite_calculate_number_of_stems",
    "basal_dendrite_extent_x",
    "basal_dendrite_extent_y",
    "basal_dendrite_frac_above_apical_dendrite",
    "basal_dendrite_frac_below_apical_dendrite",
    "basal_dendrite_frac_intersect_apical_dendrite",
    "basal_dendrite_max_branch_order",
    "basal_dendrite_max_euclidean_distance",
    "basal_dendrite_max_path_distance",
    "basal_dendrite_mean_contraction",
    "basal_dendrite_mean_diameter",
    "basal_dendrite_num_branches",
    "basal_dendrite_soma_percentile_x",
    "basal_dendrite_soma_percentile_y",
    "basal_dendrite_stem_exit_down",
    "basal_dendrite_stem_exit_side",
    "basal_dendrite_stem_exit_up",
    "basal_dendrite_total_length",
    "basal_dendrite_total_surface_area",
    "soma_aligned_dist_from_pia",
    "soma_surface_area"
};

void MorphologyDescription::setData(const std::vector<QString>& header, const std::vector<float>& values)
{
    // Make a header-value map
    std::unordered_map<QString, float> headerValueMap;

    // Populate it
    for (int i = 0; i < header.size(); i++)
    {
        headerValueMap[header[i]] = values[i];
    }

    for (int i = 0; i < header.size(); i++)
    {
        const QString& columnName = header[i];
        float val = values[i];

        // Apical Dendrite
        if (columnName == "apical_dendrite_bias_x") { _apicalDendriteDesc.bias.x = val; continue; }
        if (columnName == "apical_dendrite_bias_y") { _apicalDendriteDesc.bias.y = val; continue; }
        if (columnName == "apical_dendrite_depth_pc_0") { _apicalDendriteDesc.depthPc[0] = val; continue; }
        if (columnName == "apical_dendrite_depth_pc_1") { _apicalDendriteDesc.depthPc[1] = val; continue; }
        if (columnName == "apical_dendrite_depth_pc_2") { _apicalDendriteDesc.depthPc[2] = val; continue; }
        if (columnName == "apical_dendrite_depth_pc_3") { _apicalDendriteDesc.depthPc[3] = val; continue; }
        if (columnName == "apical_dendrite_early_branch_path") { _apicalDendriteDesc.earlyBranchPath = val; continue; }
        if (columnName == "apical_dendrite_emd_with_basal_dendrite") { _apicalDendriteDesc.emdWithBasalDendrite = val; continue; }
        if (columnName == "apical_dendrite_extent_x") { _apicalDendriteDesc.extent.x = val; continue; }
        if (columnName == "apical_dendrite_extent_y") { _apicalDendriteDesc.extent.y = val; continue; }
        if (columnName == "apical_dendrite_frac_above_basal_dendrite") { _apicalDendriteDesc.fracAboveBasalDendrite = val; continue; }
        if (columnName == "apical_dendrite_frac_below_basal_dendrite") { _apicalDendriteDesc.fracBelowBasalDendrite = val; continue; }
        if (columnName == "apical_dendrite_frac_intersect_basal_dendrite") { _apicalDendriteDesc.fracIntersectBasalDendrite = val; continue; }
        if (columnName == "apical_dendrite_max_branch_order") { _apicalDendriteDesc.maxBranchOrder = val; continue; }
        if (columnName == "apical_dendrite_max_euclidean_distance") { _apicalDendriteDesc.maxEuclideanDistance = val; continue; }
        if (columnName == "apical_dendrite_max_path_distance") { _apicalDendriteDesc.maxPathDistance = val; continue; }
        if (columnName == "apical_dendrite_mean_contraction") { _apicalDendriteDesc.meanContraction = val; continue; }
        if (columnName == "apical_dendrite_mean_diameter") { _apicalDendriteDesc.meanDiameter = val; continue; }
        if (columnName == "apical_dendrite_mean_moments_along_max_distance_projection") { _apicalDendriteDesc.meanMomentsAlongMaxDistanceProjection = val; continue; }
        if (columnName == "apical_dendrite_num_branches") { _apicalDendriteDesc.numBranches = val; continue; }
        if (columnName == "apical_dendrite_num_outer_bifurcations") { _apicalDendriteDesc.numOuterBifurcations = val; continue; }
        if (columnName == "apical_dendrite_soma_percentile_x") { _apicalDendriteDesc.somaPercentile.x = val; continue; }
        if (columnName == "apical_dendrite_soma_percentile_y") { _apicalDendriteDesc.somaPercentile.y = val; continue; }
        if (columnName == "apical_dendrite_std_moments_along_max_distance_projection") { _apicalDendriteDesc.stdMomentsAlongMaxDistanceProjection = val; continue; }
        if (columnName == "apical_dendrite_total_length") { _apicalDendriteDesc.totalLength = val; continue; }
        if (columnName == "apical_dendrite_total_surface_area") { _apicalDendriteDesc.totalSurfaceArea = val; continue; }

        // Axon
        //if (columnName == "axon_exit_distance") _apicalDendriteDesc.bias.x = val;
        //if (columnName == "axon_exit_theta") _apicalDendriteDesc.bias.x = val;

        // Basal Dendrite
        if (columnName == "basal_dendrite_bias_x") { _basalDendriteDesc.bias.x = val; continue; }
        if (columnName == "basal_dendrite_bias_y") { _basalDendriteDesc.bias.y = val; continue; }
        if (columnName == "basal_dendrite_calculate_number_of_stems") { _basalDendriteDesc.calculateNumberOfStems = val; continue; }
        if (columnName == "basal_dendrite_extent_x") { _basalDendriteDesc.extent.x = val; continue; }
        if (columnName == "basal_dendrite_extent_y") { _basalDendriteDesc.extent.y = val; continue; }
        if (columnName == "basal_dendrite_frac_above_apical_dendrite") { _basalDendriteDesc.fracAboveApicalDendrite = val; continue; }
        if (columnName == "basal_dendrite_frac_below_apical_dendrite") { _basalDendriteDesc.fracBelowApicalDendrite = val; continue; }
        if (columnName == "basal_dendrite_frac_intersect_apical_dendrite") { _basalDendriteDesc.fracIntersectApicalDendrite = val; continue; }
        if (columnName == "basal_dendrite_max_branch_order") { _basalDendriteDesc.maxBranchOrder = val; continue; }
        if (columnName == "basal_dendrite_max_euclidean_distance") { _basalDendriteDesc.maxEuclideanDistance = val; continue; }
        if (columnName == "basal_dendrite_max_path_distance") { _basalDendriteDesc.maxPathDistance = val; continue; }
        if (columnName == "basal_dendrite_mean_contraction") { _basalDendriteDesc.meanContraction = val; continue; }
        if (columnName == "basal_dendrite_mean_diameter") { _basalDendriteDesc.meanDiameter = val; continue; }
        if (columnName == "basal_dendrite_num_branches") { _basalDendriteDesc.numBranches = val; continue; }
        if (columnName == "basal_dendrite_soma_percentile_x") { _basalDendriteDesc.somaPercentile.x = val; continue; }
        if (columnName == "basal_dendrite_soma_percentile_y") { _basalDendriteDesc.somaPercentile.y = val; continue; }
        if (columnName == "basal_dendrite_stem_exit_down") { _basalDendriteDesc.stemExitDown = val; continue; }
        if (columnName == "basal_dendrite_stem_exit_side") { _basalDendriteDesc.stemExitSide = val; continue; }
        if (columnName == "basal_dendrite_stem_exit_up") { _basalDendriteDesc.stemExitUp = val; continue; }
        if (columnName == "basal_dendrite_total_length") { _basalDendriteDesc.totalLength = val; continue; }
        if (columnName == "basal_dendrite_total_surface_area") { _basalDendriteDesc.totalSurfaceArea = val; continue; }

        // Soma
        //if ("soma_aligned_dist_from_pia") _apicalDendriteDesc.bias.x = val;
        //if ("soma_surface_area") _apicalDendriteDesc.bias.x = val;
    }
}
