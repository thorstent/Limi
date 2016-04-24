git = git -c user.name="Auto" -c user.email="auto@auto.com" 

BUILD = build

BUILDDIR = $(PWD)/$(BUILD)
SRCDIR = $(PWD)/timbuk

all : release

.PHONY : release debug clean doc

release : $(BUILD)/buildr/Makefile
	make -C $(BUILDDIR)/buildr -j2

debug : $(BUILD)/buildd/Makefile
	make -C $(BUILDDIR)/buildd -j2

$(BUILD)/buildr/Makefile: 
	mkdir -p $(BUILDDIR)/buildr
	cd $(BUILDDIR)/buildr; cmake -DCMAKE_BUILD_TYPE=Release $(SRCDIR)

$(BUILD)/buildd/Makefile: 
	mkdir -p $(BUILDDIR)/buildd
	cd $(BUILDDIR)/buildd; cmake -DCMAKE_BUILD_TYPE=Debug $(SRCDIR)

clean :
	rm -rf $(BUILDDIR)/buildr
	rm -rf $(BUILDDIR)/buildd

doc:
	doxygen Doxyfile