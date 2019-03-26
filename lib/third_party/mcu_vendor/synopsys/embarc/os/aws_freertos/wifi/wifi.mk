WIFI_DIR = $(AWS_FREERTOS_ROOT)/lib/wifi/portable/synopsys/embarc

##
# \brief lib component related dirs
##
WIFI_CSRCDIR = $(WIFI_DIR)
WIFI_ASMSRCDIR = $(WIFI_DIR)
WIFI_INCDIR =

# find all the source files in the target directories
WIFI_CSRCS = $(call get_csrcs, $(WIFI_CSRCDIR))
WIFI_ASMSRCS = $(call get_asmsrcs, $(WIFI_ASMSRCDIR))

# get object files
WIFI_COBJS = $(call get_aws_relobjs, $(WIFI_CSRCS))
WIFI_ASMOBJS = $(call get_aws_relobjs, $(WIFI_ASMSRCS))
WIFI_OBJS = $(WIFI_COBJS) $(WIFI_ASMOBJS)

# get dependency files
WIFI_DEPS = $(call get_deps, $(WIFI_OBJS))

# genearte library
AWS_LIB_WIFI = $(AWS_FREERTOS_OUT_DIR)/libwifi.a

# library generation rule
$(AWS_LIB_WIFI): $(WIFI_OBJS)
	$(TRACE_ARCHIVE)
	$(Q)$(AR) $(AR_OPT) $@ $(WIFI_OBJS)


AWS_FREERTOS_INCDIR += $(WIFI_INCDIR)
AWS_FREERTOS_CSRCDIR += $(WIFI_CSRCDIR)
AWS_FREERTOS_ASMSRCDIR += $(WIFI_ASMSRCDIR)

AWS_FREERTOS_CSRCS += $(WIFI_CSRCS)
AWS_FREERTOS_CXXSRCS +=
AWS_FREERTOS_ASMSRCS += $(WIFI_ASMSRCS)
AWS_FREERTOS_ALLSRCS += $(WIFI_CSRCS) $(WIFI_ASMSRCS)

AWS_FREERTOS_COBJS += $(WIFI_COBJS)
AWS_FREERTOS_CXXOBJS +=
AWS_FREERTOS_ASMOBJS += $(WIFI_ASMOBJS)
AWS_FREERTOS_ALLOBJS += $(WIFI_OBJS)

AWS_FREERTOS_DEFINES += $(WIFI_DEFINES)
AWS_FREERTOS_DEPS += $(WIFI_DEPS)
AWS_FREERTOS_LIBS += $(AWS_LIB_WIFI)
