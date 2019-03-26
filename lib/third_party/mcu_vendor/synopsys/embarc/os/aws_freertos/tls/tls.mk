TLS_DIR = $(AWS_FREERTOS_ROOT)/lib/tls

##
# \brief lib component related dirs
##
TLS_CSRCDIR = $(TLS_DIR)
TLS_ASMSRCDIR = $(TLS_DIR)
TLS_INCDIR =

# find all the source files in the target directories
TLS_CSRCS = $(call get_csrcs, $(TLS_CSRCDIR))
TLS_ASMSRCS = $(call get_asmsrcs, $(TLS_ASMSRCDIR))

# get object files
TLS_COBJS = $(call get_aws_relobjs, $(TLS_CSRCS))
TLS_ASMOBJS = $(call get_aws_relobjs, $(TLS_ASMSRCS))
TLS_OBJS = $(TLS_COBJS) $(TLS_ASMOBJS)

# get dependency files
TLS_DEPS = $(call get_deps, $(TLS_OBJS))

# genearte library
AWS_LIB_TLS = $(AWS_FREERTOS_OUT_DIR)/libtls.a

# library generation rule
$(AWS_LIB_TLS): $(TLS_OBJS)
	$(TRACE_ARCHIVE)
	$(Q)$(AR) $(AR_OPT) $@ $(TLS_OBJS)


AWS_FREERTOS_INCDIR += $(TLS_INCDIR)
AWS_FREERTOS_CSRCDIR += $(TLS_CSRCDIR)
AWS_FREERTOS_ASMSRCDIR += $(TLS_ASMSRCDIR)

AWS_FREERTOS_CSRCS += $(TLS_CSRCS)
AWS_FREERTOS_CXXSRCS +=
AWS_FREERTOS_ASMSRCS += $(TLS_ASMSRCS)
AWS_FREERTOS_ALLSRCS += $(TLS_CSRCS) $(TLS_ASMSRCS)

AWS_FREERTOS_COBJS += $(TLS_COBJS)
AWS_FREERTOS_CXXOBJS +=
AWS_FREERTOS_ASMOBJS += $(TLS_ASMOBJS)
AWS_FREERTOS_ALLOBJS += $(TLS_OBJS)

AWS_FREERTOS_DEFINES += $(TLS_DEFINES)
AWS_FREERTOS_DEPS += $(TLS_DEPS)
AWS_FREERTOS_LIBS += $(AWS_LIB_TLS)
