//
//  kern_igfx.hpp
//  IGFX
//
//  Copyright © 2017 lvs1974. All rights reserved.
//

#ifndef kern_igfx_hpp
#define kern_igfx_hpp

#include <Headers/kern_patcher.hpp>
#include <Headers/kern_cpu.hpp>
#include <Headers/kern_iokit.hpp>

class IGFX {
public:
	bool init();
	void deinit();

	/**
	 *  Lock device access
	 */
	static void lockDeviceAccess();

	/**
	 *  Unlock device access
	 */
	static void unlockDeviceAccess();

	/**
	 *  Check whether the frame has no connectors
	 *
	 *  @return true if offline
	 */
	static bool isConnectorLessFrame();

	/**
	 *  Inject IGPU properties and hooks (or terminate the device)
	 *
	 *  @param obj            HDAU device
	 *  @param connectorLess  IGPU has connectors
	 *  @param mislabeled     HDAU is mislabeled
	 */
	static void correctGraphicsAudioProperties(IORegistryEntry *obj, bool connectorLess, bool mislabeled);
private:
	/**
	 *  Obtain necessary symbols from the kernel
	 *
	 *  @param patcher KernelPatcher instance
	 */
	void processKernel(KernelPatcher &patcher);

	/**
	 *  Patch kext if needed and prepare other patches
	 *
	 *  @param patcher KernelPatcher instance
	 *  @param index   kinfo handle
	 *  @param address kinfo load address
	 *  @param size    kinfo memory size
	 */
	void processKext(KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size);

	/**
	 *  Patch kext to support loading IGScheduler4.
	 *
	 *  @param patcher KernelPatcher instance
	 *  @param index   kinfo handle
	 */
	void loadIGScheduler4Patches(KernelPatcher &patcher, size_t index);

	/**
	 *  Patch kext to support loading IGGuC.
	 *
	 *  @param patcher KernelPatcher instance
	 *  @param index   kinfo handle
	 */
	void loadIGGuCPatches(KernelPatcher &patcher, size_t index);

	/**
	 *  Runs over the devices (if not done previously) and corrects their properties.
	 */
	void correctDeviceProperties();

	/**
	 *  Iterate over I/O Registry and find the devices we care about.
	 *
	 *  @param igpu       IGPU device to look up
	 *  @param imei       IMEI device to look up
	 *  @param hdau       HDAU device to look up
	 *  @param hasAMD     AMD GPU availability
	 *  @param hasNVIDIA  NVIDIA GPU availability
	 */
	void getDeviceInfo(IORegistryEntry **igpu, IORegistryEntry **imei, IORegistryEntry **hdau, bool *hasAMD, bool *hasNVIDIA);

	/**
	 *  Obtain framebuffer identifier
	 *
	 *  @param igpu       IGPU device
	 *  @param hasAMD     AMD device availability
	 *  @param hasNVIDIA  NVIDIA device availability
	 *  @param update     update IGPU device in igpu if necessary
	 *
	 *  @return framebuffer identifier or DefaultInvalidPlatformId
	 */
	uint32_t getFramebufferId(IORegistryEntry *igpu, bool hasAMD, bool hasNVIDIA, bool update=false);

	/**
	 *  Inject IGPU properties and hooks
	 *
	 *  @param obj   IGPU device
	 *  @param name  IGPU device name
	 */
	void correctGraphicsProperties(IORegistryEntry *obj, const char *name);

	/**
	 *  PAVP session command type
	 */
	using PAVPSessionCommandID_t = int32_t;

	/**
	 *  PAVP session callback type
	 */
	using t_pavp_session_callback = uint32_t (*)(void *, PAVPSessionCommandID_t, uint32_t, uint32_t *, bool);

	/**
	 *  frameBufferInit type
	 */
	using t_frame_buffer_init = void (*)(void *);

	/**
	 *  computeLaneCount type
	 */
	using t_compute_lane_count = bool (*)(void *, void *, unsigned int, int, int *);

	/**
	 *  AppleIntelXXXXGraphics::start callback type
	 */
	using t_intel_graphics_start = bool (*)(IOService *that, IOService *);

	/**
	 *  IGHardwareGuC::loadGuCBinary or IGGuC::loadBinary callback type
	 *  The latter has one more arg.
	 */
	using t_load_guc_binary = bool (*)(void *that, bool flag);

	/**
	 *  IGScheduler4::loadFirmware callback type.
	 *  We have to wrap this to implement sleep wake firmware loading code in IGScheduler4.
	 */
	using t_load_firmware = bool (*)(void *that);

	/**
	 *  IGHardwareGuC::initSchedControl or IGGuC::initGucCtrl callback type
	 */
	using t_init_sched_control = bool (*)(void *that, void *ctrl);

	/**
	 *  IGSharedMappedBuffer::withOptions callback type
	 */
	using t_ig_buffer_with_options = void *(*)(void *accelTask, unsigned long size, unsigned int type, unsigned int flags);

	/**
	 *  IGSharedMappedBuffer::getGPUVirtualAddress callback type
	 */
	using t_ig_get_gpu_vaddr = uint64_t (*)(void *that);

	/**
	 *  IGGuC::dmaHostToGuC callback type (we use it to correct the sizes)
	 */
	using t_dma_host_to_guc = bool (*)(void *that, uint64_t gpuAddr, uint32_t gpuReg, uint32_t dataLen, uint32_t dmaType, bool unk);

	using t_init_intr_services = void (*)(void *that);

	using t_safe_force_wake = void (*)(void *that, bool a, uint32_t b);

	/**
	 *  Hooked methods / callbacks
	 */
	static uint32_t pavpSessionCallback(void *intelAccelerator, PAVPSessionCommandID_t passed_session_cmd, uint32_t a3, uint32_t *a4, bool passed_flag);
	static void frameBufferInit(void *that);
	static bool computeLaneCount(void *that, void *timing, unsigned int bpp, int availableLanes, int *laneCount);
	static bool intelGraphicsStart(IOService *that, IOService *provider);
	static bool loadGuCBinary(void *that, bool flag);
	static bool loadFirmware(IOService *that);
	static void systemWillSleep(IOService *that);
	static void systemDidWake(IOService *that);
	static bool initSchedControl(void *that, void *ctrl);
	static void *igBufferWithOptions(void *accelTask, unsigned long size, unsigned int type, unsigned int flags);
	static uint64_t igBufferGetGpuVirtualAddress(void *that);
	static bool dmaHostToGuC(void *that, uint64_t gpuAddr, uint32_t gpuReg, uint32_t dataLen, uint32_t dmaType, bool unk);
	static void initInterruptServices(void *that);

	/**
	 *  IGPU PCI Config device-id faking wrappers
	 */
	static uint16_t configRead16(IORegistryEntry *service, uint32_t space, uint8_t offset);
	static uint32_t configRead32(IORegistryEntry *service, uint32_t space, uint8_t offset);

	static uint32_t mmioRead(void *fw, uint32_t reg);
	static void mmioWrite(void *fw, uint32_t reg, uint32_t v);
	static bool doDmaTransfer(void *that, uint64_t gpuAddr, uint32_t gpuReg, uint32_t dataLen, uint32_t dmaType);
	static void resetFirmware(void *that);
	bool loadCustomBinary(void *that, bool restore);

	/**
	 *  Trampolines for original method invocations
	 */
	t_pavp_session_callback orgPavpSessionCallback {nullptr};
	t_frame_buffer_init orgFrameBufferInit {nullptr};
	t_compute_lane_count orgComputeLaneCount {nullptr};
	t_intel_graphics_start orgGraphicsStart {nullptr};
	t_load_guc_binary orgLoadGuCBinary {nullptr};
	t_load_firmware orgLoadFirmware {nullptr};
	t_init_sched_control orgInitSchedControl {nullptr};
	t_ig_buffer_with_options orgIgBufferWithOptions {nullptr};
	t_ig_get_gpu_vaddr orgIgGetGpuVirtualAddress {nullptr};
	t_dma_host_to_guc orgDmaHostToGuC {nullptr};
	t_init_intr_services orgInitInterruptServices {nullptr};
	t_safe_force_wake orgSafeForceWake {nullptr};

	/**
	 *  Original IGPU PCI Config readers
	 */
	WIOKit::t_PCIConfigRead16 orgConfigRead16 {nullptr};
	WIOKit::t_PCIConfigRead32 orgConfigRead32 {nullptr};

	/**
	 *  External global variables
	 */
	uint8_t *gIOFBVerboseBootPtr {nullptr};
	uint8_t *gKmGen9GuCBinary {nullptr};
	uint8_t *canUseSpringboard {nullptr};

	/**
	 *  FBRESET enforces -v like usual patch.
	 *  FBCOPY  enforces screen copy.
	 */
	enum FramebufferFixMode {
		FBDEFAULT  = 0,
		FBRESET    = 1,
		FBCOPY     = 2
	};

	/**
	 *  Framebuffer distortion fix mode
	 */
	uint32_t resetFramebuffer {FBDEFAULT};

	/**
	 *  Scheduler types
	 */
	enum SchedulerDecision {
		BasicScheduler,
		ReferenceScheduler,
#ifdef IGFX_APPLE_SCHEDULER
		AppleScheduler,
		AppleCustomScheduler,
#endif
		TotalSchedulers
	};

	/**
	 *  Scheduler loading modes:
	 *  0 - disable firmware (IGScheduler2)
	 *  1 - use reference firmware scheduler (IGScheduler4)
	 *  2 - use Apple firmware scheduler (IGGuC)
	 */
	uint32_t decideLoadScheduler {BasicScheduler};

	/**
	 *  CPU generation
	 */
	CPUInfo::CpuGeneration cpuGeneration {CPUInfo::CpuGeneration::Unknown};

	/**
	 *  Console info structure, taken from osfmk/console/video_console.h
	 *  Last updated from XNU 4570.1.46.
	 */
	struct vc_info {
		unsigned int   v_height;        /* pixels */
		unsigned int   v_width;         /* pixels */
		unsigned int   v_depth;
		unsigned int   v_rowbytes;
		unsigned long  v_baseaddr;
		unsigned int   v_type;
		char           v_name[32];
		uint64_t       v_physaddr;
		unsigned int   v_rows;          /* characters */
		unsigned int   v_columns;       /* characters */
		unsigned int   v_rowscanbytes;  /* Actualy number of bytes used for display per row*/
		unsigned int   v_scale;
		unsigned int   v_rotate;
		unsigned int   v_reserved[3];
	};

	/**
	 *  vinfo presence status
	 */
	bool gotInfo {false};

	/**
	 *  connector-less frame
	 */
	bool connectorLessFrame {false};

	/**
	 *  External GPU status
	 */
	bool hasExternalNVIDIA {false};

	/**
	 *  External GPU status
	 */
	bool hasExternalAMD {false};

	/**
	 *  Loaded vinfo
	 */
	vc_info vinfo {};

	/**
	 *  Console buffer backcopy
	 */
	uint8_t *consoleBuffer {nullptr};

	/**
	 *  We are currently trying to load the firmware
	 */
	bool performingFirmwareLoad {false};

	/**
	 *  Dummy firmware buffer to store unused old firmware in
	 */
	uint8_t *dummyFirmwareBuffer[4] {};

	/**
	 *  Actual firmware buffer we store our new firmware in
	 */
	uint8_t *realFirmwareBuffer[4] {};

	/**
	 *  Actual firmware address for GPU DMA
	 */
	uint64_t gpuFirmwareAddress[4] {};

	/**
	 *  Actual intercepted binary sizes
	 */
	uint32_t realBinarySize[4] {};

	/**
	 *  Pointer to the size assignment
	 */
	uint32_t *firmwareSizePointer {nullptr};

	/**
	 *  Pointer to the signature
	 */
	uint8_t *signaturePointer[4] {};

	/**
	 *  Current binary index
	 *  0 is GuC, 1 is HuC, 2 is HuC signature, 3 is GuC public key.
	 */
	int32_t currentBinaryIndex {-1};

	/**
	 *  Current dma load index
	 *  0 is HuC, 1 is GuC.
	 */
	int32_t currentDmaIndex {-1};
    
	/**
	 *  Property injection lock
	 */
	IOLock *access {nullptr};

	/**
	 *  Decides on whether to intercept binary loading.
	 */
	bool binaryInterception[4] {true, true, false, false};

	/**
	 *  Current progress mask
	 */
	struct ProcessingState {
		enum {
			NothingReady = 0,
			CallbackPavpSessionRouted = 1,
			CallbackFrameBufferInitRouted = 2,
			CallbackComputeLaneCountRouted = 4,
			CallbackDriverStartRouted = 8,
			CallbackGuCFirmwareUpdateRouted = 16,
			EverythingDone = CallbackPavpSessionRouted |
				CallbackFrameBufferInitRouted |
				CallbackComputeLaneCountRouted |
				CallbackDriverStartRouted |
				CallbackGuCFirmwareUpdateRouted,
		};
	};
	int progressState {ProcessingState::NothingReady};
};

#endif /* kern_azul_pavp_disabler_hpp */
