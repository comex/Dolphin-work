// Copyright 2013 Dolphin Emulator Project
// Licensed under GPLv2
// Refer to the license.txt file included.

#include "../ConfigManager.h"
#include "../Core.h" // Local core functions
#include "WII_IPC_HLE_Device_usb_kbd.h"

enum
{
	KbdLedRequest,
	KbdKeyRequest,
};

CWII_IPC_HLE_Device_usb_kbd::CWII_IPC_HLE_Device_usb_kbd(const std::string& _rDeviceName)
: IWII_IPC_HLE_Device(_rDeviceName)
{
	USBInterface::RefInterface();
	USBInterface::RegisterDeviceChangeClient(this);
}

CWII_IPC_HLE_Device_usb_kbd::~CWII_IPC_HLE_Device_usb_kbd()
{
	USBInterface::DeregisterDeviceChangeClient(this);
}

static void SetLEDs(u32 Uid, CWII_IPC_HLE_Device_usb_kbd::SKeyboardInfo& Keyboard, u8 Flags)
{
	USBInterface::USBSetup Setup;
	Setup.bmRequestType = 0x21;
	Setup.bRequest = 0x9;
	Setup.wValue = 0x200;
	Setup.wIndex = Keyboard.IId;
	Setup.wLength = 1;
	u32 UserData[2] = { KbdLedRequest, Uid };
	Keyboard.Device->ControlRequest(&Setup, &Flags, UserData);
}

void CWII_IPC_HLE_Device_usb_kbd::DoState(PointerWrap& p)
{
	p.Do(m_MessageQueue);
	p.Do(m_CommandAddress);
	p.Do(m_BufferOut);
}

bool CWII_IPC_HLE_Device_usb_kbd::Write(u32 _CommandAddress)
{
	u32 BufferIn = Memory::Read_U32(_CommandAddress + 0xc);
	u32 KeyboardID = Memory::Read_U32(BufferIn);
	u8 Flags = Memory::Read_U8(BufferIn + 4);
	if (Flags & 0x80)
	{
		for (TOpenDevices::iterator itr = m_OpenDevices.begin(); itr != m_OpenDevices.end(); ++itr)
		{
			SetLEDs(itr->first, itr->second, 0);
		}
	}
	else
	{
		TOpenDevices::iterator itr = m_OpenDevices.find(KeyboardID);
		if (itr != m_OpenDevices.end())
		{
			SetLEDs(itr->first, itr->second, Flags);
		}
	}

	return true;
}
void CWII_IPC_HLE_Device_usb_kbd::CheckQueue()
{
	if (!m_MessageQueue.empty() && m_CommandAddress)
	{
		*(SMessageData*) Memory::GetPointer(m_BufferOut) = m_MessageQueue.front();
		Memory::Write_U32(0, m_CommandAddress + 0x4);
		WII_IPC_HLE_Interface::EnqReply(m_CommandAddress);
		m_CommandAddress = 0;
	}
}

bool CWII_IPC_HLE_Device_usb_kbd::IOCtl(u32 _CommandAddress)
{
	m_CommandAddress = _CommandAddress;
	m_BufferOut      = Memory::Read_U32(_CommandAddress + 0x18);
	CheckQueue();
	return false;
}

void CWII_IPC_HLE_Device_usb_kbd::USBDevicesChanged(USBInterface::TDeviceList& List)
{
	for (USBInterface::TDeviceList::iterator itr = List.begin(); itr != List.end(); ++itr)
	{
		USBInterface::USBDeviceDescriptorEtc* DevDesc = &*itr;
		USBInterface::USBConfigDescriptorEtc* Config;
		size_t CId, IId, EId;
		if (m_OpenDevices.find(DevDesc->Uid) != m_OpenDevices.end())
		{
			// This should fail anyway, but just in case.
			continue;
		}
		for (CId = 0; CId < DevDesc->Configs.size(); CId++)
		{
			Config = &DevDesc->Configs[CId];
			for (IId = 0; IId < Config->Interfaces.size(); IId++)
			{
				USBInterface::USBInterfaceDescriptorEtc* Interface = &Config->Interfaces[IId];
				if (Interface->bInterfaceClass == 3 &&
					Interface->bInterfaceSubClass == 1 &&
					Interface->bInterfaceProtocol == 1 &&
					Interface->Endpoints.size() > 0)
				{
					EId = Interface->Endpoints[0].bEndpointAddress;
					goto ok;
				}

			}
		}
		continue;
		ok:
		// Note that this will fail often when it's already open.
		u32 Uid = DevDesc->Uid;
		USBInterface::IUSBDevice* Device = USBInterface::OpenUid(Uid, this);
		if (!Device)
		{
			continue;
		}
		if (!Device->SetConfig(CId))
		{
			WARN_LOG(WII_IPC_USB, "Tried to open device %x but couldn't set config %zu", Uid, CId);
			Device->Close();
			continue;
		}
		// TODO parse descriptor because overcomplicated is fun
		SKeyboardInfo Info = { Device, (int) IId, (int) EId };
		m_OpenDevices[Uid] = Info;
		m_MessageQueue.push_back(SMessageData(0, Uid, 0, NULL));
		CheckQueue();
		DoKeyRequest(Uid);
		return;
	}
}

void CWII_IPC_HLE_Device_usb_kbd::DoKeyRequest(u32 Uid)
{
	//SKeyboardInfo& Info = m_OpenDevices[Uid];
	abort();
	//Info.Device->InterruptRequest(Info.EId, 
}

void CWII_IPC_HLE_Device_usb_kbd::USBRequestComplete(void* UserData, u32 Status)
{
	u32* Data = (u32*) UserData;
	if (Data[0] == KbdKeyRequest)
	{
		u32 Uid = Data[1];
		abort();
		//m_MessageQueue.push_back(SMessageData(2, Uid, Modifiers, PressedKeys));
		CheckQueue();
		DoKeyRequest(Uid);
	}
}


