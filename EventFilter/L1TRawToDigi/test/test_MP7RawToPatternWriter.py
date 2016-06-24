import FWCore.ParameterSet.Config as cms

process = cms.Process('L1')

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
# process.load('Configuration.StandardSequences.SimL1Emulator_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

# process.maxEvents = cms.untracked.PSet(
#     input = cms.untracked.int32(1000)
# )

source='pool'

if source == 'pool':

    dataDir='file:/home/ale/Development/cmssw/l1t_b/data/'
    # Input source
    process.source = cms.Source(
        "PoolSource",
        fileNames = cms.untracked.vstring([
            # "/store/data/Commissioning2016/MinimumBias/RAW/v1/000/268/955/00000/CE6DF691-ECFD-E511-9390-02163E01436A.root",
            # 'file:run268955_ls125-X-CE6DF691-ECFD-E511-9390-02163E01436A.root',
            # 'file:run268955_lsX-Y-147E0992-ECFD-E511-AFA0-02163E0144F2.root',
            # 'file:1E96FED5-F714-E611-8212-02163E0119DA.root',
            # 'dataDir+'run272728_Commissioning_7E3D1226-B113-E611-A4F3-02163E014121.root',
            # dataDir+'run272728_SingleElectron_3AD1C923-B113-E611-97D8-02163E011E29.root',
            dataDir+'run272728_ZeroBias_CA3F1246-B113-E611-89E9-02163E0118B7.root'
        ]),
        # eventsToProcess = cms.untracked.VEventRange('247215:1-247215:MAX')
        # skipEvents=cms.untracked.uint32(13000),
        # lumisToProcess = cms.untracked.VLuminosityBlockRange('272728:110-272728:max')
    )

elif source == 'streamer' :
    process.source = cms.Source (
        'NewEventStreamFileReader',
        # fileNames = cms.untracked.vstring (['file:testInput.dat']),
        fileNames = cms.untracked.vstring (['file:run273714_ls0002_streamPhysics_StorageManager.dat']),
        skipEvents=cms.untracked.uint32(0)
    )
#---

process.options = cms.untracked.PSet(
    SkipEvent = cms.untracked.vstring('ProductNotFound')
)


# Output definition

# process.output = cms.OutputModule(
#     "PoolOutputModule",
#     splitLevel = cms.untracked.int32(0),
#     eventAutoFlushCompressedSize = cms.untracked.int32(5242880),
#     outputCommands = cms.untracked.vstring("keep *",
# 					   "drop *_mix_*_*"),
#     fileName = cms.untracked.string('L1T_EDM.root'),
#     dataset = cms.untracked.PSet(
#         filterName = cms.untracked.string(''),
#         dataTier = cms.untracked.string('')
#     )
# )

# Additional output definition

# Other statements
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:startup', '')

process.MessageLogger.cerr.FwkReport.reportEvery = 1000

# TTree output file
process.load('CommonTools.UtilAlgos.TFileService_cfi')
process.TFileService.fileName = cms.string('l1t.root')

# user stuff

# dump raw data
process.dumpRaw = cms.EDAnalyzer( 
    'DumpFEDRawDataProduct',
    label = cms.untracked.string("rawDataCollector"),
    feds = cms.untracked.vint32 ( 1360 ),
    dumpPayload = cms.untracked.bool ( True )
)

# raw to digi
process.load('EventFilter.L1TRawToDigi.mp7RawToPatter_cfi')
process.mp7RawToPatternWriter.InputLabel = cms.InputTag('rawDataCollector')
process.mp7RawToPatternWriter.feds = cms.untracked.vint32 ( 1360 )
# process.mp7RawToPatternWriter.debug = cms.untracked.bool(True)

# Path and EndPath definitions
process.path = cms.Path(
    # process.dumpRaw +
    process.mp7RawToPatternWriter
)

# process.out = cms.EndPath(
#     process.output
# )
