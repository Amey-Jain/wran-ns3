
#ifdef NS3_MODULE_COMPILATION
# error "Do not include ns3 module aggregator headers from other modules; these are meant only for end user scripts."
#endif

#ifndef NS3_MODULE_WRAN
    

// Module headers:
#include "simple-ofdm-wran-channel.h"
#include "simple-ofdm-wran-phy.h"
#include "wran-bs-net-device.h"
#include "wran-bs-scheduler-rtps.h"
#include "wran-bs-scheduler-simple.h"
#include "wran-bs-scheduler.h"
#include "wran-bs-service-flow-manager.h"
#include "wran-bs-uplink-scheduler-mbqos.h"
#include "wran-bs-uplink-scheduler-rtps.h"
#include "wran-bs-uplink-scheduler-simple.h"
#include "wran-bs-uplink-scheduler.h"
#include "wran-channel.h"
#include "wran-cid.h"
#include "wran-connection-manager.h"
#include "wran-connection.h"
#include "wran-cs-parameters.h"
#include "wran-helper.h"
#include "wran-ipcs-classifier-record.h"
#include "wran-ipcs-classifier.h"
#include "wran-mac-header.h"
#include "wran-mac-messages.h"
#include "wran-mac-queue.h"
#include "wran-mac-to-mac-header.h"
#include "wran-net-device.h"
#include "wran-phy.h"
#include "wran-service-flow-manager.h"
#include "wran-service-flow-record.h"
#include "wran-service-flow.h"
#include "wran-simple-ofdm-send-param.h"
#include "wran-ss-manager.h"
#include "wran-ss-net-device.h"
#include "wran-ss-record.h"
#include "wran-ss-service-flow-manager.h"
#include "wran-tlv.h"
#include "wran-ul-job.h"
#endif
