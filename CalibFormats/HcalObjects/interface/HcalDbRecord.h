#ifndef HCALDBPRODUCER_HCALDBRECORD_H
#define HCALDBPRODUCER_HCALDBRECORD_H
// -*- C++ -*-
//
// Package:     HcalDbProducer
// Class  :     HcalDbRecord
// 
/**\class HcalDbRecord HcalDbRecord.h CalibFormats/HcalDbProducer/interface/HcalDbRecord.h

 Description: <one line class summary>

 Usage:
    <usage>

*/
//
// Author:      
// Created:     Tue Aug  9 19:10:36 CDT 2005
//
#include "boost/mpl/vector.hpp"
#include "FWCore/Framework/interface/DependentRecordImplementation.h"
// #include "FWCore/Framework/interface/EventSetupRecordImplementation.h"

#include "CondFormats/DataRecord/interface/HcalAllRcds.h"
#include "Geometry/Records/interface/IdealGeometryRecord.h"
#include "Geometry/Records/interface/HcalRecNumberingRecord.h"

// class HcalDbRecord : public edm::eventsetup::EventSetupRecordImplementation<HcalDbRecord> {};

class HcalDbRecord : public edm::eventsetup::DependentRecordImplementation <HcalDbRecord,  
  boost::mpl::vector<HcalRecNumberingRecord, IdealGeometryRecord, HcalPedestalsRcd, HcalPedestalWidthsRcd, HcalGainsRcd, HcalGainWidthsRcd, 
  HcalQIEDataRcd, HcalQIETypesRcd, HcalChannelQualityRcd, HcalZSThresholdsRcd, HcalRespCorrsRcd, 
  HcalL1TriggerObjectsRcd, HcalElectronicsMapRcd, HcalTimeCorrsRcd, HcalLUTCorrsRcd, HcalPFCorrsRcd,
  HcalLutMetadataRcd > > {}; 

#endif /* HCALDBPRODUCER_HCALDBRECORD_H */

