import FWCore.ParameterSet.Config as cms

mp7RawToPatternWriter = cms.EDAnalyzer(
    "MP7RawToPatternWriter",
    InputLabel = cms.InputTag("l1tDigiToRaw"),
    FedIds = cms.vint32(1360),
    lenSlinkHeader = cms.untracked.int32(8),
    lenSlinkTrailer = cms.untracked.int32(8),
    lenAMCHeader = cms.untracked.int32(8),
    lenAMCTrailer = cms.untracked.int32(0),
    lenAMC13Header = cms.untracked.int32(8),
    lenAMC13Trailer = cms.untracked.int32(8)
)
