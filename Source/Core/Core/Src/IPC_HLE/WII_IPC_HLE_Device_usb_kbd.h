// Copyright 2013 Dolphin Emulator Project
// Licensed under GPLv2
// Refer to the license.txt file included.

#ifndef _WII_IPC_HLE_DEVICE_USB_KBD_H_
#define _WII_IPC_HLE_DEVICE_USB_KBD_H_

#include "WII_IPC_HLE.h"
#include "WII_IPC_HLE_Device.h"
#include "HW/USBInterface.h"
#include <queue>

class CWII_IPC_HLE_Device_usb_kbd :
public IWII_IPC_HLE_Device, public CWII_IPC_HLE_Device_Singleton<CWII_IPC_HLE_Device_usb_kbd>,
public USBInterface::IUSBDeviceChangeClient, USBInterface::IUSBDeviceClient
{
public:
	CWII_IPC_HLE_Device_usb_kbd(const std::string& _rDeviceName);
	virtual ~CWII_IPC_HLE_Device_usb_kbd();

	virtual void DoState(PointerWrap& p);

	virtual bool Write(u32 _CommandAddress);
	virtual bool IOCtl(u32 _CommandAddress);

	virtual void USBDevicesChanged(USBInterface::TDeviceList&);
	virtual void USBRequestComplete(void* UserData, u32 Status);

	struct SKeyboardInfo
	{
		USBInterface::IUSBDevice* Device;
		int IId, EId;
	};

	static const char* GetBaseName() { return "/dev/usb/kbd"; }
private:
	void CheckQueue();
	void DoKeyRequest(u32 Uid);

	enum
	{
		MSG_KBD_CONNECT = 0,
		MSG_KBD_DISCONNECT,
		MSG_EVENT
	};

	#pragma pack(push, 1)
	struct SMessageData : public TotallyPod
	{
		u32 MsgType;
		u32 KeyboardId;
		u8 Modifiers;
		u8 Unk2;
		u8 PressedKeys[6];

		SMessageData() {}
		SMessageData(u32 _MsgType, u32 _KeyboardId, u8 _Modifiers, u8 *_PressedKeys) {
			MsgType = Common::swap32(_MsgType);
			KeyboardId = Common::swap32(_KeyboardId);
			Modifiers = _Modifiers;
			Unk2 = 0;

			if (_PressedKeys) // Doesn't need to be in a specific order
				memcpy(PressedKeys, _PressedKeys, sizeof(PressedKeys));
			else
				memset(PressedKeys, 0, sizeof(PressedKeys));
		}
	};
	#pragma pack(pop)
	std::list<SMessageData> m_MessageQueue;
	u32 m_CommandAddress, m_BufferOut;

	typedef std::map<u32, SKeyboardInfo> TOpenDevices;
	TOpenDevices m_OpenDevices;
};

#endif // _WII_IPC_HLE_DEVICE_USB_KBD_H_
