/*
 * BCM5722D
 * Copyright (C) 2010 Adlan Razalan <dev at adlan dot name dot my>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BCM5722D_BCM5722D_H
#define BCM5722D_BCM5722D_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
    
#include <libkern/libkern.h>
#include <libkern/OSAtomic.h>
#include <machine/limits.h>
#include <net/ethernet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_var.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <sys/appleapiopts.h>
#include <sys/errno.h>
#include <sys/kpi_mbuf.h>
    
#ifdef __cplusplus
}
#endif // __cplusplus

#ifdef __cplusplus

#include <IOKit/network/IOMbufMemoryCursor.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/IOCommandGate.h>
#include <IOKit/IOFilterInterruptEventSource.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOLocks.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/IOTypes.h>
#include <IOKit/network/IOEthernetController.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IOBasicOutputQueue.h>
#include <IOKit/pci/IOPCIDevice.h>

#endif // __cplusplus

#include "Common.h"
#include "PHY.h"

#define BCM5722D my_name_adlan_BCM5722D

#define BUMP_NETSTAT(x) do { networkStats->x += 1; } while (0)
#define BUMP_ETHSTAT(x) do { ethernetStats->dot3StatsEntry.x += 1; } while (0)
#define BUMP_TXSTAT(x) do { ethernetStats->dot3TxExtraEntry.x += 1; } while (0)
#define BUMP_RXSTAT(x) do { ethernetStats->dot3RxExtraEntry.x += 1; } while (0)

enum
{
  kMaxPacketSize     = kIOEthernetMaxPacketSize +  4,
  kTxBDCount         = 512,
  kRxBDCount         = 512,
  kTxMaxSegmentCount = 384,  // 0.75 *ring size(512) = 384
  kRxMaxSegmentCount = 1,
  kWatchDogTimeout   = 5000  // ms
};


enum BOption
{
  kOptionDefault = 0,
  kBDOptionReuse,      // [configureRxDescriptor] Reuse the buffer descriptor
};


class BCM5722D : public IOEthernetController
{
  OSDeclareDefaultStructors(BCM5722D);

 private:
  IOPCIDevice            *pciNub;
  IOMemoryMap            *csrMap;
  IOWorkLoop             *workLoop;
  volatile void          *csrBase;
  IOInterruptEventSource *interruptSource;
  IOEthernetInterface    *netIface;
  IOTimerEventSource     *timerSource;
  IOEthernetAddress       ethAddress;
  IOOutputQueue          *transmitQueue;
  IONetworkStats         *networkStats;
  IOEthernetStats        *ethernetStats;
  OSDictionary           *mediumDict;

  IOBufferMemoryDescriptor  *txMD;
  BTxBufferDescriptor       *txBD;
  IOPhysicalAddress          txAddress;
  IOMbufNaturalMemoryCursor *txCursor;
  mbuf_t                     txPacketArray[kTxBDCount];

  IOBufferMemoryDescriptor  *rxMD;
  BRxBufferDescriptor       *rxBD;
  IOPhysicalAddress          rxAddress;
  IOMbufNaturalMemoryCursor *rxCursor;
  mbuf_t                     rxPacketArray[kRxBDCount];

  IOBufferMemoryDescriptor *rxReturnMD;
  BRxBufferDescriptor      *rxReturnBD;
  IOPhysicalAddress         rxReturnAddress;

  IOBufferMemoryDescriptor *statusBlockMD;
  BStatusBlock             *statusBlock;
  IOPhysicalAddress         statusBlockAddress;

  UInt16        asicRevision;
  UInt16        deviceID;
  UInt32        txQueueLength;
    bool          queueStalled;
  bool          magicPacketSupported;
  bool          wakeOnLanEnabled;
  bool          adapterEnabled;
  bool          interruptEnabled;
  bool          promiscuousModeEnabled;
  unsigned long currentPowerState;

  UInt16 txProducerIdx;
  UInt16 txLocalConsumerIdx;
    SInt16 txFreeSlot;

  UInt16 rxProducerIdx;
  UInt16 rxReturnConsumerIdx;
  UInt16 rxSegmentLength[kRxBDCount];

  UInt32 rxMode;
  UInt32 txMode;
  UInt32 macMode;
  UInt32 pciMiscHostControl;

  bool        autoNegotiate;
  UInt8       currentMediumIndex;
  UInt32      phyID;
  UInt16      phyFlags;
  MediaStatus media;

 public:

#pragma mark -
#pragma mark IOService Methods

  virtual bool start(IOService *provider) override;
  virtual void stop(IOService *provider) override;
  virtual void free(void) override;

#pragma mark -
#pragma mark IONetworkController Methods

  virtual IOReturn        enable(IONetworkInterface *iface) override;
  virtual IOReturn        disable(IONetworkInterface *iface) override;
  virtual void            systemWillShutdown(IOOptionBits specifier) override;
  virtual bool            createWorkLoop() override;
  virtual IOWorkLoop     *getWorkLoop(void) const override;
  virtual IOOutputQueue  *createOutputQueue() override;
  virtual bool            configureInterface(IONetworkInterface *iface) override;
  virtual IOReturn        getChecksumSupport(UInt32 *checksumMask,
                                             UInt32 checksumFamily,
                                             bool isOutput) override;
  virtual IOReturn        getPacketFilters(const OSSymbol *group,
                                           UInt32 *filters) const override;
  virtual IOReturn        getMaxPacketSize(UInt32 *maxSize) const override;
  virtual IOReturn        registerWithPolicyMaker(IOService *policyMaker) override;
  virtual IOReturn        setPowerState(unsigned long powerStateOrdinal,
                                        IOService *policyMaker) override;
  virtual UInt32          outputPacket(mbuf_t m,
                                       void *param) override;
  virtual IOReturn        selectMedium(const IONetworkMedium *medium) override;
  virtual const OSString *newModelString() const override;
  virtual const OSString *newRevisionString() const override;
  virtual const OSString *newVendorString() const override;

#pragma mark -
#pragma mark IOEthernetController Methods

  virtual IOReturn getHardwareAddress(IOEthernetAddress *address) override;
  virtual IOReturn setHardwareAddress(const IOEthernetAddress *address) override;
  virtual IOReturn setWakeOnMagicPacket(bool active) override;
  virtual IOReturn setMulticastMode(bool active) override;
  virtual IOReturn setMulticastList(IOEthernetAddress *addrs,
                                    UInt32 count) override;
  virtual IOReturn setPromiscuousMode(bool active) override;

#pragma mark -
#pragma mark MAC
#pragma mark -

#pragma mark -
#pragma mark Initialization/Reset

  bool     setupDriver(IOService *provider);
  bool     resetAdapter();
  void     initializePCIConfig();
  void     prepareDriver();
  bool     initializeAdapter();
  void     stopAdapter();
  IOReturn lockNVRAM();
  void     configureMACAddress();
  void     clearStatistics();
  void     updateStatistics();


#pragma mark -
#pragma mark Memory/Ring

  bool allocateDescriptorMemory(IOBufferMemoryDescriptor **memory,
                                IOByteCount size);
  void freeDescriptorMemory(IOBufferMemoryDescriptor **memory);
  bool allocateDriverMemory();
  bool configureRxDescriptor(UInt16 index, BOption options = kOptionDefault);
  bool initTxRing();
  void freeTxRing();
  bool initRxRing();
  void freeRxRing();


#pragma mark -
#pragma mark Interrupt Handling

  void enableInterrupts(bool active);
  void interruptOccurred(IOInterruptEventSource *source, int count);
  void timeoutOccurred(IOTimerEventSource *source);
  void serviceTxInterrupt(UInt16 consumerIdx);
  void serviceRxInterrupt(UInt16 producerIdx);


#pragma mark -
#pragma mark Reader/Writer

  void     writeCSR(UInt32 offset, UInt32 value);
  UInt32   readCSR(UInt32 offset);
  void     writeMemoryIndirect(UInt32 offset, UInt32 value);
  UInt32   readMemoryIndirect(UInt32 offset);
  void     writeMailbox(UInt32 offset, UInt32 value);
  UInt32   readMailbox(UInt32 offset);
  void     clearBit(UInt32 offset, UInt32 bit);
  void     setBit(UInt32 offset, UInt32 bit);


#pragma mark -
#pragma mark Helper

  UInt32 computeEthernetCRC(const UInt8 *address, int length);
  void   prepareForWakeOnLanMode();


#pragma mark -
#pragma mark PHY
#pragma mark -

#pragma mark -
#pragma mark Initialization/Reset/Fixes

  bool     probePHY();
  bool     setupPHY();
  IOReturn resetPHY();
  void     acknowledgeInterrupt();
  void     configureMAC();
  void     enableLoopback();
  void     fixJitterBug();
  void     fixAdjustTrim();


#pragma mark -
#pragma mark Medium

  void     addNetworkMedium(UInt32 type,
                            UInt32 speed,
                            UInt32 index);
  void     probeMediaCapability();
  IOReturn setMedium(const IONetworkMedium *medium);


#pragma mark -
#pragma mark Link

  UInt16   resolvePauseAdvertisement(FlowControl flowControl);
  void     setupFlowControl(UInt16 local, UInt16 partner);
  void     configureLinkAdvertisement(LinkSpeed linkSpeed,
                                      LinkDuplex linkDuplex);
  bool     startAutoNegotiation(LinkSpeed changeSpeed,
                                LinkDuplex changeDuplex);
  bool     forceLinkSpeedDuplex(LinkSpeed changeSpeed,
                                LinkDuplex changeDuplex);
  void     resolveOperatingSpeedAndLinkDuplex(UInt16 status);
  void     enableAutoMDIX(bool active);
  void     enableEthernetAtWirespeed();
  void     reportLinkStatus();


#pragma mark -
#pragma mark Interrupt Handling

  void serviceLinkInterrupt();


#pragma mark -
#pragma mark Reader/Writer

  IOReturn writeMII(UInt8 reg, UInt16 value);
  IOReturn readMII(UInt8 reg, UInt16 *value);

};

#endif
