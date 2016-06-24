// -*- C++ -*-
//
// Package:    L1Trigger/L1TCalorimeter
// Class:      MP7RawToPatternWriter
// 
/**\class MP7RawToPatternWriter MP7RawToPatternWriter.cc L1Trigger/L1TCalorimeter/plugins/MP7RawToPatternWriter.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  James Brooke
//         Created:  Tue, 11 Mar 2014 14:55:45 GMT
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/FEDRawData/interface/FEDHeader.h"
#include "DataFormats/FEDRawData/interface/FEDNumbering.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/FEDRawData/interface/FEDTrailer.h"

#include "EventFilter/L1TRawToDigi/interface/AMC13Spec.h"
#include "EventFilter/L1TRawToDigi/interface/Block.h"

#include <EventFilter/FEDInterface/interface/FED1024.h>

#include <fstream>
#include <iostream>
#include <iomanip>

//
// class declaration
//

class MP7RawToPatternWriter : public edm::EDAnalyzer {
public:
  explicit MP7RawToPatternWriter(const edm::ParameterSet&);
  ~MP7RawToPatternWriter();
  
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
  
private:
  virtual void beginJob() override;
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;
  
  //virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
  //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
  //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
  
  // ----------member data ---------------------------
  edm::EDGetTokenT<FEDRawDataCollection> fedData_;
  std::vector<int> fedIds_;
  
  // header and trailer sizes in chars
  int slinkHeaderSize_;
  int slinkTrailerSize_;
  int amcHeaderSize_;
  int amcTrailerSize_;
  int amc13HeaderSize_;
  int amc13TrailerSize_;

  int warns_;
  // -------------------------------------------------
  edm::EDGetToken m_towerToken;

  std::string filename_;

  // constants
  unsigned nChan_;  // number of channels per quad
  unsigned nQuad_;
  unsigned nLink_;
  unsigned nHeaderFrames_;
  unsigned nPayloadFrames_;
  unsigned nClearFrames_;
  unsigned nFrame_;
  
  // data arranged by link and frame
  std::vector< std::vector<int> > data_;

  // data valid flags (just one per frame for now)
  std::vector<int> dataValid_;

  // map of towers onto links/frames
  std::map< int, int > map_;

};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
MP7RawToPatternWriter::MP7RawToPatternWriter(const edm::ParameterSet& iConfig) :
  fedIds_(iConfig.getParameter<std::vector<int>>("FedIds")) {

    fedData_ = consumes<FEDRawDataCollection>(iConfig.getParameter<edm::InputTag>("InputLabel"));

    slinkHeaderSize_ = iConfig.getUntrackedParameter<int>("lenSlinkHeader");
    slinkTrailerSize_ = iConfig.getUntrackedParameter<int>("lenSlinkTrailer");
    amcHeaderSize_ = iConfig.getUntrackedParameter<int>("lenAMCHeader");
    amcTrailerSize_ = iConfig.getUntrackedParameter<int>("lenAMCTrailer");
    amc13HeaderSize_ = iConfig.getUntrackedParameter<int>("lenAMC13Header");
    amc13TrailerSize_ = iConfig.getUntrackedParameter<int>("lenAMC13Trailer");

    //--------------------------------------------------------------

   //now do what ever initialization is needed

  // register what you consume and keep token for later access:
  // m_towerToken   = consumes<l1t::CaloTowerBxCollection>  (iConfig.getParameter<edm::InputTag>("towerToken"));

  filename_ = iConfig.getUntrackedParameter<std::string>("filename", "pattern.txt");

  nChan_ = iConfig.getUntrackedParameter<unsigned>("nChanPerQuad", 4);
  nQuad_ = iConfig.getUntrackedParameter<unsigned>("nQuads", 18);

  nHeaderFrames_ = iConfig.getUntrackedParameter<unsigned>("nHeaderFrames", 1);
  nPayloadFrames_ = iConfig.getUntrackedParameter<unsigned>("nPayloadFrames", 39);
  nClearFrames_ = iConfig.getUntrackedParameter<unsigned>("nClearFrames", 6);
  nFrame_ = 0;

  nLink_ = nChan_ * nQuad_;
  data_.resize(nLink_);
  LogDebug("L1TDebug") << "Preparing for " << nLink_ << " links" << std::endl;

}


MP7RawToPatternWriter::~MP7RawToPatternWriter()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called for each event  ------------
void
MP7RawToPatternWriter::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

  // std::unique_ptr<UnpackerCollections> coll = prov_->getCollections(event);

  edm::Handle<FEDRawDataCollection> feds;
  iEvent.getByToken(fedData_, feds);

  if (!feds.isValid()) {
     LogError("L1T") << "Cannot unpack: no FEDRawDataCollection found";
     return;
  }


  const FEDRawData& tcdsRcd = feds->FEDData(1024);
  evf::evtn::TCDSRecord record((unsigned char *)tcdsRcd.data());

  for (const auto& fedId: fedIds_) {
    const FEDRawData& l1tRcd = feds->FEDData(fedId);
    LogDebug("L1T") << "Found FEDRawDataCollection with ID " << fedId << " and size " << l1tRcd.size();

    if ((int) l1tRcd.size() < slinkHeaderSize_ + slinkTrailerSize_ + amc13HeaderSize_ + amc13TrailerSize_ + amcHeaderSize_ + amcTrailerSize_) {
      if (l1tRcd.size() > 0) {
        LogError("L1T") << "Cannot unpack: invalid L1T raw data (size = "
          << l1tRcd.size() << ") for ID " << fedId << ". Returning empty collections!";
      } else if (warns_ < 5) {
        warns_++;
        LogWarning("L1T") << "Cannot unpack: empty L1T raw data (size = "
          << l1tRcd.size() << ") for ID " << fedId << ". Returning empty collections!";
      }
      continue;
    }
  

    const unsigned char *data = l1tRcd.data();
    FEDHeader header(data);

    if (header.check()) {
      LogDebug("L1T") << "Found SLink header:"
         << " Trigger type " << header.triggerType()
         << " L1 event ID " << header.lvl1ID()
         << " BX Number " << header.bxID()
         << " FED source " << header.sourceID()
         << " FED version " << header.version();
    } else {
      LogWarning("L1T") << "Did not find a SLink header!";
    }

    FEDTrailer trailer(data + (l1tRcd.size() - slinkTrailerSize_));

    if (trailer.check()) {
      LogDebug("L1T") << "Found SLink trailer:"
         << " Length " << trailer.lenght()
         << " CRC " << trailer.crc()
         << " Status " << trailer.evtStatus()
         << " Throttling bits " << trailer.ttsBits();
    } else {
      LogWarning("L1T") << "Did not find a SLink trailer!";
    }

    // FIXME Hard-coded firmware version for first 74x MC campaigns.
    // Will account for differences in the AMC payload, MP7 payload,
    // and unpacker setup.
    bool legacy_mc = false; // fwOverride_ && ((fwId_ >> 24) == 0xff);


    amc13::Header hdr = amc13::Header((const uint64_t*) (data + slinkHeaderSize_));

    amc13::Packet packet;
    if (!packet.parse(
            (const uint64_t*) data,
            (const uint64_t*) (data + slinkHeaderSize_),
            (l1tRcd.size() - slinkHeaderSize_ - slinkTrailerSize_) / 8,
            header.lvl1ID(),
            header.bxID(),
            legacy_mc)) {
      LogError("L1T")
         << "Could not extract AMC13 Packet.";
      return;
    }

    std::vector< amc::Packet > payload = packet.payload();

    std::ostringstream msg;
    msg << "--------------------" << std::endl;
    msg << "Event: "
      << " ID " << iEvent.id().event() << ", " 
      << " LumiBlock " << iEvent.luminosityBlock() << ", "
      << " BX " << iEvent.bunchCrossing() << ", "
      << " Orbit " << iEvent. orbitNumber() << std::endl;
    msg << "--------------------" << std::endl;
    msg << "TCDS Header:"
      << " Trigger counts " << record.getHeader().getData().header.triggerCount << ", "
      << " Number " << record.getHeader().getData().header.eventNumber << ", "
      << std::endl;
    msg << "Slink Header:"
      << " Trigger type " << header.triggerType() << ", "
      << " L1 event ID " << header.lvl1ID() << ", "
      << " BX Number " << header.bxID() << ", "
      << " FED source " << header.sourceID() << ", "
      << " FED version " << header.version() << std::endl;

    msg << "AMC13 Header:"
      << " No of AMCs " << hdr.getNumberOfAMCs() << std::endl;

    msg << "AMC13 packet:" 
      << " Size = " << packet.size() 
      << " No amcs: " << payload.size() << std::endl;

    std::ios::fmtflags flgs( msg.flags() );

    bool anyMPFat = false;
    bool l1IdFat = (header.lvl1ID()%107==0);
    bool tcFat =   (record.getHeader().getData().header.triggerCount%107==0);
    // bool evIdFat = (iEvent.id().event()%107==0);

    for ( const amc::Packet& pkt : payload) {
      msg.flags( flgs );

      amc::Header amcHdr = pkt.header();
      msg << "AMC " << std::setfill ('0') << std::setw (2) << amcHdr.getAMCNumber() << ": ";
      msg.flags( flgs );
      msg << "L1ID " << amcHdr.getLV1ID() << ", "
        << "BX " << amcHdr.getBX() << ", "
        << "UserData " << std::hex << std::showbase << amcHdr.getUserData() << ", "
        << "L1ID\%107 = " << (amcHdr.getLV1ID()%107) << ", "
        << "TC\%107 = " << (record.getHeader().getData().header.triggerCount%107) << ", "
        << "EvID\%107 = " << (iEvent.id().event()%107)
        << std::endl;

        anyMPFat |= ( amcHdr.getAMCNumber() < 10 && amcHdr.getUserData() == 0xc0 );
    }
    msg.flags( flgs );
    msg << "isMPFat " << anyMPFat << " isL1TIDFat " << l1IdFat << std::endl;

    bool force = false;
    // if ( anyMPFat || force ) {
    // if ( anyMPFat && !l1IdFat || force) {
    // if ( (anyMPFat && !evIdFat) || force)
    
    if ( (anyMPFat && !tcFat) || force) {
        LogInfo("L1T") << "New Event";
        std::cout << msg.str();
    }

  }

}


// ------------ method called once each job just before starting event loop  ------------
void 
MP7RawToPatternWriter::beginJob()
{


}

// ------------ method called once each job just after ending the event loop  ------------
void 
MP7RawToPatternWriter::endJob() 
{
/*
  LogDebug("L1TDebug") << "Read " << nFrame_ << " frames" << std::endl;

  // write file
  std::ofstream file( filename_ );

  file << "Board MP7_TEST" << std::endl;

  // quad/chan numbers
  file << " Quad/Chan : ";
  for ( unsigned i=0; i<nQuad_; ++i ) {
    for ( unsigned j=0; j<nChan_; ++j ) {
      file << "   q" << i << "c" << j << "   ";
    }
  }
  file << std::endl;

  // link numbers
  file << "      Link : ";
  for ( unsigned i=0; i<nQuad_; ++i ) {
    for ( unsigned j=0; j<nChan_; ++j ) {
      file << "    " << (i*nChan_)+j << "       ";
    }
  }

  file << std::endl;

  // then the data
  for ( unsigned iFrame=0; iFrame<nFrame_; ++iFrame ) {
    file << "Frame " << std::dec << std::setw(4) << std::setfill('0') << iFrame << " : ";
    for ( unsigned iQuad=0; iQuad<nQuad_; ++iQuad ) {
      for ( unsigned iChan=0; iChan<nChan_; ++iChan ) {
	unsigned iLink = (iQuad*nChan_)+iChan;
	if (iLink<data_.size() && iFrame<data_.at(iLink).size()) {
	  file << std::hex << ::std::setw(1) << dataValid_.at(iFrame) << "v" << std::hex << std::setw(8) << std::setfill('0') << data_.at(iLink).at(iFrame) << " ";
	}
	else {
	  std::cerr << "Out of range : " << iLink << ", " << iFrame << std::endl;
	}
      }
    }
    file << std::endl;
  }

  file.close();

  */
  
}

// ------------ method called when starting to processes a run  ------------
/*
void 
MP7RawToPatternWriter::beginRun(edm::Run const&, edm::EventSetup const&)
{
}
*/

// ------------ method called when ending the processing of a run  ------------
/*
void 
MP7RawToPatternWriter::endRun(edm::Run const&, edm::EventSetup const&)
{
}
*/

// ------------ method called when starting to processes a luminosity block  ------------
/*
void 
MP7RawToPatternWriter::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/

// ------------ method called when ending the processing of a luminosity block  ------------
/*
void 
MP7RawToPatternWriter::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
MP7RawToPatternWriter::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(MP7RawToPatternWriter);
