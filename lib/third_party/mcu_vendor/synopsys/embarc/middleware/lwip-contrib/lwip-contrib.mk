# dir declaration
MID_LWIP_CONTRIB_DIR		= $(MIDDLEWARES_ROOT)/lwip-contrib
MID_LWIP_CONTRIB_APP_DIR	= $(MID_LWIP_CONTRIB_DIR)/apps

MID_LWIP_CONTRIB_ASMSRCDIR	=
MID_LWIP_CONTRIB_CSRCDIR	= $(MID_LWIP_CONTRIB_DIR)/ports/arc \
					$(MID_LWIP_CONTRIB_DIR)/ports/arc/netif
MID_LWIP_CONTRIB_INCDIR		= $(MID_LWIP_CONTRIB_DIR)/ports/arc/include \
					$(MID_LWIP_CONTRIB_DIR)/ports/arc

ifdef OS_SEL
### OS SELECTED ###
ifeq ($(OS_SEL), freertos)
MID_LWIP_CONTRIB_CSRCDIR	+= $(MID_LWIP_CONTRIB_DIR)/ports/arc/sys_arch/freertos
else
	ifeq ($(OS_SEL), aws_freertos)
	MID_LWIP_CONTRIB_CSRCDIR	+= $(MID_LWIP_CONTRIB_DIR)/ports/arc/sys_arch/freertos
	else
	MID_LWIP_CONTRIB_CSRCDIR	+= $(MID_LWIP_CONTRIB_DIR)/ports/arc/sys_arch/baremetal
	endif
endif
### OS SELECTED ###
else  # NO OS #
MID_LWIP_CONTRIB_CSRCDIR	+= $(MID_LWIP_CONTRIB_DIR)/ports/arc/sys_arch/baremetal
endif

##
# select what apps to be used
##
LWIP_CONTRIB_APPS ?=

MID_LWIP_CONTRIB_APP_CSRCDIR = $(wildcard $(addprefix $(MID_LWIP_CONTRIB_APP_DIR)/, $(LWIP_CONTRIB_APPS)))
MID_LWIP_CONTRIB_APP_INCDIR  = $(wildcard $(addprefix $(MID_LWIP_CONTRIB_APP_DIR)/, $(LWIP_CONTRIB_APPS)))

MID_LWIP_CONTRIB_CSRCDIR	+= $(MID_LWIP_CONTRIB_APP_CSRCDIR)
MID_LWIP_CONTRIB_INCDIR		+= $(MID_LWIP_CONTRIB_APP_INCDIR)

# find all the source files in the target directories
MID_LWIP_CONTRIB_CSRCS = $(call get_csrcs, $(MID_LWIP_CONTRIB_CSRCDIR))
MID_LWIP_CONTRIB_ASMSRCS = $(call get_asmsrcs, $(MID_LWIP_CONTRIB_ASMSRCDIR))

# get object files
MID_LWIP_CONTRIB_COBJS = $(call get_relobjs, $(MID_LWIP_CONTRIB_CSRCS))
MID_LWIP_CONTRIB_ASMOBJS = $(call get_relobjs, $(MID_LWIP_CONTRIB_ASMSRCS))
MID_LWIP_CONTRIB_OBJS = $(MID_LWIP_CONTRIB_COBJS) $(MID_LWIP_CONTRIB_ASMOBJS)

# get dependency files
MID_LWIP_CONTRIB_DEPS = $(call get_deps, $(MID_LWIP_CONTRIB_OBJS))

# extra macros to be defined
MID_LWIP_CONTRIB_DEFINES = -DMID_LWIP_CONTRIB

# genearte library
MID_LIB_LWIP_CONTRIB = $(OUT_DIR)/libmidlwip-contrib.a

# library generation rule
$(MID_LIB_LWIP_CONTRIB): $(MID_LWIP_CONTRIB_OBJS)
	$(TRACE_ARCHIVE)
	$(Q)$(AR) $(AR_OPT) $@ $(MID_LWIP_CONTRIB_OBJS)

# specific compile rules
# user can add rules to compile this middleware
# if not rules specified to this middleware, it will use default compiling rules

# Middleware Definitions
MID_INCDIR += $(MID_LWIP_CONTRIB_INCDIR)
MID_CSRCDIR += $(MID_LWIP_CONTRIB_CSRCDIR)
MID_ASMSRCDIR += $(MID_LWIP_CONTRIB_ASMSRCDIR)

MID_CSRCS += $(MID_LWIP_CONTRIB_CSRCS)
MID_CXXSRCS +=
MID_ASMSRCS += $(MID_LWIP_CONTRIB_ASMSRCS)
MID_ALLSRCS += $(MID_LWIP_CONTRIB_CSRCS) $(MID_LWIP_CONTRIB_ASMSRCS)

MID_COBJS += $(MID_LWIP_CONTRIB_COBJS)
MID_CXXOBJS +=
MID_ASMOBJS += $(MID_LWIP_CONTRIB_ASMOBJS)
MID_ALLOBJS += $(MID_LWIP_CONTRIB_OBJS)

MID_DEFINES += $(MID_LWIP_CONTRIB_DEFINES)
MID_DEPS += $(MID_LWIP_CONTRIB_DEPS)
MID_LIBS += $(MID_LIB_LWIP_CONTRIB)