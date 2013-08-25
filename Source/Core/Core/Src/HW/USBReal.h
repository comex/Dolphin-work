// Copyright 2013 Dolphin Emulator Project
// Licensed under GPLv2
// Refer to the license.txt file included.

#pragma once

#include "../Core.h"
#include "USBInterface.h"
#include "StdThread.h"
#include "MsgHandler.h"
#include "libusb.h"
#include <set>
#include <map>
#include <atomic>

#ifdef libusb_hotplug_match_any
#define cusbdevice_supports_hotplug
#endif

namespace USBInterface
{

class CUSBControllerReal;

class CUSBDeviceReal : public IUSBDevice
{
public:
	CUSBDeviceReal(libusb_device* Device, u32 Uid, libusb_device_handle* Handle, CUSBControllerReal* Controller, IUSBDeviceClient* Client);

	virtual u32 SetConfig(int Config);
	virtual u32 SetInterfaceAltSetting(int Interface, int Setting);
	virtual void BulkRequest(u8 Endpoint, size_t Length, void* Payload, void* UserData);
	virtual void InterruptRequest(u8 Endpoint, size_t Length, void* Payload, void* UserData);
	virtual void IsochronousRequest(u8 Endpoint, size_t Length, size_t NumPackets, u16* PacketLengths, void* Payload, void* UserData);

protected:
	virtual void CancelRequest(CUSBRequest* Request);
	virtual void _ControlRequest(const USBSetup* Request, void* Payload, void* UserData);
	virtual void _Close();

private:
	static void TransferCallback(libusb_transfer*);
	libusb_device* m_Device;
	libusb_device_handle* m_DeviceHandle;
	CUSBControllerReal* m_Controller;
	int m_NumInterfaces;
};

class CUSBControllerReal : public IUSBController
{
public:
	CUSBControllerReal();
	virtual void Destroy();
	virtual IUSBDevice* OpenUid(u32 Uid, IUSBDeviceClient* Client);
	virtual void UpdateShouldScan();
	virtual void UpdateDeviceList(TDeviceList& List);
private:
	virtual ~CUSBControllerReal();
	void PollDevices(bool UseEvents);
	void USBThread();
	static void USBThreadFunc(CUSBControllerReal* Self) { Self->USBThread(); }
	CUSBDeviceReal* OpenDevice(libusb_device* Device, u32 Uid, IUSBDeviceClient* Client);
#ifdef CUSBDEVICE_SUPPORTS_HOTPLUG
	static int HotplugCallback(libusb_context* Ctx, libusb_device* Device, libusb_hotplug_event Event, void* Data);
#endif

	libusb_context* m_UsbContext;
#ifdef CUSBDEVICE_SUPPORTS_HOTPLUG
	libusb_hotplug_callback_handle* m_HotplugHandle;
	bool m_UseHotplug;
#endif
	std::thread* m_Thread;

	std::mutex m_PollResultsLock, m_PollingLock;
	TDeviceList m_PollResults;

	std::map<libusb_device*, u32> m_LastUidsRev;
	std::map<u32, libusb_device*> m_LastUids;
	//libusb_device** m_OldList;
	//ssize_t m_OldCount;
	u32 m_NextUid;

	volatile bool m_ShouldDestroy;
protected:
	std::set<libusb_device*> m_OpenedDevices;
	friend class CUSBDeviceReal;
};

} // namespace
