def fast_detection_agc_settings(phy, model):

    # PKD Settings
    phy.profile_outputs.AGC_GAINRANGE_PNGAINSTEP.override = 3  # 6dB adjustments
    phy.profile_outputs.AGC_AGCPERIOD0_PERIODHISTL.override = 0  # Enable IFPKD high period counter right after gain change
    phy.profile_outputs.AGC_AGCPERIOD0_MAXHICNTTHD.override = 8  # Maximum count of 8 (after settling time of 6)
    phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION0.override = 4
    phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION1.override = 5
    phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION2.override = 6
    phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION3.override = 7
    phy.profile_outputs.AGC_HICNTREGION1_HICNTREGION4.override = 8
    phy.profile_outputs.AGC_STEPDWN_STEPDWN0.override = 1
    phy.profile_outputs.AGC_STEPDWN_STEPDWN1.override = 2
    phy.profile_outputs.AGC_STEPDWN_STEPDWN2.override = 3
    phy.profile_outputs.AGC_STEPDWN_STEPDWN3.override = 3
    phy.profile_outputs.AGC_STEPDWN_STEPDWN4.override = 5
    phy.profile_outputs.AGC_STEPDWN_STEPDWN5.override = 5

    # AGC Period Settings
    phy.profile_outputs.AGC_CTRL7_SUBPERIOD.override = 1
    phy.profile_outputs.AGC_CTRL7_SUBDEN.override = 2
    phy.profile_outputs.AGC_CTRL7_SUBINT.override = 4
    phy.profile_outputs.AGC_CTRL7_SUBNUM.override = 0