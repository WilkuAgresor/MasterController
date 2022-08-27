#pragma once

#include <QObject>
#include <hardware/Pin.hpp>
#include <hardware/PinMapping.hpp>
#include <serialListener/SerialConnection.hpp>

class Components;

class GrandCentral : public QObject
{
    Q_OBJECT
public:
    GrandCentral(QObject* parent, Components* components);

    void setPinType(PinIdentifier id, PinType type);
    void addOrUpdatePinMapping(const PinMapping& mapping);

    void setInputState(int mappingId, LogicState state);
    void stateChangeNotif(PinIdentifier id, bool state);

    int getPinsGroupingId(const PinIdentifier& pin);
    int getInputPinsGroupingId(const PinIdentifier &pin);
    OutputState getPinDefaultOutputState(const PinIdentifier& pin);

    void reprovisionOutputValues();

    void setInitialized(bool value);
    bool isInitialized();

signals:
    void inputPinStateChangeNotif(PinIdentifier id, bool state);

private:
    void resetGrandCentralSettings();
    void setInOutMappings();

    void initializePins();
    std::uint16_t getPinIndex(const PinIdentifier& id);

    std::vector<Pin*> mPins;
    std::vector<PinMapping> mMappings;
    Components* mComponents;

    SerialConnection* mSerialConnection;

    bool mIsInitialized = true;
};
