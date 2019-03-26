DEMOS_DIR = $(AWS_FREERTOS_ROOT)/demos

##
# \brief lib component related dirs
##
DEMOS_CSRCDIR = $(DEMOS_DIR)/synopsys/$(BOARD)/common/application_code \
				$(DEMOS_DIR)/common/demo_runner \
				$(DEMOS_DIR)/common/logging \
				$(DEMOS_DIR)/common/devmode_key_provisioning \
				$(DEMOS_DIR)/common/mqtt
DEMOS_ASMSRCDIR =
DEMOS_INCDIR =	$(DEMOS_DIR)/synopsys/$(BOARD)/common/config_files \
				$(DEMOS_DIR)/common/include

# find all the source files in the target directories
DEMOS_CSRCS = $(call get_csrcs, $(DEMOS_CSRCDIR))
DEMOS_ASMSRCS = $(call get_asmsrcs, $(DEMOS_ASMSRCDIR))

# get object files
DEMOS_COBJS = $(call get_aws_relobjs, $(DEMOS_CSRCS))
DEMOS_ASMOBJS = $(call get_aws_relobjs, $(DEMOS_ASMSRCS))
DEMOS_OBJS = $(DEMOS_COBJS) $(DEMOS_ASMOBJS)

# get dependency files
DEMOS_DEPS = $(call get_deps, $(DEMOS_OBJS))

# genearte library
AWS_LIB_DEMOS = $(AWS_FREERTOS_OUT_DIR)/libdemos.a

# library generation rule
$(AWS_LIB_DEMOS): $(DEMOS_OBJS)
	$(TRACE_ARCHIVE)
	$(Q)$(AR) $(AR_OPT) $@ $(DEMOS_OBJS)


AWS_FREERTOS_INCDIR += $(DEMOS_INCDIR)
AWS_FREERTOS_CSRCDIR += $(DEMOS_CSRCDIR)
AWS_FREERTOS_ASMSRCDIR += $(DEMOS_ASMSRCDIR)

AWS_FREERTOS_CSRCS += $(DEMOS_CSRCS)
AWS_FREERTOS_CXXSRCS +=
AWS_FREERTOS_ASMSRCS += $(DEMOS_ASMSRCS)
AWS_FREERTOS_ALLSRCS += $(DEMOS_CSRCS) $(DEMOS_ASMSRCS)

AWS_FREERTOS_COBJS += $(DEMOS_COBJS)
AWS_FREERTOS_CXXOBJS +=
AWS_FREERTOS_ASMOBJS += $(DEMOS_ASMOBJS)
AWS_FREERTOS_ALLOBJS += $(DEMOS_OBJS)

AWS_FREERTOS_DEFINES += $(DEMOS_DEFINES)
AWS_FREERTOS_DEPS += $(DEMOS_DEPS)
AWS_FREERTOS_LIBS += $(AWS_LIB_DEMOS)
