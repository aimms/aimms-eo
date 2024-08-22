FROM almalinux:8.9
LABEL maintainer="AIMMS <support@aimms.com>"
ARG AIMMS_VERSION_MAJOR
ARG AIMMS_VERSION_MINOR

#########################
#      base image       #
#########################
RUN dnf upgrade -y almalinux-release

RUN dnf -y update \ 
    && dnf -y install  \
            gcc-toolset-11 \
            unixODBC \ 
            ca-certificates \
            gnupg \
            openssl \
            glibc-langpack-en \
            glibc-locale-source \
            dos2unix \
            wget \
            curl \
    && rm -f /etc/odbcinst.ini \
    && localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8 \
    && dnf clean all 
RUN    echo "source scl_source enable gcc-toolset-11" > /etc/profile.d/enable_gcc_toolset_11.sh \
    && chmod a+rx /etc/profile.d/enable_gcc_toolset_11.sh

RUN wget -q https://download.aimms.com/aimms/download/data/${AIMMS_VERSION_MAJOR}/${AIMMS_VERSION_MINOR}/Aimms-${AIMMS_VERSION_MAJOR}.${AIMMS_VERSION_MINOR}-installer.run && \
    chmod a+rx Aimms-$AIMMS_VERSION_MAJOR.$AIMMS_VERSION_MINOR-installer.run && \
    ./Aimms-$AIMMS_VERSION_MAJOR.$AIMMS_VERSION_MINOR-installer.run --target /usr/local/Aimms --noexec && \
	rm -rf /usr/local/Aimms/WebUIDev && \
    rm -f ./Aimms-$AIMMS_VERSION_MAJOR.$AIMMS_VERSION_MINOR-installer.run

VOLUME /data
VOLUME /model

COPY ./docker-entry.sh /docker-entry.sh
RUN chmod a+rx /docker-entry.sh && dos2unix /docker-entry.sh

ENTRYPOINT ["/docker-entry.sh"]
ENV LD_LIBRARY_PATH="/usr/local/Aimms/Bin"
CMD ["AimmsCmd"]

# altenratively you can build your own executable that interacts with AIMMS
# to control the execution of the model
# COPY jobrunner.c /tmp/jobrunner.c
# RUN gcc -I /usr/local/Aimms/Api /tmp/jobrunner.c -L /usr/local/Aimms/Bin -laimms3 -Wl,--hash-style=both -Wl,-R,'$ORIGIN/../Bin' -Wl,-R,'$ORIGIN/../Solvers' -o /usr/local/Aimms/Bin/jobrunner
# CMD ["/usr/local/Aimms/Bin/jobrunner"]




