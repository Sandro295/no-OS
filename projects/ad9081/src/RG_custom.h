#ifndef RG_CUSTOM_H_
#define RG_CUSTOM_H_


typedef enum {
	success,
	fail
} ExecStatus;

typedef enum {
	Version         = 0x00,
	Reset           = 0x04,
	ILASupport      = 0x08,
	Scrambling      = 0x0C,
	SysrefHandling  = 0x10,
	TestModes       = 0x18,
	OctetsPerFrame  = 0x20,
	LanesInUse      = 0x28,
	SubclassMode    = 0x2C,
	SyncStatus      = 0x38,

	TestModeErrorCount_lane0 = 0x820,
	LinkErrorCount_lane0     = 0x824,
	TestModeILACount_lane0   = 0x828,
	TestModeMultiframeCount_lane0 = 0x82C,
	BufferAdjust = 0x830
} RegMap;

ExecStatus resetCore();
ExecStatus setSubclass(const uint32_t subclass);
ExecStatus setTestMode(uint32_t rawMode);

#ifdef XPAR_JESD204_0_BASEADDR
#define XPAR_JESD204_DAC_BASEADDR XPAR_JESD204_0_BASEADDR
#endif

uint32_t m_baseAddress = XPAR_JESD204_DAC_BASEADDR;

typedef uint32_t RegisterType;

void writeBit(const uint32_t regIndex, const uint8_t bitIndex, const RegisterType value)
{
	volatile RegisterType * address = (RegisterType *)(m_baseAddress + regIndex);
	uint32_t val = *address;
	*address = (val & ~(1U << bitIndex)) | ((value & 0x01) << bitIndex); // 6 bitwise operations. TODO can do better?
}

RegisterType readBit(const uint32_t regIndex, const uint8_t bitIndex)
{
    volatile RegisterType * address = (RegisterType *)(m_baseAddress + regIndex);

    return ((*address) >> bitIndex) & 0x01;
}

void writeRegister(const uint32_t regIndex, const RegisterType value)
{
    volatile RegisterType * finAddress = (RegisterType *)(m_baseAddress + regIndex);
    *finAddress = value;
}

void writeRegisterMasked(
    const uint32_t regIndex,
    const RegisterType value,
    const RegisterType mask)
{
    volatile RegisterType *address = (RegisterType *)(m_baseAddress + regIndex);
    volatile RegisterType val = (*address) & (~mask);
    *address = (val) | (value & mask);
}

RegisterType readRegister(const uint32_t regIndex)
{
    volatile RegisterType * address = (RegisterType *)(m_baseAddress + regIndex);
    return *(address);
}

ExecStatus setSubclass(const uint32_t subclass)
{
    writeBit(SubclassMode, 0, subclass);

    resetCore();
    uint32_t read = readBit(SubclassMode, 0);
    if (read != subclass) {
        return fail;
    }
    return success;
}

// 204b core's reset is self clearing by default unlike 204c's
ExecStatus resetCore()
{
    const uint32_t maxTries = 50;
    uint32_t tries = 0;
    writeBit(Reset, 0, 1);

    while(readBit(Reset, 0) && tries < maxTries) {
//    	printf("\nunreset yet\n");
        tries++;
    }
    if (tries >= maxTries) {
    	printf("\n!!unresettable on its own!!\n");
//        writeRegister(Reset, 0);
        return fail;
    }
    return success;
}

ExecStatus setTestMode(uint32_t rawMode)
{
    writeRegister(TestModes, rawMode & 0x1F);
    resetCore();
    uint32_t read = readRegister(TestModes);
    if (read != rawMode) {
        return fail;
    }
    return success;
}

ExecStatus printInfo()
{
    uint32_t value = readRegister((Version));
    printf("IP Version: %"PRIu32".%"PRIu32" rev %"PRIu32"\n",
    		((value >> 24) & 0xFF), ((value >> 16) & 0xFF), ((value >> 8) & 0xFF));

    value = readRegister(Reset);
    printf("Reset status: %"PRIu32"\n" , (value & 0x01));
    printf("Raw reg value: %"PRIu32"\n" , value);

    value = readRegister((ILASupport));
    printf("Inter Lane Alignment Support status: %"PRIu32"\n" , (value & 0x01));
    printf("Raw reg value: %"PRIu32"\n" , value);

    value = readRegister((Scrambling));
    printf("Scrambling status: %"PRIu32"\n" , (value & 0x01));
    printf("Raw reg value: %"PRIu32"\n" , value);

    value = readRegister((SysrefHandling));
    printf("Sysref event required: %"PRIu32"\n", (value & (1 << 16)));
    printf("Sysref delay: %"PRIu32" clock cycles\n", ((value >> 8) & 0xF));
    printf("Sysref always alligns: %"PRIu32"\n" , (value & 0x01));
    printf("Raw reg value: %"PRIu32"\n" , value);

    value = readRegister((TestModes));
    printf("Test mode: %"PRIu32"\n" , (value & 0x1F));
    printf("Raw reg value: %"PRIu32"\n" , value);

    value = readRegister((OctetsPerFrame));
    printf("Octets per frame: %"PRIu32"\n" , (value & 0xFF));
    printf("Raw reg value: %"PRIu32"\n" , value);

    value = readRegister((LanesInUse));
    printf("Lanes in use: %"PRIu32"\n" , (value & 0xFF));
    printf("Raw reg value: %"PRIu32"\n" , value);

    value = readRegister((SubclassMode));
    printf("Subclass: %"PRIu32"\n" , (value & 0b11));
    printf("Raw reg value: %"PRIu32"\n" , value);

    value = readRegister(SyncStatus);
    printf("SYSREF captured: %"PRIu32"\n" , ((value >> 16) & 0x01));
    printf("Sync status: %"PRIu32"\n" , (value & 0x01));
    printf("Raw reg value: %"PRIu32"\n" , value);

    return success;
}

#endif // RG_CUSTOM_H_

