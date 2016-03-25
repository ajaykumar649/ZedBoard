ifndef PETALINUX
$(error "Error: PETALINUX environment variable not set.  Change to the root of your PetaLinux install, and source the settings.sh file")
endif

include apps.common.mk

APP = xdaqinterface
#SOURCES = HGC_base.c FMC_IO_lib.c
# Add any other object files to this list below
APP_OBJS = xdaqinterface.o
#APP_OBJS1 = HGC_base.o 
#APP_OBJS2 = FMC_IO_lib.o
dep1 = ParseCmd
dep2 = FMC_IO_lib
all: build install

build: $(APP)

$(APP): 
	#$(APP_OBJS) $(APP_OBJS1) $(APP_OBJS2)
	$(CXX) $(APP).cpp $(dep1).c $(dep2).c -o $(APP)

#$(APP_OBJS): $(APP).c
#	$(XX) $(APP).c
#$(APP_OBJS1): $(dep1).c
#	$(XX) $(dep1).c
#$(APP_OBJS2): $(dep2).c
#	$(XX) $(dep2).c

clean:
	-rm -f $(APP) *.elf *.gdb *.o

.PHONY: install image

install: $(APP)
	$(TARGETINST) -d $(APP) /bin/$(APP)

#%.o: %.cpp
%.o: %.cpp %.cxx
	$(CXX) -c $(CFLAGS) -o $@ $<

help:
	@echo ""
	@echo "Quick reference for various supported build targets for $(INSTANCE)."
	@echo "----------------------------------------------------"
	@echo "  clean                  clean out build objects"
	@echo "  all                    build $(INSTANCE) and install to rootfs host copy"
	@echo "  build                  build subsystem"
	@echo "  install                install built objects to rootfs host copy"
