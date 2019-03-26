TESTS_DIR = $(AWS_FREERTOS_ROOT)/tests
DEMO_ADDUP_DIR = $(AWS_FREERTOS_ROOT)/demos
##
# \brief lib component related dirs
##

DEMO_ADDUP_CSRCDIR = $(DEMO_ADDUP_DIR)/common/logging \
					$(DEMO_ADDUP_DIR)/common/devmode_key_provisioning
TESTS_CSRCDIR = $(DEMO_ADDUP_CSRCDIR) \
				$(TESTS_DIR)/synopsys/$(BOARD)/common/application_code \
				$(TESTS_DIR)/common/framework \
				$(TESTS_DIR)/common/memory_leak \
				$(TESTS_DIR)/common/test_runner \
				$(TESTS_DIR)/common/wifi \
				$(TESTS_DIR)/common/secure_sockets \
				$(TESTS_DIR)/common/pkcs11 \
				$(TESTS_DIR)/common/crypto \
				$(TESTS_DIR)/common/tls \
				$(TESTS_DIR)/common/mqtt
TESTS_ASMSRCDIR =
DEMO_ADDUP_INCDIR =
TESTS_INCDIR =	$(DEMO_ADDUP_INCDIR) \
				$(TESTS_DIR)/synopsys/$(BOARD)/common/config_files \
				$(TESTS_DIR)/common/include

# find all the source files in the target directories
TESTS_CSRCS = $(call get_csrcs, $(TESTS_CSRCDIR))
TESTS_ASMSRCS = $(call get_asmsrcs, $(TESTS_ASMSRCDIR))

# get object files
TESTS_COBJS = $(call get_aws_relobjs, $(TESTS_CSRCS))
TESTS_ASMOBJS = $(call get_aws_relobjs, $(TESTS_ASMSRCS))
TESTS_OBJS = $(TESTS_COBJS) $(TESTS_ASMOBJS)

# get dependency files
TESTS_DEPS = $(call get_deps, $(TESTS_OBJS))

# genearte library
AWS_LIB_TESTS = $(AWS_FREERTOS_OUT_DIR)/libtests.a

# library generation rule
$(AWS_LIB_TESTS): $(TESTS_OBJS)
	$(TRACE_ARCHIVE)
	$(Q)$(AR) $(AR_OPT) $@ $(TESTS_OBJS)


AWS_FREERTOS_INCDIR += $(TESTS_INCDIR)
AWS_FREERTOS_CSRCDIR += $(TESTS_CSRCDIR)
AWS_FREERTOS_ASMSRCDIR += $(TESTS_ASMSRCDIR)

AWS_FREERTOS_CSRCS += $(TESTS_CSRCS)
AWS_FREERTOS_CXXSRCS +=
AWS_FREERTOS_ASMSRCS += $(TESTS_ASMSRCS)
AWS_FREERTOS_ALLSRCS += $(TESTS_CSRCS) $(TESTS_ASMSRCS)

AWS_FREERTOS_COBJS += $(TESTS_COBJS)
AWS_FREERTOS_CXXOBJS +=
AWS_FREERTOS_ASMOBJS += $(TESTS_ASMOBJS)
AWS_FREERTOS_ALLOBJS += $(TESTS_OBJS)

AWS_FREERTOS_DEFINES += $(TESTS_DEFINES)
AWS_FREERTOS_DEPS += $(TESTS_DEPS)
AWS_FREERTOS_LIBS += $(AWS_LIB_TESTS)
