PLATFORMSRCS = src/platform/baremetal/baremetal.c
PLATFORMSRCS += src/platform/baremetal/cmdma_hw.c
PLATFORMDEPS = $(patsubst %.c,%.o,$(PLATFORMSRCS))

cleanplatform:
	-rm $(PLATFORMDEPS)
