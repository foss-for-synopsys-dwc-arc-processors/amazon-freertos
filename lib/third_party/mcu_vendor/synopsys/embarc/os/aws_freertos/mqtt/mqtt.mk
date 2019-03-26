MQTT_DIR = $(AWS_FREERTOS_ROOT)/lib/mqtt

##
# \brief lib component related dirs
##
MQTT_CSRCDIR = $(MQTT_DIR)
MQTT_ASMSRCDIR = $(MQTT_DIR)
MQTT_INCDIR =

# find all the source files in the target directories
MQTT_CSRCS = $(call get_csrcs, $(MQTT_CSRCDIR))
MQTT_ASMSRCS = $(call get_asmsrcs, $(MQTT_ASMSRCDIR))

# get object files
MQTT_COBJS = $(call get_aws_relobjs, $(MQTT_CSRCS))
MQTT_ASMOBJS = $(call get_aws_relobjs, $(MQTT_ASMSRCS))
MQTT_OBJS = $(MQTT_COBJS) $(MQTT_ASMOBJS)

# get dependency files
MQTT_DEPS = $(call get_deps, $(MQTT_OBJS))

# genearte library
AWS_LIB_MQTT = $(AWS_FREERTOS_OUT_DIR)/libmqtt.a

# library generation rule
$(AWS_LIB_MQTT): $(MQTT_OBJS)
	$(TRACE_ARCHIVE)
	$(Q)$(AR) $(AR_OPT) $@ $(MQTT_OBJS)


AWS_FREERTOS_INCDIR += $(MQTT_INCDIR)
AWS_FREERTOS_CSRCDIR += $(MQTT_CSRCDIR)
AWS_FREERTOS_ASMSRCDIR += $(MQTT_ASMSRCDIR)

AWS_FREERTOS_CSRCS += $(MQTT_CSRCS)
AWS_FREERTOS_CXXSRCS +=
AWS_FREERTOS_ASMSRCS += $(MQTT_ASMSRCS)
AWS_FREERTOS_ALLSRCS += $(MQTT_CSRCS) $(MQTT_ASMSRCS)

AWS_FREERTOS_COBJS += $(MQTT_COBJS)
AWS_FREERTOS_CXXOBJS +=
AWS_FREERTOS_ASMOBJS += $(MQTT_ASMOBJS)
AWS_FREERTOS_ALLOBJS += $(MQTT_OBJS)

AWS_FREERTOS_DEFINES += $(MQTT_DEFINES)
AWS_FREERTOS_DEPS += $(MQTT_DEPS)
AWS_FREERTOS_LIBS += $(AWS_LIB_MQTT)
