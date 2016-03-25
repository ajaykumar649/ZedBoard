ifndef PETALINUX
$(error "Error: PETALINUX environment variable not set.  Change to the root of your PetaLinux install, and source the settings.sh file")
endif

include apps.common.mk

APP = sim
#SOURCES = HGC_base.c FMC_IO_lib.c
# Add any other object files to this list below
APP_OBJS = sim.o
#APP_OBJS1 = HGC_base.o 
#APP_OBJS2 = FMC_IO_lib.o
dep1 = HGC_base
dep2 = FMC_IO_lib
all: build install

build: $(APP)

$(APP): 
	#$(APP_OBJS) $(APP_OBJS1) $(APP_OBJS2)
	$(CC) $(APP).c $(dep1).c $(dep2).c -o $(APP)

#$(APP_OBJS): $(APP).c
#	$(CC) $(APP).c
#$(APP_OBJS1): $(dep1).c
#	$(CC) $(dep1).c
#$(APP_OBJS2): $(dep2).c
#	$(CC) $(dep2).c

clean:
	-rm -f $(APP) *.elf *.gdb *.o

.PHONY: install image

install: $(APP)
	$(TARGETINST) -d $(APP) /bin/$(APP)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

help:
	@echo ""
	@echo "Quick reference for various supported build targets for $(INSTANCE)."
	@echo "----------------------------------------------------"
	@echo "  clean                  clean out build objects"
	@echo "  all                    build $(INSTANCE) and install to rootfs host copy"
	@echo "  build                  build subsystem"
	@echo "  install                install built objects to rootfs host copy"
