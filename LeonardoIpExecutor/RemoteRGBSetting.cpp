#include "RemoteRGBSetting.hpp"
#include <QColor>

void RGBSetting::setRGBFromColor(const QString &color)
{
    QColor qColor(color);
    qColor.getRgb(&mRedValue, &mGreenValue, &mBlueValue);

    mRedValue = gammaCorrect(mRedValue);
    mGreenValue = gammaCorrect(mGreenValue);
    mBlueValue = gammaCorrect(mBlueValue);
}

int gammaCorrect(int value)
{
    return gamma_lut[value];
}
