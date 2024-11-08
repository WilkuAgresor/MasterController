#pragma once
#include <hardware/PinIdentifier.hpp>

int gammaCorrect(int value, bool percentage = true);

struct RemotePinSetting
{
    QString mControllerIpAddr;
    int mControllerPort;

    PinIdentifier mPin;
    int mValue = 0;

    int mOnSetting = 0;
    int mOffSetting = 0;
};

struct RGBSetting
{
    void setRGBFromColor(const QString& color);

    int mRedValue = 0;
    int mGreenValue = 0;
    int mBlueValue = 0;

    std::vector<RemotePinSetting> getRedPins();
    std::vector<RemotePinSetting> getGreenPins();
    std::vector<RemotePinSetting> getBluePins();


    std::vector<RemotePinSetting> mRedPins;
    std::vector<RemotePinSetting> mGreenPins;
    std::vector<RemotePinSetting> mBluePins;
};

// Gamma brightness lookup table <https://victornpb.github.io/gamma-table-generator>
// gamma = 2.00 steps = 256 range = 0-4096
const uint16_t gamma_lut[256] = {
     0,   0,   0,   1,   1,   2,   2,   3,   4,   5,   6,   8,   9,  11,  12,  14,
    16,  18,  20,  23,  25,  28,  30,  33,  36,  39,  43,  46,  49,  53,  57,  61,
    65,  69,  73,  77,  82,  86,  91,  96, 101, 106, 111, 116, 122, 128, 133, 139,
   145, 151, 157, 164, 170, 177, 184, 191, 198, 205, 212, 219, 227, 234, 242, 250,
   258, 266, 274, 283, 291, 300, 309, 318, 327, 336, 345, 354, 364, 373, 383, 393,
   403, 413, 424, 434, 444, 455, 466, 477, 488, 499, 510, 522, 533, 545, 557, 568,
   581, 593, 605, 617, 630, 643, 655, 668, 681, 694, 708, 721, 735, 748, 762, 776,
   790, 804, 819, 833, 848, 862, 877, 892, 907, 922, 938, 953, 969, 984,1000,1016,
  1032,1048,1065,1081,1098,1114,1131,1148,1165,1182,1200,1217,1235,1252,1270,1288,
  1306,1324,1343,1361,1380,1398,1417,1436,1455,1475,1494,1513,1533,1553,1573,1592,
  1613,1633,1653,1674,1694,1715,1736,1757,1778,1799,1820,1842,1864,1885,1907,1929,
  1951,1973,1996,2018,2041,2064,2087,2110,2133,2156,2179,2203,2226,2250,2274,2298,
  2322,2346,2371,2395,2420,2445,2470,2495,2520,2545,2570,2596,2621,2647,2673,2699,
  2725,2752,2778,2804,2831,2858,2885,2912,2939,2966,2994,3021,3049,3077,3104,3132,
  3161,3189,3217,3246,3275,3303,3332,3361,3390,3420,3449,3479,3508,3538,3568,3598,
  3628,3659,3689,3720,3750,3781,3812,3843,3874,3906,3937,3969,4000,4032,4064,4096
  };
