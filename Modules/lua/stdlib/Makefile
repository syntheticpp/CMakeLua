all:
	@echo "REL=v make dist"

dist:
	cd modules && ../utils/ldoc *.lua
	cd .. && tar czf stdlib-${REL}.tar.gz --exclude=CVS --exclude=.cvsignore --exclude=".#*" --exclude=template-rrt.lua stdlib