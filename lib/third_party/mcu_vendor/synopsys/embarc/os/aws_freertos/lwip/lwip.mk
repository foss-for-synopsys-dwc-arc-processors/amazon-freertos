LWIP_DIR = $(AWS_FREERTOS_ROOT)/lib/third_party/lwip/src
SECURE_SOCKETS_DIR = $(AWS_FREERTOS_ROOT)/lib/secure_sockets/portable/lwip

##
# \brief lib component related dirs
##
LWIP_CSRCDIR =	$(SECURE_SOCKETS_DIR) \
				$(LWIP_DIR)/api \
				$(LWIP_DIR)/core \
				$(LWIP_DIR)/core/ipv4 \
				$(LWIP_DIR)/core/ipv6 \
				$(LWIP_DIR)/netif

LWIP_ASMSRCDIR = $(LWIP_CSRCDIR)
LWIP_INCDIR =	$(LWIP_DIR)/include \
				$(LWIP_DIR)/include/lwip

# find all the source files in the target directories
LWIP_CSRCS = $(call get_csrcs, $(LWIP_CSRCDIR))
LWIP_ASMSRCS = $(call get_asmsrcs, $(LWIP_ASMSRCDIR))

# get object files
LWIP_COBJS = $(call get_aws_relobjs, $(LWIP_CSRCS))
LWIP_ASMOBJS = $(call get_aws_relobjs, $(LWIP_ASMSRCS))
LWIP_OBJS = $(LWIP_COBJS) $(LWIP_ASMOBJS)

# get dependency files
LWIP_DEPS = $(call get_deps, $(LWIP_OBJS))

# genearte library
AWS_LIB_LWIP = $(AWS_FREERTOS_OUT_DIR)/liblwip.a

# library generation rule
$(AWS_LIB_LWIP): $(LWIP_OBJS)
	$(TRACE_ARCHIVE)
	$(Q)$(AR) $(AR_OPT) $@ $(LWIP_OBJS)


AWS_FREERTOS_INCDIR += $(LWIP_INCDIR)
AWS_FREERTOS_CSRCDIR += $(LWIP_CSRCDIR)
AWS_FREERTOS_ASMSRCDIR += $(LWIP_ASMSRCDIR)

AWS_FREERTOS_CSRCS += $(LWIP_CSRCS)
AWS_FREERTOS_CXXSRCS +=
AWS_FREERTOS_ASMSRCS += $(LWIP_ASMSRCS)
AWS_FREERTOS_ALLSRCS += $(LWIP_CSRCS) $(LWIP_ASMSRCS)

AWS_FREERTOS_COBJS += $(LWIP_COBJS)
AWS_FREERTOS_CXXOBJS +=
AWS_FREERTOS_ASMOBJS += $(LWIP_ASMOBJS)
AWS_FREERTOS_ALLOBJS += $(LWIP_OBJS)

AWS_FREERTOS_DEFINES += $(LWIP_DEFINES)
AWS_FREERTOS_DEPS += $(LWIP_DEPS)
AWS_FREERTOS_LIBS += $(AWS_LIB_LWIP)
