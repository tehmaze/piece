all:
	scons -Q

clean:
	scons --clean

doc:
	$(MAKE) -C docs
