#include "CondFormats/Common/interface/SerializationTest.h"

#include "CondFormats/Luminosity/interface/Serialization.h"

int main()
{
    testSerialization<lumi::BunchCrossingInfo>();
    testSerialization<lumi::HLTInfo>();
    testSerialization<lumi::LumiSectionData>();
    testSerialization<lumi::TriggerInfo>();
    testSerialization<std::vector<lumi::BunchCrossingInfo>>();
    testSerialization<std::vector<lumi::HLTInfo>>();
    testSerialization<std::vector<lumi::TriggerInfo>>();

    return 0;
}
