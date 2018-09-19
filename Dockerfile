ARG APAMA_VERSION=10.3
ARG APAMA_BUILDER=store/softwareag/apama-builder:${APAMA_VERSION}
ARG APAMA_IMAGE=store/softwareag/apama-correlator:${APAMA_VERSION}

FROM ${APAMA_BUILDER} as builder

USER root
RUN yum install -y gcc-c++
USER sagadmin

COPY --chown=sagadmin:sagadmin . $APAMA_WORK/plugin

RUN mkdir -p $APAMA_WORK/lib $APAMA_WORK/monitors
RUN g++ -std=c++0x -o $APAMA_WORK/lib/libconnectivity-batching-codec.so -I$APAMA_HOME/include -L$APAMA_HOME/lib -lapclient -I$APAMA_WORK/plugin -shared -fPIC $APAMA_WORK/plugin/BatchingCodec.cpp
RUN g++ -std=c++0x -o $APAMA_WORK/plugin/tests/libEchoTransport.so -I$APAMA_HOME/include -L$APAMA_HOME/lib -lapclient -I$APAMA_WORK/plugin -shared -fPIC $APAMA_WORK/plugin/EchoTransport.cpp

RUN cd ${APAMA_WORK}/plugin/tests && pysys run | tee logfile && grep 'THERE WERE NO NON PASSES' logfile

FROM ${APAMA_IMAGE}

COPY --chown=sagadmin:sagadmin --from=builder ${APAMA_WORK}/lib ${APAMA_WORK}/lib
COPY --chown=sagadmin:sagadmin --from=builder ${APAMA_WORK}/monitors ${APAMA_WORK}/monitors


