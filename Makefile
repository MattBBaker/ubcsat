all: ubcsat

hfiles = src/algorithms.h \
         src/mt19937ar.h \
         src/mylocal.h \
         src/reports.h \
         src/ubcsat-globals.h \
         src/ubcsat-internal.h \
         src/ubcsat-io.h \
         src/ubcsat-limits.h \
         src/ubcsat-lit.h \
         src/ubcsat-mem.h \
         src/ubcsat-time.h \
         src/ubcsat-triggers.h \
         src/ubcsat-types.h \
         src/ubcsat.h

cfiles = src/adaptnovelty.c \
         src/algorithms.c \
         src/ddfw.c \
         src/derandomized.c \
         src/g2wsat.c \
         src/gsat-tabu.c \
         src/gsat.c \
         src/gwsat.c \
         src/hsat.c \
         src/hwsat.c \
         src/irots.c \
         src/mt19937ar.c \
         src/mylocal.c \
         src/novelty+p.c \
         src/novelty.c \
         src/parameters.c \
         src/paws.c \
         src/random.c \
         src/reports.c \
         src/rgsat.c \
         src/rnovelty.c \
         src/rots.c \
         src/samd.c \
         src/saps.c \
         src/sparrow.c \
         src/ubcsat-help.c \
         src/ubcsat-internal.c \
         src/ubcsat-io.c \
         src/ubcsat-mem.c \
         src/ubcsat-reports.c \
         src/ubcsat-time.c \
         src/ubcsat-triggers.c \
         src/ubcsat-version.c \
         src/ubcsat.c \
         src/vw.c \
         src/walksat-tabu.c \
         src/walksat.c \
         src/weighted.c

ubcsat: $(hfiles) $(cfiles)
	gcc -static -O3 -D__CRAYXE -D__CRAY_INTERLAGOS -D__CRAYXT_COMPUTE_LINUX_TARGET -upthread_mutex_trylock -upthread_mutex_destroy -upthread_create -D__TARGET_LINUX__ -o sparrow2011 $(cfiles) -lm -I/opt/cray/libsci/13.0.1/GNU/49/interlagos/include -I/opt/cray/mpt/7.1.0/gni/sma/include -I/opt/cray/mpt/7.0.4/gni/mpich2-gnu/49/include -I/opt/cray/rca/1.0.0-2.0502.53711.3.125.gem/include -I/opt/cray/pmi/5.0.4-1.0000.10161.132.18.gem/include -I/opt/cray/xpmem/0.1-2.0502.55507.3.2.gem/include -I/opt/cray/dmapp/7.0.1-1.0502.9501.5.211.gem/include -I/opt/cray/gni-headers/3.0-1.0502.9684.5.2.gem/include -I/opt/cray/ugni/5.0-1.0502.9685.4.24.gem/include -I/opt/cray/udreg/2.3.2-1.0502.9275.1.25.gem/include -I/opt/cray/alps/5.2.1-2.0502.9479.20.1.gem/include -I/opt/cray/wlm_detect/1.0-1.0502.53341.1.1.gem/include -I/opt/cray/krca/1.0.0-2.0502.53880.4.103.gem/include -I/opt/cray-hss-devel/7.2.0/include -L/opt/cray/libsci/13.0.1/GNU/49/interlagos/lib -L/opt/cray/mpt/7.1.0/gni/sma/lib64 -L/opt/cray/dmapp/default/lib64 -L/opt/cray/mpt/7.0.4/gni/mpich2-gnu/49/lib -L/opt/cray/dmapp/default/lib64 -L/opt/cray/mpt/7.0.4/gni/mpich2-gnu/49/lib -L/opt/cray/rca/1.0.0-2.0502.53711.3.125.gem/lib64 -L/opt/cray/pmi/5.0.4-1.0000.10161.132.18.gem/lib64 -L/opt/cray/xpmem/0.1-2.0502.55507.3.2.gem/lib64 -L/opt/cray/dmapp/7.0.1-1.0502.9501.5.211.gem/lib64 -L/opt/cray/ugni/5.0-1.0502.9685.4.24.gem/lib64 -L/opt/cray/udreg/2.3.2-1.0502.9275.1.25.gem/lib64 -L/opt/cray/alps/5.2.1-2.0502.9479.20.1.gem/lib64 -L/opt/cray/atp/1.7.5/lib -L/opt/cray/wlm_detect/1.0-1.0502.53341.1.1.gem/lib64 -lAtpSigHandler -lAtpSigHCommData -Wl,--undefined=_ATP_Data_Globals -Wl,--undefined=__atpHandlerInstall -lpthread -lsci_gnu_49_mpi -lm -lsci_gnu_49 -lm -lsma -lpmi -ldmapp -lpthread -lmpich_gnu_49 -lrt -lugni -lpthread -lpmi -lm -lxpmem -ludreg -lmpl -lpmi -lpthread -lalpslli -lpthread -lwlm_detect -lugni -lpthread -lalpsutil -lpthread -lrca -Wl,--as-needed,-lgfortran,-lquadmath,--no-as-needed -Wl,--as-needed,-lm,--no-as-needed -Wl,--as-needed,-lpthread,--no-as-needed
#cc -O3 -static -o sparrow2011 $(cfiles) -lm
#cc -Wall -O3 -static -o sparrow2011 $(cfiles) -lm

clean:
	rm -f sparrow2011
