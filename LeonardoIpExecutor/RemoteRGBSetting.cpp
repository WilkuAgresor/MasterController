#include "RemoteRGBSetting.hpp"
#include <QColor>

void RGBSetting::setRGBFromColor(const QString &color)
{
    qDebug() << "color: "<<color;

    QColor qColor(color);
    qColor.getRgb(&mRedValue, &mGreenValue, &mBlueValue);

    qDebug() << "R: "<<mRedValue <<" G:"<<mGreenValue<<" B:"<<mBlueValue;

    mRedValue = gammaCorrect(mRedValue, false);
    mGreenValue = gammaCorrect(mGreenValue, false);
    mBlueValue = gammaCorrect(mBlueValue, false);
}

std::vector<RemotePinSetting> RGBSetting::getRedPins()
{
    auto settings = mRedPins;
    for(auto& setting: settings)
    {
        setting.mValue = mRedValue;
    }
    return settings;
}

std::vector<RemotePinSetting> RGBSetting::getGreenPins()
{
    auto settings = mGreenPins;
    for(auto& setting: settings)
    {
        setting.mValue = mGreenValue;
    }
    return settings;
}

std::vector<RemotePinSetting> RGBSetting::getBluePins()
{
    auto settings = mBluePins;
    for(auto& setting: settings)
    {
        setting.mValue = mBlueValue;
    }
    return settings;
}

int gammaCorrect(int value, bool percentage)
{
    auto realValue = value;
    if(percentage)
    {
       realValue = 256 * value / 100;
    }

    qDebug() << "gamma corrected: "<<value<<" to: "<<gamma_lut[realValue];

    return gamma_lut[realValue];
}
