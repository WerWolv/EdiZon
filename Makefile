.PHONY: homebrew sysmodule all clean

all: homebrew sysmodule

homebrew:
	$(MAKE) -C homebrew

sysmodule:
	$(MAKE) -C sysmodule/libs/libstratosphere
	$(MAKE) -C sysmodule
	
clean:
	$(MAKE) clean -C homebrew
	$(MAKE) clean -C sysmodule
	$(MAKE) clean -C sysmodule/libs/libstratosphere
